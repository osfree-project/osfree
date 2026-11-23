#pragma	title("Entry Field Replacement  --  Version 1.0 -- (EntryFld.C)")
#pragma	subtitle("   Entry Field Control - Interface Definitions")

/* Program name: Listbox.C    Title: A Entry Field Replacement		*/
/*									*/
/* OS/2	Developer Magazine, Issue:  July '94 GUI Corner                 */
/* Author:  Mark Benge	   IBM Corp.					*/
/*	    Matt Smith	   Prominare Inc.				*/
/* Description:	 Replacement for OS/2 List Box,	third of a series.	*/
/*									*/
/* Program Requirements:  OS/2 2.x					*/
/*			  IBM C	Set/2					*/
/*			  WATCOM C 386/9.0				*/
/*			  Borland C++ for OS/2				*/
/*			  OS/2 Toolkit					*/

/* Copyright ¸ International Business Machines Corp. 1991-1994		*/
/* Copyright ¸ 1989-1994  Prominare Inc.  All Rights Reserved.		*/


/************************************************************************/
/************************************************************************/
/*			     SPECIAL NOTE				*/
/************************************************************************/
/************************************************************************/
/*     The behaviour of	this control is	similar	to that	provided	*/
/*     by OS/2 PM.  Although it	may be based on	documentation		*/
/*     describing programming interfaces and techniques, it should	*/
/*     not be misconstrued that	this sample code is an official		*/
/*     release of the OS/2 PM control and is supported as such.		*/
/************************************************************************/
/************************************************************************/

/************************************************************************/
/************************************************************************/
/*		       DISCLAIMER OF WARRANTIES.			*/
/************************************************************************/
/************************************************************************/
/*     The following [enclosed]	code is	source code created by the	*/
/*     authors.	 This source code is  provided to you solely		*/
/*     for the purpose of assisting you	in the development of your	*/
/*     applications.  The code is provided "AS IS", without		*/
/*     warranty	of any kind.  The authors shall	not be liable		*/
/*     for any damages arising out of your use of the source code,	*/
/*     even if they have been advised of the possibility of such	*/
/*     damages.	 It is provided	purely for instructional and		*/
/*     illustrative purposes.						*/
/************************************************************************/
/************************************************************************/

#pragma	info(noext)
#pragma	strings(readonly)

#define	INCL_DOS		   /* Include OS/2 DOS Kernal		*/
#define	INCL_GPI		   /* Include OS/2 PM GPI Interface	*/
#define	INCL_WIN		   /* Include OS/2 PM Windows Interface	*/

#include <os2.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

#include "entryfld.h"

/* This	module contains	the common routines used by the	Entry Field	*/
/* along with the window procedure for the Entry Field.			*/
/*									*/
/* Equivalent command line invocation of each module using the		*/
/* IBM C Set++ Compiler	Version	2.0 is:					*/
/*									*/
/*     Icc -G3e- -O+ -Rn -C -W3	-FoEntryfld Entryfld.C			*/

/* Filename:   Entryfld.C						*/

/*  Version:   1.0							*/
/*  Created:   1994-03-03						*/
/*  Revised:   1994-07-03						*/

/* Routines:   static LONG lGetPresParam(HWND hWnd, ULONG ulID1,	*/
/*					 ULONG ulID2, LONG lDefault);	*/
/*	       static VOID SizeEntryField(PENTRYFWIN pefw);		*/
/*	       static VOID QueryCharPoints(PENTRYFWIN pefw);		*/
/*	       static LONG lFindCharacterIndex(PENTRYFWIN pefw,		*/
/*					       LONG xClick);		*/
/*	       static LONG lCalcDisplayPoint(PENTRYFWIN	pefw,		*/
/*					     LONG iChar);		*/
/*	       static BOOL fScrollRight(PENTRYFWIN pefw);		*/
/*	       static VOID ShowCursor(PENTRYFWIN pefw, LONG iSel);	*/
/*	       static VOID DrawOvertype(PENTRYFWIN pefw);		*/
/*	       static VOID DrawTextString(PENTRYFWIN pefw, LONG	iStart,	*/
/*					  LONG iEnd, BOOL fHighlighted);*/
/*	       static VOID DrawText(PENTRYFWIN pefw);			*/
/*	       static VOID DrawSubText(PENTRYFWIN pefw,	LONG iStart,	*/
/*				       LONG iEnd);			*/
/*	       static VOID DrawOvertypeText(PENTRYFWIN pefw,		*/
/*					    BOOL fMoveRight);		*/
/*	       static VOID SelectText(PENTRYFWIN pefw, LONG iAnchor,	*/
/*				      LONG iSelected);			*/
/*	       static VOID RefreshMetrics(PENTRYFWIN pefw);		*/
/*	       static VOID DecodeCtlData(PENTRYFWIN pefw, MPARAM mp1);	*/
/*	       MRESULT EXPENTRY	mrBaseFieldHandler(HWND	hWnd, ULONG msg,*/
/*						   MPARAM mp1,		*/
/*						   MPARAM mp2);		*/
/*	       APIRET APIENTRY EntryFieldExceptionHandler(		*/
/*			       PEXCEPTIONREPORTRECORD pxcptrepr,	*/
/*			       PEXCEPTIONREGISTRATIONRECORD pxcptregr,	*/
/*			       PCONTEXTRECORD pcr, PVOID sysinfo);	*/
/*	       static VOID DeleteText(PENTRYFWIN pefw, LONG iSelStart,	*/
/*				      LONG iSelEnd, BOOL fRedraw);	*/
/*	       static PSZ pszCopyText(PENTRYFWIN pefw, PSZ pszBuffer,	*/
/*				      LONG iSelStart, LONG iSelEnd);	*/
/*	       static VOID PasteText(PENTRYFWIN	pefw, PSZ pszInsText,	*/
/*				     LONG cLen);			*/
/*	       BOOL EXPENTRY fRegisterEntryField(HAB hAB);		*/
/*	       MRESULT EXPENTRY	EntryWndProc(HWND hWnd,	ULONG msg,	*/
/*					     MPARAM mp1, MPARAM	mp2);	*/


/* --------------------------------------------------------------------	*/

/************************************************************************/
/*									*/
/* Standard Entry Field	Styles						*/
/*									*/
/*     ES_LEFT			  0x00000000L				*/
/*     ES_CENTER		  0x00000001L				*/
/*     ES_RIGHT			  0x00000002L				*/
/*     ES_AUTOSCROLL		  0x00000004L				*/
/*     ES_MARGIN		  0x00000008L				*/
/*     ES_AUTOTAB		  0x00000010L				*/
/*     ES_READONLY		  0x00000020L				*/
/*     ES_COMMAND		  0x00000040L				*/
/*     ES_UNREADABLE		  0x00000080L				*/
/*     ES_AUTOSIZE		  0x00000200L				*/
/*     ES_ANY			  0x00000000L				*/
/*     ES_SBCS			  0x00001000L				*/
/*     ES_DBCS			  0x00002000L				*/
/*     ES_MIXED			  0x00003000L				*/
/*									*/
/* Standard Entry Field	notification messages				*/
/*									*/
/*     EN_SETFOCUS		  0x0001				*/
/*     EN_KILLFOCUS		  0x0002				*/
/*     EN_CHANGE		  0x0004				*/
/*     EN_SCROLL		  0x0008				*/
/*     EN_MEMERROR		  0x0010				*/
/*     EN_OVERFLOW		  0x0020				*/
/*     EN_INSERTMODETOGGLE	  0x0040				*/
/*									*/
/* Standard Entry Field	messages					*/
/*									*/
/*     EM_QUERYCHANGED		  0x0140				*/
/*     EM_QUERYSEL		  0x0141				*/
/*     EM_SETSEL		  0x0142				*/
/*     EM_SETTEXTLIMIT		  0x0143				*/
/*     EM_CUT			  0x0144				*/
/*     EM_COPY			  0x0145				*/
/*     EM_CLEAR			  0x0146				*/
/*     EM_PASTE			  0x0147				*/
/*     EM_QUERYFIRSTCHAR	  0x0148				*/
/*     EM_SETFIRSTCHAR		  0x0149				*/
/*     EM_QUERYREADONLY		  0x014a				*/
/*     EM_SETREADONLY		  0x014b				*/
/*     EM_SETINSERTMODE		  0x014c				*/

jmp_buf	jBuf;			   /* Jump Buffer			*/

/************************************************************************/
/*									*/
/* Module Prototype Definitions						*/
/*									*/
/************************************************************************/

static LONG lGetPresParam(HWND hWnd, ULONG ulID1, ULONG	ulID2, LONG lDefault);
static VOID SizeEntryField(PENTRYFWIN pefw);
static VOID QueryCharPoints(PENTRYFWIN pefw);
static LONG lFindCharacterIndex(PENTRYFWIN pefw, LONG xClick);
static LONG lCalcDisplayPoint(PENTRYFWIN pefw, LONG iChar);
static BOOL fScrollRight(PENTRYFWIN pefw);
static VOID ShowCursor(PENTRYFWIN pefw,	LONG iSel);
static VOID DrawOvertype(PENTRYFWIN pefw);
static VOID DrawTextString(PENTRYFWIN pefw, LONG iStart, LONG iEnd, BOOL fHighlighted);
static VOID DrawText(PENTRYFWIN	pefw);
static VOID DrawSubText(PENTRYFWIN pefw, LONG iStart, LONG iEnd);
static VOID DrawOvertypeText(PENTRYFWIN	pefw, BOOL fMoveRight);
static VOID SelectText(PENTRYFWIN pefw,	LONG iAnchor, LONG iSelected);
static VOID RefreshMetrics(PENTRYFWIN pefw);
static VOID DecodeCtlData(PENTRYFWIN pefw, MPARAM mp1);
MRESULT	EXPENTRY mrBaseFieldHandler(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2);
APIRET	APIENTRY EntryFieldExceptionHandler(PEXCEPTIONREPORTRECORD pxcptrepr,
					    PEXCEPTIONREGISTRATIONRECORD pxcptregr,
					    PCONTEXTRECORD pcr,	PVOID sysinfo);
static VOID DeleteText(PENTRYFWIN pefw,	LONG iSelStart,	LONG iSelEnd, BOOL fRedraw);
static PSZ  pszCopyText(PENTRYFWIN pefw, PSZ pszBuffer,	LONG iSelStart,	LONG iSelEnd);
static VOID PasteText(PENTRYFWIN pefw, PSZ pszBuffer, LONG cLen);

/************************************************************************/
/************************************************************************/
/*									*/
/* Module Private Functions						*/
/*									*/
/************************************************************************/
/************************************************************************/

#pragma	subtitle("   Entry Field Control - Presentation Parameter Retrieve Procedure")
#pragma	page( )

/* --- lGetPresParam ----------------------------------	[ Private } ---	*/
/*									*/
/*     This function is	used to	retrieve a presentation	parameter	*/
/*     that may	be present.  If	the presentation parameter is not,	*/
/*     the default colour passed to the	function will be used.	All	*/
/*     colours queried for are RGB or their RGB	equivalents.		*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HWND  hWnd;     = Window	Handle					*/
/*     ULONG ulID1;    = Presentation Parameter	1 ID (RGB)		*/
/*     ULONG ulID2;    = Presentation Parameter	2 ID (Index)		*/
/*     LONG  lDefault; = Default Colour					*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     lGetPresParam = Colour to Use (RGB)				*/
/*									*/
/* --------------------------------------------------------------------	*/

static LONG lGetPresParam(HWND hWnd, ULONG ulID1, ULONG	ulID2, LONG lDefault)

{
HPS   hPS;			   /* Presentation Space Handle		*/
LONG  lClr;			   /* Presentation Parameter Colour	*/
ULONG ulID;			   /* Presentation Parameter ID		*/

		       /* First	try to determine if any	presentation	*/
		       /* parameters are set for the area requested.	*/
		       /* The first ID is RGB index and	the second ID	*/
		       /* is colour index.				*/

if ( WinQueryPresParam(hWnd, ulID1, ulID2, &ulID, sizeof(LONG),	(PVOID)&lClr,
		       QPF_NOINHERIT | QPF_ID2COLORINDEX | QPF_PURERGBCOLOR) )

		       /* Presentation parameter set for the area,	*/
		       /* return the RGB colour				*/
   return(lClr);
else
		       /* No presentation parameters set, check	to see	*/
		       /* if the default is a system colour in which	*/
		       /* only need to query the index for the RGB	*/
		       /* value						*/

   if (	(lDefault >= SYSCLR_SHADOWHILITEBGND) &&
	(lDefault <= SYSCLR_HELPHILITE)	)
       return(WinQuerySysColor(HWND_DESKTOP, lDefault, 0L));
   else
		       /* Standard colour index, get the RGB equivalent	*/
		       /* for the index					*/

       if ( (hPS = WinGetPS(hWnd)) != (HPS)NULL	)
	   if (	(lClr =	GpiQueryRGBColor(hPS, LCOLOPT_REALIZED,	lDefault)) == GPI_ALTERROR )
	       {
		       /* Colour was not found,	return white as	colour	*/

	       WinReleasePS(hPS);
	       return(RGB_WHITE);
	       }
	   else
	       {
		       /* Colour found,	return the RGB value		*/

	       WinReleasePS(hPS);
	       return(lClr);
	       }
       else
	   return(RGB_WHITE);
}
#pragma	subtitle("   Entry Field Control - Entry Field Sizing Procedure")
#pragma	page( )

/* --- SizeEntryField ---------------------------------	[ Private } ---	*/
/*									*/
/*     This function is	used to	calculate the display points and	*/
/*     rectangles for the entry	field when it has changed in size.	*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	= Entry	Field Data Pointer			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID SizeEntryField(PENTRYFWIN pefw)

{
		       /* Form the bounding rectangle for the control	*/

pefw->rcl.xLeft	= pefw->rcl.yBottom = 0L;
pefw->rcl.xRight = --pefw->cx;
pefw->rcl.yTop	 = --pefw->cy;

		       /* Save this rectangle within the bounding	*/
		       /* rectangle of the text.  Also,	adjust the text	*/
		       /* boundaries to	allow for a little "white"	*/
		       /* space	at the margins.				*/

pefw->rclText =	pefw->rcl;

pefw->rclText.xLeft   += EFMARGIN_LEFT;
pefw->rclText.xRight  -= EFMARGIN_RIGHT;
pefw->rclText.yBottom += EFMARGIN_BOTTOM;
pefw->rclText.yTop    -= EFMARGIN_TOP;

pefw->rclDraw =	pefw->rclText;

		       /* Calculate the	scrolling shift	count in	*/
		       /* characters.  The formula used	here is	to take	*/
		       /* the width of the control and find its	1/3	*/
		       /* point.  Using	this 1/3 width,	now calculate	*/
		       /* the average number of	characters that	would	*/
		       /* fit in the space.				*/

pefw->cScroll =	(pefw->xScrollLimit = pefw->rcl.xRight / 3L) / pefw->cxChar;

		       /* Set the baseline for the text	as being the	*/
		       /* descender size plus the "white"space margin	*/

pefw->ptlDraw.y	= pefw->yBaseline = EFMARGIN_BOTTOM + pefw->yDesc;

		       /* Calculate the	border margin outside points	*/
		       /* which	is the dark rectangle			*/

pefw->aptlOutside[0].x = 0L;
pefw->aptlOutside[0].y = 1L;
pefw->aptlOutside[1].x = 0L;
pefw->aptlOutside[1].y = pefw->cy;
pefw->aptlOutside[2].x = pefw->cx - 1L;
pefw->aptlOutside[2].y = pefw->cy;
pefw->aptlOutside[3].x = pefw->cx - 1L;
pefw->aptlOutside[3].y = 1L;
pefw->aptlOutside[4].x = 0L;
pefw->aptlOutside[4].y = 1L;

		       /* Calculate the	border margin inside points	*/
		       /* which	is the light rectangle			*/

pefw->aptlInside[0].x =	1L;
pefw->aptlInside[0].y =	0L;
pefw->aptlInside[1].x =	1L;
pefw->aptlInside[1].y =	pefw->cy - 1L;
pefw->aptlInside[2].x =	pefw->cx;
pefw->aptlInside[2].y =	pefw->cy - 1L;
pefw->aptlInside[3].x =	pefw->cx;
pefw->aptlInside[3].y =	0L;
pefw->aptlInside[4].x =	1L;
pefw->aptlInside[4].y =	0L;

}
#pragma	subtitle("   Entry Field Control - Text Character Position Procedure")
#pragma	page( )

/* --- QueryCharPoints --------------------------------	[ Private } ---	*/
/*									*/
/*     This function is	used to	build the point	array which contains	*/
/*     the starting position of	each character in the text of the	*/
/*     entry field.  This function should only be called when the	*/
/*     text within the control has changed.				*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	= Entry	Field Data Pointer			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID QueryCharPoints(PENTRYFWIN pefw)

{
POINTL	rgptl[TXTBOX_COUNT];	   /* Text Box Point Array		*/

		       /* Check	to see if a previous array defined and	*/
		       /* if the case, release the array while clearing	*/
		       /* the pointer to make sure that	an empty entry	*/
		       /* field	is handled properly			*/
if ( pefw->aptl	)
   {
   HeapFree(pefw->hHeap, pefw->aptl);
   pefw->aptl =	(PPOINTL)NULL;
   }
		       /* Check	to see if any text contained within the	*/
		       /* entry	field and if the case, begin the	*/
		       /* process of building the point	array		*/
