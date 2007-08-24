#pragma	title("List Box Replacement  --  Version 1.1 -- (TraffMgr.C)")
#pragma	subtitle("   Traffic Manager - Interface Definitions")

/* Program name: Listbox.Dll	Title: A List Box Replacement		*/
/*									*/
/* OS/2	Developer Magazine, Issue:  May	'94, page 12                    */
/* Author:  Mark Benge	   IBM Corp.					*/
/*	    Matt Smith	   Prominare Inc.				*/
/* Description:	 Replacement for OS/2 List Box,	first of a series.	*/
/*									*/
/* Program Requirements:  OS/2 2.x					*/
/*			  IBM C	Set++					*/
/*			  WATCOM C 386/9.0				*/
/*			  Borland C++ for OS/2				*/
/*			  OS/2 Toolkit					*/

/* Copyright ¸ International Business Machines Corp. 1991-1994		*/
/* Copyright ¸ 1989-1994  Prominare Inc.  All Rights Reserved.		*/

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

#define	INCL_GPI		   /* Include OS/2 PM GPI Interface	*/
#define	INCL_WIN		   /* Include OS/2 PM Windows Interface	*/

#include <os2.h>
#include <stdlib.h>

#include "entryfld.h"
#include "listbox.h"

/* This	module contains	the routines that handle the traffic management	*/
/* for the list	box.							*/
/*									*/
/* Equivalent command line invocation of each module using the		*/
/* IBM C Set++ Compiler	Version	2.0 is:					*/
/*									*/
/*     Icc -G3e- -O+ -Rn -C -W3	-FoTraffMgr TraffMgr.C			*/

/* Filename:   TraffMgr.C						*/

/*  Version:   1.1							*/
/*  Created:   1993-10-14						*/
/*  Revised:   1994-04-02						*/

/* Routines:   LONG lGetPresParam(HWND hWnd, ULONG ulID1, ULONG	ulID2,	*/
/*				  LONG lDefault);			*/
/*	       VOID SizeListBox(PLISTBOXWIN plbw);			*/
/*	       VOID FocusChange(PLISTBOXWIN plbw, LONG iFocus,		*/
/*				BOOL fSet);				*/
/*	       VOID SetFocus(PLISTBOXWIN plbw, LONG iFocus);		*/
/*	       VOID RemoveFocus(PLISTBOXWIN plbw);			*/
/*	       BOOL fDecodeChkSelect(PLISTBOXWIN plbw, MPARAM mp1);	*/
/*	       BOOL fDecodeMouse(PLISTBOXWIN plbw, MPARAM mp1);		*/
/*	       BOOL fDecodeExtendedMouse(PLISTBOXWIN plbw, MPARAM mp1,	*/
/*					 BOOL fBlock);			*/
/*	       VOID MeasureItem(PLISTBOXWIN plbw, LONG lMaxBaselineExt);*/
/*	       BOOL fAddHorzScroll(HWND	hWnd, PLISTBOXWIN plbw);	*/
/*	       VOID LineUp(PLISTBOXWIN plbw);				*/
/*	       VOID LineDown(PLISTBOXWIN plbw);				*/
/*	       VOID DragUp(PLISTBOXWIN plbw);				*/
/*	       VOID DragDown(PLISTBOXWIN plbw);				*/


/* --------------------------------------------------------------------	*/

#pragma	subtitle("   Traffic Manager - Presentation Parameter Retrieve Procedure")
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

LONG lGetPresParam(HWND	hWnd, ULONG ulID1, ULONG ulID2,	LONG lDefault)

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
#pragma	subtitle("   Traffic Manager - List Box Sizing Procedure")
#pragma	page( )

/* --- SizeListBox ------------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	calculate the positions	of the		*/
/*     scroll bars that	are part of the	list box as well as the		*/
/*     border points that are to be drawn when the control is		*/
/*     painted.								*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Control Information	Pointer		*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID SizeListBox(PLISTBOXWIN plbw)

{
LONG  lMargin;			   /* Margin Size			*/
ULONG cswp;			   /* SWP Array	Count			*/

		       /* Initialize the values	for the	list box	*/
		       /* pertaining to	the number of items in the list	*/
		       /* box, display line count, etc.			*/

plbw->cLinesPage = (plbw->rcl.yTop - plbw->rcl.yBottom)	/ plbw->cyItem;
if ( (plbw->rcl.yTop - plbw->rcl.yBottom) % plbw->cyItem )
   ++plbw->cLinesPage;

if ( plbw->flStyle & LS_NOADJUSTPOS )
   plbw->cy = plbw->cyWindow;
else
   {
   lMargin = 2L	+ (plbw->flStyle & LS_HORZSCROLL ? plbw->cyScroll : 2L);
   plbw->cy = ((plbw->cyWindow - lMargin) / plbw->cyItem) * plbw->cyItem + lMargin;
   plbw->rclAdjusted.yTop    = plbw->cyWindow;
   plbw->rclAdjusted.yBottom = plbw->cy;
   plbw->rclAdjusted.xLeft   = 0L;
   plbw->rclAdjusted.xRight  = plbw->cxWindow;
   }
		       /* Calculate the	width of the group such	that it	*/
		       /* is almost the	width of the window and	also	*/
		       /* calculate the	width of the list box such that	*/
		       /* it is	almost the width of the	group box	*/

plbw->aswp[SWP_VERT].x	= (plbw->cx = plbw->cxWindow) -	plbw->cxScroll;
plbw->aswp[SWP_VERT].cy	= plbw->cy;

if ( plbw->flStyle & LS_HORZSCROLL )
   {
		       /*				   2		*/
		       /* 1ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄ¿		*/
		       /*  ³				    ³^³		*/
		       /*  ³				    ÃÄ´		*/
		       /*  ³				    ³ ³		*/
		       /*  ³				    ³ ³		*/
		       /*  ³				    ÃÄ´		*/
		       /*  ³				    ³v³		*/
		       /* 0ÃÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÅÄÙ		*/
		       /*  ³<³				  ³>³		*/
		       /*  ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÙ		*/
   plbw->cptl =	2L;
   plbw->aptlOutside[0].x = 0L;
   plbw->aptlOutside[0].y = 1L;
   plbw->aptlOutside[1].x = 0L;
   plbw->aptlOutside[1].y = plbw->cy - 1L;
   plbw->aptlOutside[2].x = plbw->aswp[SWP_VERT].x;
   plbw->aptlOutside[2].y = plbw->cy - 1L;

   plbw->aptlInside[0].x = 1L;
   plbw->aptlInside[0].y = 0L;
   plbw->aptlInside[1].x = 1L;
   plbw->aptlInside[1].y = plbw->cy - 2L;
   plbw->aptlInside[2].x = plbw->aswp[SWP_VERT].x;
   plbw->aptlInside[2].y = plbw->cy - 2L;

   plbw->rcl.yTop    = plbw->cy	- 2L;
   plbw->rcl.yBottom = plbw->aswp[SWP_HORZ].cy;
   plbw->rcl.xRight  = plbw->aswp[SWP_VERT].x;
   plbw->rcl.xLeft   = 2L;

   plbw->aswp[SWP_HORZ].cx = plbw->aswp[SWP_VERT].x;;
   plbw->aswp[SWP_VERT].y  = plbw->cyScroll;
   plbw->aswp[SWP_VERT].cy = plbw->cy -	plbw->cyScroll;
   cswp	= 2UL;
   plbw->rclCorner.xLeft   = plbw->aswp[SWP_VERT].x;
   plbw->rclCorner.xRight  = plbw->cx;
   plbw->rclCorner.yBottom = 0L;
   plbw->rclCorner.yTop	   = plbw->aswp[SWP_VERT].y;
   }
else
   {
		       /*  2				    3		*/
		       /* 1ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄ¿		*/
		       /*  ³				    ³^³		*/
		       /*  ³				    ÃÄ´		*/
		       /*  ³				    ³ ³		*/
		       /*  ³				    ³ ³		*/
		       /*  ³				    ÃÄ´		*/
		       /*  ³				    ³v³		*/
		       /*  ÀÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÙ		*/
		       /*  1				    0		*/
   plbw->cptl =	3L;
   plbw->aptlOutside[0].x = plbw->aswp[SWP_VERT].x;
   plbw->aptlOutside[0].y = 1L;
   plbw->aptlOutside[1].x = 0L;
   plbw->aptlOutside[1].y = 1L;
   plbw->aptlOutside[2].x = 0L;
   plbw->aptlOutside[2].y = plbw->cy - 1L;
   plbw->aptlOutside[3].x = plbw->aswp[SWP_VERT].x;
   plbw->aptlOutside[3].y = plbw->cy - 1L;

   plbw->aptlInside[0].x = plbw->aswp[SWP_VERT].x;
   plbw->aptlInside[0].y = 0L;
   plbw->aptlInside[1].x = 1L;
   plbw->aptlInside[1].y = 0L;
   plbw->aptlInside[2].x = 1L;
   plbw->aptlInside[2].y = plbw->cy - 2L;
   plbw->aptlInside[3].x = plbw->aswp[SWP_VERT].x;
   plbw->aptlInside[3].y = plbw->cy - 2L;

   plbw->rcl.yTop    = plbw->cy	- 2L;
   plbw->rcl.yBottom = 2L;
   plbw->rcl.xRight  = plbw->aswp[SWP_VERT].x;
   plbw->rcl.xLeft   = 2L;
   cswp	= 1UL;
   }
		       /* Window has been resized, calculate the new	*/
		       /* values for the number	of lines displayed and	*/
		       /* the relative number of characters displayed	*/

plbw->cLinesPage = (plbw->rcl.yTop - plbw->rcl.yBottom)	/ plbw->cyItem;
if ( (plbw->rcl.yTop - plbw->rcl.yBottom) % plbw->cyItem )
    ++plbw->cLinesPage;
plbw->cCharsPage = (plbw->rcl.xRight - plbw->rcl.xLeft)	/ plbw->xChar;
if ( (plbw->rcl.xRight - plbw->rcl.xLeft) % plbw->xChar	)
    ++plbw->cCharsPage;
		       /* Reposition and resize	the controls		*/

WinSetMultWindowPos(plbw->hAB, plbw->aswp, cswp);

}
#pragma	subtitle("   Traffic Manager - Focus Change Routine")
#pragma	page( )