if ( pefw->cText )
   {
		       /* When the entry field is defined as being	*/
		       /* unreadable, the base text holder will	contain	*/
		       /* asterisks (*)	instead	of the actual text.	*/
		       /* This is done to prevent the input from being	*/
		       /* displayed.  A	second input buffer is used to	*/
		       /* hold the actual text and the standard	text	*/
		       /* buffer is filled with	asterisks.		*/

   if (	pefw->flStyle &	ES_UNREADABLE )
       {
       memcpy(pefw->szPassword,	pefw->szText, (UINT)pefw->cText);
       memset(pefw->szText, '*', (UINT)pefw->cText);
       pefw->szText[pefw->cText] = 0;
       }
		       /* Allocate the point holders for the text and	*/
		       /* query	the points.  Make sure that there is	*/
		       /* one additional point holder for the right	*/
		       /* edge of the last character.			*/

   GpiQueryCharStringPos(pefw->hPS, 0UL, pefw->cText, pefw->szText, (PLONG)NULL,
			 pefw->aptl = (PPOINTL)HeapMalloc(pefw->hHeap,
							  (ULONG)(sizeof(POINTL) * (pefw->cText	+ 1))));

		       /* Determine the	actual text box	bounding	*/
		       /* rectangle					*/

   GpiQueryTextBox(pefw->hPS, pefw->cText, pefw->szText, 5L, rgptl);

		       /* Save the right edge of the last character	*/
		       /* in the last point holder.  This will allow	*/
		       /* to a fast lookup of mouse clicks since all	*/
		       /* characters edges are horizontally defined.	*/

   pefw->aptl[pefw->cText].x = rgptl[TXTBOX_CONCAT].x -	rgptl[TXTBOX_BOTTOMLEFT].x;
   }
else
   pefw->iOffset = 0L;
		       /* Set the change flag to reflect the new text	*/
		       /* entered					*/
pefw->fChanged = TRUE;

}
#pragma	subtitle("   Entry Field Control - Text Character Position Procedure")
#pragma	page( )

/* --- lFindCharacterIndex ----------------------------	[ Private } ---	*/
/*									*/
/*     This function is	used to	determine where	the mouse pointer	*/
/*     has clicked in terms of the characters that compose the text	*/
/*     of the entry field.  The	method used is a binary	search on	*/
/*     the character point array that has been cached internally with	*/
/*     the private control data.  Further selection is performed for	*/
/*     the selection point depending on	whether	the point is in	the	*/
/*     front half of the character box or the later half.  When	the	*/
/*     point is	in the front half, the character index for the		*/
/*     found character box is used otherwise if	it is in the later	*/
/*     half, the next character	is used.				*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	  = Entry Field	Data Pointer			*/
/*     LONG	  xClick; = Horizontal Mouse Click Point		*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     lFindCharacterIndex = Character Index				*/
/*									*/
/* --------------------------------------------------------------------	*/

static LONG lFindCharacterIndex(PENTRYFWIN pefw, LONG xClick)

{
LONG iHigh;			   /* High Index			*/
LONG iLow;			   /* Low Index				*/
register LONG i;		   /* Index Positions			*/

		       /* Check	to see if the mouse click is past the	*/
		       /* right-most character in the entry field text	*/
		       /* in which case	indicate the text position as	*/
		       /* being	just after the last character		*/

if ( (xClick +=	(pefw->aptl[pefw->iOffset].x - EFMARGIN_LEFT)) >= pefw->aptl[pefw->cText].x )
   return(pefw->cText);
else
		       /* Check	to make	sure that the relative position	*/
		       /* does not extend to the left of the left	*/
		       /* margin					*/
   if (	xClick > 0L )
       {
		       /* Have determined that the mouse click is	*/
		       /* with the current text	limit.	Initialize the	*/
		       /* the high and low points for the binary search	*/
		       /* as being the shift offset as the low starting	*/
		       /* boundary and the length of the text as the	*/
		       /* high ending boundary.				*/

       iLow  = i = pefw->iOffset;
       iHigh = pefw->cText - 1L;

		       /* Loop through the point array to see which	*/
		       /* cell falls under the mouse click		*/

       while( iLow <= iHigh )
	   if (	xClick < pefw->aptl[i =	(iLow +	iHigh) / 2L].x )
	       iHigh = i - 1L;
	   else
	       if ( xClick >= pefw->aptl[i + 1L].x )
		   iLow	= i + 1L;
	       else
		       /* Have found the cell where the	mouse was	*/
		       /* clicked over.	 Now have to further refine the	*/
		       /* selection.  When the mouse has been clicked	*/
		       /* in the left half of the cell,	that cell is	*/
		       /* deemed as being the character	cell selected.	*/
		       /* When the mouse has been clicked in the right	*/
		       /* half of the cell, the	next character is	*/
		       /* deemed as being selected.			*/

		   if (	xClick > ((pefw->aptl[i	+ 1L].x	- pefw->aptl[i].x) / 2L) + pefw->aptl[i].x)
		       return(i	+ 1L);
		   else
		       return(i);

       return(i);
       }

return(0L);
}
#pragma	subtitle("   Entry Field Control - Text Character Position Procedure")
#pragma	page( )

/* --- lCalcDisplayPoint ------------------------------	[ Private } ---	*/
/*									*/
/*     This function is	used to	determine the left edge	of a selected	*/
/*     character cell.	The display point is rationalized to be		*/
/*     contained within	the boundary rectangle for the text.  What	*/
/*     this means is that when the display point is beyond the right	*/
/*     margin of the bounding rectangle, the point is rationalized to	*/
/*     be that margin.							*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	 = Entry Field Data Pointer			*/
/*     LONG	  iChar; = Character Index				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     lCalcDisplayPoint = Character Display Point			*/
/*									*/
/* --------------------------------------------------------------------	*/

static LONG lCalcDisplayPoint(PENTRYFWIN pefw, LONG iChar)

{
LONG xPoint;			   /* Point Position			*/

		       /* Check	to see if there	is a shifting offset	*/
		       /* indicating that the text has been scrolled	*/
		       /* such that portions of	the text have been	*/
		       /* scrolled off to the left of the entry	field.	*/
		       /* When this is the case, the point needs to be	*/
		       /* adjusted for this shift.  The	method used is	*/
		       /* to subtract the starting point for the first	*/
		       /* visible character from the desired character.	*/
		       /* If not the case, the actual display point is	*/
		       /* used.						*/
if ( pefw->iOffset )
		       /* Make sure that the character requested is not	*/
		       /* in front of the shifting offset which	if the	*/
		       /* case,	use the	offset as the bounding edge	*/

   if (	pefw->iOffset >	iChar )
       xPoint =	pefw->aptl[pefw->iOffset].x + EFMARGIN_LEFT;
   else
       xPoint =	pefw->aptl[iChar].x - pefw->aptl[pefw->iOffset].x + EFMARGIN_LEFT;
else
   xPoint = pefw->aptl[iChar].x	+ EFMARGIN_LEFT;

		       /* Make sure that the point within the text	*/
		       /* bounding rectangle on	the right edge		*/

return((xPoint < pefw->rclText.xRight) ? xPoint	: pefw->rclText.xRight);
}
#pragma	subtitle("   Entry Field Control - Scroll Right Determination Procedure")
#pragma	page( )

/* --- fScrollRight -----------------------------------	[ Private } ---	*/
/*									*/
/*     This function is	used to	determine if the entry field should	*/
/*     be scrolled right depending on the position of the current	*/
/*     selection such that if it is at the right margin	and less than	*/
/*     half an average character from the edge,	scrolling to the	*/
/*     right should occur.						*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	= Entry	Field Data Pointer			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     fScrollRight =  TRUE : Scrolling	to Right Should	Occur		*/
/*		    = FALSE : No Scrolling Should Occur
/*									*/
/* --------------------------------------------------------------------	*/

static BOOL fScrollRight(PENTRYFWIN pefw)

{
		       /* Check	to see if the character	selection point	*/
		       /* is within the	right margin area		*/

if ( (pefw->aptl[pefw->iSel].x - pefw->aptl[pefw->iOffset].x) >
     (pefw->rclText.xRight - pefw->cxMargin) )

		       /* Current selection point within the margin	*/
		       /* area,	check to see if	scrolling should	*/
		       /* occur						*/

   if (	(pefw->aptl[pefw->cText].x - pefw->aptl[pefw->iOffset].x) < (pefw->rclText.xRight / 3L)	)

		       /* Scrolling not	required since the end of the	*/
		       /* displayed text is fully visible within the	*/
		       /* entry	field					*/
       return(FALSE);
   else
		       /* Scrolling permissible, return	with the	*/
		       /* scroll flag					*/
       return(TRUE);
else
		       /* Not within the margin	area, return with the	*/
		       /* no scroll flag				*/
   return(FALSE);
}
#pragma	subtitle("   Entry Field Control - Cursor Management Procedure")
#pragma	page( )

/* --- ShowCursor -------------------------------------	[ Private } ---	*/
/*									*/
/*     This function is	used to	determine if the cursor	location	*/
/*     calculated for the current insertion point is visible and when	*/
/*     the case	to display it in that location otherwise it is not	*/
/*     shown.								*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	= Entry	Field Data Pointer			*/
/*     LONG	  iSel;	= Selection Point				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID ShowCursor(PENTRYFWIN pefw,	LONG iSel)

{
LONG xCursor;			   /* New Cursor Location		*/

if ( pefw->aptl	)
   xCursor = pefw->aptl[iSel].x	- pefw->aptl[pefw->iOffset].x +	EFMARGIN_LEFT;
else
   xCursor = EFMARGIN_LEFT;

		       /* Calculate the	new position of	the cursor and	*/
		       /* if it	is different from the previous location	*/
		       /* determine if it should be shown and		*/
		       /* repositioned					*/

if ( pefw->xCursor != xCursor )
   {
		       /* New cursor location calculated, save the new	*/
		       /* location internally within the private	*/
		       /* control data					*/

   pefw->xCursor = xCursor;

		       /* Check	to see if the new cursor location is	*/
		       /* within the bounding text rectangle of	the	*/
		       /* entry	field					*/

   if (	(pefw->xCursor >= pefw->rclText.xLeft) &&
	(pefw->xCursor <= pefw->rclText.xRight)	)
       {
		       /* Place	the cursor in its new position		*/

       WinCreateCursor(pefw->hWnd, pefw->xCursor, pefw->yCursor,
		       0L, 0L, CURSOR_SETPOS, NULL);

		       /* Cursor is within the rectangle, check	to see	*/
		       /* if it	was previously hidden and if the case,	*/
		       /* make it visible				*/

       if ( !pefw->fCursorShown	)
	   WinShowCursor(pefw->hWnd, pefw->fCursorShown	= TRUE);
       }
   else
		       /* Cursor is not	visible	within the bounding	*/
		       /* text rectangle.  Check to see	if the it is	*/
		       /* currently visible and	if the case, hide it.	*/

       if ( pefw->fCursorShown )
	   WinShowCursor(pefw->hWnd, pefw->fCursorShown	= FALSE);
   }
}
#pragma	subtitle("   Entry Field Control - Overtype Character Drawing Procedure")
#pragma	page( )

/* --- DrawOvertype -----------------------------------	[ Private } ---	*/
/*									*/
/*     This function is	used to	draw the selected overtype character.	*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	= Entry	Field Data Pointer			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID DrawOvertype(PENTRYFWIN pefw)

{
		       /* Determine the	left edge of the drawing area	*/

pefw->ptlDraw.x	= pefw->rclDraw.xLeft =	lCalcDisplayPoint(pefw,	pefw->iSel);

		       /* Determine the	right edge of the drawing area	*/

pefw->rclDraw.xRight = lCalcDisplayPoint(pefw, pefw->iSel + 1);

		       /* Set the foreground and background colour for	*/
		       /* the text to be drawn.	 This is the highlight	*/
		       /* text drawing colour.				*/

GpiSetColor(pefw->hPS, pefw->lClrHilite);
WinFillRect(pefw->hPS, &pefw->rclDraw, pefw->lClrHiliteBackground);

		       /* Draw the one character that is to be high-	*/
		       /* lighted					*/

GpiCharStringPosAt(pefw->hPS, &pefw->ptlDraw, &pefw->rclDraw, CHS_CLIP,
		   1L, &pefw->szText[pefw->iSel], NULL);
}
#pragma	subtitle("   Entry Field Control - Text Drawing Procedure")
#pragma	page( )

/* --- DrawTextString ---------------------------------	[ Private } ---	*/
/*									*/
/*     This function is	used to	draw the text string using range	*/
/*     provided	either in normal or highlighted	mode.			*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;		= Entry	Field Data Pointer		*/
/*     LONG	  iStart;	= Starting Point			*/
/*     LONG	  iEnd;		= Ending Point				*/
/*     BOOL	  fHighlighted;	= Highlighting Required	Flag		*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID DrawTextString(PENTRYFWIN pefw, LONG iStart, LONG iEnd,
			   BOOL	fHighlighted)

{
		       /* Determine the	left edge of the drawing area	*/

pefw->ptlDraw.x	= pefw->rclDraw.xLeft =	lCalcDisplayPoint(pefw,	iStart);

		       /* Determine the	right edge of the drawing area	*/

pefw->rclDraw.xRight = lCalcDisplayPoint(pefw, iEnd);

		       /* Set the foreground and background colour for	*/
		       /* the text to be drawn.	 This is the highlight	*/
		       /* text drawing colour.				*/

GpiSetColor(pefw->hPS, fHighlighted ? pefw->lClrHilite : pefw->lClrText);
WinFillRect(pefw->hPS, &pefw->rclDraw, fHighlighted ?
				       pefw->lClrHiliteBackground :
				       pefw->lClrBackground);

		       /* Draw the desired text	in the location		*/
		       /* calculated					*/

GpiCharStringPosAt(pefw->hPS, &pefw->ptlDraw, &pefw->rclDraw, CHS_CLIP,
		   iEnd	- iStart, &pefw->szText[iStart], NULL);
}
#pragma	subtitle("   Entry Field Control - Text Character Position Procedure")
#pragma	page( )

/* --- DrawText	---------------------------------------	[ Private } ---	*/
/*									*/
/*     This function is	used to	determine where	the mouse pointer	*/
/*     has clicked in terms of the characters that compose the text	*/
/*     of the entry field.						*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	= Entry	Field Data Pointer			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID DrawText(PENTRYFWIN	pefw)

{
		       /* Check	to see if the cursor is	being shown and	*/
		       /* if so, hide it before	drawing			*/

if ( pefw->fCursorShown	)
   WinShowCursor(pefw->hWnd, pefw->fCursorShown	= FALSE);

		       /* Check	to see if there	is no selected text or	*/
		       /* that the control has lost its	focus which	*/
		       /* causes the highlighted text to be dehigh-	*/
		       /* lighted.  Also check for a special case where	*/
		       /* the shift offset is greater than the end of	*/
		       /* the selected text.				*/

if ( (pefw->iSelStart == pefw->iSelEnd)	|| !pefw->fFocus ||
     (pefw->iOffset > pefw->iSelEnd) )
   {
		       /* Draw the text	starting from the shift	offset	*/
		       /* point						*/

   DrawTextString(pefw,	pefw->iOffset, pefw->cText, EFDISPLAY_NORMAL);

		       /* Check	to see if in overtype mode which means	*/
		       /* that the character immediately to the	right	*/
		       /* of the insertion point should	be highlighted.	*/
		       /* This will only occur if a character is indeed	*/
		       /* adjacent to the cursor and the left mouse	*/
		       /* button is not	currently being	pressed.	*/

   if (	pefw->fOvertype	&& (pefw->iSelStart == pefw->iSelEnd) &&
	pefw->fFocus &&	(pefw->iSel != pefw->cText) && !pefw->fBtnDown )
       DrawOvertype(pefw);
   }
else
		       /* Check	to see if the shift offset is after the	*/
		       /* selection starting point			*/

   if (	pefw->iOffset >	pefw->iSelStart	)
       {
		       /* Draw the text	starting from the shift	offset	*/
		       /* point						*/

       DrawTextString(pefw, pefw->iOffset, pefw->iSelEnd, EFDISPLAY_HIGHLIGHT);

		       /* Check	to see if any text is to be drawn to	*/
		       /* the right of the selected text and if	the	*/
		       /* case,	draw it					*/

       if ( (pefw->iSelEnd < pefw->cText) &&
	    (pefw->rclDraw.xRight < pefw->rclText.xRight) )

		       /* Draw the text	starting from the highlighted	*/
		       /* point						*/

	   DrawTextString(pefw,	pefw->iSelEnd, pefw->cText, EFDISPLAY_NORMAL);
       else
		       /* No text to the right of the highlighted text,	*/
		       /* erase	the remaining portion of the rectangle	*/
	   {
	   pefw->rclDraw.xLeft = pefw->rclDraw.xRight;
	   pefw->rclDraw.xRight	= pefw->rclText.xRight;
	   WinFillRect(pefw->hPS, &pefw->rclDraw, pefw->lClrBackground);
	   }
       }
   else
		       /* Check	to see if the selection	point is not	*/
		       /* starting at the begining of the text of the	*/
		       /* entry	field which means that the displayed	*/
		       /* text needs to	be displayed normally before	*/
		       /* the highlighted text is drawn			*/
       {
       if ( pefw->iSelStart && (pefw->iOffset <	pefw->iSelStart) )

		       /* Draw the text	starting from the shift	offset	*/
		       /* point						*/

	   DrawTextString(pefw,	pefw->iOffset, pefw->iSelStart,	EFDISPLAY_NORMAL);
       else
		       /* No normal display text before	the selected	*/
		       /* text,	calculate the starting right edge of	*/
		       /* the non-display text				*/

	   pefw->rclDraw.xRight	= lCalcDisplayPoint(pefw, pefw->iSelStart);

		       /* Check	to see if any highlighted text needs to	*/
		       /* be displayed.	 The basic trick here is to	*/
		       /* check	to see if the previously drawn text is	*/
		       /* within the text bounding rectangle.		*/

       if ( pefw->rclDraw.xRight < pefw->rclText.xRight	)
	   {
		       /* Draw the text	starting from the highlighting	*/
		       /* point						*/

	   DrawTextString(pefw,	pefw->iSelStart, pefw->iSelEnd,	EFDISPLAY_HIGHLIGHT);

		       /* Check	to see if there	is any text that needs	*/
		       /* to be	drawn after the	highlighted text that	*/
		       /* is within the	text boudning rectangle		*/

	   if (	(pefw->cText > pefw->iSelEnd) &&
		(pefw->rclDraw.xRight <	pefw->rclText.xRight) )

		       /* Draw the text	starting from the highlighting	*/
		       /* point						*/

	       DrawTextString(pefw, pefw->iSelEnd, pefw->cText,
			      EFDISPLAY_NORMAL);

		       /* No text to the right of the highlighted text,	*/
		       /* erase	the remaining portion of the rectangle	*/

	   pefw->rclDraw.xLeft = pefw->rclDraw.xRight;
	   pefw->rclDraw.xRight	= pefw->rclText.xRight;
	   WinFillRect(pefw->hPS, &pefw->rclDraw, pefw->lClrBackground);
	   }
       }
		       /* Redisplay the	cursor since all of the	drawing	*/
		       /* is now complete				*/