/* --- FocusChange ------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	draw the focus indicator around	a	*/
/*     list box	item.							*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw;   = List Box Data Pointer			*/
/*     LONG	   iFocus; = Focus Item	Index				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID FocusChange(PLISTBOXWIN plbw, LONG	iFocus,	BOOL fSet)

{
if ( (iFocus < plbw->cItems) &&	plbw->fFocus )
   if (	fSet )
       {
		       /* Check	to see if the item receiving the focus	*/
		       /* is visible in	which case the focus indicator	*/
		       /* needs	to be drawn for	it			*/

       if ( fItemVisible(plbw, iFocus) )
	   {
	   WinCreateCursor(plbw->hWnd, plbw->rcl.xLeft,
			   (plbw->rcl.yTop - (iFocus - plbw->iVertScroll) *
					      plbw->cyItem) - plbw->cyItem,
			   0L, 0L, CURSOR_SETPOS, NULL);
	   if (	!plbw->fFocusShown )
	       WinShowCursor(plbw->hWnd, plbw->fFocusShown = TRUE);
	   }
       }
   else
		       /* Check	to see if the item losing the focus	*/
		       /* is visible in	which case the focus indicator	*/
		       /* needs	to be drawn for	it			*/

       if ( plbw->fFocusShown )
	   WinShowCursor(plbw->hWnd, plbw->fFocusShown = FALSE);
}
#pragma	subtitle("   Traffic Manager - Focus Draw Routine")
#pragma	page( )

/* --- SetFocus	---------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	draw the focus indicator around	a	*/
/*     list box	item.							*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw;   = List Box Data Pointer			*/
/*     LONG	   iFocus; = Focus Item	Index				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID SetFocus(PLISTBOXWIN plbw,	LONG iFocus)

{
if ( iFocus < plbw->cItems )
   {
		       /* Check	to see if the item receiving the focus	*/
		       /* is visible in	which case the focus indicator	*/
		       /* needs	to be drawn for	it			*/

   if (	WinIsWindowShowing(plbw->hWnd) )
       FocusChange(plbw, iFocus, TRUE);

   if (	!(plbw->plc[0].apli[iFocus]->fl	& LI_FOCUS) )
       {
		       /* Check	to see if a previous item had focus it	*/
		       /* which	if the case, the item should have its	*/
		       /* focus	flag cleared				*/

       if ( plbw->iFocus != LIT_NONE )
	   plbw->plc[0].apli[plbw->iFocus]->fl &= ~LI_FOCUS;

		       /* Set the focus	flag for the item now receiving	*/
		       /* the focus					*/

       plbw->plc[0].apli[plbw->iFocus =	iFocus]->fl |= LI_FOCUS;
       }
   }
}
#pragma	subtitle("   Traffic Manager - Focus Remove Routine")
#pragma	page( )

/* --- RemoveFocus ------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	draw the focus indicator around	a	*/
/*     list box	item.							*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Data Pointer			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID RemoveFocus(PLISTBOXWIN plbw)

{

if ( plbw->iFocus != LIT_NONE )
   {
   plbw->plc[0].apli[plbw->iFocus]->fl &= ~LI_FOCUS;
   FocusChange(plbw, plbw->iFocus, FALSE);
   }
}
#pragma	subtitle("   Traffic Manager - Mouse Position Decode Routine")
#pragma	page( )

/* --- fCreateEditArea --------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	determine if the current location of	*/
/*     the mouse within	the list box is	over a valid entry and to	*/
/*     create the direct editing entry field window.			*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Data Pointer			*/
/*     MPARAM	   mp1;	 = Message Parameter 1				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     fCreateEditArea =  TRUE : Edit Area Created			*/
/*		       = FALSE : Mouse Pointer outside Edit Area	*/
/*									*/
/* --------------------------------------------------------------------	*/

BOOL fCreateEditArea(PLISTBOXWIN plbw, MPARAM mp1)

{
POINTL ptl;			   /* Display Point			*/

ptl.x =	(LONG)SHORT1FROMMP(mp1);
ptl.y =	(LONG)SHORT2FROMMP(mp1);
if ( WinPtInRect(plbw->hAB, &plbw->rcl,	&ptl) )
   {
   if (	(plbw->iEditing	= lItemFromPoint(plbw, (LONG)SHORT2FROMMP(mp1))) != plbw->iFocus )
       RemoveFocus(plbw);

   if (	(plbw->hwndEdit	= WinCreateWindow(plbw->hWnd, CLASS_ENTRYFIELD,	plbw->plc[0].apli[plbw->iEditing]->pszText,
					  WS_VISIBLE | ES_LEFT | ES_AUTOSCROLL,
					  plbw->xTextOffset, plbw->rcl.yTop - ((plbw->iEditing - plbw->iVertScroll + 1)	* plbw->cyItem),
					  plbw->rcl.xRight - plbw->rcl.xLeft - plbw->xTextOffset, plbw->cyItem,
					  plbw->hWnd, HWND_TOP,	512UL,
					  (PVOID)NULL, (PVOID)NULL)) !=	(HWND)NULL )
       {
       WinSetFocus(HWND_DESKTOP, plbw->hwndEdit);
       return(TRUE);
       }
   else
       return(FALSE);
   }
return(FALSE);
}
#pragma	subtitle("   Traffic Manager - Mouse Position Decode Routine")
#pragma	page( )

/* --- fDecodeChkSelect	-------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	determine if the current location of	*/
/*     the mouse within	the list box is	over a valid entry and to	*/
/*     either select or	deselect the item.				*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Data Pointer			*/
/*     MPARAM	   mp1;	 = Message Parameter 1				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     fDecodeChkSelect	=  TRUE	: Mouse	Pointer	within List Area	*/
/*			= FALSE	: Mouse	Pointer	outside	List Area	*/
/*									*/
/* --------------------------------------------------------------------	*/

BOOL fDecodeChkSelect(PLISTBOXWIN plbw,	MPARAM mp1)

{
LONG   iSelected;		   /* Selected Item Index		*/
POINTL ptl;			   /* Display Point			*/

ptl.x =	(LONG)SHORT1FROMMP(mp1);
ptl.y =	(LONG)SHORT2FROMMP(mp1);
if ( WinPtInRect(plbw->hAB, &plbw->rcl,	&ptl) )
   {
   if (	(iSelected = lItemFromPoint(plbw, (LONG)SHORT2FROMMP(mp1))) != plbw->iFocus )
       RemoveFocus(plbw);

   if (	plbw->plc[0].apli[iSelected]->fl & LI_CHECKED )
       plbw->plc[0].apli[iSelected]->fl	&= ~LI_CHECKED;
   else
       plbw->plc[0].apli[iSelected]->fl	|= LI_CHECKED;

		       /* Draw the updated item				*/

   DrawSelection(plbw, iSelected);

   if (	!(plbw->plc[0].apli[iSelected]->fl & LI_FOCUS) )
       SetFocus(plbw, iSelected);

   mrNotifyOwner(plbw, LNX_CHECKED);

   return(TRUE);
   }
return(FALSE);
}
#pragma	subtitle("   Traffic Manager - Mouse Position Decode Routine")
#pragma	page( )

/* --- fDecodeMouse -----------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	determine if the current location of	*/
/*     the mouse within	the list box is	over a valid entry and to	*/
/*     either select or	deselect the item.				*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Data Pointer			*/
/*     MPARAM	   mp1;	 = Message Parameter 1				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     fDecodeMouse =  TRUE : Mouse Pointer within List	Area		*/
/*		    = FALSE : Mouse Pointer outside List Area		*/
/*									*/
/* --------------------------------------------------------------------	*/

BOOL fDecodeMouse(PLISTBOXWIN plbw, MPARAM mp1)

{
LONG   iSelected;		   /* Selected Item Index		*/
POINTL ptl;			   /* Display Point			*/

ptl.x =	(LONG)SHORT1FROMMP(mp1);
ptl.y =	(LONG)SHORT2FROMMP(mp1);
if ( WinPtInRect(plbw->hAB, &plbw->rcl,	&ptl) )
   {
   if (	(iSelected = lItemFromPoint(plbw, (LONG)SHORT2FROMMP(mp1))) != plbw->iFocus )
       RemoveFocus(plbw);

   if (	iSelected < plbw->cItems )
       if ( plbw->flStyle & LS_EXTENDEDSEL )
	   {
	   if (	plbw->fCapture && (iSelected ==	plbw->iFocus) )
	       return(TRUE);
	   else
	       if ( plbw->fCapture )
		   {
		   SelectExtendedItems(plbw, 0L, iSelected);
		   SetFocus(plbw, iSelected);
		   }
	       else
		   if (	!(plbw->plc[0].apli[iSelected]->fl & LI_SELECTED) &&
			fSelectItem(plbw, 0L, iSelected, TRUE) )
		       {
		       SetFocus(plbw, iSelected);
		       mrNotifyOwner(plbw, LN_SELECT);
		       }

	   if (	!(plbw->plc[0].apli[iSelected]->fl & LI_FOCUS) )
	       SetFocus(plbw, iSelected);

	   return(TRUE);
	   }
       else
	   if (	plbw->flStyle &	LS_MULTIPLESEL )
	       {
	       if ( plbw->fCapture && (iSelected == plbw->iFocus) )
		   return(TRUE);
	       else
		   if (	plbw->fCapture )
		       SetFocus(plbw, iSelected);
		   else
		       {
		       if ( fSelectItem(plbw, 0L, iSelected,
					(BOOL)(plbw->plc[0].apli[iSelected]->fl	& LI_SELECTED ?	FALSE :	TRUE)) )
			   {
			   SetFocus(plbw, iSelected);
			   mrNotifyOwner(plbw, LN_SELECT);
			   return(TRUE);
			   }
		       }
	       }
	   else
	       if ( (iSelected != plbw->iSelected) &&
		    fSelectItem(plbw, 0L, iSelected, TRUE) )
		   {
		   SetFocus(plbw, iSelected);
		   mrNotifyOwner(plbw, LN_SELECT);
		   return(TRUE);
		   }
   }
return(FALSE);
}
#pragma	subtitle("   Traffic Manager - Mouse Position Decode Routine")
#pragma	page( )

/* --- fDecodeExtendedMouse ---------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	determine if the current location of	*/
/*     the mouse within	the list box is	over a valid entry and to	*/
/*     either select or	deselect the item.				*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw;   = List Box Data Pointer			*/
/*     MPARAM	   mp1;	   = Message Parameter 1			*/
/*     BOOL	   fBlock; = Block Selected Flag			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     fDecodeExtendedMouse =  TRUE : Mouse Pointer within List	Area	*/
/*			    = FALSE : Mouse Pointer outside List Area	*/
/*									*/
/* --------------------------------------------------------------------	*/

BOOL fDecodeExtendedMouse(PLISTBOXWIN plbw, MPARAM mp1,	BOOL fBlock)