WinShowCursor(pefw->hWnd, pefw->fCursorShown = TRUE);
}
#pragma	subtitle("   Entry Field Control - Text Character Position Procedure")
#pragma	page( )

/* --- DrawSubText ------------------------------------	[ Private } ---	*/
/*									*/
/*     This function is	used to	determine where	the mouse pointer	*/
/*     has clicked in terms of the characters that compose the text	*/
/*     of the entry field.						*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	  = Entry Field	Data Pointer			*/
/*     LONG	  iStart; = Starting Character Index			*/
/*     LONG	  iEnd;	  = Ending Character Index			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID DrawSubText(PENTRYFWIN pefw, LONG iStart, LONG iEnd)

{
		       /* Check	to see if the starting point is	before	*/
		       /* the shift offset and if the case make	the	*/
		       /* starting point the shift offset		*/

if ( iStart < pefw->iOffset )
   iStart = pefw->iOffset;

		       /* Check	to see if the ending point is after the	*/
		       /* last character and if	the case make the	*/
		       /* ending point the last	character offset	*/

if ( iEnd > pefw->cText	)
   iEnd	= pefw->cText;
		       /* Check	to see if the cursor is	being shown and	*/
		       /* if so, hide it before	drawing			*/

if ( pefw->fCursorShown	)
   WinShowCursor(pefw->hWnd, pefw->fCursorShown	= FALSE);

		       /* Check	to see if there	is any highlighting	*/
		       /* required which is indicated by a selecting	*/
		       /* range	and when not the case, just draw the	*/
		       /* text range normally				*/

if ( (pefw->iSelStart == pefw->iSelEnd)	|| (iStart > pefw->iSelEnd) ||
     (iEnd < pefw->iSelStart) )
   {
		       /* Draw the text	starting from the starting	*/
		       /* point						*/

   DrawTextString(pefw,	iStart,	iEnd, EFDISPLAY_NORMAL);

		       /* Check	to see if in overtype mode which means	*/
		       /* that the character immediately to the	right	*/
		       /* of the insertion point should	be highlighted.	*/
		       /* This will only occur if a character is indeed	*/
		       /* adjacent to the cursor and the left mouse	*/
		       /* button is not	currently being	pressed.	*/

   if (	pefw->fOvertype	&& (pefw->iSelStart == pefw->iSelEnd) &&
	pefw->fFocus &&	(pefw->iSel != pefw->cText) && !pefw->fBtnDown )
       DrawOvertype(pefw);
   }
else
		       /* Check	to see if just highlighted text	is to	*/
		       /* be drawn					*/

   if (	(iStart	> pefw->iSelStart) && (iEnd < pefw->iSelEnd) )

		       /* Draw the text	starting from the starting	*/
		       /* point	highlighted				*/

       DrawTextString(pefw, iStart, iEnd, EFDISPLAY_HIGHLIGHT);
   else
       if ( iStart > pefw->iSelStart )
	   {
		       /* Draw the text	starting from the starting	*/
		       /* point	highlighted				*/

	   DrawTextString(pefw,	iStart,	pefw->iSelEnd, EFDISPLAY_HIGHLIGHT);

		       /* Check	to see if the right edge of the	high-	*/
		       /* lighted area was within the text bounding	*/
		       /* rectangle in which case there	is normal text	*/
		       /* that still needs to be drawn to the right	*/
		       /* of the highlighted area			*/

	   if (	pefw->rclDraw.xRight < pefw->rclText.xRight )

		       /* Draw the text	starting from the starting	*/
		       /* point	normally				*/

	       DrawTextString(pefw, pefw->iSelEnd, iEnd, EFDISPLAY_NORMAL);
	   }
       else
		       /* Check	to see if the end point	is before the	*/
		       /* the selection	end point			*/
	   if (	iEnd < pefw->iSelEnd )
	       {
		       /* First	draw the normal	text that appears	*/
		       /* before the highlighted selection		*/

	       DrawTextString(pefw, iStart, pefw->iSelStart, EFDISPLAY_NORMAL);

		       /* Check	to make	sure that the right edge of the	*/
		       /* text just drawn falls	within the text		*/
		       /* bounding rectangle which indicates that the	*/
		       /* remaining text can be	drawn			*/

	       if ( pefw->rclDraw.xRight < pefw->rclText.xRight	)

		       /* Draw the text	starting from the starting	*/
		       /* point	highlighted				*/

		   DrawTextString(pefw,	pefw->iSelStart, iEnd,
				  EFDISPLAY_HIGHLIGHT);
	       }
	   else
	       {
		       /* First	draw the normal	text that appears	*/
		       /* before the highlighted selection		*/

	       DrawTextString(pefw, iStart, pefw->iSelStart, EFDISPLAY_NORMAL);

		       /* Check	to make	sure that the right edge of the	*/
		       /* text just drawn falls	within the text		*/
		       /* bounding rectangle which indicates that the	*/
		       /* remaining text can be	drawn			*/

	       if ( pefw->rclDraw.xRight < pefw->rclText.xRight	)
		   {
		       /* Next draw the	highlighted text		*/

		   DrawTextString(pefw,	pefw->iSelStart, pefw->iSelEnd,
				  EFDISPLAY_HIGHLIGHT);

		       /* Check	to make	sure that the right edge of the	*/
		       /* text just drawn falls	within the text		*/
		       /* bounding rectangle which indicates that the	*/
		       /* remaining text can be	drawn			*/

		   if (	pefw->rclDraw.xRight < pefw->rclText.xRight )

		       /* Lastly draw the normal text that appears	*/
		       /* after	the highlighted	selection		*/

		       DrawTextString(pefw, pefw->iSelEnd, iEnd,
				      EFDISPLAY_NORMAL);
		   }
	       }
		       /* Redisplay the	cursor since all of the	drawing	*/
		       /* is now complete				*/

WinShowCursor(pefw->hWnd, pefw->fCursorShown = TRUE);
}
#pragma	subtitle("   Entry Field Control - Overtype Text Draw Procedure")
#pragma	page( )

/* --- DrawOvertypeText	-------------------------------	[ Private } ---	*/
/*									*/
/*     This function is	used to	draw the text indicating the overtype	*/
/*     location.							*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	      =	Entry Field Data Pointer		*/
/*     BOOL	  fMoveRight; =	Move Right Direction			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID DrawOvertypeText(PENTRYFWIN	pefw, BOOL fMoveRight)

{
		       /* Check	to see if the cursor is	being shown and	*/
		       /* if so, hide it before	drawing			*/

if ( pefw->fCursorShown	)
   WinShowCursor(pefw->hWnd, pefw->fCursorShown	= FALSE);

		       /* Get the presentation space for the control	*/
		       /* place	the colour table into RGB mode before	*/
		       /* performing any drawing			*/

GpiCreateLogColorTable(pefw->hPS = WinGetPS(pefw->hWnd),
		       0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);

		       /* Check	to see if any text to the right	of the	*/
		       /* selection point or if	not moving to the right	*/
		       /* just calculate the left edge			*/

if ( !fMoveRight && ((pefw->iSel + 1L) == pefw->cText) )
   pefw->rclDraw.xLeft = lCalcDisplayPoint(pefw, pefw->iSel + 1);
else
   {
		       /* Calculate the	left drawing point for the	*/
		       /* text						*/

   pefw->rclDraw.xLeft = pefw->ptlDraw.x = lCalcDisplayPoint(pefw, fMoveRight ?	pefw->iSel - 1 : pefw->iSel + 1);

		       /* Calculate the	right drawing point for	the	*/
		       /* text						*/

   pefw->rclDraw.xRight	= lCalcDisplayPoint(pefw, fMoveRight ? pefw->iSel : pefw->iSel + 2);

		       /* Set the foreground and background colour for	*/
		       /* the text to be drawn.	 This is the normal	*/
		       /* text drawing colour.				*/

   GpiSetColor(pefw->hPS, pefw->lClrText);
   WinFillRect(pefw->hPS, &pefw->rclDraw, pefw->lClrBackground);

		       /* Draw the text					*/

   GpiCharStringPosAt(pefw->hPS, &pefw->ptlDraw, &pefw->rclDraw, CHS_CLIP, 1L,
		      &pefw->szText[fMoveRight ? pefw->iSel - 1	: pefw->iSel + 1], NULL);
   }
		       /* Determine if the current selection point is	*/
		       /* not at the end of the	text string which if	*/
		       /* the case need	to draw	overtype text character	*/

if ( pefw->iSel	< pefw->cText )
   {
		       /* Check	to see if a move right performed which	*/
		       /* causes the display points to be adjusted for	*/
		       /* the action					*/
   if (	fMoveRight )
       {
       pefw->rclDraw.xLeft = pefw->ptlDraw.x = pefw->rclDraw.xRight;
       pefw->rclDraw.xRight = lCalcDisplayPoint(pefw, pefw->iSel + 1);
       }
   else
       {
		       /* Just displaying the overtype point, find the	*/
		       /* display bounding rectangle for the character	*/

       pefw->rclDraw.xRight = pefw->rclDraw.xLeft;
       pefw->rclDraw.xLeft = pefw->ptlDraw.x = lCalcDisplayPoint(pefw, pefw->iSel);
       }
		       /* Set the colour for the highlighted character	*/

   GpiSetColor(pefw->hPS, pefw->lClrHilite);
   WinFillRect(pefw->hPS, &pefw->rclDraw, pefw->lClrHiliteBackground);

		       /* Draw the highlighted character		*/

   GpiCharStringPosAt(pefw->hPS, &pefw->ptlDraw, &pefw->rclDraw, CHS_CLIP,
		      1L, &pefw->szText[pefw->iSel], NULL);
   }
		       /* Release the presentation space		*/
WinReleasePS(pefw->hPS);
		       /* Redisplay the	cursor since all of the	drawing	*/
		       /* is now complete				*/

WinShowCursor(pefw->hWnd, pefw->fCursorShown = TRUE);
}
#pragma	subtitle("   Entry Field Control - Text Character Position Procedure")
#pragma	page( )

/* --- SelectText -------------------------------------	[ Private } ---	*/
/*									*/
/*     This function is	used to	determine where	the mouse pointer	*/
/*     has clicked in terms of the characters that compose the text	*/
/*     of the entry field.						*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	     = Entry Field Data	Pointer			*/
/*     LONG	  iAnchor;   = Anchor Position				*/
/*     LONG	  iSelected; = New Selection Position			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID SelectText(PENTRYFWIN pefw,	LONG iAnchor, LONG iSelected)

{
LONG iSelStart;			   /* Previous Selected	Start		*/
LONG iSelEnd;			   /* Previous Selected	End		*/

		       /* Check	to see if the selection	point specified	*/
		       /* is not the current selection point which if	*/
		       /* the case, just ignore	selection request	*/

if ( iSelected != pefw->iSel )
   {
		       /* Get the presentation space for the control	*/
		       /* place	the colour table into RGB mode before	*/
		       /* performing any drawing			*/

   GpiCreateLogColorTable(pefw->hPS = WinGetPS(pefw->hWnd), 0L,	LCOLF_RGB,
			  0L, 0L, (PLONG)NULL);

		       /* Save the current selection points		*/

   iSelStart = pefw->iSelStart;
   iSelEnd   = pefw->iSelEnd;

		       /* Check	to see if the anchor point is the	*/
		       /* current selection point which	if the case	*/
		       /* need to remove any selection indicators	*/

   if (	iSelected == iAnchor )
       {
		       /* Redraw the revised text non-highlighting	*/
		       /* after	setting	the selection points to	the	*/
		       /* new anchor point				*/

       pefw->iSelStart = pefw->iSelEnd = iAnchor;
       DrawSubText(pefw, iSelStart, iSelEnd);
       }
   else
		       /* Check	to see if the anchor point is before	*/
		       /* the new selection point which	if the case	*/
		       /* means	that the movement is going from	left	*/
		       /* to right generally				*/

       if ( iSelected <	iAnchor	)
	   {
		       /* Save the new selection points			*/

	   pefw->iSelStart = iSelected;
	   pefw->iSelEnd   = iAnchor;

		       /* Refresh the text display showing the new	*/
		       /* selection					*/

	   DrawSubText(pefw,
		       iSelStart < pefw->iSelStart ? iSelStart : pefw->iSelStart,
		       pefw->iSelEnd);
	   }
       else
		       /* Anchor point is after	the selection point,	*/
		       /* which	means that the movement	is going from	*/
		       /* right	to left	generally			*/
	   {
		       /* Save the new selection points			*/

	   pefw->iSelStart = iAnchor;
	   pefw->iSelEnd   = iSelected;

		       /* Refresh the text display showing the new	*/
		       /* selection					*/

	   DrawSubText(pefw, pefw->iSelStart,
		       iSelEnd > pefw->iSelEnd ? iSelEnd : pefw->iSelEnd);
	   }
		       /* Release the presentation space		*/

   WinReleasePS(pefw->hPS);
   }
}
#pragma	subtitle("   Entry Field Control - Entry Field Metrics Update Procedure")
#pragma	page( )

/* --- RefreshMetrics ---------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	update entry field metrics when		*/
/*     a possible font change has occurred or when the style of		*/
/*     the entry field has changed.					*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	= Entry	Field Private Data			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID RefreshMetrics(PENTRYFWIN pefw)

{
FONTMETRICS fm;			   /* Font Metrics			*/

		       /* Get the font metrics for the presentation	*/
		       /* space						*/

GpiQueryFontMetrics(pefw->hPS, sizeof(FONTMETRICS), &fm);

		       /* Check	to see if any text defined currently	*/
		       /* for the entry	field and if so, cause the	*/
		       /* display points array to be updated such that	*/
		       /* it properly reflects the revised font	metrics	*/
		       /* differences that may have been caused	due to	*/
		       /* a font change					*/
if ( pefw->cText )
    {
		       /* Get the revised points array			*/

    QueryCharPoints(pefw);

		       /* Calculate the	new cursor position and	force	*/
		       /* it to	the new	location			*/

    pefw->xCursor = pefw->aptl[pefw->iSel].x + EFMARGIN_LEFT;
    ShowCursor(pefw, pefw->iSel);
    }
else
		       /* Set the cursor location for the start	of the	*/
		       /* empty	entry field				*/

    pefw->xCursor = EFMARGIN_LEFT;

		       /* Calculate the	new metrics for	the entry field	*/

pefw->cxMargin = (pefw->cxChar = fm.lAveCharWidth) / 2L;
pefw->cyCursor = fm.lMaxBaselineExt;
pefw->yDesc    = fm.lMaxDescender;
pefw->yCursor  = EFMARGIN_BOTTOM;

		       /* Adjust the sizing of the entry field		*/

if ( (pefw->cx > 0L) &&	(pefw->cx > 0L)	)
    SizeEntryField(pefw);
}
#pragma	subtitle("   Entry Field Control - Entry Field Control Data Decoding Procedure")
#pragma	page( )

/* --- DecodeCtlData ----------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	process	the entry field	control	data	*/
/*     that may	be provided when the entry field is first created.	*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	= Entry	Field Private Data			*/
/*     MPARAM	  mp1;	= Message Parameter 1				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID DecodeCtlData(PENTRYFWIN pefw, MPARAM mp1)

{
PENTRYFCDATA pefcd;		   /* Entry Field Control Data Pointer	*/

		       /* Check	to see if any control data specified	*/
		       /* in the first message parameter		*/

if ( (pefcd = (PENTRYFCDATA)PVOIDFROMMP(mp1)) == NULL )

		       /* No control data defined during the creation,	*/
		       /* set up the control for the default text limit	*/
   pefw->cLimit	= 32L;
else
		       /* Data present,	check to see if	the control	*/
		       /* data the older 16-bit	type or	the newer	*/
		       /* 32-bit type by checking the size of the	*/
		       /* structure					*/

   if (	pefcd->efd.cb == sizeof(ENTRYFDATA) )
       {
		       /* 16-bit layout, get the maximum amount	of text	*/
		       /* the control is to handle along with any	*/
		       /* predefined selections				*/

       pefw->cLimit  = pefcd->efd.cchEditLimit;
       pefw->iSel    = pefw->iSelStart = pefw->iAnchor = pefcd->efd.ichMinSel;
       pefw->iSelEnd = pefcd->efd.ichMaxSel;
       }
   else
       {
		       /* 32-bit layout, get the maximum amount	of text	*/
		       /* the control is to handle along with any	*/
		       /* predefined selections				*/

       pefw->cLimit  = (LONG)pefcd->efd2.cchEditLimit;
       pefw->iSel    = pefw->iSelStart = pefw->iAnchor = (LONG)pefcd->efd2.ichMinSel;
       pefw->iSelEnd = (LONG)pefcd->efd2.ichMaxSel;
       }
}
#pragma	subtitle("   Entry Field Control - Entry Field Message Processing Procedure")
#pragma	page( )

/* --- mrBaseFieldHandler -----------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	process	the base messages for the	*/
/*     entry field control window.					*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HWND   hWnd; = Window Handle					*/
/*     ULONG  msg;  = PM Message					*/
/*     MPARAM mp1;  = Message Parameter	1				*/
/*     MPARAM mp2;  = Message Parameter	2				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     mrBaseFieldHandler = Message Handling Result			*/
/*									*/
/* --------------------------------------------------------------------	*/

MRESULT	EXPENTRY mrBaseFieldHandler(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)