{
LONG   iSelected;		   /* Selected Item Index		*/
POINTL ptl;			   /* Display Point			*/

ptl.x =	(LONG)SHORT1FROMMP(mp1);
ptl.y =	(LONG)SHORT2FROMMP(mp1);
if ( WinPtInRect(plbw->hAB, &plbw->rcl,	&ptl) )
   {
   if (	(iSelected = lItemFromPoint(plbw, (LONG)SHORT2FROMMP(mp1))) != plbw->iFocus )
       RemoveFocus(plbw);

   if (	iSelected < plbw->cItems )
       if ( fBlock )
	   {
	   SelectExtendedItems(plbw, 0L, iSelected);
	   SetFocus(plbw, iSelected);
	   mrNotifyOwner(plbw, LN_SELECT);
	   return(TRUE);
	   }
       else
	   if (	fSelectMultipleItem(plbw, 0L, iSelected,
				    (BOOL)(plbw->plc[0].apli[iSelected]->fl & LI_SELECTED ? FALSE : TRUE)) )
	       {
	       if ( (plbw->iFocus == iSelected)	&& !(plbw->flStyle & LS_OWNERDRAW) )
		   plbw->iSelected = iSelected;
	       else
		   SetFocus(plbw, plbw->iSelected = iSelected);
	       mrNotifyOwner(plbw, LN_SELECT);
	       return(TRUE);
	       }
   }

return(FALSE);
}
#pragma	subtitle("   Traffic Manager - Item Measure Routine")
#pragma	page( )

/* --- MeasureItem ----------------------------------- [ Private ] --- */
/*									*/
/*     This function is	used to	determine if the item is within	the	*/
/*     visible portion of the list box.					*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw;	    = List Box Data Pointer		*/
/*     LONG	   lMaxBaselineExt; = Maximum Font Height		*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID MeasureItem(PLISTBOXWIN plbw, LONG	lMaxBaselineExt)

{
ULONG ul;			   /* Return Value			*/

if ( plbw->flStyle & LS_OWNERDRAW )
   {
   if (	(plbw->cyItem =	(LONG)SHORT1FROMMR((ul = (ULONG)WinSendMsg(plbw->hwndOwner,
								   WM_MEASUREITEM,
								   MPFROMLONG(plbw->id),
								   MPFROMLONG(0UL))))) < lMaxBaselineExt )
       plbw->cyItem  = lMaxBaselineExt;

   if (	plbw->flStyle &	LS_HORZSCROLL )
       plbw->cxItem = (LONG)SHORT2FROMMR(ul);
   }
else
   plbw->cyItem	 = lMaxBaselineExt;
}
#pragma	subtitle("   Traffic Manager - Horizontal Scroll Creation Routine")
#pragma	page( )

/* --- fAddHorzScroll ---------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	create the horizontal scroll bar for	*/
/*     the list	box.							*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HWND	   hWnd; = List	Box Window Handle			*/
/*     PLISTBOXWIN plbw; = List	Box Data Pointer			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     fAddHorzScroll =	 TRUE :	Scroll Bar Created			*/
/*		      =	FALSE :	Scroll Bar Creation Error		*/
/*									*/
/* --------------------------------------------------------------------	*/

BOOL fAddHorzScroll(HWND hWnd, PLISTBOXWIN plbw)

{
LONG cxScroll;			   /* Scroll Bar Width			*/

		       /* Calculate the	width of the scroll bar	and	*/
		       /* make sure that it is a valid size otherwise	*/
		       /* make its initial width zero			*/

if ( (cxScroll = plbw->cx - plbw->cxScroll) < 0L )
   cxScroll = 0L;

if ( (plbw->hwndScrollBottom = WinCreateWindow(hWnd, WC_SCROLLBAR, (PSZ)NULL,
					       SBS_HORZ	| WS_VISIBLE,
					       0L, 0L, cxScroll, plbw->cyScroll,
					       hWnd, HWND_TOP, 0x0000c002UL,
					       (PVOID)NULL, (PVOID)NULL)) != (HWND)NULL	)
    {
		       /* Sub-class the	scroll bar to allow the		*/
		       /* monitoring of	the button up events which will	*/
		       /* allow	the proper refocusing to the list box	*/

    WinSetWindowULong(plbw->hwndScrollBottom, QWL_USER,
		      (ULONG)WinSubclassWindow(plbw->hwndScrollBottom,
					       (PFNWP)ScrollBarWndProc));

    WinQueryWindowPos(plbw->hwndScrollBottom, &plbw->aswp[SWP_HORZ]);
    WinEnableWindow(plbw->hwndScrollBottom, FALSE);
    return(TRUE);
    }
else
    return(FALSE);
}
#pragma	subtitle("   Traffic Manager - Line Up Routine")
#pragma	page( )

/* --- LineUp -----------------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	move the focus one line	up within	*/
/*     the list	box.							*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Data Pointer			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID LineUp(PLISTBOXWIN	plbw)

{
		       /* Check	to see if the focus has	been placed on	*/
		       /* the list box and if it is visible remove the	*/
		       /* focus	indicator				*/