{
#pragma	info(nopar)

BOOL	   fChanged;		   /* Previous Changed Flag		*/
BOOL	   fInsertMode;		   /* Previous Insert Mode Flag		*/
BOOL	   fReadOnly;		   /* Previous Read Only Flag		*/
PCHAR	   pchClipText;		   /* Clipboard	Text Pointer		*/
PENTRYFWIN pefw;		   /* Entry Field Internal Data	Pointer	*/
register LONG n;		   /* Text Length Holder		*/

pefw = (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

switch ( msg )
   {

   /*********************************************************************/
   /*  Base message:	   EM_CLEAR					*/
   /*			   mp1 = 0L;					*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	EM_CLEAR :
		       /* Check	to make	sure that the entry field is	*/
		       /* not read-only	and that a selection has been	*/
		       /* made before trying to	delete the text		*/

       if ( !pefw->fReadOnly &&	(pefw->iSelStart != pefw->iSelEnd) )
	   {
	   DeleteText(pefw, pefw->iSelStart, pefw->iSelEnd, TRUE);
	   return(MRFROMLONG(TRUE));
	   }
       else
	   return(MRFROMLONG(FALSE));

   /*********************************************************************/
   /*  Base message:	   EM_COPY					*/
   /*			   mp1 = 0L;					*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	EM_COPY	:
		       /* Check	to make	sure that a selection within	*/
		       /* the text has been made before	trying to copy	*/
		       /* the selection	to the clipboard		*/

       if ( (pefw->iSelStart !=	pefw->iSelEnd) )
	   {
		       /* Check	to see if the entry field is unreadable	*/
		       /* which	means that it is being used for	some-	*/
		       /* thing	like a password	entry area.  In	this	*/
		       /* case do not allow the	text to	be copied to	*/
		       /* the clipboard.  (Note:  this differs from	*/
		       /* the OS/2 entry field in that it will allow	*/
		       /* you to copy the text to the clipboard)	*/

	   if (	pefw->flStyle &	ES_UNREADABLE )
	       {
	       WinAlarm(HWND_DESKTOP, WA_ERROR);
	       return(MRFROMLONG(FALSE));
	       }
	   else
	       {
		       /* Allocate the shared memory for the text	*/
		       /* making sure that there is enough memory for	*/
		       /* the text plus	the NULL terminating byte	*/

	       DosAllocSharedMem((PPVOID)(PVOID)&pchClipText, NULL,
				 (ULONG)(pefw->iSelEnd - pefw->iSelStart + 1),
				 fALLOCSHR);

		       /* Open and empty the clipboard before copying	*/
		       /* the selected text to the shared memory	*/
		       /* and placing the pointer on the clipboard	*/

	       WinOpenClipbrd(pefw->hAB);
	       WinEmptyClipbrd(pefw->hAB);
	       WinSetClipbrdData(pefw->hAB, (ULONG)pszCopyText(pefw, pchClipText,
							       pefw->iSelStart,
							       pefw->iSelEnd),
				 CF_TEXT, CFI_POINTER);

		       /* Close	the clipboard to allow the copied text	*/
		       /* to be	used by	other processes	as well	as this	*/
		       /* one						*/

	       WinCloseClipbrd(pefw->hAB);
	       return(MRFROMLONG(TRUE));
	       }
	   }
       else
	   return(MRFROMLONG(FALSE));

   /*********************************************************************/
   /*  Base message:	   EM_CUT					*/
   /*			   mp1 = 0L;					*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	EM_CUT :
		       /* Check	to make	sure that the entry field is	*/
		       /* not read-only	and that a selection has been	*/
		       /* made before trying to	cut the	text from the	*/
		       /* entry	field and place	it on the system	*/
		       /* clipboard					*/

       if ( !pefw->fReadOnly &&	(pefw->iSelStart != pefw->iSelEnd) )
	   {
		       /* Allocate the shared memory for the text	*/
		       /* making sure that there is enough memory for	*/
		       /* the text plus	the NULL terminating byte	*/

	   DosAllocSharedMem((PPVOID)(PVOID)&pchClipText, NULL,
			     (ULONG)(pefw->iSelEnd - pefw->iSelStart + 1),
			     fALLOCSHR);

		       /* Open and empty the clipboard before copying	*/
		       /* the selected text to the shared memory	*/
		       /* and placing the pointer on the clipboard	*/

	   WinOpenClipbrd(pefw->hAB);
	   WinEmptyClipbrd(pefw->hAB);
	   WinSetClipbrdData(pefw->hAB,
			     (ULONG)pszCopyText(pefw, pchClipText, pefw->iSelStart, pefw->iSelEnd),
			     CF_TEXT, CFI_POINTER);

		       /* Close	the clipboard to allow the copied text	*/
		       /* to be	used by	other processes	as well	as this	*/
		       /* one						*/

	   WinCloseClipbrd(pefw->hAB);

		       /* Delete the selected text			*/

	   DeleteText(pefw, pefw->iSelStart, pefw->iSelEnd, TRUE);
	   return(MRFROMLONG(TRUE));
	   }
       else
	   return(MRFROMLONG(FALSE));

   /*********************************************************************/
   /*  Base message:	   EM_PASTE					*/
   /*			   mp1 = 0L;					*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	EM_PASTE :
		       /* Check	to make	sure that the entry field is	*/
		       /* not read-only					*/

       if ( pefw->fReadOnly )
	   return(MRFROMLONG(FALSE));
       else
	   {
		       /* Open the clipboard and then check to see if	*/
		       /* any text has been placed on it that can be	*/
		       /* pasted within	the entry field			*/

	   WinOpenClipbrd(pefw->hAB);
	   if (	WinQueryClipbrdData(pefw->hAB, CF_TEXT)	)
	       {
		       /* Text contained within	the clipboard, get the	*/
		       /* pointer to the text				*/

	       if ( (pchClipText = (PCHAR)WinQueryClipbrdData(pefw->hAB, CF_TEXT)) != NULL )

		       /* Check	to make	sure that the selection	within	*/
		       /* the clipboard	will fit within	the entry field	*/

		   if (	((n = (LONG)strlen(pchClipText)) + pefw->cText)	< pefw->cLimit )
		       {
		       /* Check	to see if any text is currently		*/
		       /* selected which means that the	text should be	*/
		       /* replaced by the text from the	clipboard.  If	*/
		       /* the case, delete the selected	text.		*/

		       if ( pefw->iSelStart != pefw->iSelEnd )
			   DeleteText(pefw, pefw->iSelStart, pefw->iSelEnd, FALSE);

		       /* Paste	the text into the entry	field at the	*/
		       /* current selection point			*/

		       PasteText(pefw, pchClipText, n);
		       }
		       /* Finished with	the clipboard, close it		*/

	       WinCloseClipbrd(pefw->hAB);
	       return(MRFROMLONG(TRUE));
	       }
	   else
		       /* No text within the clipboard,	close it	*/
	       {
	       WinCloseClipbrd(pefw->hAB);
	       return(MRFROMLONG(FALSE));
	       }
	   }

   /*********************************************************************/
   /*  Base message:	   EM_QUERYCHANGED				*/
   /*			   mp1 = 0L;					*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	EM_QUERYCHANGED	:
       fChanged	= pefw->fChanged;
       pefw->fChanged =	FALSE;
       return(MRFROMLONG(fChanged));

   /*********************************************************************/
   /*  Base message:	   EM_QUERYFIRSTCHAR				*/
   /*			   mp1 = 0L;					*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	EM_QUERYFIRSTCHAR :
		       /* Return the offset to the first displayed	*/
		       /* character in the entry field			*/

       return(MRFROMLONG(pefw->iOffset));

   /*********************************************************************/
   /*  Base message:	   EM_QUERYREADONLY				*/
   /*			   mp1 = 0L;					*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	EM_QUERYREADONLY :

		       /* Return the read-only status of the entry	*/
		       /* field						*/

       return(MRFROMLONG(pefw->fReadOnly));

   /*********************************************************************/
   /*  Base message:	   EM_QUERYSEL					*/
   /*			   mp1 = 0L;					*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	EM_QUERYSEL :
		       /* Return the selection range			*/

       return(MRFROM2SHORT(pefw->iSelStart, pefw->iSelEnd));

   /*********************************************************************/
   /*  Base message:	   EM_SETFIRSTCHAR				*/
   /*			   mp1 = MPFROMSHORT(sOffset);			*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	EM_SETFIRSTCHAR	:
		       /* Check	to make	sure that the value specified	*/
		       /* for the message is within the	text limits	*/

       if ( ((LONG)(SHORT)SHORT1FROMMP(mp1) >= 0L) &&
	    ((LONG)(SHORT)SHORT1FROMMP(mp1) <= pefw->cText) )
	   {
		       /* Check	to see if the requested	starting point	*/
		       /* is the current offset	in which case nothing	*/
		       /* needs	to be done				*/

	   if (	pefw->iOffset != (LONG)(SHORT)SHORT1FROMMP(mp1)	)
	       {
		       /* New offset being defined, save the new value	*/

	       pefw->iOffset = (LONG)(SHORT)SHORT1FROMMP(mp1);

		       /* Get the presentation space handle for	the	*/
		       /* entry	field and place	it in RGB mode before	*/
		       /* causing the text to be redrawn showing the	*/
		       /* new offset					*/

	       GpiCreateLogColorTable(pefw->hPS	= WinGetPS(pefw->hWnd),
				      0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
	       DrawText(pefw);
	       WinReleasePS(pefw->hPS);
	       }
	   return(MRFROMLONG(TRUE));
	   }
       else
	   return(MRFROMLONG(FALSE));

   /*********************************************************************/
   /*  Base message:	   EM_SETINSERTMODE				*/
   /*			   mp1 = MPFROMSHORT(usInsert);			*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	EM_SETINSERTMODE :
		       /* Determine the	current	insert mode and	save	*/
		       /* since	it is returned				*/

       if ( pefw->fOvertype )
	   fInsertMode = FALSE;
       else
	   fInsertMode = TRUE;

		       /* Determine the	new insert mode	and save the	*/
		       /* overtype equivalent				*/

       if ( (BOOL)SHORT1FROMMP(mp1) )
	   pefw->fOvertype = FALSE;
       else
	   pefw->fOvertype = TRUE;

		       /* Update the internal OS/2 PM insert mode to	*/
		       /* reflect the change				*/

       WinSetSysValue(HWND_DESKTOP, SV_INSERTMODE,
		      (LONG)(BOOL)SHORT1FROMMP(mp1));

		       /* Check	to see if the overtype character needs	*/
		       /* to be	redrawn	do to the change of the	insert	*/
		       /* mode						*/

       if ( fInsertMode	!= (BOOL)SHORT1FROMMP(mp1) )
	   {
	   GpiCreateLogColorTable(pefw->hPS = WinGetPS(hWnd),
				  0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
	   DrawText(pefw);
	   WinReleasePS(pefw->hPS);
	   }
       return(MRFROMLONG(fInsertMode));

   /*********************************************************************/
   /*  Base message:	   EM_SETREADONLY				*/
   /*			   mp1 = MPFROMSHORT(usReadOnly);		*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	EM_SETREADONLY :
       fReadOnly = pefw->fReadOnly;

       pefw->fReadOnly = (BOOL)SHORT1FROMMP(mp1);
       return(MRFROMLONG(fReadOnly));

   /*********************************************************************/
   /*  Base message:	   EM_SETSEL					*/
   /*			   mp1 = MPFROM2SHORT(usMinSel,	usMaxSel);	*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	EM_SETSEL :
       if ( ((LONG)(SHORT)SHORT1FROMMP(mp1) >= 0L) &&
	    ((LONG)(SHORT)SHORT1FROMMP(mp1) <= (LONG)(SHORT)SHORT2FROMMP(mp1)) )
	   {
		       /* Check	to see if the selection	range is	*/
		       /* different in which case need to reflect the	*/
		       /* new range otherwise just ignore the request	*/

	   if (	(pefw->iSelStart != (LONG)(SHORT)SHORT1FROMMP(mp1)) &&
		(pefw->iSelEnd	 != (LONG)(SHORT)SHORT2FROMMP(mp1)) )
	       {
		       /* New selection	range being specified, save the	*/
		       /* starting value				*/

	       pefw->iSelStart = (LONG)(SHORT)SHORT1FROMMP(mp1);

		       /* Check	the ending selection value to make sure	*/
		       /* that if it is	greater	than the amount	of text	*/
		       /* contained within the entry field, the	range	*/
		       /* ends at the end of the text			*/

	       if ( (pefw->iSelEnd = (LONG)(SHORT)SHORT2FROMMP(mp1)) > pefw->cText )
		   pefw->iSelEnd = pefw->cText;

		       /* Get the presentation space handle for	the	*/
		       /* entry	field and place	it in RGB mode before	*/
		       /* causing the text to be redrawn showing the	*/
		       /* new selection					*/

	       GpiCreateLogColorTable(pefw->hPS	= WinGetPS(pefw->hWnd),
				  0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
	       DrawText(pefw);
	       WinReleasePS(pefw->hPS);
	       }

	   return(MRFROMLONG(TRUE));
	   }
       else
	   return(MRFROMLONG(FALSE));

   /*********************************************************************/
   /*  Base message:	   EM_SETTEXTLIMIT				*/
   /*			   mp1 = MPFROMSHORT(sTextLimit);		*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	EM_SETTEXTLIMIT	:
		       /* Check	to make	sure that the text limit	*/
		       /* specified is within the current text holder	*/
		       /* limits					*/

       if ( ((LONG)(SHORT)SHORT1FROMMP(mp1) >= 0L) &&
	    ((LONG)(SHORT)SHORT1FROMMP(mp1) < 1024L) )
	   {
		       /* New limit within control limits, save	the	*/
		       /* value	internally				*/

	   pefw->cLimit	= (LONG)(SHORT)SHORT1FROMMP(mp1);
	   return(MRFROMLONG(TRUE));
	   }
       else
		       /* Requested text limit too high, inform	the	*/
		       /* owner	of the problem				*/
	   {
	   mrNotifyOwner(pefw, EN_MEMERROR);
	   return(MRFROMLONG(FALSE));
	   }
   }
return(0L);
}
#pragma	info(par)

#pragma	subtitle("   Entry Field Control   - Exception Handler Routine")
#pragma	page( )

/* --- EntryFieldExceptionHandler ---------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used process exceptions	under 2.x and to,	*/
/*     depending on the	exception, provide a method of performing	*/
/*     lazy pointer validation for the data types.			*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PEXCEPTIONREPORTRECORD	    pxcptrepr; = Report	Record		*/
/*     PEXCEPTIONREGISTRATIONRECORD pxcptregr; = Registration Record	*/
/*     PCONTEXTRECORD		    pcr;       = Context Record		*/
/*     PVOID			    sysinfo;   = System	Reserved	*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     EntryFieldExceptionHandler = Exception Handling Value		*/
/*									*/
/* --------------------------------------------------------------------	*/

APIRET APIENTRY	EntryFieldExceptionHandler(PEXCEPTIONREPORTRECORD pxcptrepr,
					   PEXCEPTIONREGISTRATIONRECORD	pxcptregr,
					   PCONTEXTRECORD pcr, PVOID sysinfo)

{
#pragma	info(nopar)

if ( (EH_EXIT_UNWIND & pxcptrepr->fHandlerFlags) ||
     (EH_UNWINDING   & pxcptrepr->fHandlerFlags) ||
     (EH_NESTED_CALL & pxcptrepr->fHandlerFlags) )
   return(XCPT_CONTINUE_SEARCH);

if ( pxcptrepr->ExceptionNum ==	XCPT_ACCESS_VIOLATION )
   longjmp((INT	*)jBuf,	1);

return(XCPT_CONTINUE_SEARCH);
}
#pragma	info(par)

#pragma	subtitle("   Entry Field Control - Delete Text Routine")
#pragma	page( )

/* --- DeleteText -------------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used delete the	selected range of text from	*/
/*     the entry field and to save the deleted text within the undo	*/
/*     buffer for later	retrieval if required.				*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	     = Entry Field Private Data			*/
/*     LONG	  iSelStart; = Deletion	Start Index			*/
/*     LONG	  iSelEnd;   = Deletion	End Index			*/
/*     BOOL	  fRedraw;   = Redraw Flag				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID DeleteText(PENTRYFWIN pefw,	LONG iSelStart,	LONG iSelEnd,
		       BOOL fRedraw)

{
register LONG n;		   /* Length Value			*/
EXCEPTIONREGISTRATIONRECORD xcptregr;	   /* Exception	Record		*/

		       /* Check	to make	sure that the selection	start	*/
		       /* is not after the end of the text		*/

if ( iSelStart < pefw->cText )
   {
		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

   xcptregr.prev_structure   = NULL;
   xcptregr.ExceptionHandler = &EntryFieldExceptionHandler;

   DosSetExceptionHandler(&xcptregr);

   if (	!setjmp(jBuf) )
       {
		       /* Check	to make	sure that the ending range for	*/
		       /* the text to be deleted does not exceed the	*/
		       /* current text in which	case make the ending	*/
		       /* selection value the text limits		*/

       if ( iSelEnd > pefw->cText )
	   iSelEnd = pefw->cText;

		       /* Copy the selected range of text to the undo	*/
		       /* buffer					*/

       memcpy(pefw->szUndo, &pefw->szText[iSelStart],
	      (UINT)(n = iSelEnd - iSelStart));
       pefw->szUndo[n] = 0;

		       /* Check	to see if the selection	was to the end	*/
		       /* of the text in which case NULL terminate the	*/
		       /* text string at the selection starting	point	*/

       if ( iSelEnd == pefw->cText )
	   pefw->szText[iSelStart] = 0;
       else
		       /* Move up the text that	was after the selection	*/

	   memmove(&pefw->szText[iSelStart], &pefw->szText[iSelEnd],
		   (UINT)(pefw->cText -	iSelEnd	+ 1));

		       /* Update the text length			*/
       pefw->cText -= n;
		       /* Set the new selection	point to the starting	*/
		       /* location where the previous selection	started	*/

       pefw->iSel = pefw->iSelStart = pefw->iSelEnd = pefw->iAnchor = iSelStart;

		       /* Get the presentation space handle for	the	*/
		       /* entry	field					*/

       pefw->hPS = WinGetPS(pefw->hWnd);

		       /* Get the new point array for the text string	*/

       QueryCharPoints(pefw);

       if ( pefw->cText	&& pefw->iOffset && (pefw->aptl[pefw->cText].x < pefw->rclText.xRight) )
	   {
	   pefw->iOffset = 0L;
	   if (	!fRedraw )
	       {
		       /* Deletion changes need	to be redrawn, place	*/
		       /* the colour table into	RGB mode and draw the	*/
		       /* revised text string within the entry field	*/
		       /* so as	to reflect the changes made		*/

	       GpiCreateLogColorTable(pefw->hPS, 0L, LCOLF_RGB,
				      0L, 0L, (PLONG)NULL);
	       DrawText(pefw);
	       }
	   }
       else
	   {
	   GpiCreateLogColorTable(pefw->hPS, 0L, LCOLF_RGB,
				  0L, 0L, (PLONG)NULL);
	   if (	pefw->fCursorShown )
	       WinShowCursor(pefw->hWnd, pefw->fCursorShown = FALSE);
	   WinFillRect(pefw->hPS, &pefw->rcl, pefw->lClrBackground);
	   }
		       /* Check	to see if the changes should be	redrawn	*/
       if ( fRedraw )
	   {
		       /* Deletion changes need	to be redrawn, place	*/
		       /* the colour table into	RGB mode and draw the	*/
		       /* revised text string within the entry field	*/
		       /* so as	to reflect the changes made		*/

	   GpiCreateLogColorTable(pefw->hPS, 0L, LCOLF_RGB,
				  0L, 0L, (PLONG)NULL);
	   DrawText(pefw);

		       /* Notify the owner that	changes	have occurred	*/

	   mrNotifyOwner(pefw, EN_CHANGE);
	   }
		       /* Release the presentation space handle		*/

       WinReleasePS(pefw->hPS);
       }
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

   DosUnsetExceptionHandler(&xcptregr);
   }
}
#pragma	subtitle("   Entry Field Control - Copy Text Routine")
#pragma	page( )

/* --- pszCopyText ------------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	copy the text selection	to the		*/
/*     supplied	buffer.							*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	     = Entry Field Private Data			*/
/*     PSZ	  pszBuffer; = Copy Buffer				*/
/*     LONG	  iSelStart; = Deletion	Start Index			*/
/*     LONG	  iSelEnd;   = Deletion	End Index			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     pszCopyText = Address of	pszBuffer				*/
/*									*/
/* --------------------------------------------------------------------	*/

static PSZ pszCopyText(PENTRYFWIN pefw,	PSZ pszBuffer, LONG iSelStart,
		       LONG iSelEnd)

{
register LONG i;		   /* Length Holder			*/
EXCEPTIONREGISTRATIONRECORD xcptregr;	   /* Exception	Record		*/

		       /* Check	to make	sure that the selection	start	*/
		       /* is not after the end of the text		*/

if ( iSelStart < pefw->cText )
   {
		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

   xcptregr.prev_structure   = NULL;
   xcptregr.ExceptionHandler = &EntryFieldExceptionHandler;

   DosSetExceptionHandler(&xcptregr);

   if (	!setjmp(jBuf) )
       {
		       /* Check	to make	sure that the ending range for	*/
		       /* the text to be deleted does not exceed the	*/
		       /* current text in which	case make the ending	*/
		       /* selection value the text limits		*/

       if ( iSelEnd > pefw->cText )
	   iSelEnd = pefw->cText;

		       /* Check	to make	sure that the start of the	*/
		       /* selection is not after the selection end	*/
		       /* which	would be incorrect			*/

       if ( iSelStart <	iSelEnd	)
	   {
		       /* Copy the text	from the internal text buffer	*/
		       /* to the supplied buffer			*/

	   memcpy(pszBuffer,
		  (PSZ)(pefw->flStyle &	ES_UNREADABLE ?
			&pefw->szPassword[iSelStart] :
			&pefw->szText[iSelStart]), (UINT)(i = iSelEnd -	iSelStart));
	   pszBuffer[i]	= 0;
	   }
       else
		       /* No text to copy, make	the supplied buffer	*/
		       /* reflect the condition				*/

	   pszBuffer[0]	= 0;
       }
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

   DosUnsetExceptionHandler(&xcptregr);
   }
return(pszBuffer);
}
#pragma	subtitle("   Entry Field Control - Paste Text Routine")
#pragma	page( )

/* --- PasteText --------------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used process exceptions	under 2.x and to,	*/
/*     depending on the	exception, provide a method of performing	*/
/*     lazy pointer validation for the data types.			*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;	      =	Entry Field Private Data		*/
/*     PSZ	  pszInsText; =	Copy Buffer				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID PasteText(PENTRYFWIN pefw, PSZ pszInsText, LONG cLen)

{
PSZ pszBuffer;			   /* Text Buffer Pointer		*/
EXCEPTIONREGISTRATIONRECORD xcptregr;	   /* Exception	Record		*/

		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

xcptregr.prev_structure	  = NULL;
xcptregr.ExceptionHandler = &EntryFieldExceptionHandler;

DosSetExceptionHandler(&xcptregr);

if ( !setjmp(jBuf) )
   {
		       /* Check	to see which text buffer should	be used	*/
		       /* since	the entry field	allows for unreadable	*/
		       /* display					*/

   if (	pefw->flStyle &	ES_UNREADABLE )
       pszBuffer = pefw->szPassword;
   else
       pszBuffer = pefw->szText;

		       /* Check	to see if the insertion	point is before	*/
		       /* the end of the string				*/

   if (	pefw->iSel < pefw->cText )
       {
		       /* Text being inserted within the entry field	*/
		       /* string, open up exactly the space required	*/
		       /* to allow the new text	to be inserted		*/

       memmove(&pszBuffer[pefw->iSel + cLen], &pszBuffer[pefw->iSel],
	       (UINT)((pefw->cText + cLen) - pefw->iSel));

		       /* Check	to see if a full string	is being	*/
		       /* inserted or just a character.	 In either	*/
		       /* case,	insert the text	appropriately.		*/
       if ( cLen > 1L )
	   memmove(&pszBuffer[pefw->iSel], pszInsText, (UINT)cLen);
       else
	   pszBuffer[pefw->iSel] = pszInsText[0];

		       /* Update the insert selection point along with	*/
		       /* the text length				*/

       pefw->iSel += cLen;
       pefw->cText += cLen;
       }
   else
       {
		       /* Text being concatonated to the end of	the	*/
		       /* entry	field string, check to see which	*/
		       /* concatonation	method should be used		*/
       if ( cLen > 1L )
	   strncat(pszBuffer, pszInsText, (UINT)cLen + 1);
       else
	   {
	   pszBuffer[pefw->iSel] = pszInsText[0];
	   pszBuffer[pefw->iSel	+ 1L] =	0;
	   }
		       /* Update the insert selection point along with	*/
		       /* the text length				*/

       pefw->iSel = pefw->cText	+= cLen;
       }
		       /* Update the selection indices			*/

   pefw->iSelEnd = pefw->iSelStart = pefw->iAnchor = pefw->iSel;

		       /* Insertion changes need to be redrawn,	place	*/
		       /* the colour table into	RGB mode and draw the	*/
		       /* revised text string within the entry field	*/
		       /* so as	to reflect the changes made		*/

   GpiCreateLogColorTable(pefw->hPS = WinGetPS(pefw->hWnd),
			  0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);

		       /* Calculate the	new display points for the text	*/
		       /* string					*/
   QueryCharPoints(pefw);

		       /* Check	to see if the new cursor position will	*/
		       /* be outside the range of the entry field which	*/
		       /* means	that the text will need	to be scrolled	*/
   if (	fScrollRight(pefw) )
       {
		       /* Check	to see if the scrolling	is within	*/
		       /* limits, making sure that the scrolled	text	*/
		       /* is properly shown				*/

       if ( (pefw->iOffset += pefw->cScroll) > pefw->cText )
	   pefw->iOffset = pefw->cText;
       DrawText(pefw);
		       /* Notify of the	owner of the entry field that	*/
		       /* the entry field was scrolled			*/

       mrNotifyOwner(pefw, EN_SCROLL);
       }
   else
		       /* No scrolling required, update	the text	*/
		       /* displayed within the entry field to reflect	*/
		       /* the text added				*/

       DrawTextString(pefw, 0L,	pefw->cText, FALSE);

		       /* Release the presentation space		*/
   WinReleasePS(pefw->hPS);

		       /* Inform the owner of the entry	field that a	*/
		       /* change in the	contents has occurred		*/

   mrNotifyOwner(pefw, EN_CHANGE);

		       /* Show the cursor in the new location to	*/
		       /* reflection the new insertion point		*/

   ShowCursor(pefw, pefw->iSel);
   }
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

DosUnsetExceptionHandler(&xcptregr);
}

/************************************************************************/
/************************************************************************/
/*									*/
/* Module Public Functions						*/
/*									*/
/************************************************************************/
/************************************************************************/

#pragma	subtitle("   Entry Field Control - Entry Field Registration Procedure")
#pragma	page( )

/* --- fRegisterEntryField ----------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	register the Entry Field for the	*/
/*     application with	PM.						*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HAB hAB;	= Anchor Block Handle					*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     fRegisterEntryField =  TRUE : Entry Field Registration		*/
/*				     Successfull			*/
/*			   = FALSE : Entry Field Registration Failed	*/
/*									*/
/* --------------------------------------------------------------------	*/

BOOL EXPENTRY fRegisterEntryField(HAB hAB)

{
		       /* Register the replace entry field class	*/

if ( WinRegisterClass(hAB, "EntryFieldWindow", (PFNWP)EntryWndProc,
		      CS_SYNCPAINT | CS_SIZEREDRAW | CS_PARENTCLIP | CS_CLIPCHILDREN,
		      USER_RESERVED) )
   return(TRUE);
else
   return(FALSE);
}
#pragma	subtitle("   Entry Field Control - Entry Field Window Procedure")
#pragma	page( )

/* --- EntryWndProc ------------------------------------ [ Public ] ---	*/
/*									*/
/*     This function is	used to	process	the messages for the entry	*/
/*     field control window.						*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HWND   hWnd; = Window Handle					*/
/*     ULONG  msg;  = PM Message					*/
/*     MPARAM mp1;  = Message Parameter	1				*/
/*     MPARAM mp2;  = Message Parameter	2				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     EntryWndProc = Message Handling Result				*/
/*									*/
/* --------------------------------------------------------------------	*/

MRESULT	EXPENTRY EntryWndProc(HWND hWnd, ULONG msg, MPARAM mp1,	MPARAM mp2)

{
HHEAPMEM      hHeap;		   /* Heap Handle			*/
HWND	      hwndHelp;		   /* Help Window Handle		*/
LONG	      iSelCurrent;	   /* Current Selection	Pointer		*/
PCHAR	      pchClipText;	   /* Clipboard	Text Pointer		*/
PCREATESTRUCT pcrst;		   /* Create Structure Pointer		*/
PENTRYFWIN    pefw;		   /* Entry Field Internal Data	Pointer	*/
POINTL	      ptl;		   /* Display Point			*/
PSWP	      pswp;		   /* Size Window Position Pointer	*/
PWNDPARAMS    pwprm;		   /* Window Parameters	Pointer		*/
RECTL	      rclPaint;		   /* Display Rectangle			*/
ULONG	      flStyle;		   /* Style Flags			*/
register LONG n;		   /* Length Counter			*/
EXCEPTIONREGISTRATIONRECORD xcptregr;	   /* Exception	Record		*/

switch ( msg )
   {

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	1: Control creation coding					*/
/*									*/
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Control creation							*/
   /*********************************************************************/

   case	WM_CREATE :
		       /* Allocate space for the internal control data	*/
		       /* as well as a small heap.  Save the address of	*/
		       /* the internal control data in the control's    */
		       /* reserved memory to allow it to be referenced	*/
		       /* as required by the control.			*/

       WinSetWindowPtr(hWnd, QUCWP_WNDP,
		       (PVOID)(pefw = (PENTRYFWIN)HeapMalloc(hHeap = HeapAlloc(4096UL, 4096UL),
							     sizeof(ENTRYFWIN))));
       pefw->hHeap = hHeap;

		       /* Get the control's creation structure address  */
		       /* to copy the relevant information such	as the	*/
		       /* size,	position and text of the control into	*/
		       /* the internal control data			*/

       pcrst = (PCREATESTRUCT)PVOIDFROMMP(mp2);

		       /* Save the owner and parent of the control so	*/
		       /* notification messages	can be sent back to	*/
		       /* the proper locations within the owning	*/
		       /* application					*/

       pefw->hAB	= WinQueryAnchorBlock(hWnd);
       pefw->hWnd	= hWnd;
       pefw->hwndOwner	= pcrst->hwndOwner;
       pefw->hwndParent	= pcrst->hwndParent;
       pefw->id		= pcrst->id;

       if ( pcrst->pszText )
	   memcpy(pefw->szText,	pcrst->pszText,
		  (UINT)((pefw->cText =	(LONG)strlen(pcrst->pszText)) +	1));

		       /* Save the size	of the Entry Field along with	*/
		       /* the current style				*/

       pefw->cx	     = pcrst->cx;
       pefw->cy	     = pcrst->cy;
       if ( (pefw->flStyle = pcrst->flStyle) & ES_READONLY )
	   pefw->fReadOnly = TRUE;

		       /* Get the default colours for the entry	field	*/

       pefw->lClrText		  = lGetPresParam(hWnd,	PP_FOREGROUNDCOLOR,
						  PP_FOREGROUNDCOLORINDEX,
						  SYSCLR_OUTPUTTEXT);
       pefw->lClrBorder		  = lGetPresParam(hWnd,	PP_BORDERCOLOR,
						  PP_BORDERCOLORINDEX,
						  SYSCLR_BUTTONDARK);
       pefw->lClrBackground	  = lGetPresParam(hWnd,	PP_BACKGROUNDCOLOR,
						  PP_BACKGROUNDCOLORINDEX,
						  SYSCLR_ENTRYFIELD);
       pefw->lClrHilite		  = lGetPresParam(hWnd,	PP_HILITEFOREGROUNDCOLOR,
						  PP_HILITEFOREGROUNDCOLORINDEX,
						  SYSCLR_HILITEFOREGROUND);
       pefw->lClrHiliteBackground = lGetPresParam(hWnd,	PP_HILITEBACKGROUNDCOLOR,
						  PP_HILITEBACKGROUNDCOLORINDEX,
						  SYSCLR_HILITEBACKGROUND);

		       /* Get the text pointer shape for use when the	*/
		       /* pointer is within the	confines of the	entry	*/
		       /* field	rectangle				*/

       pefw->hptr = WinQuerySysPointer(HWND_DESKTOP, SPTR_TEXT,	FALSE);

		       /* Get the font metrics to determine the	height	*/
		       /* and width of a character			*/

       if ( pefw->cText	)
	   pefw->iSel =	pefw->iSelStart	= pefw->iSelEnd	= pefw->iAnchor	= pefw->cText;
       pefw->hPS = WinGetPS(hWnd);
       RefreshMetrics(pefw);
       WinReleasePS(pefw->hPS);

		       /* Calculate the	scroll rates			*/

       pefw->ulScrollRateStart = WinQuerySysValue(HWND_DESKTOP,	SV_SCROLLRATE) * 4UL;
       pefw->ulScrollRate      = WinQuerySysValue(HWND_DESKTOP,	SV_SCROLLRATE) * 2UL;

		       /* Determine the	current	insertion mode		*/

       if ( !WinQuerySysValue(HWND_DESKTOP, SV_INSERTMODE) )
	   pefw->fOvertype = TRUE;

		       /* Decode the control data for the control	*/

       DecodeCtlData(pefw, mp1);
       break;

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	2: Control activation						*/
/*									*/
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Control being activated/deactivated				*/
   /*********************************************************************/

   case	WM_ACTIVATE :
       if ( (hwndHelp =	WinQueryHelpInstance(hWnd)) != (HWND)NULL )

		       /* When the window is being activated or		*/
		       /* deactivated, make sure that the control	*/
		       /* responds properly to help requests when it	*/
		       /* has the focus					*/

	   if (	SHORT1FROMMP(mp1) )
	       WinSendMsg(hwndHelp, HM_SET_ACTIVE_WINDOW,
			  MPFROMHWND(WinQueryWindow(hWnd, QW_PARENT)),
			  MPFROMHWND(WinQueryWindow(hWnd, QW_PARENT)));
	   else
	       WinSendMsg(hwndHelp, HM_SET_ACTIVE_WINDOW, 0L, 0L);
       break;

   /*********************************************************************/
   /*  Control receiving/losing	focus					*/
   /*********************************************************************/

   case	WM_SETFOCUS :
		       /* Get the Entry	Field information pointer from	*/
		       /* the reserved memory of the window		*/

       pefw = (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* When the window is being activated or		*/
		       /* deactivated, make sure that the control	*/
		       /* responds properly to help requests when it	*/
		       /* has the focus					*/

       if ( SHORT1FROMMP(mp2) )
	   {
		       /* Notify the owner of the Entry	Field that that	*/
		       /* Entry	Field is gaining focus			*/

	   mrNotifyOwner(pefw, EN_SETFOCUS);
	   if (	!pefw->fFocus )
	       {
		       /* Check	to see if the insert mode has changed	*/
		       /* since	the last time the control received	*/
		       /* focus						*/

	       if ( !WinQuerySysValue(HWND_DESKTOP, SV_INSERTMODE) )
		   pefw->fOvertype = TRUE;

		       /* Determine if some text has been selected	*/
		       /* within the entry field and if	the case add	*/
		       /* the highlighting				*/

	       if ( pefw->iSelStart != pefw->iSelEnd )
		   {
		   GpiCreateLogColorTable(pefw->hPS = WinGetPS(hWnd), 0L,
					  LCOLF_RGB, 0L, 0L, (PLONG)NULL);
		   DrawText(pefw);
		   WinReleasePS(pefw->hPS);
		   }
		       /* Create the cursor where the current insertion	*/
		       /* point	is					*/

	       WinCreateCursor(hWnd, pefw->xCursor, pefw->yCursor,
			       0L, pefw->cyCursor, CURSOR_SOLID	| CURSOR_FLASH,	NULL);
	       WinShowCursor(hWnd, pefw->fCursorShown =	TRUE);
	       }
		       /* Set the focus	flag				*/

	   pefw->fFocus	= TRUE;
	   }
       else
		       /* Focus	being lost, only update	internal flags	*/
		       /* if the control has received focus previously	*/

	   if (	pefw->fFocus )
	       {
		       /* Notify the owner of the entry	field that that	*/
		       /* entry	field is losing	focus			*/

	       mrNotifyOwner(pefw, EN_KILLFOCUS);

		       /* Destroy the cursor				*/

	       WinDestroyCursor(hWnd);

		       /* Clear	the focus flags				*/

	       pefw->fCursorShown = pefw->fFocus = FALSE;

		       /* Determine if some text has been selected	*/
		       /* within the entry field and if	the case remove	*/
		       /* the highlighting				*/

	       if ( (pefw->iSelStart !=	pefw->iSelEnd) || pefw->fOvertype )
		   {
		   GpiCreateLogColorTable(pefw->hPS = WinGetPS(hWnd), 0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
		   DrawText(pefw);
		   WinReleasePS(pefw->hPS);
		   }
	       }
       break;

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	3: Control sizing						*/
/*									*/
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Control changing	size or	position				*/
   /*********************************************************************/

   case	WM_ADJUSTWINDOWPOS :
       pswp = (PSWP)PVOIDFROMMP(mp1);
       if ( (pswp->fl &	SWP_SIZE) && (pswp->cx > 0) && (pswp->cx > 0) )

		       /* Get the address of the control info from the	*/
		       /* control's reserved memory and save the new    */
		       /* size of the control				*/
	   {
	   pefw	= (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
	   pefw->cx = pswp->cx;
	   pefw->cy = pswp->cy;
	   if (	(pswp->cx > 0L)	&& (pswp->cy > 0L) )
	       SizeEntryField(pefw);
	   }
       break;

   /*********************************************************************/
   /*  Convert position	query						*/
   /*********************************************************************/

   case	WM_QUERYCONVERTPOS :
       return(MRFROMLONG(QCP_NOCONVERT));

   /*********************************************************************/
   /*  Control changing	size						*/
   /*********************************************************************/

   case	WM_SIZE	:
		       /* Get the address of the control info from the	*/
		       /* control's reserved memory and save the new    */
		       /* size of the control				*/

       pefw = (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
       pefw->cx	= (LONG)(SHORT)SHORT1FROMMP(mp2);
       pefw->cy	= (LONG)(SHORT)SHORT2FROMMP(mp2);
       if ( ((LONG)(SHORT)SHORT1FROMMP(mp2) > 0L) &&
	    ((LONG)(SHORT)SHORT2FROMMP(mp2) > 0L) )
	   SizeEntryField(pefw);
       break;

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	3: Mouse interface						*/
/*									*/
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Button 1	click							*/
   /*********************************************************************/

   case	WM_BUTTON1DOWN :
		       /* Get the Entry	Field information pointer from	*/
		       /* the reserved memory of the window		*/

       pefw = (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Make sure that the entry field has focus	*/

       if ( !pefw->fFocus )
	   WinSetFocus(HWND_DESKTOP, hWnd);

		       /* Check	to see if the entry field has any text	*/
		       /* before trying	to interprete the mouse		*/
		       /* position					*/
       if ( pefw->cText	)
	   {
		       /* Get the mouse	hot-spot and check to see if it	*/
		       /* is within the	bounds of the entry field	*/

	   ptl.x = (LONG)SHORT1FROMMP(mp1);
	   ptl.y = (LONG)SHORT2FROMMP(mp1);
	   if (	WinPtInRect(pefw->hAB, &pefw->rcl, &ptl) )
	       {
		       /* Mouse	hot-spot within	the entry field, set	*/
		       /* the mouse movement flags			*/

	       pefw->fScrollStart = pefw->fCapture = pefw->fBtnDown = TRUE;

		       /* Locate the character where the mouse pointer	*/
		       /* has been clicked				*/

	       FINDCHARACTER((LONG)SHORT1FROMMP(mp1));

		       /* Check	to see if the extended selection using	*/
		       /* the shift key	is being performed		*/

	       if ( !((ULONG)WinGetKeyState(HWND_DESKTOP, VK_SHIFT) & 0x0008000UL) )
		   {
		       /* Normal selection being performed, save the	*/
		       /* location where the mouse was clicked as being	*/
		       /* the new selection point			*/

		   pefw->iSelStart = pefw->iSelEnd = pefw->iAnchor = pefw->iSel;

		       /* Get the presentation space for the entry	*/
		       /* field	and draw the changes made due to the	*/
		       /* button click					*/

		   GpiCreateLogColorTable(pefw->hPS = WinGetPS(hWnd), 0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
		   DrawText(pefw);
		   WinReleasePS(pefw->hPS);
		   }
	       else
		       /* Extended mouse selection being performed,	*/
		       /* select the text				*/

		   SelectText(pefw, pefw->iAnchor, pefw->iSel);

		       /* Capture the mouse input while	the mouse	*/
		       /* is pressed to	make sure that the button	*/
		       /* release is registered	even if	the mouse	*/
		       /* pointer is outside the confines of the entry	*/
		       /* field						*/

	       WinSetCapture(HWND_DESKTOP, hWnd);

		       /* Display the mouse cursor in the new location	*/
		       /* as reflected by the mouse pointer position	*/

	       if ( pefw->fCursorShown )
		   WinCreateCursor(hWnd, pefw->xCursor,	pefw->yCursor,
				   0L, 0L, CURSOR_SETPOS, NULL);
	       }
	   }
       return(MRFROMLONG(TRUE));

   /*********************************************************************/
   /*  Button 1	double click						*/
   /*********************************************************************/

   case	WM_BUTTON1DBLCLK :
		       /* Get the entry	field information pointer from	*/
		       /* the reserved memory of the window		*/

       pefw = (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
       return(MRFROMLONG(TRUE));

   /*********************************************************************/
   /*  Button 1	released						*/
   /*********************************************************************/

   case	WM_BUTTON1UP :
		       /* Get the entry	field information pointer from	*/
		       /* the reserved memory of the window		*/

       pefw = (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see if the mouse is being captured	*/
		       /* and if the case, release the capture		*/

       if ( pefw->fCapture && (WinQueryCapture(HWND_DESKTOP) ==	hWnd) )
	   {
	   WinSetCapture(HWND_DESKTOP, (HWND)NULL);
	   pefw->fCapture = FALSE;
	   }
		       /* Clear	the button down	flag now that the	*/
		       /* has been released				*/

       pefw->fBtnDown =	FALSE;

		       /* See if the scroll timer is active and	if the	*/
		       /* case,	stop it	since it is no longer required	*/

       if ( pefw->fTimer )
	   {
	   pefw->fTimer	= FALSE;
	   WinStopTimer(pefw->hAB, hWnd, TID_SCROLL);
	   }
		       /* Check	to see if in overtype mode in which	*/
		       /* case the overtype character needs to be	*/
		       /* displayed only if there is no	text selection	*/

       if ( pefw->fOvertype && (pefw->iSelStart	== pefw->iSelEnd) )
	   {
	   GpiCreateLogColorTable(pefw->hPS = WinGetPS(pefw->hWnd),
				  0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
	   DrawSubText(pefw, pefw->iSel, pefw->iSel + 1L);
	   WinReleasePS(pefw->hPS);
	   }
		       /* Show the cursor in the new location		*/

       ShowCursor(pefw,	pefw->iSel);
       return(MRFROMLONG(TRUE));

   /*********************************************************************/
   /*  Button 2	clicked							*/
   /*  Button 3	clicked							*/
   /*********************************************************************/

   case	WM_BUTTON2DOWN :
   case	WM_BUTTON2DBLCLK :
   case	WM_BUTTON3DOWN :
   case	WM_BUTTON3DBLCLK :
       WinSetActiveWindow(HWND_DESKTOP,	hWnd);
       break;

   /*********************************************************************/
   /* Process mouse movement over the entry field			*/
   /*********************************************************************/

   case	WM_MOUSEMOVE :
		       /* Get the entry	field information pointer from	*/
		       /* the reserved memory of the window		*/

       pefw = (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Send the WM_CONTROLPOINTER message to	the	*/
		       /* owner	of the control to allow	the owner of	*/
		       /* control to change the	pointer	shape from the	*/
		       /* current defined arrow	shape			*/

       if ( WinIsWindowEnabled(hWnd) )
	   WinSetPointer(HWND_DESKTOP,
			 (HPOINTER)WinSendMsg(pefw->hwndOwner,
					      WM_CONTROLPOINTER,
					      MPFROMLONG(pefw->id),
					      MPFROMLONG(pefw->hptr)));

		       /* Check	to see if the mouse left button	is	*/
		       /* depressed which means	the user is drag-	*/
		       /* selecting the	text within the	entry field	*/

       if ( pefw->fBtnDown )

		       /* Check	to see if any text contained within the	*/
		       /* entry	field.	When no	text in	the entry field	*/
		       /* the dragging should be ignored since there is	*/
		       /* nothing to select.				*/

	   if (	pefw->cText )
	       {
		       /* Check	to se if the mouse pointer is within	*/
		       /* the bounds of	the entry field	horizontally	*/
		       /* which	means the timer	scroll is no longer	*/
		       /* needed since the points can be mapped	with	*/
		       /* the current mouse pointer position		*/

	       if ( ((LONG)(SHORT)SHORT1FROMMP(mp1) >= pefw->rcl.xLeft)	&&
		    ((LONG)(SHORT)SHORT1FROMMP(mp1) <= pefw->rcl.xRight) )
		   {
		       /* Check	to see if the scroll timer is being	*/
		       /* used and if the case,	turn it	off		*/

		   if (	pefw->fTimer )
		       {
		       WinStopTimer(pefw->hAB, hWnd, TID_SCROLL);
		       pefw->fTimer = FALSE;
		       }
		   }
	       else
		       /* Mouse	pointer	is not within the horizontal	*/
		       /* bounds of the	entry field, turn on the scroll	*/
		       /* timer	only if	auto scrolling is specified for	*/
		       /* the control					*/

		   if (	pefw->flStyle &	ES_AUTOSCROLL )
		       {
		       /* Start	the timer off slowly for the start of	*/
		       /* scrolling and	faster if scrolling has	already	*/
		       /* be started					*/

		       WinStartTimer(pefw->hAB,	hWnd, TID_SCROLL,
				     pefw->fScrollStart	? pefw->ulScrollRateStart : pefw->ulScrollRate);
		       pefw->fTimer = TRUE;
		       }
		       /* Check	the current selected character against	*/
		       /* the recorded one and if different, reflect	*/
		       /* the selection	change and move	the cursor to	*/
		       /* the new location				*/

	       if ( (iSelCurrent = lFindCharacterIndex(pefw, (LONG)(SHORT)SHORT1FROMMP(mp1))) != pefw->iSel )
		   {
		   SelectText(pefw, pefw->iAnchor, iSelCurrent);
		   FINDCHARACTER((LONG)(SHORT)SHORT1FROMMP(mp1));
		   ShowCursor(pefw, pefw->iAnchor);
		   }
	       }
       return(MRFROMLONG(TRUE));

   /*********************************************************************/
   /* Process timer notification					*/
   /*********************************************************************/

   case	WM_TIMER :
		       /* Check	to make	sure that the timer is the	*/
		       /* scroll timer for the entry field when	mouse	*/
		       /* capture has been started during the drag	*/
		       /* operation of the mouse and the mouse pointer	*/
		       /* is outside the entry field display area	*/

       if ( SHORT1FROMMP(mp1) == TID_SCROLL )
	   {
		       /* Get the address of the control info from the	*/
		       /* control's reserved memory                     */

	   pefw	= (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Get the current mouse	pointer	position	*/

	   WinQueryPointerPos(HWND_DESKTOP, &ptl);

		       /* Convert the mouse pointer position, which is	*/
		       /* in desktop co-ordinates to the entry field	*/
		       /* co-ordinates					*/

	   WinMapWindowPoints(HWND_DESKTOP, hWnd, &ptl,	1L);

		       /* Check	to see if the mouse pointer is inside	*/
		       /* the display rectangle	in which case the timer	*/
		       /* should be stopped to allow the normal		*/
		       /* mouse	handling to be performed		*/

	   if (	WinPtInRect(pefw->hAB, &pefw->rcl, &ptl) )
	       {
	       pefw->fTimer = FALSE;
	       WinStopTimer(pefw->hAB, hWnd, TID_SCROLL);
	       }
	   else
	       {
		       /* Check	to see if this is the starting of the	*/
		       /* timer	scrolling in which case	the scrolling	*/
		       /* rate should be made faster			*/

	       if ( pefw->fScrollStart )
		   {
		   if (	pefw->fTimer )
		       {
		       WinStopTimer(pefw->hAB, hWnd, TID_SCROLL);
		       WinStartTimer(pefw->hAB,	hWnd, TID_SCROLL,
				     pefw->ulScrollRate);
		       }
		   pefw->fScrollStart =	FALSE;
		   }
		       /* Check	to see if the mouse pointer is to the	*/
		       /* right	of the entry field which means that	*/
		       /* scrolling is being performed from left to	*/
		       /* right						*/

	       if ( ptl.x > pefw->rcl.xRight )

		       /* Check	to see if there	is any text further to	*/
		       /* right	edge of	the entry field	that cannot be	*/
		       /* seen and should be scrolled			*/

		   if (	FALLOWSCROLLRIGHT(pefw)	)
		       {
		       /* Scrolling of text required, update the offset	*/
		       /* shift	factor making sure that	it does	not go	*/
		       /* beyond the end of the	text			*/

		       if ( (pefw->iOffset += pefw->cScroll) > pefw->cText )
			   pefw->iOffset = pefw->cText;

		       /* Update the selection end point making	sure	*/
		       /* that it does not go beyond the end of	the	*/
		       /* text						*/

		       if ( (pefw->iSelEnd += pefw->cScroll) > pefw->cText )
			   pefw->iSelEnd = pefw->cText;

		       /* Get the presentation space handle for	the	*/
		       /* entry	field and place	the colour table into	*/
		       /* RGB mode before drawing the selected text	*/

		       GpiCreateLogColorTable(pefw->hPS	= WinGetPS(hWnd),
					      0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
		       DrawText(pefw);
		       WinReleasePS(pefw->hPS);

		       /* Inform the owner of the entry	field that the	*/
		       /* contents have	been scrolled			*/

		       mrNotifyOwner(pefw, EN_SCROLL);

		       /* Move the cursor to the new location		*/

		       ShowCursor(pefw,	pefw->iAnchor);
		       }
		   else
		       /* No scrolling required	but check to make sure	*/
		       /* the end selection point is shifted		*/

		       if ( pefw->iSelEnd < pefw->cText	)
			   {

		       /* Update the end point for the newly selected	*/
		       /* text						*/

			   ++pefw->iSelEnd;

		       /* Get the presentation space handle for	the	*/
		       /* entry	field and place	the colour table into	*/
		       /* RGB mode before drawing the selected text	*/

			   GpiCreateLogColorTable(pefw->hPS = WinGetPS(hWnd),
						  0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
			   DrawText(pefw);
			   WinReleasePS(pefw->hPS);
			   ShowCursor(pefw, pefw->iAnchor);
			   }
		       else

		       /* No further selection necessary, but make sure	*/
		       /* that the selection point is at the end of the	*/
		       /* selection field				*/

			   pefw->iSel =	pefw->iSelEnd;
	       else
		       /* Mouse	is to the left of the entry field,	*/
		       /* scrolling is being performed from the	right	*/
		       /* to left.  Check to make sure that further	*/
		       /* scrolling can	be permitted to	the left.	*/

		   if (	pefw->iOffset )
		       {
		       /* Scrolling of text required, update the offset	*/
		       /* shift	factor making sure that	it does	not go	*/
		       /* beyond the start of the text			*/

		       if ( (pefw->iOffset -= pefw->cScroll) < 0 )
			   pefw->iOffset = 0L;

		       /* Update the selection end point making	sure	*/
		       /* that it does not go beyond the start of the	*/
		       /* text						*/

		       if ( (pefw->iSelStart -=	pefw->cScroll) < 0 )
			    pefw->iSelStart = 0L;

		       /* Get the presentation space handle for	the	*/
		       /* entry	field and place	the colour table into	*/
		       /* RGB mode before drawing the selected text	*/

		       GpiCreateLogColorTable(pefw->hPS	= WinGetPS(hWnd),
					      0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
		       DrawText(pefw);
		       WinReleasePS(pefw->hPS);

		       /* Inform the owner of the entry	field that the	*/
		       /* contents have	been scrolled			*/

		       mrNotifyOwner(pefw, EN_SCROLL);

		       /* Move the cursor to the new location		*/

		       ShowCursor(pefw,	pefw->iAnchor);
		       }
		   else
		       pefw->iSel = pefw->iSelStart;
	       }
	   }
       else
		       /* Check	to see if the timer is for the flashing	*/
		       /* of the cursor	in which case make sure	that	*/
		       /* the message gets through to the default	*/
		       /* window procedure otherwise it	will not flash	*/

	   if (	SHORT1FROMMP(mp1) == TID_CURSOR	)
	       return(WinDefWindowProc(hWnd, msg, mp1, mp2));
       break;

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	4: Keyboard interface						*/
/*									*/
/************************************************************************/
/************************************************************************/


/************************************************************************/
/************************************************************************/
/*									*/
/************************************************************************/
/************************************************************************/
/*     The keystrokes processed	by an entry field control are:		*/
/*									*/
/*     Left arrow	   Move	the cursor one character to the	left.	*/
/*     Right arrow	   Move	the cursor one character to the	right.	*/
/*     Shift+Left arrow	   Extend the selection	by one character to the	*/
/*			   left.					*/
/*     Shift+Right arrow   Extend the selection	by one character to the	*/
/*			   right.					*/
/*     Home		   Move	the cursor to the beginning of the text.*/
/*     End		   Move	the cursor to the end of the text.	*/
/*     Backspace	   Delete the character	to the left of the	*/
/*			   cursor.					*/
/*     Delete		   When	the selection is an insertion point,	*/
/*			   delete the character	to the right of	the	*/
/*			   cursor, otherwise delete the	current		*/
/*			   selection, but do not put it	in the		*/
/*			   clipboard.					*/
/*     Shift+Del	   Cut the current selection to	the clipboard.	*/
/*     Shift+Ins	   Replace the current selection with the text	*/
/*			   contents from the clipboard.			*/
/*     Ctrl+Del		   Delete to the end of	the field.		*/
/*     Ctrl+Ins		   Copy	the current selection to the clipboard.	*/
/*									*/
/*     If the control contains more text than can be shown, the		*/
/*     actions defined above that move the cursor cause	the text to be	*/
/*     scrolled. The amount of scrolling varies	from key to key, and	*/
/*     the position of the text	within the control varies for the same	*/
/*     cursor position.							*/
/************************************************************************/
/************************************************************************/
/*									*/
/************************************************************************/
/************************************************************************/


   /*********************************************************************/
   /*  Key being pressed						*/
   /*********************************************************************/

		       /* Keyboard character received			*/
   case	WM_CHAR	:
		       /* Only act on key down requests	therefore if a	*/
		       /* key up event,	ignore it			*/

       if ( CHARMSG(&msg)->fs &	KC_KEYUP )
	   break;
		       /* Check	for virtual key	presses	first		*/

       if ( CHARMSG(&msg)->fs &	KC_VIRTUALKEY )
	   switch ( CHARMSG(&msg)->vkey	)
	       {

	       /*********************************************************/
	       /*********************************************************/
	       /*      Home						*/
	       /*********************************************************/
	       /*********************************************************/

	       case VK_HOME :

		       /* Get the entry	field data pointer from	the	*/
		       /* window					*/

		   pefw	= (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see if an offset shift being	used	*/
		       /* and if there is, set it along	with the	*/
		       /* selections to	the first character in the text	*/
		       /* string and redraw the	text in	the entry field	*/

		   if (	pefw->iOffset )
		       {
		       pefw->iOffset = pefw->iSelStart = pefw->iSelEnd = pefw->iAnchor = pefw->iSel = 0L;

		       /* Get a	presentation space handle for the entry	*/
		       /* field, turn the colour table to RGB mode and	*/
		       /* redraw the entry field text			*/

		       GpiCreateLogColorTable(pefw->hPS	= WinGetPS(pefw->hWnd),
					      0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
		       DrawText(pefw);
		       WinReleasePS(pefw->hPS);
		       mrNotifyOwner(pefw, EN_SCROLL);
		       }
		   else
		       /* No offset shift being	used, but check	to see	*/
		       /* if any text has been selected	in which case	*/
		       /* it needs to be deselected and	the selection	*/
		       /* placed at the	first character	in the text	*/
		       /* string forcing the text to be	redrawn	as well	*/

		       if ( pefw->iSelStart != pefw->iSelEnd )
			   {
			   pefw->iOffset = pefw->iSelStart = pefw->iSelEnd = pefw->iAnchor = pefw->iSel	= 0L;

		       /* Get a	presentation space handle for the entry	*/
		       /* field, turn the colour table to RGB mode and	*/
		       /* redraw the entry field text			*/

			   GpiCreateLogColorTable(pefw->hPS = WinGetPS(pefw->hWnd),
						  0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
			   DrawText(pefw);
			   WinReleasePS(pefw->hPS);
			   }
		       else

		       /* No shift offset or text is selected, just	*/
		       /* make sure that the cursor is at the beginning	*/
		       /* of the entry field				*/

			   pefw->iSel =	0L;

		   ShowCursor(pefw, 0L);
		   return(MRFROMLONG(TRUE));

	       /*********************************************************/
	       /*********************************************************/
	       /*      End						*/
	       /*********************************************************/
	       /*********************************************************/

	       case VK_END :

		       /* Get the entry	field data pointer from	the	*/
		       /* window					*/

		   pefw	= (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see if the right part of the	text	*/
		       /* within the entry field is visible and	if it	*/
		       /* isn't, scroll the text to make it visible     */

		   if (	pefw->aptl[pefw->cText].x > pefw->rclText.xRight )
		       {
		       /* Set the cursor selection point to be just	*/
		       /* after	the last character in the text string	*/

		       pefw->iSelStart = pefw->iSelEnd = pefw->iAnchor = pefw->iSel = pefw->cText;

		       /* Determine the	offset value that should be	*/
		       /* used making sure that	it is within limits	*/

		       if ( (pefw->iOffset = pefw->cText - pefw->cScroll * 2L) < 0L )
			   pefw->iOffset = 0L;

		       /* Get a	presentation space handle for the entry	*/
		       /* field, turn the colour table to RGB mode and	*/
		       /* redraw the entry field text			*/

		       GpiCreateLogColorTable(pefw->hPS	= WinGetPS(pefw->hWnd),
					      0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
		       DrawText(pefw);
		       WinReleasePS(pefw->hPS);
		       mrNotifyOwner(pefw, EN_SCROLL);
		       }
		   else
		       if ( pefw->iSelStart != pefw->iSelEnd )
			   {
			   pefw->iSelStart = pefw->iSelEnd = pefw->iAnchor = pefw->iSel	= pefw->cText;
			   pefw->iOffset = 0L;

		       /* Get a	presentation space handle for the entry	*/
		       /* field, turn the colour table to RGB mode and	*/
		       /* redraw the entry field text			*/

			   GpiCreateLogColorTable(pefw->hPS = WinGetPS(pefw->hWnd),
						  0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
			   DrawText(pefw);
			   WinReleasePS(pefw->hPS);
			   }
		       else

		       /* No shift offset or text is selected, just	*/
		       /* make sure that the cursor is at the end of	*/
		       /* entry	field					*/

			   pefw->iSel =	pefw->cText;

		   ShowCursor(pefw, pefw->cText);
		   return(MRFROMLONG(TRUE));

	       /*********************************************************/
	       /*********************************************************/
	       /*      Backspace					*/
	       /*********************************************************/
	       /*********************************************************/

	       case VK_BACKSPACE :

		       /* Get the entry	field data pointer from	the	*/
		       /* window					*/

		   pefw	= (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see if the entry field is read-only	*/
		       /* which	means that any editing requests	have to	*/
		       /* be ignored					*/

		   if (	pefw->fReadOnly	)
		       return(MRFROMLONG(FALSE));
		   else
		       {
		       /* Check	to see if any text has been selected	*/
		       /* which	in this	case, the backspace operates	*/
		       /* exactly like the Del key, only the selected	*/
		       /* text is deleted				*/

		       if ( pefw->iSelStart != pefw->iSelEnd )
			   DeleteText(pefw, pefw->iSelStart, pefw->iSelEnd, TRUE);
		       else

		       /* No text has been selected, delete the		*/
		       /* character that reside	prior to the cursor	*/
		       /* location unless the cursor is	at the		*/
		       /* beginning of the text				*/

			   if (	pefw->iSel )
			       DeleteText(pefw,	pefw->iSel - 1,	pefw->iSel, TRUE);

		       ShowCursor(pefw,	pefw->iSel);
		       return(MRFROMLONG(TRUE));
		       }

	       /*********************************************************/
	       /*********************************************************/
	       /*      Del						*/
	       /*      Ctrl+Del						*/
	       /*      Shift+Del					*/
	       /*********************************************************/
	       /*********************************************************/

	       case VK_DELETE :

		       /* Get the entry	field data pointer from	the	*/
		       /* window					*/

		   pefw	= (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see if the entry field is read-only	*/
		       /* which	means that any editing requests	have to	*/
		       /* be ignored					*/

		   if (	pefw->fReadOnly	)
		       return(MRFROMLONG(FALSE));
		   else
		       {
		       /* Check	to see if Ctrl+Del is being pressed,	*/
		       /* which	means that the text from the cursor	*/
		       /* position to the end of the text string should	*/
		       /* be deleted					*/

		       if ( CHARMSG(&msg)->fs &	KC_CTRL	)
			   DeleteText(pefw, pefw->iSelStart, pefw->cText, TRUE);
		       else

		       /* Check	to see if the Shift+Del	is being	*/
		       /* pressed, which is the	convention for cutting	*/
		       /* text from the	entry field			*/

			   if (	(CHARMSG(&msg)->fs & KC_SHIFT) &&
				(pefw->iSelStart != pefw->iSelEnd) )
			       {

		       /* Allocate the shared memory for the text	*/
		       /* making sure that there is enough memory for	*/
		       /* the text plus	the NULL terminating byte	*/

			       DosAllocSharedMem((PPVOID)(PVOID)&pchClipText, NULL,
						 (ULONG)(pefw->iSelEnd -
							 pefw->iSelStart + 1),
						 fALLOCSHR);

		       /* Open and empty the clipboard before copying	*/
		       /* the selected text to the shared memory	*/
		       /* and placing the pointer on the clipboard	*/

			       WinOpenClipbrd(pefw->hAB);
			       WinEmptyClipbrd(pefw->hAB);
			       WinSetClipbrdData(pefw->hAB,
						 (ULONG)pszCopyText(pefw,
								    pchClipText,
								    pefw->iSelStart,
								    pefw->iSelEnd),
						 CF_TEXT, CFI_POINTER);

		       /* Close	the clipboard to allow the copied text	*/
		       /* to be	used by	other processes	as well	as this	*/
		       /* one						*/

			       WinCloseClipbrd(pefw->hAB);

		       /* Delete the selected text			*/

			       DeleteText(pefw,	pefw->iSelStart,
					  pefw->iSelEnd, TRUE);
			       }
			   else

		       /* Just the Del key is being pressed, check to	*/
		       /* see if any text is selected which is to be	*/
		       /* deleted					*/

			       if ( pefw->iSelStart != pefw->iSelEnd )
				   DeleteText(pefw, pefw->iSelStart,
					      pefw->iSelEnd, TRUE);
			       else

		       /* No text has been selected, just delete the	*/
		       /* character immediately	to the right of	the	*/
		       /* cursor					*/

				   DeleteText(pefw, pefw->iSel,
					      pefw->iSel + 1, TRUE);

		       ShowCursor(pefw,	pefw->iSel);
		       return(MRFROMLONG(TRUE));
		       }

	       /*********************************************************/
	       /*********************************************************/
	       /*      Left						*/
	       /*********************************************************/
	       /*********************************************************/

	       case VK_LEFT :

		       /* Get the entry	field data pointer from	the	*/
		       /* window					*/

		   pefw	= (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
		   if (	pefw->iSel )
		       {
		       if ( (pefw->aptl[--pefw->iSel].x	- pefw->aptl[pefw->iOffset].x) < EFMARGIN_LEFT )
			   {
			   if (	(pefw->iOffset -= pefw->cScroll) < 0L )
			       pefw->iOffset = 0L;

		       /* Get a	presentation space handle for the entry	*/
		       /* field, turn the colour table to RGB mode and	*/
		       /* redraw the entry field text			*/

			   GpiCreateLogColorTable(pefw->hPS = WinGetPS(pefw->hWnd),
						  0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
			   DrawText(pefw);
			   WinReleasePS(pefw->hPS);
			   mrNotifyOwner(pefw, EN_SCROLL);
			   }
		       else
			   if (	pefw->fOvertype	&& (pefw->iSelStart == pefw->iSelEnd) )
			       DrawOvertypeText(pefw, EFMOVE_LEFT);
		       ShowCursor(pefw,	pefw->iSel);
		       }
		   return(MRFROMLONG(TRUE));

	       /*********************************************************/
	       /*********************************************************/
	       /*      Right						*/
	       /*********************************************************/
	       /*********************************************************/

	       case VK_RIGHT :

		       /* Get the entry	field data pointer from	the	*/
		       /* window					*/

		   pefw	= (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
		   if (	pefw->iSel < pefw->cText )
		       {
		       if ( (pefw->aptl[++pefw->iSel].x	- pefw->aptl[pefw->iOffset].x) > pefw->rclText.xRight )
			   {
			   if (	FALLOWSCROLLRIGHT(pefw)	)
			       {
			       if ( (pefw->iOffset += pefw->cScroll) > pefw->cText )
				   pefw->iOffset = pefw->cText;

		       /* Get a	presentation space handle for the entry	*/
		       /* field, turn the colour table to RGB mode and	*/
		       /* redraw the entry field text			*/

			       GpiCreateLogColorTable(pefw->hPS	= WinGetPS(pefw->hWnd),
						      0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
			       DrawText(pefw);
			       WinReleasePS(pefw->hPS);
			       mrNotifyOwner(pefw, EN_SCROLL);
			       }
			   }
		       else
			   if (	pefw->fOvertype	&& (pefw->iSelStart == pefw->iSelEnd) )
			       DrawOvertypeText(pefw, EFMOVE_RIGHT);
		       ShowCursor(pefw,	pefw->iSel);
		       }
		   return(MRFROMLONG(TRUE));

	       /*********************************************************/
	       /*********************************************************/
	       /*      Ins						*/
	       /*      Ctrl+Ins						*/
	       /*      Shift+Ins					*/
	       /*********************************************************/
	       /*********************************************************/

	       case VK_INSERT :

		       /* Get the entry	field data pointer from	the	*/
		       /* window					*/

		   pefw	= (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to make	sure that a selection within	*/
		       /* the text has been made before	trying to copy	*/
		       /* the selection	to the clipboard		*/

		   if (	(CHARMSG(&msg)->fs & KC_CTRL) &&
			(pefw->iSelEnd != pefw->iSelStart) )
		       {
		       /* Check	to see if the entry field is unreadable	*/
		       /* which	means that it is being used for	some-	*/
		       /* thing	like a password	entry area.  In	this	*/
		       /* case do not allow the	text to	be copied to	*/
		       /* the clipboard.  (Note:  this differs from	*/
		       /* the OS/2 entry field in that it will allow	*/
		       /* you to copy the text to the clipboard)	*/

		       if ( pefw->flStyle & ES_UNREADABLE )
			   WinAlarm(HWND_DESKTOP, WA_ERROR);
		       else
			   {

		       /* Allocate the shared memory for the text	*/
		       /* making sure that there is enough memory for	*/
		       /* the text plus	the NULL terminating byte	*/

			   DosAllocSharedMem((PPVOID)(PVOID)&pchClipText, NULL,
					     (ULONG)(pefw->iSelEnd - pefw->iSelStart + 1), fALLOCSHR);

		       /* Open and empty the clipboard before copying	*/
		       /* the selected text to the shared memory	*/
		       /* and placing the pointer on the clipboard	*/

			   WinOpenClipbrd(pefw->hAB);
			   WinEmptyClipbrd(pefw->hAB);
			   WinSetClipbrdData(pefw->hAB,
					     (ULONG)pszCopyText(pefw, pchClipText, pefw->iSelStart, pefw->iSelEnd),
					     CF_TEXT, CFI_POINTER);

		       /* Close	the clipboard to allow the copied text	*/
		       /* to be	used by	other processes	as well	as this	*/
		       /* one						*/

			   WinCloseClipbrd(pefw->hAB);
			   }
		       }
		   else
		       /* Check	to see if the entry field is read-only	*/
		       /* which	means that any editing requests	have to	*/
		       /* be ignored					*/

		       if ( (CHARMSG(&msg)->fs & KC_SHIFT) && !pefw->fReadOnly )
			   {

		       /* Open the clipboard and then check to see if	*/
		       /* any text has been placed on it that can be	*/
		       /* pasted within	the entry field			*/

			   WinOpenClipbrd(pefw->hAB);
			   if (	WinQueryClipbrdData(pefw->hAB, CF_TEXT)	)

		       /* Text contained within	the clipboard, get the	*/
		       /* pointer to the text				*/

			       if ( (pchClipText = (PCHAR)WinQueryClipbrdData(pefw->hAB, CF_TEXT)) != NULL )

		       /* Check	to make	sure that the selection	within	*/
		       /* the clipboard	will fit within	the entry field	*/

				   if (	((n = (LONG)strlen(pchClipText)) + pefw->cText -
					 (pefw->iSelEnd	- pefw->iSelStart)) < pefw->cLimit )
				       {

		       /* Check	to see if any text is currently		*/
		       /* selected which means that the	text should be	*/
		       /* replaced by the text from the	clipboard.  If	*/
		       /* the case, delete the selected	text.		*/

				       if ( pefw->iSelStart != pefw->iSelEnd )
					   DeleteText(pefw, pefw->iSelStart, pefw->iSelEnd, FALSE);

		       /* Paste	the text into the entry	field at the	*/
		       /* current selection point			*/

				       PasteText(pefw, pchClipText, n);
				       }

		       /* Finished with	the clipboard, close it		*/

			   WinCloseClipbrd(pefw->hAB);
			   }
		       else
			   {
			   if (	pefw->fOvertype	)
			       {
			       pefw->fOvertype = FALSE;
			       if ( pefw->iSelStart == pefw->iSelEnd )
				   {
				   GpiCreateLogColorTable(pefw->hPS = WinGetPS(hWnd),
							  0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
				   DrawSubText(pefw, pefw->iSel, pefw->iSel + 1);
				   WinReleasePS(pefw->hPS);
				   }
			       }
			   else
			       {
			       pefw->fOvertype = TRUE;
			       if ( pefw->iSelStart == pefw->iSelEnd )
				   {
				   GpiCreateLogColorTable(pefw->hPS = WinGetPS(hWnd),
							  0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
				   DrawSubText(pefw, pefw->iSel, pefw->iSel + 1);
				   WinReleasePS(pefw->hPS);
				   }
			   mrNotifyOwner(pefw, EN_INSERTMODETOGGLE);
			   }
		       }
		   return(WinDefWindowProc(hWnd, msg, mp1, mp2));

	       /*********************************************************/
	       /*********************************************************/
	       /*      Down						*/
	       /*      Up						*/
	       /*********************************************************/
	       /*********************************************************/

	       case VK_DOWN :
	       case VK_UP :

		       /* Get the entry	field data pointer from	the	*/
		       /* window					*/

		   pefw	= (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Down or up arrow key pressed,	determine	*/
		       /* which	control	is the previous/next entry and	*/
		       /* set the focus	on that	control			*/

		   WinSetFocus(HWND_DESKTOP,
			       WinEnumDlgItem(pefw->hwndOwner, hWnd,
					      (ULONG)(CHARMSG(&msg)->vkey == VK_DOWN ?
						      EDI_NEXTGROUPITEM	:
						      EDI_PREVGROUPITEM)));
		   return(MRFROMLONG(TRUE));

	       /*********************************************************/
	       /*********************************************************/
	       /*      Tab						*/
	       /*      Shift+Tab					*/
	       /*********************************************************/
	       /*********************************************************/

	       case VK_TAB :
	       case VK_BACKTAB :

		       /* Get the entry	field data pointer from	the	*/
		       /* window					*/

		   pefw	= (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* TAB or Shift+TAB key pressed,	determine which	*/
		       /* control is the previous/next tab stop	and set	*/
		       /* the focus on that control			*/

		   WinSetFocus(HWND_DESKTOP,
			       WinEnumDlgItem(pefw->hwndOwner, hWnd,
					      (ULONG)(CHARMSG(&msg)->vkey == VK_TAB ?
						      EDI_NEXTTABITEM :
						      EDI_PREVTABITEM)));
		   return(MRFROMLONG(TRUE));
	       }
       else
		       /* Process normal character input		*/

	   if (	CHARMSG(&msg)->fs & KC_CHAR )
	       {
		       /* Get the entry	field data pointer from	the	*/
		       /* window					*/

	       pefw = (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see if the entry field is read-only	*/
		       /* which	means that any editing requests	have to	*/
		       /* be ignored					*/

	       if ( pefw->fReadOnly )
		   return(MRFROMLONG(FALSE));
	       else
		   {
		       /* Check	to see if the entry field input	limit	*/
		       /* has been met in which	case the keystroke	*/
		       /* should simply	be discarded if	the owner of	*/
		       /* control does not return a TRUE response to	*/
		       /* the overflow notification			*/

		   if (	pefw->cText >= pefw->cLimit )
		       if ( LONGFROMMR(mrNotifyOwner(pefw, EN_OVERFLOW)) == TRUE )
			   {
			   if (	pefw->cText >= pefw->cLimit )
			       {
			       WinAlarm(HWND_DESKTOP, WA_ERROR);
			       return(MRFROMLONG(TRUE));
			       }
			   }
		       else
			   {
			   WinAlarm(HWND_DESKTOP, WA_ERROR);
			   return(MRFROMLONG(TRUE));
			   }

		       /* Check	to see if any text has been selected	*/
		       /* which	means that the selected	text is		*/
		       /* to be	replaced with the keystroke character	*/

		   if (	pefw->iSelStart	!= pefw->iSelEnd )
		       DeleteText(pefw,	pefw->iSelStart, pefw->iSelEnd,
				  FALSE);
		   else
		       /* No preselected text, now check to see	if the	*/
		       /* entry	field is in overtype mode which	means	*/
		       /* that the character immediately to the	right	*/
		       /* of the cursor	is to be replaced with the	*/
		       /* keystroke character				*/

		       if ( pefw->fOvertype && (pefw->iSel < pefw->cText) )
			   DeleteText(pefw, pefw->iSel,	pefw->iSel + 1L,
				      FALSE);

		       /* Save the keystroke within the	text buffer	*/				
		       /* and then insert the text into	the entry	*/
		       /* field						*/

		   pefw->szInput[0] = (BYTE)CHARMSG(&msg)->chr;
		   PasteText(pefw, pefw->szInput, 1L);

		       /* Check	to see if the text limit has been	*/
		       /* reached in which case	the auto tab style	*/
		       /* should be checked for	and if present,	cause	*/
		       /* the focus to move from the entry field to the	*/
		       /* next tabable control				*/

		   if (	(pefw->flStyle & ES_AUTOTAB) &&
			(pefw->cText ==	pefw->cLimit) )
		       WinSetFocus(HWND_DESKTOP,
				   WinEnumDlgItem(pefw->hwndOwner, hWnd,
						  EDI_NEXTTABITEM));
		   return(MRFROMLONG(TRUE));
		   }
	       }
       return(WinDefWindowProc(hWnd, msg, mp1, mp2));

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	5: Control messages						*/
/*									*/
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Base messages							*/
   /*********************************************************************/

   case	EM_QUERYCHANGED	:
   case	EM_QUERYSEL :
   case	EM_SETSEL :
   case	EM_SETTEXTLIMIT	:
   case	EM_CUT :
   case	EM_COPY	:
   case	EM_CLEAR :
   case	EM_PASTE :
   case	EM_QUERYFIRSTCHAR :
   case	EM_SETFIRSTCHAR	:
   case	EM_QUERYREADONLY :
   case	EM_SETREADONLY :
   case	EM_SETINSERTMODE :
       mrBaseFieldHandler(hWnd,	msg, mp1, mp2);
       break;

   /*********************************************************************/
   /*  Extended	messages						*/
   /*********************************************************************/

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	6: Text	manipulation						*/
/*									*/
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Process window parameters query					*/
   /*********************************************************************/

   case	WM_QUERYWINDOWPARAMS :

		       /* Get the address for the windows parameters	*/
		       /* structure					*/

       if ( (pwprm = (PWNDPARAMS)PVOIDFROMMP(mp1)) != NULL )
	   {

		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

	   xcptregr.prev_structure   = NULL;
	   xcptregr.ExceptionHandler = &EntryFieldExceptionHandler;

	   DosSetExceptionHandler(&xcptregr);

	   if (	!setjmp(jBuf) )
	       {
		       /* Text length being asked for, get the address	*/
		       /* of the text string stored in the heap		*/

	       pefw = (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Determine the	type of	query			*/

	       if ( pwprm->fsStatus & WPM_TEXT )
		   {

		   if (	pefw->flStyle &	ES_UNREADABLE )
		       memcpy(pwprm->pszText, pefw->szPassword,	(UINT)pefw->cText);
		   else
		       memcpy(pwprm->pszText, pefw->szText, (UINT)pefw->cText);
		   pefw->fChanged = FALSE;
		   }

	       if ( pwprm->fsStatus & WPM_CCHTEXT )
		   {
		       /* Place	the length the string within the	*/
		       /* structure					*/

		   pwprm->cchText = (ULONG)pefw->cText;
		   pefw->fChanged = FALSE;
		   }

	       if ( pwprm->fsStatus & WPM_CBCTLDATA )
		   {

		       /* Set the control data length to zero		*/

		   pwprm->cbCtlData = 0;
		   pefw->fChanged = FALSE;
		   }
	       }
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	   DosUnsetExceptionHandler(&xcptregr);
	   return(MRFROMLONG(TRUE));
	   }
       break;
		       /* Process window parameters setting		*/

   case	WM_SETWINDOWPARAMS :

		       /* Get the address for the windows parameters	*/
		       /* structure					*/

       if ( (pwprm = (PWNDPARAMS)PVOIDFROMMP(mp1)) != NULL )
	   {

		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

	   xcptregr.prev_structure   = NULL;
	   xcptregr.ExceptionHandler = &EntryFieldExceptionHandler;

	   DosSetExceptionHandler(&xcptregr);

	   if (	!setjmp(jBuf) )
	       {

		       /* Check	to see if the text for the control is	*/
		       /* being	set					*/

	       if ( pwprm->fsStatus & WPM_TEXT )
		   {
		       /* Text being set, get the address of the text	*/
		       /* string stored	in the heap			*/

		   pefw	= (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see if any text is being set		*/

		   if (	pwprm->cchText )

		       /* Check	to make	sure that the text that	is to	*/
		       /* be set is not	greater	than the memory		*/
		       /* allocated					*/

		       if ( pwprm->cchText > 1023UL )
			   {
			   memcpy(pefw->szText,	pwprm->pszText,	1023);
			   pefw->szText[pefw->cText = 1023] = 0;
			   }
		       else
			   {
			   memcpy(pefw->szText,	pwprm->pszText,	pwprm->cchText);
			   pefw->szText[pefw->cText = (LONG)pwprm->cchText] = 0;
			   }
		   else
		       /* No text is being set,	clear any existing text	*/

		       pefw->szText[pefw->cText	= 0L] =	0;

		   GpiCreateLogColorTable(pefw->hPS = WinGetPS(hWnd),
					  0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
		   QueryCharPoints(pefw);
		   DrawText(pefw);
		   WinReleasePS(pefw->hPS);

		   mrNotifyOwner(pefw, EN_CHANGE);
		   }
	       }
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	   DosUnsetExceptionHandler(&xcptregr);
	   }
       break;

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	7: Painting and	display						*/
/*									*/
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  System colour/Presentation parameter change			*/
   /*********************************************************************/

   case	WM_SYSCOLORCHANGE :
   case	WM_PRESPARAMCHANGED :

		       /* Get the Entry	Field data pointer from	the window */

       pefw = (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Presentation parameters or system colours	*/
		       /* have changed,	get the	new values for the	*/
		       /* colours					*/

       pefw->lClrText		  = lGetPresParam(hWnd,	PP_FOREGROUNDCOLOR,
						  PP_FOREGROUNDCOLORINDEX,
						  SYSCLR_OUTPUTTEXT);
       pefw->lClrBorder		  = lGetPresParam(hWnd,	PP_BORDERCOLOR,
						  PP_BORDERCOLORINDEX,
						  SYSCLR_BUTTONDARK);
       pefw->lClrBackground	  = lGetPresParam(hWnd,	PP_BACKGROUNDCOLOR,
						  PP_BACKGROUNDCOLORINDEX,
						  SYSCLR_ENTRYFIELD);
       pefw->lClrHilite		  = lGetPresParam(hWnd,	PP_HILITEFOREGROUNDCOLOR,
						  PP_HILITEFOREGROUNDCOLORINDEX,
						  SYSCLR_HILITEFOREGROUND);
       pefw->lClrHiliteBackground = lGetPresParam(hWnd,	PP_HILITEBACKGROUNDCOLOR,
						  PP_HILITEBACKGROUNDCOLORINDEX,
						  SYSCLR_HILITEBACKGROUND);

		       /* Refresh the metrics for the control		*/

       pefw->hPS = WinGetPS(hWnd);
       RefreshMetrics(pefw);
       WinReleasePS(pefw->hPS);

		       /* Cause	the control to be repainted if it is	*/
		       /* visible					*/

       if ( WinIsWindowShowing(hWnd) )
	   WinInvalidateRect(hWnd, &pefw->rcl, FALSE);
       break;

   /*********************************************************************/
   /*  Paint control							*/
   /*********************************************************************/

   case	WM_PAINT :
		       /* Get the address of the control info from the	*/
		       /* control's reserved memory                     */

       pefw = (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Set the colour table to RGB mode		*/

       if ( (pefw->hPS = WinBeginPaint(hWnd, (HPS)NULL,	&rclPaint)) != (HPS)NULL )
	   {	
		       /* Check	to see if a style change has occurred	*/
		       /* and if the case, save	the new	style		*/
		       /* internally and refresh the control metrics	*/

	   if (	((flStyle = WinQueryWindowULong(hWnd, QWL_STYLE)) &
		  0x0000ffffUL)	!= (pefw->flStyle & 0x0000ffffUL) )
	       {
	       pefw->flStyle = flStyle;
	       RefreshMetrics(pefw);
	       }
		       /* Place	the colour table into RGB mode		*/

	   GpiCreateLogColorTable(pefw->hPS, 0L, LCOLF_RGB, 0L,	0L, (PLONG)NULL);

		       /* Erase	the rectangle for the control		*/

	   WinFillRect(pefw->hPS, &pefw->rcl, pefw->lClrBackground);

		       /* Check	to see if a margin for the entry field	*/
		       /* is desired and if so,	draw it			*/

	   if (	pefw->flStyle &	ES_MARGIN )
	       {
		       /* First	set the	colour of the border and then	*/
		       /* draw the inside edge of the border.  The	*/
		       /* border follows the typical 3D	effect of	*/
		       /* one line being dark and the second line being	*/
		       /* light.					*/

	       GpiSetColor(pefw->hPS, RGB_WHITE);
	       GpiMove(pefw->hPS, pefw->aptlInside);
	       GpiPolyLine(pefw->hPS, 4L, &pefw->aptlInside[1]);

		       /* Having drawn the inside edge,	now draw the	*/
		       /* outside edge of the border			*/

	       GpiSetColor(pefw->hPS, pefw->lClrBorder);
	       GpiMove(pefw->hPS, pefw->aptlOutside);
	       GpiPolyLine(pefw->hPS, 4L, &pefw->aptlOutside[1]);
	       }
		       /* Check	to see if any text needs to be drawn	*/
		       /* and if present, draw it			*/

	   if (	pefw->szText[0]	)
	       DrawText(pefw);

	   WinEndPaint(pefw->hPS);
	   }
       break;

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	8: Control destruction coding					*/
/*									*/
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Control being destroyed,	perform	clean-up			*/
   /*********************************************************************/

   case	WM_DESTROY :
		       /* Get the address of the control info from the	*/
		       /* control's reserved memory and release the     */
		       /* heap.						*/

       pefw = (PENTRYFWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
       HeapRelease(pefw->hHeap);
       break;
		       /* Default message processing			*/
   default :
       return(WinDefWindowProc(hWnd, msg, mp1, mp2));
   }
return(0L);
}