if ( plbw->iFocus > 0L )
   {
		       /* Check	to see if the list box is in single	*/
		       /* select mode which means that the item	which	*/
		       /* contains the focus also is the current item	*/
		       /* selected					*/

   if (	(plbw->flStyle & (LS_MULTIPLESEL | LS_EXTENDEDSEL)) == 0UL )
       {
       SaveSelectState(0L, plbw->iFocus);
       plbw->plc[0].apli[plbw->iFocus]->fl &= ~(LI_FOCUS | LI_SELECTED);
       if ( fItemVisible(plbw, plbw->iFocus) )
	   {
	   FocusChange(plbw, plbw->iFocus, FALSE);
	   DrawItemSelection(plbw, plbw->iFocus);
	   }
		       /* Update the selected item index		*/

       --plbw->iSelected;

		       /* Check	to see if the new position calculated	*/
		       /* will be before the current list top display	*/
		       /* item,	in which case, the list	will need to be	*/
		       /* shifted upwards and the list redisplayed	*/

       if ( --plbw->iFocus < plbw->iVertScroll )
	   {
		       /* Set the new scroll bar position and force the	*/
		       /* repainting of	the output window		*/

	   mrNotifyOwner(plbw, LN_SCROLL);
	   --plbw->iVertScroll;
	   WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
		      MPFROMSHORT(plbw->iVertScroll = max(0, min(plbw->iVertScroll, plbw->cVertScroll))), 0L);
	   WinScrollWindow(plbw->hWnd, 0L, -plbw->cyItem,
			   (PRECTL)&plbw->rcl, (PRECTL)&plbw->rcl,
			   (HRGN)NULL, (PRECTL)NULL, SW_INVALIDATERGN);
	   WinUpdateWindow(plbw->hWnd);
	   }

       SaveSelectState(0L, plbw->iFocus);
       plbw->plc[0].apli[plbw->iFocus]->fl |= (LI_FOCUS	| LI_SELECTED);
       DrawItemSelection(plbw, plbw->iFocus);
       FocusChange(plbw, plbw->iFocus, TRUE);
       mrNotifyOwner(plbw, LN_SELECT);
       }
   else
       {
       RemoveFocus(plbw);

		       /* Check	to see if the new position calculated	*/
		       /* will be before the current list top display	*/
		       /* item,	in which case, the list	will need to be	*/
		       /* shifted upwards and the list redisplayed	*/

       if ( --plbw->iFocus < plbw->iVertScroll )
	   {
		       /* Set the new scroll bar position and force the	*/
		       /* repainting of	the output window		*/

	   mrNotifyOwner(plbw, LN_SCROLL);
	   --plbw->iVertScroll;
	   WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
		      MPFROMSHORT(plbw->iVertScroll = max(0, min(plbw->iVertScroll, plbw->cVertScroll))), 0L);
	   WinScrollWindow(plbw->hWnd, 0L, -plbw->cyItem,
			   (PRECTL)&plbw->rcl, (PRECTL)&plbw->rcl,
			   (HRGN)NULL, (PRECTL)NULL, SW_INVALIDATERGN);
	   WinUpdateWindow(plbw->hWnd);
	   }
       SetFocus(plbw, plbw->iFocus);
       mrNotifyOwner(plbw, LN_SELECT);
       }
   }
}
#pragma	subtitle("   Traffic Manager - Line Down Routine")
#pragma	page( )

/* --- LineDown	---------------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	move the focus one line	down within	*/
/*     the list	box.							*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Data Pointer			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID LineDown(PLISTBOXWIN plbw)

{
		       /* Check	to see if the focus has	been placed on	*/
		       /* the list box and if it is visible remove the	*/
		       /* focus	indicator				*/

if ( (plbw->iFocus != LIT_NONE)	&& (plbw->iFocus < (plbw->cItems - 1L))	)
   if (	(plbw->flStyle & (LS_MULTIPLESEL | LS_EXTENDEDSEL)) == 0UL )
       {
		       /* Remove the focus from	the selected item	*/

       SaveSelectState(0L, plbw->iFocus);
       FocusChange(plbw, plbw->iFocus, FALSE);
       plbw->plc[0].apli[plbw->iFocus]->fl &= ~(LI_FOCUS | LI_SELECTED);
       if ( fItemVisible(plbw, plbw->iFocus) &&	(plbw->iSelected != LIT_NONE) )
	   DrawItemSelection(plbw, plbw->iSelected);

		       /* Check	to see if the new position calculated	*/
		       /* will be after	the list bottom	display	item,	*/
		       /* in which case, the list will need to be	*/
		       /* shifted downwards and	the list redisplayed	*/

       if ( (plbw->iSelected = ++plbw->iFocus) >= (plbw->iVertScroll + plbw->cLinesPage) )
	   {
		       /* Set the new scroll bar position and force the	*/
		       /* repainting of	the output window		*/

	   mrNotifyOwner(plbw, LN_SCROLL);
	   ++plbw->iVertScroll;
	   WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
		      MPFROMSHORT(plbw->iVertScroll = max(0, min(plbw->iVertScroll, plbw->cVertScroll))), 0L);
	   WinScrollWindow(plbw->hWnd, 0L, plbw->cyItem,
			   (PRECTL)&plbw->rcl, (PRECTL)&plbw->rcl,
			   (HRGN)NULL, (PRECTL)NULL, SW_INVALIDATERGN);
	   WinUpdateWindow(plbw->hWnd);
	   }

       SaveSelectState(0L, plbw->iFocus);
       plbw->plc[0].apli[plbw->iFocus]->fl |= (LI_FOCUS	| LI_SELECTED);
       FocusChange(plbw, plbw->iFocus, TRUE);
       DrawItemSelection(plbw, plbw->iFocus);
       mrNotifyOwner(plbw, LN_SELECT);
       }
   else
       {
       RemoveFocus(plbw);
		       /* Check	to see if the list box is in single	*/
		       /* select mode which means that the item	which	*/
		       /* contains the focus also is the current item	*/
		       /* selected					*/

       if ( (plbw->flStyle & LS_MULTIPLESEL) ==	0UL )
	   plbw->iSelected = ++plbw->iFocus;
       else
	   ++plbw->iFocus;

		       /* Check	to see if the new position calculated	*/
		       /* will be after	the list bottom	display	item,	*/
		       /* in which case, the list will need to be	*/
		       /* shifted downwards and	the list redisplayed	*/

       if ( plbw->iFocus >= (plbw->iVertScroll + plbw->cLinesPage) )
	   {
		       /* Set the new scroll bar position and force the	*/
		       /* repainting of	the output window		*/

	   mrNotifyOwner(plbw, LN_SCROLL);
	   ++plbw->iVertScroll;
	   WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
		      MPFROMSHORT(plbw->iVertScroll = max(0, min(plbw->iVertScroll, plbw->cVertScroll))), 0L);
	   WinScrollWindow(plbw->hWnd, 0L, plbw->cyItem,
			   (PRECTL)&plbw->rcl, (PRECTL)&plbw->rcl,
			   (HRGN)NULL, (PRECTL)NULL, SW_INVALIDATERGN);
	   WinUpdateWindow(plbw->hWnd);
	   }
       mrNotifyOwner(plbw, LN_SELECT);
       SetFocus(plbw, plbw->iFocus);
       }
}
#pragma	subtitle("   Traffic Manager - Drag Up Routine")
#pragma	page( )

/* --- LineUp -----------------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	move the focus one line	up within	*/
/*     the list	box.							*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Data Pointer			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID DragUp(PLISTBOXWIN	plbw)

{
		       /* Check	to see if the focus has	been placed on	*/
		       /* the list box and if it is visible remove the	*/
		       /* focus	indicator				*/
if ( plbw->iFocus > 0L )
   {
		       /* Check	to see if the list box is in single	*/
		       /* select mode which means that the item	which	*/
		       /* contains the focus also is the current item	*/
		       /* selected					*/

   if (	(plbw->flStyle & (LS_MULTIPLESEL | LS_EXTENDEDSEL)) == 0UL )
       {
       SaveSelectState(0L, plbw->iFocus);
       FocusChange(plbw, plbw->iFocus, FALSE);
       plbw->plc[0].apli[plbw->iFocus]->fl &= ~(LI_FOCUS | LI_SELECTED);
       if ( fItemVisible(plbw, plbw->iFocus) )
	   DrawItemSelection(plbw, plbw->iFocus);

		       /* Check	to see if the new position calculated	*/
		       /* will be before the current list top display	*/
		       /* item,	in which case, the list	will need to be	*/
		       /* shifted upwards and the list redisplayed	*/

       if ( (plbw->iSelected = --plbw->iFocus) < plbw->iVertScroll )
	   {
	   SaveSelectState(0L, plbw->iFocus);
	   plbw->plc[0].apli[plbw->iFocus]->fl |= (LI_FOCUS | LI_SELECTED);

		       /* Set the new scroll bar position and force the	*/
		       /* repainting of	the output window		*/

	   mrNotifyOwner(plbw, LN_SCROLL);
	   --plbw->iVertScroll;
	   FocusChange(plbw, plbw->iFocus, TRUE);
	   WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
		      MPFROMSHORT(plbw->iVertScroll = max(0, min(plbw->iVertScroll, plbw->cVertScroll))), 0L);
	   WinScrollWindow(plbw->hWnd, 0L, -plbw->cyItem,
			   (PRECTL)&plbw->rcl, (PRECTL)&plbw->rcl,
			   (HRGN)NULL, (PRECTL)NULL, SW_INVALIDATERGN);
	   WinUpdateWindow(plbw->hWnd);
	   }
       else
	   {
	   SaveSelectState(0L, plbw->iFocus);
	   plbw->plc[0].apli[plbw->iFocus]->fl |= (LI_FOCUS | LI_SELECTED);
	   FocusChange(plbw, plbw->iFocus, TRUE);
	   DrawItemSelection(plbw, plbw->iFocus);
	   }
       mrNotifyOwner(plbw, LN_SELECT);
       }
   else
       {
       RemoveFocus(plbw);

		       /* Check	to see if the drag is in an area where	*/
		       /* the current focus item will be deselected	*/

       if ( plbw->iFocus > plbw->iAnchor )
	   {
		       /* In an	area where the drag direction is	*/
		       /* deselecting items, deselect the current item	*/
		       /* and redraw it					*/

	   SaveSelectState(0L, plbw->iFocus);
	   plbw->plc[0].apli[plbw->iFocus]->fl &= ~LI_SELECTED;
	   DrawItemSelection(plbw, plbw->iFocus);
	   }
		       /* Check	to see if the new position calculated	*/
		       /* will be before the current list top display	*/
		       /* item,	in which case, the list	will need to be	*/
		       /* shifted upwards and the list redisplayed	*/

       if ( --plbw->iFocus < plbw->iVertScroll )
	   {
	   SaveSelectState(0L, plbw->iFocus);
	   plbw->plc[0].apli[plbw->iFocus]->fl |= LI_SELECTED;

		       /* Set the new scroll bar position and force the	*/
		       /* repainting of	the output window		*/

	   mrNotifyOwner(plbw, LN_SCROLL);
	   --plbw->iVertScroll;
	   WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
		      MPFROMSHORT(plbw->iVertScroll = max(0, min(plbw->iVertScroll, plbw->cVertScroll))), 0L);
	   WinScrollWindow(plbw->hWnd, 0L, -plbw->cyItem,
			   (PRECTL)&plbw->rcl, (PRECTL)&plbw->rcl,
			   (HRGN)NULL, (PRECTL)NULL, SW_INVALIDATERGN);
	   WinUpdateWindow(plbw->hWnd);
	   }
       else
	   {
	   SaveSelectState(0L, plbw->iFocus);
	   plbw->plc[0].apli[plbw->iFocus]->fl |= LI_SELECTED;
	   }

       mrNotifyOwner(plbw, LN_SELECT);
       SetFocus(plbw, plbw->iFocus);
       }
   }
}
#pragma	subtitle("   Traffic Manager - Drag Down Routine")
#pragma	page( )

/* --- DragDown	---------------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	move the focus one line	down within	*/
/*     the list	box.							*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Data Pointer			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID DragDown(PLISTBOXWIN plbw)

{
		       /* Check	to see if the focus has	been placed on	*/
		       /* the list box and if it is visible remove the	*/
		       /* focus	indicator				*/

if ( (plbw->iFocus != LIT_NONE)	&& (plbw->iFocus < (plbw->cItems - 1L))	)
   if (	(plbw->flStyle & (LS_MULTIPLESEL | LS_EXTENDEDSEL)) == 0UL )
       {
       SaveSelectState(0L, plbw->iFocus);
       plbw->plc[0].apli[plbw->iFocus]->fl &= ~(LI_FOCUS | LI_SELECTED);
       if ( fItemVisible(plbw, plbw->iFocus) )
	   DrawItemSelection(plbw, plbw->iFocus);

		       /* Check	to see if the new position calculated	*/
		       /* will be after	the list bottom	display	item,	*/
		       /* in which case, the list will need to be	*/
		       /* shifted downwards and	the list redisplayed	*/

       if ( (plbw->iSelected = ++plbw->iFocus) >= (plbw->iVertScroll + plbw->cLinesPage) )
	   {
	   SaveSelectState(0L, plbw->iFocus);
	   plbw->plc[0].apli[plbw->iFocus]->fl |= (LI_FOCUS | LI_SELECTED);
	   FocusChange(plbw, plbw->iFocus, TRUE);

		       /* Set the new scroll bar position and force the	*/
		       /* repainting of	the output window		*/

	   mrNotifyOwner(plbw, LN_SCROLL);
	   ++plbw->iVertScroll;
	   WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
		      MPFROMSHORT(plbw->iVertScroll = max(0, min(plbw->iVertScroll, plbw->cVertScroll))), 0L);
	   WinScrollWindow(plbw->hWnd, 0L, plbw->cyItem,
			   (PRECTL)&plbw->rcl, (PRECTL)&plbw->rcl,
			   (HRGN)NULL, (PRECTL)NULL, SW_INVALIDATERGN);
	   WinUpdateWindow(plbw->hWnd);
	   }
       else
	   {
	   SaveSelectState(0L, plbw->iFocus);
	   plbw->plc[0].apli[plbw->iFocus]->fl |= (LI_FOCUS | LI_SELECTED);
	   FocusChange(plbw, plbw->iFocus, TRUE);
	   DrawItemSelection(plbw, plbw->iFocus);
	   }
       mrNotifyOwner(plbw, LN_SELECT);
       }
   else
       {
       RemoveFocus(plbw);

		       /* Check	to see if the drag is in an area where	*/
		       /* the current focus item will be deselected	*/

       if ( plbw->iFocus < plbw->iAnchor )
	   {
		       /* In an	area where the drag direction is	*/
		       /* deselecting items, deselect the current item	*/
		       /* and redraw it					*/

	   SaveSelectState(0L, plbw->iFocus);
	   plbw->plc[0].apli[plbw->iFocus]->fl &= ~LI_SELECTED;
	   DrawItemSelection(plbw, plbw->iFocus);
	   }
		       /* Check	to see if the list box is in single	*/
		       /* select mode which means that the item	which	*/
		       /* contains the focus also is the current item	*/
		       /* selected					*/

       if ( (plbw->flStyle & LS_MULTIPLESEL) ==	0UL )
	   plbw->iSelected = ++plbw->iFocus;
       else
	   ++plbw->iFocus;

		       /* Check	to see if the new position calculated	*/
		       /* will be after	the list bottom	display	item,	*/
		       /* in which case, the list will need to be	*/
		       /* shifted downwards and	the list redisplayed	*/

       if ( plbw->iFocus >= (plbw->iVertScroll + plbw->cLinesPage) )
	   {
	   SaveSelectState(0L, plbw->iFocus);
	   plbw->plc[0].apli[plbw->iFocus]->fl |= LI_SELECTED;

		       /* Set the new scroll bar position and force the	*/
		       /* repainting of	the output window		*/

	   mrNotifyOwner(plbw, LN_SCROLL);
	   ++plbw->iVertScroll;
	   WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
		      MPFROMSHORT(plbw->iVertScroll = max(0, min(plbw->iVertScroll, plbw->cVertScroll))), 0L);
	   WinScrollWindow(plbw->hWnd, 0L, plbw->cyItem,
			   (PRECTL)&plbw->rcl, (PRECTL)&plbw->rcl,
			   (HRGN)NULL, (PRECTL)NULL, SW_INVALIDATERGN);
	   WinUpdateWindow(plbw->hWnd);
	   }
       else
	   {
	   SaveSelectState(0L, plbw->iFocus);
	   plbw->plc[0].apli[plbw->iFocus]->fl |= LI_SELECTED;
	   }

       mrNotifyOwner(plbw, LN_SELECT);
       SetFocus(plbw, plbw->iFocus);
       }
}
