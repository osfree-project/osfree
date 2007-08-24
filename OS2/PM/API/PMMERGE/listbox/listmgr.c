#pragma	title("List Box Replacement  --  Version 1.1 -- (ListMgr.C)")
#pragma	subtitle("   List Manager - Interface Definitions")

/* Program name: Listbox.Dll  Title: A List Box	Replacement		*/
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

#define	INCL_DOS		   /* Include OS/2 DOS Kernal		*/
#define	INCL_GPI		   /* Include OS/2 PM GPI Interface	*/
#define	INCL_WIN		   /* Include OS/2 PM Windows Interface	*/

#include <os2.h>
#include <setjmp.h>
#include <string.h>

#include "listbox.h"

/* This	module contains	the routines that handle the list management	*/
/* for the list	box.							*/
/*									*/
/* Equivalent command line invocation of each module using the		*/
/* IBM C Set++ Compiler	Version	2.0 is:					*/
/*									*/
/*     Icc -G3e- -O+ -Rn -C -W3	-FoListMgr ListMgr.C			*/

/* Filename:   ListMgr.C						*/

/*  Version:   1.1							*/
/*  Created:   1993-10-14						*/
/*  Revised:   1994-07-03						*/

/* Routines:   static VOID SortAscending(PLISTITEM *pli, INT sLeft,	*/
/*					 INT sRight);			*/
/*	       static VOID SortDescending(PLISTITEM *pli, INT sLeft,	*/
/*					  INT sRight);			*/
/*	       static INT strnicmp(const CHAR *psz1, const CHAR	*psz2,	*/
/*				   UINT	n);				*/
/*	       static VOID RefreshList(PLISTBOXWIN plbw, LONG iItem);	*/
/*	       INT stricmp(const CHAR *psz1, const CHAR	*psz2);		*/
/*	       LONG lSetItemText(HHEAPMEM hHeap, HWND hWnd,		*/
/*				 PLISTITEM pli,	PSZ pszText);		*/
/*	       VOID SaveExtendedState(PLISTBOXWIN plbw);		*/
/*	       VOID RestoreExtendedState(PLISTBOXWIN plbw);		*/
/*	       BOOL fSelectMultipleItem(PLISTBOXWIN plbw, LONG iCol,	*/
/*					LONG iSelected,	BOOL fSelect);	*/
/*	       BOOL fSelectItem(PLISTBOXWIN plbw, LONG iCol,		*/
/*				LONG iSelected,	BOOL fSelect);		*/
/*	       VOID SelectExtendedItems(PLISTBOXWIN plbw, LONG iCol,	*/
/*					LONG iSelected);		*/
/*	       VOID SetControlDataList(PLISTBOXWIN plbw, ULONG cItems,	*/
/*				       PBYTE pb);			*/
/*	       MRESULT EXPENTRY	mrBaseListHandler(HWND hWnd, ULONG msg,	*/
/*						  MPARAM mp1,		*/
/*						  MPARAM mp2);		*/
/*	       MRESULT EXPENTRY	mrExtendedListHandler(HWND hWnd,	*/
/*						      ULONG msg,	*/
/*						      MPARAM mp1,	*/
/*						      MPARAM mp2);	*/


/* --------------------------------------------------------------------	*/

/* Standard List Box messages						*/
/*									*/
/*     LM_QUERYITEMCOUNT	  0x0160				*/
/*     LM_INSERTITEM		  0x0161				*/
/*     LM_SETTOPINDEX		  0x0162				*/
/*     LM_DELETEITEM		  0x0163				*/
/*     LM_SELECTITEM		  0x0164				*/
/*     LM_QUERYSELECTION	  0x0165				*/
/*     LM_SETITEMTEXT		  0x0166				*/
/*     LM_QUERYITEMTEXTLENGTH	  0x0167				*/
/*     LM_QUERYITEMTEXT		  0x0168				*/
/*     LM_SETITEMHANDLE		  0x0169				*/
/*     LM_QUERYITEMHANDLE	  0x016a				*/
/*     LM_SEARCHSTRING		  0x016b				*/
/*     LM_SETITEMHEIGHT		  0x016c				*/
/*     LM_QUERYTOPINDEX		  0x016d				*/
/*     LM_DELETEALL		  0x016e				*/

jmp_buf	jBuf;			   /* Jump Buffer			*/

/************************************************************************/
/*									*/
/* Module Prototype Definitions						*/
/*									*/
/************************************************************************/

static VOID SortAscending(PLISTITEM *pli, INT sLeft, INT sRight);
static VOID SortDescending(PLISTITEM *pli, INT sLeft, INT sRight);
static VOID RefreshList(PLISTBOXWIN plbw, LONG iItem);
APIRET APIENTRY	ListBoxExceptionHandler(PEXCEPTIONREPORTRECORD pxcptrepr,
					PEXCEPTIONREGISTRATIONRECORD pxcptregr,
					PCONTEXTRECORD pcr, PVOID sysinfo);

/************************************************************************/
/************************************************************************/
/*									*/
/* Module Private Functions						*/
/*									*/
/************************************************************************/
/************************************************************************/

#pragma	subtitle("   List Manager - Ascending Sort Routine")
#pragma	page( )

/* --- SortAscending ----------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	sort the list box entries in ascending	*/
/*     order.  The routine implemented is the "Quick Sort" algorithm.	*/
/*     The function uses recursion to perform the sort.	 All sorts of	*/
/*     of the list box are case	insensitive.				*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTITEM pli;	 = List	Box Item Array Pointer			*/
/*     INT	 sLeft;	 = Low Index Value				*/
/*     INT	 sRight; = High	Index Value				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID SortAscending(PLISTITEM *pli, INT sLeft, INT sRight)

{
PSZ	  pszText;		   /* String Pointer			*/
PLISTITEM pliSwap;		   /* List Item	Holder			*/
register INT i;			   /* Low Index	Counter			*/
register INT n;			   /* High Index Counter		*/

		       /* Determine target pointer and retrieve	string	*/
		       /* address					*/

pszText	= pli[((i = sLeft) + (n	= sRight)) / 2]->pszText;

do
   {
   while ( (stricmp(pli[i]->pszText, pszText) <	0) && (i < sRight) )
       ++i;
   while ( (stricmp(pli[n]->pszText, pszText) >	0) && (n > sLeft) )
       --n;

   if (	i <= n )
       {
       pliSwap = pli[i];
       pli[i++]	= pli[n];
       pli[n--]	= pliSwap;
       }
   } while ( i <= n );

if ( sLeft < n )
   SortAscending(pli, sLeft, n);

if ( i < sRight	)
   SortAscending(pli, i, sRight);
}
#pragma	subtitle("   List Manager - Descending Sort Routine")
#pragma	page( )

/* --- SortDescending ---------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	sort the list box entries in descending	*/
/*     order.  The routine implemented is the "Quick Sort" algorithm.	*/
/*     The function uses recursion to perform the sort.	 All sorts of	*/
/*     of the list box are case	insensitive.				*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTITEM pli;	 = List	Box Item Array Pointer			*/
/*     INT	 sLeft;	 = Low Index Value				*/
/*     INT	 sRight; = High	Index Value				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID SortDescending(PLISTITEM *pli, INT sLeft, INT sRight)

{
PLISTITEM pliSwap;		   /* List Item	Holder			*/
PSZ	  pszText;		   /* String Pointer			*/
register INT i;			   /* Low Index	Counter			*/
register INT n;			   /* High Index Counter		*/

		       /* Determine target pointer and retrieve	string	*/
		       /* address					*/

pszText	= pli[((i = sLeft) + (n	= sRight)) / 2]->pszText;

do
   {
   while ( (stricmp(pli[i]->pszText, pszText) >	0) && (i < sRight) )
       ++i;
   while ( (stricmp(pli[n]->pszText, pszText) <	0) && (n > sLeft) )
       --n;

   if (	i <= n )
       {
       pliSwap = pli[i];
       pli[i++]	= pli[n];
       pli[n--]	= pliSwap;
       }
   } while ( i <= n );

if ( sLeft < n )
   SortDescending(pli, sLeft, n);

if ( i < sRight	)
   SortDescending(pli, i, sRight);
}

#if defined(__IBMC__) || defined(__IBMCPP__)

#pragma	subtitle("   List Manager - String Case Insensitive Compare Function")
#pragma	page( )

/* --- stricmp ----------------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	compare	two strings for	equality being	*/
/*     insensitive to case.  This function is defined here since the	*/
/*     IBM C Set/2 and C Set++ compilers have decided in their		*/
/*     collective wisdom to not	supply this function within the		*/
/*     subsystem support libraries!!!					*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PSZ psz1; = String 1						*/
/*     PSZ psz2; = String 2						*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     stricmp = -1 : String 1	< String 2				*/
/*	       =  0 : String 1 == String 2				*/
/*	       =  1 : String 1	> String 2				*/
/*									*/
/* --------------------------------------------------------------------	*/

INT stricmp(const CHAR *psz1, const CHAR *psz2)

{
while (	*psz1 && *psz2 && (*psz1 & 223)	== (*psz2 & 223) )
   {
   ++psz1;
   ++psz2;
   }

return((*psz1 &	223) - (*psz2 &	223));
}
#pragma	subtitle("   List Manager - String Case Insensitive Compare Function")
#pragma	page( )

/* --- strnicmp	---------------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	compare	two strings for	equality being	*/
/*     insensitive to case.  This function is defined here since the	*/
/*     IBM C Set/2 and C Set++ compilers have decided in their		*/
/*     collective wisdom to not	supply this function within the		*/
/*     subsystem support libraries!!!					*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PSZ  psz1; = String 1						*/
/*     PSZ  psz2; = String 2						*/
/*     UINT n;	  = Number of Characters to Compare			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     strnicmp	= -1 : String 1	 < String 2				*/
/*		=  0 : String 1	== String 2				*/
/*		=  1 : String 1	 > String 2				*/
/*									*/
/* --------------------------------------------------------------------	*/

static INT strnicmp(const CHAR *psz1, const CHAR *psz2,	UINT n)

{
while (	n-- && *psz1 &&	*psz2 && (*psz1	& 223) == (*psz2 & 223)	)
   {
   ++psz1;
   ++psz2;
   }

return((*psz1 &	223) - (*psz2 &	223));
}

#endif

#pragma	subtitle("   List Manager - Refresh List Determination Function")
#pragma	page( )

/* --- fRefreshList -----------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	determine if the list box needs	to be	*/
/*     refreshed after an item has been	inserted in it.			*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Internal Data Pointer		*/
/*     LONG	  iItem; = Inserted Item Index				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID RefreshList(PLISTBOXWIN plbw, LONG iItem)

{
RECTL rcl;			   /* Display Rectangle			*/

if ( WinIsWindowShowing(plbw->hWnd) )
   {
   if (	iItem <	(plbw->iVertScroll + plbw->cLinesPage) )
       if ( iItem < plbw->iVertScroll )
	   WinInvalidateRect(plbw->hWnd, &plbw->rcl, FALSE);
       else
	   {
	   rcl = plbw->rcl;
	   rcl.yTop = plbw->rcl.yTop - (iItem -	plbw->iVertScroll) * plbw->cyItem;
	   if (	plbw->cItems < plbw->cLinesPage	)
	       rcl.yBottom = plbw->rcl.yTop - plbw->cItems * plbw->cyItem;
	   WinInvalidateRect(plbw->hWnd, &rcl, FALSE);
	   }
   UpdateScrollBars(plbw);
   }
else
   plbw->fDirty	= TRUE;
}

/************************************************************************/
/************************************************************************/
/*									*/
/* Module Public Functions						*/
/*									*/
/************************************************************************/
/************************************************************************/

#pragma	subtitle("   List Manager - Descending Insert Item Locate Routine")
#pragma	page( )

/* --- lSetItemText -----------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	allocate the memory for	the text of	*/
/*     the item	being added or replaced.  The routine also calculates	*/
/*     the width of the	text to	allow the proper updating of the	*/
/*     horizontal scroll bar.						*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HHEAPMEM	 hHeap;	     = Heap Handle				*/
/*     HWND	 hWnd;	     = List Box	Window Handle			*/
/*     PLISTITEM pli;	     = List Box	Item Array Pointer		*/
/*     PSZ	 pszText;    = Inserted	Item Text			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     lSetItemText = Item Width in Pixels				*/
/*									*/
/* --------------------------------------------------------------------	*/

LONG lSetItemText(HHEAPMEM hHeap, HWND hWnd, PLISTITEM pli, PSZ	pszText)

{
HPS    hPS;			   /* Presentation Space Handle		*/
POINTL rgptl[TXTBOX_COUNT];	   /* Text Box Point Array		*/

		       /* Check	to see that the	text for the item is	*/
		       /* not a	NULL pointer				*/
if ( pszText )
    {
		       /* Allocate space for the text of the item and	*/
		       /* copy the text	to the holder			*/
    pli->cText = strlen(pszText);
    memcpy(pli->pszText	= (PSZ)HeapMalloc(hHeap, pli->cText + 1),
	   pszText, pli->cText);

		       /* Get the presentation space for the list box	*/

   if (	(hPS = WinGetPS(hWnd)) != (HPS)NULL )
       {
		       /* Get the display rectangle for	the text	*/

       GpiQueryTextBox(hPS, (LONG)pli->cText, pszText, 5L, rgptl);

		       /* Release the presentation space		*/
       WinReleasePS(hPS);

		       /* Calculate the	width of the text		*/

       pli->cxItem = rgptl[TXTBOX_CONCAT].x - rgptl[TXTBOX_BOTTOMLEFT].x + 2L;
       }
   else
       pli->cxItem = 0L;
    }
else
   {
		       /* Empty	item being inserted create an empty	*/
		       /* entry						*/

    pli->pszText = (PSZ)HeapMalloc(hHeap, 1UL);
    pli->cxItem	= 0L;
    }

return(pli->cxItem);
}
#pragma	subtitle("   List Manager - Extended State Save Routine")
#pragma	page( )

/* --- SaveExtendedState ------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	save the current extended selection	*/
/*     state so	as to allow for	it to be restored upon an undo		*/
/*     request.								*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Internal Data Pointer		*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID SaveExtendedState(PLISTBOXWIN plbw)

{
register INT i;			   /* Loop Counter			*/

		       /* Loop through the items within	the list and	*/
		       /* save the current flags within	the list	*/

for ( i	= 0; i < plbw->cItems; i++ )
   plbw->plc[0].apli[i]->fl = (plbw->plc[0].apli[i]->fl	& 0x0000ffffUL)	<< 16UL	|
			     (plbw->plc[0].apli[i]->fl & 0x0000ffffUL);

		       /* Save the selection, focus and	anchor indices	*/

plbw->iSelectedLast = plbw->iSelected;
plbw->iFocusLast    = plbw->iFocus;
plbw->iAnchorLast   = plbw->iAnchor;
}
#pragma	subtitle("   List Manager - Extended State Restore Routine")
#pragma	page( )

/* --- RestoreExtendedState ---------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	restore	the previous extended state	*/
/*     when the	undo request is	received.  The old state is exchanged	*/
/*     with the	current	state to allow an undo of the undo.		*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Internal Data Pointer		*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID RestoreExtendedState(PLISTBOXWIN plbw)

{
LONG  iSelected;		   /* Selected Item Index		*/
LONG  iFocus;			   /* Focus Item Index			*/
LONG  iAnchor;			   /* Anchor Item Index			*/
register INT i;			   /* Loop Counter			*/

		       /* Loop through the items within	the list and	*/
		       /* exchange the current flags within the	list	*/

for ( i	= 0; i < plbw->cItems; i++ )
   plbw->plc[0].apli[i]->fl = plbw->plc[0].apli[i]->fl >> 16UL |
			    (plbw->plc[0].apli[i]->fl &	0x0000ffffUL) << 16UL;

		       /* Exchange the selection, focus	and anchor	*/
		       /* indices					*/

iSelected	    = plbw->iSelected;
iFocus		    = plbw->iFocus;
iAnchor		    = plbw->iAnchor;
plbw->iSelected	    = plbw->iSelectedLast;
plbw->iFocus	    = plbw->iFocusLast;
plbw->iAnchor	    = plbw->iAnchorLast;
plbw->iSelectedLast = iSelected;
plbw->iFocusLast    = iFocus;
plbw->iAnchorLast   = iAnchor;
}
#pragma	subtitle("   List Manager - Item Selection Routine")
#pragma	page( )

/* --- fSelectMultipleItem ----------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	select an item within the list using	*/
/*     the selection criteria given.  The routine is used within	*/
/*     multiple	selection list where the selection of an item does	*/
/*     not change the state of the other items.				*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw;      =	List Box Internal Data Pointer		*/
/*     LONG	   iCol;      =	Column					*/
/*     LONG	   iSelected; =	Selected Item				*/
/*     BOOL	   fSelect;   =	Selection Flag				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     fSelectMultipleItem =  TRUE : Item Selected			*/
/*			   = FALSE : Item Not Selected			*/
/*									*/
/* --------------------------------------------------------------------	*/

BOOL fSelectMultipleItem(PLISTBOXWIN plbw, LONG	iCol, LONG iSelected, BOOL fSelect)

{
		       /* Check	to see that items within the list box	*/
		       /* and that the item requested is within	the	*/
		       /* range	of items				*/

if ( iSelected < plbw->cItems )
   {
		       /* Multiple selections allowed, select or	*/
		       /* deselected the requested item			*/

   SaveSelectState(0L, iSelected);
   if (	fSelect	)
       plbw->plc[iCol].apli[iSelected]->fl |= LI_SELECTED;
   else
       plbw->plc[iCol].apli[iSelected]->fl &= ~LI_SELECTED;

		       /* Invert the rectangle of the entry to return	*/
		       /* it back to its normal	display	colour		*/

   DrawSelection(plbw, iSelected);
   return(TRUE);
   }
return(FALSE);
}
#pragma	subtitle("   List Manager - Item Selection Routine")
#pragma	page( )

/* --- fSelectItem ------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	select an item within the list using	*/
/*     the selection criteria given.					*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw;      =	List Box Internal Data Pointer		*/
/*     LONG	   iCol;      =	Column					*/
/*     LONG	   iSelected; =	Selected Item				*/
/*     BOOL	   fSelect;   =	Selection Flag				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     fSelectItem =  TRUE : Item Selected				*/
/*		   = FALSE : Item Not Selected				*/
/*									*/
/* --------------------------------------------------------------------	*/

BOOL fSelectItem(PLISTBOXWIN plbw, LONG	iCol, LONG iSelected, BOOL fSelect)

{
register INT i;			   /* Loop Index			*/

		       /* Check	to see that items within the list box	*/
		       /* and that the item requested is within	the	*/
		       /* range	of items				*/

if ( iSelected < plbw->cItems )
   {
		       /* Check	to see if the list box allows for	*/
		       /* multiple selections				*/

   if (	plbw->flStyle &	LS_MULTIPLESEL )
       {
		       /* Multiple selections allowed, select or	*/
		       /* deselected the requested item			*/

       SaveSelectState(0L, iSelected);
       if ( fSelect )
	   plbw->plc[iCol].apli[iSelected]->fl |= LI_SELECTED;
       else
	   plbw->plc[iCol].apli[iSelected]->fl &= ~LI_SELECTED;

		       /* Invert the rectangle of the entry to return	*/
		       /* it back to its normal	display	colour		*/

       DrawSelection(plbw, iSelected);
       }
   else
       {
       if ( plbw->flStyle & LS_EXTENDEDSEL )
	   for ( i = 0;	i < plbw->cItems; i++ )
	       if ( (plbw->plc[iCol].apli[i]->fl & LI_SELECTED)	&&
		    (i != iSelected) &&
		    (i != plbw->iSelected) )
		   {
		   SaveSelectState(0L, i);
		   plbw->plc[iCol].apli[i]->fl &= ~LI_SELECTED;

		       /* Invert the rectangle of the entry to return	*/
		       /* it back to its normal	display	colour		*/

		   DrawSelection(plbw, i);
		   }
		       /* Single selection only	allowed	for the	list	*/
		       /* box, deselect	the previous item and then	*/
		       /* select the desired one			*/

       SaveSelectState(0L, iSelected);
       if ( fSelect )
	   {
	   if (	plbw->iSelected	!= LIT_NONE )
	       {
	       SaveSelectState(0L, plbw->iSelected);
	       plbw->plc[iCol].apli[plbw->iSelected]->fl &= ~(LI_SELECTED | LI_FOCUS);

		       /* Invert the rectangle of the entry to return	*/
		       /* it back to its normal	display	colour		*/

	       DrawSelection(plbw, plbw->iSelected);
	       }
	   plbw->plc[iCol].apli[plbw->iSelected	= iSelected]->fl |= LI_SELECTED;
	   }
       else
	   {
	   plbw->plc[iCol].apli[iSelected]->fl &= ~(LI_SELECTED	| LI_FOCUS);
	   plbw->iSelected = LIT_NONE;
	   }
		       /* Invert the rectangle of the entry to return	*/
		       /* it back to its normal	display	colour		*/

       DrawSelection(plbw, iSelected);
       }
   return(TRUE);
   }
return(FALSE);
}
#pragma	subtitle("   List Manager - Extended Selection Routine")
#pragma	page( )

/* --- SelectExtendedItems ----------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	perform	the extended selection using	*/
/*     the defined anchor point	and the	current	item as	the bounds.	*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw;      =	List Box Internal Data Pointer		*/
/*     LONG	   iCol;      =	Column					*/
/*     LONG	   iSelected; =	Selected Item				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID SelectExtendedItems(PLISTBOXWIN plbw, LONG	iCol, LONG iSelected)

{
register INT i,	n;		   /* Index				*/

		       /* Check	to see that items within the list box	*/
		       /* and that the item requested is within	the	*/
		       /* range	of items				*/

if ( iSelected < plbw->cItems )
   {
		       /* Check	for the	case where a selection has been	*/
		       /* made on the check box	area in	which case no	*/
		       /* starting selections have been	made		*/

   if (	plbw->iSelected	== -1L )
       return;
		       /* Check	to see if the selection	is on the	*/
		       /* anchor point					*/

   if (	iSelected == plbw->iAnchor )
       {
		       /* Anchor point selected, need to deselect the	*/
		       /* other	items					*/

       if ( plbw->iSelected < plbw->iAnchor )
	   {
	   i = plbw->iSelected;
	   n = plbw->iAnchor;
	   }
       else
	   {
	   n = plbw->iSelected + 1L;
	   i = plbw->iAnchor + 1L;
	   }

       for ( ; i < n; i++ )
	   if (	plbw->plc[iCol].apli[i]->fl & LI_SELECTED )
	       {
	       SaveSelectState(0L, i);
	       plbw->plc[iCol].apli[i]->fl &= ~LI_SELECTED;

		       /* Invert the rectangle of the entry to return	*/
		       /* it back to its normal	display	colour		*/

	       DrawSelection(plbw, i);
	       }
       }
   else
       {
		       /* Check	for the	case where a selection has been	*/
		       /* made on the check box	area in	which case no	*/
		       /* starting selections have been	made yet	*/

       if ( plbw->iSelected == -1L )
	   return;
		       /* Check	to see if the selection	point is in	*/
		       /* front	of the anchor				*/

       if ( iSelected <	plbw->iAnchor )
	   {
		       /* Check	to see if the selected item is in	*/
		       /* front	of the anchor in which case need to	*/
		       /* check	to see if the block selection is	*/
		       /* changing direction in	which case the items on	*/
		       /* the other side of the	anchor need to be	*/
		       /* deselected					*/

	   if (	plbw->iSelected	< plbw->iAnchor	)
	       {
	       for ( i = plbw->iSelected; i < iSelected; i++ )
		   if (	plbw->plc[iCol].apli[i]->fl & LI_SELECTED )
		       {
		       SaveSelectState(0L, i);
		       plbw->plc[iCol].apli[i]->fl &= ~LI_SELECTED;

		       /* Invert the rectangle of the entry to return	*/
		       /* it back to its normal	display	colour		*/

		       DrawSelection(plbw, i);
		       }
	       }
	   else
	       for ( i = plbw->iSelected; i > plbw->iAnchor; i-- )
		   if (	plbw->plc[iCol].apli[i]->fl & LI_SELECTED )
		       {
		       SaveSelectState(0L, i);
		       plbw->plc[iCol].apli[i]->fl &= ~LI_SELECTED;

		       /* Invert the rectangle of the entry to return	*/
		       /* it back to its normal	display	colour		*/

		       DrawSelection(plbw, i);
		       }
	   i = iSelected;
	   n = plbw->iAnchor + 1L;
	   }
       else
		       /* Item selected	after the anchor point,	check	*/
		       /* to see if the	previous selected item was	*/
		       /* before the anchor, in	which case need	to	*/
		       /* check	to see if the block selection is	*/
		       /* changing direction in	which case the items on	*/
		       /* the other side of the	anchor need to be	*/
		       /* deselected					*/
	   {
	   if (	plbw->iSelected	< plbw->iAnchor	)
	       for ( i = plbw->iSelected; i < plbw->iAnchor; i++ )
		   if (	plbw->plc[iCol].apli[i]->fl & LI_SELECTED )
		       {
		       SaveSelectState(0L, i);
		       plbw->plc[iCol].apli[i]->fl &= ~LI_SELECTED;

		       /* Invert the rectangle of the entry to return	*/
		       /* it back to its normal	display	colour		*/

		       DrawSelection(plbw, i);
		       }
	   n = iSelected + 1L;
	   i = plbw->iAnchor;
	   }

       for ( ; i < n; i++ )
	   if (	!(plbw->plc[iCol].apli[i]->fl &	LI_SELECTED) )
	       {
	       SaveSelectState(0L, i);
	       plbw->plc[iCol].apli[i]->fl |= LI_SELECTED;

	       /* Invert the rectangle of the entry to return	*/
	       /* it back to its normal	display	colour		*/

	       DrawSelection(plbw, i);
	       }

       if ( iSelected >	plbw->iAnchor )
	   for ( i = n;	i <= plbw->iSelected; i++ )
	       if ( plbw->plc[iCol].apli[i]->fl	& LI_SELECTED )
		   {
		   SaveSelectState(0L, i);
		   plbw->plc[iCol].apli[i]->fl &= ~LI_SELECTED;

	       /* Invert the rectangle of the entry to return	*/
	       /* it back to its normal	display	colour		*/

		   DrawSelection(plbw, i);
		   }
       }
   plbw->iSelected = iSelected;
   }
}
#pragma	subtitle("   List Manager - List Box Control Data Procedure")
#pragma	page( )

/* --- SetControlDataList ------------------------------ [ Public ] ---	*/
/*									*/
/*     This function is	used to	decode the control data	that may form	*/
/*     part the	of the WM_CREATE message.  The control data is		*/
/*     designed	to allow the user to specify a simple list of items	*/
/*     that are	to be placed within the	list box.			*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw;   = List Box Internal Data Pointer		*/
/*     ULONG	   cItems; = Items in Control Data List			*/
/*     PBYTE	   pb;	   = Control Data List Pointer			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID SetControlDataList(PLISTBOXWIN plbw, ULONG	cItems,	PBYTE pb)

{
HHEAPMEM hHeap;			   /* Heap Handle			*/
register INT i;			   /* Loop Counter			*/
EXCEPTIONREGISTRATIONRECORD xcptregr;	   /* Exception	Record		*/

if ( !cItems )
   return;
		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

xcptregr.prev_structure	  = NULL;
xcptregr.ExceptionHandler = &ListBoxExceptionHandler;

DosSetExceptionHandler(&xcptregr);

		       /* Set the jump buffer and check	to see if an	*/
		       /* exception has	occurred in which case ignore	*/
		       /* the decoding of the control data and return	*/
if ( setjmp(jBuf) )
   {
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

   DosUnsetExceptionHandler(&xcptregr);
   return;
   }
		       /* Starting off the list	box from scratch,	*/
		       /* create the new heap that is to be used for	*/
		       /* list box.  Allocate the memory needed	for the	*/
		       /* control information and save the heap	handle	*/
		       /* within it.					*/

plbw->plc = (PLISTCOL)HeapMalloc(hHeap = HeapAlloc(8192UL, 4096UL), sizeof(LISTCOL));

		       /* Allocate memory for the initial start	of the	*/
		       /* list.	 The list is allocated in groups of 8,	*/
		       /* hence	whenever the list total	is a multiple	*/
		       /* of 8,	the current list is full and needs to	*/
		       /* be expanded.					*/

plbw->plc[0].apli = (PLISTITEM *)HeapMalloc(plbw->plc[0].hHeap = hHeap,
					    ((cItems / CBLK_LIST) + 1UL) * CBLK_LIST * sizeof(PLISTITEM));

		       /* Set the text of the item being inserted at	*/
		       /* the end of the list box list and check to see	*/
		       /* if the item is wider than the	rest in	which	*/
		       /* case the maximum item	width should be	updated	*/

for ( i	= 0; i < cItems; i++ )
   {
   if (	(lSetItemText(plbw->plc[0].hHeap, plbw->hWnd, plbw->plc[0].apli[i],
		      pb) > plbw->cxItem) && (plbw->flStyle & LS_HORZSCROLL) )
       plbw->cxItem = plbw->plc[0].apli[i]->cxItem;
   pb += strlen(pb) + 1;
   }
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

DosUnsetExceptionHandler(&xcptregr);
}
#pragma	subtitle("   List Manager - List Box Window Procedure")
#pragma	page( )

/* --- mrBaseListHandler ------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	process	the messages for the Colour	*/
/*     Wheel control window.  It should	be noted that all angles	*/
/*     when used with sin and cosine functions are in radians.		*/
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
/*     mrBaseListHandler = Message Handling Result			*/
/*									*/
/* --------------------------------------------------------------------	*/

MRESULT	EXPENTRY mrBaseListHandler(HWND	hWnd, ULONG msg, MPARAM	mp1, MPARAM mp2)

{
BOOL	    fSelected;		   /* Selection	State			*/
HHEAPMEM    hHeap;		   /* Heap Handle			*/
LONG	    iVertScroll;	   /* Vertical Scroll Position		*/
PLISTBOXWIN plbw;		   /* List Box Internal	Data Pointer	*/
PLISTITEM   pli;		   /* List Item	Pointer			*/
RECTL	    rcl;		   /* Display Rectangle			*/
UINT	    usLen;		   /* Length Holder			*/
register INT i,	n;		   /* Loop Counter			*/
EXCEPTIONREGISTRATIONRECORD xcptregr;	   /* Exception	Record		*/

plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

switch ( msg )
   {
   /*********************************************************************/
   /*  Standard	message:   LM_DELETEALL					*/
   /*			   mp1 = 0L;					*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	LM_DELETEALL :
		       /* Check	to see if any items within the list box	*/
		       /* which	if there are none, there is nothing to	*/
		       /* delete and there is no memory	to release	*/

       if ( plbw->cItems )
	   {
		       /* Release the heap associated with the list box	*/
		       /* items	and set	the item count to zero		*/

	   HeapRelease(plbw->plc[0].hHeap);

	   FocusChange(plbw, plbw->iFocus, FALSE);
	   plbw->iFocus	= plbw->iSelected = LIT_NONE;
	   plbw->cHorzScroll =
	   plbw->cVertScroll =
	   plbw->iHorzScroll =
	   plbw->iVertScroll = 0L;
	   plbw->cItems	= 0UL;

		       /* Check	to see if the list box is showing and	*/
		       /* if the case clear the	list area and update	*/
		       /* the scroll bars otherwise set	dirty flag to	*/
		       /* delay	the scroll bar updating	until it is	*/
		       /* visible					*/

	   if (	WinIsWindowShowing(hWnd) )
	       {
	       WinInvalidateRect(hWnd, &plbw->rcl, FALSE);
	       UpdateScrollBars(plbw);
	       }
	   else
	       plbw->fDirty = TRUE;
	   return(MRFROMLONG(TRUE));
	   }
       break;

   /*********************************************************************/
   /*  Standard	message:   LM_DELETEITEM				*/
   /*			   mp1 = MPFROMLONG(iItem);			*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	LM_DELETEITEM :
		       /* Check	to see that items within the list box	*/
		       /* and that the item requested is within	the	*/
		       /* range	of items				*/

       if ( plbw->cItems && (LONGFROMMP(mp1) < plbw->cItems) )
	   {
		       /* Reduce the list box item count and check to	*/
		       /* see if the last item is being	deleted	in	*/
		       /* which	case release the heap for the list box	*/
		       /* items						*/

	   if (	--plbw->cItems == 0UL )
	       {
	       HeapRelease(plbw->plc[0].hHeap);

	       FocusChange(plbw, plbw->iFocus, FALSE);
	       plbw->iFocus = plbw->iSelected =	LIT_NONE;
	       plbw->cHorzScroll =
	       plbw->cVertScroll =
	       plbw->iHorzScroll =
	       plbw->iVertScroll = 0L;
	       }
	   else
	       {
		       /* Release the memory allocated for the text of	*/
		       /* the item and shift the entires up one		*/
		       /* position in place of the entry being		*/
		       /* deleted					*/

	       HeapFree(plbw->plc[0].hHeap, plbw->plc[0].apli[LONGFROMMP(mp1)]->pszText);
	       HeapFree(plbw->plc[0].hHeap, plbw->plc[0].apli[LONGFROMMP(mp1)]);
	       if ( LONGFROMMP(mp1) != plbw->cItems )
		   memmove(&plbw->plc[0].apli[LONGFROMMP(mp1)],	&plbw->plc[0].apli[LONGFROMMP(mp1) + 1],
			   (UINT)((plbw->cItems	- LONGFROMMP(mp1)) * sizeof(PLISTITEM)));
	       }
	   if (	(plbw->iVertScroll + plbw->cLinesPage) >= plbw->cItems )
	       {
	       if ( (plbw->iVertScroll = plbw->cItems -	plbw->cLinesPage) < 0 )
		   plbw->iVertScroll = 0;
	       if ( plbw->iFocus > plbw->cLinesPage )
		   plbw->iFocus	= plbw->cLinesPage - 1L;
	       }
		       /* Check	to see if there	are more items within	*/
		       /* the list box than can	be displayed which if	*/
		       /* the case, perform normal refresh		*/

	   if (	plbw->cItems > plbw->cLinesPage	)
	       RefreshList(plbw, (LONG)LONGFROMMP(mp1));
	   else
		       /* Check	to see if the list box is being		*/
		       /* displayed					*/

	       if ( WinIsWindowShowing(plbw->hWnd) )
		   {
		       /* List box is visible, need to refresh the	*/
		       /* list showing the item	removed.  Form the	*/
		       /* rectangle based on the fact that the area	*/
		       /* that requires	updating includes the former	*/
		       /* last line which is now blank.			*/

		   rcl = plbw->rcl;
		   rcl.yTop = plbw->rcl.yTop - ((LONG)LONGFROMMP(mp1) -	plbw->iVertScroll) * plbw->cyItem;
		   if (	plbw->cItems < plbw->cLinesPage	)
		       rcl.yBottom = plbw->rcl.yTop - (plbw->cItems + 1) * plbw->cyItem;
		   WinInvalidateRect(plbw->hWnd, &rcl, FALSE);
		   UpdateScrollBars(plbw);
		   }
	       else
		   plbw->fDirty	= TRUE;
	   }
       return(MRFROMLONG(plbw->cItems));

   /*********************************************************************/
   /*  Standard	message:   LM_INSERTITEM				*/
   /*			   mp1 = MPFROMLONG(iItem);			*/
   /*			   mp2 = MPFROMP(pszText);			*/
   /*********************************************************************/

   case	LM_INSERTITEM :
		       /* Determine if items exist within the list box	*/
		       /* in which case	may need to resize the array	*/
		       /* the for list items otherwise will need to	*/
		       /* start	the list off from scratch		*/

       if ( plbw->cItems )
	   {
		       /* Existing list	present, determine if the	*/
		       /* modulo boundary encountered in which case	*/
		       /* reallocate the list for a set	of new entries.	*/
		       /* The list is allocated	in groups of 8,	hence	*/
		       /* whenever the list total is a multiple	of 8,	*/
		       /* the current list is full and needs to	be	*/
		       /* expanded.					*/

	   if (	(plbw->cItems %	CBLK_LIST) == 0	)
	       plbw->plc[0].apli = (PLISTITEM *)HeapRealloc(plbw->plc[0].hHeap,	plbw->plc[0].apli,
							    (((plbw->cItems / CBLK_LIST) + 1UL)	* CBLK_LIST) *
							    sizeof(PLISTITEM));
	   }
       else
	   {
		       /* Starting off the list	box from scratch,	*/
		       /* create the new heap that is to be used for	*/
		       /* list box					*/

	   hHeap = HeapAlloc(HALLOC_DEFAULT, HALLOC_DEFAULT);

		       /* Allocate the memory needed for the control	*/
		       /* information and save the heap	handle within	*/
		       /* it						*/

	   plbw->plc = (PLISTCOL)HeapMalloc(hHeap, sizeof(LISTCOL));
	   plbw->plc[0].hHeap =	hHeap;

		       /* Allcoate memory for the initial start	of the	*/
		       /* list.	 The list is allocated in groups of 8,	*/
		       /* hence	whenever the list total	is a multiple	*/
		       /* of 8,	the current list is full and needs to	*/
		       /* be expanded.					*/

	   plbw->plc[0].apli = (PLISTITEM *)HeapMalloc(hHeap, CBLK_LIST	* sizeof(PLISTITEM));
	   }
		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

       xcptregr.prev_structure	 = NULL;
       xcptregr.ExceptionHandler = &ListBoxExceptionHandler;

       DosSetExceptionHandler(&xcptregr);

       if ( setjmp(jBuf) )
	   {
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	   DosUnsetExceptionHandler(&xcptregr);
	   return(MRFROMLONG(TRUE));
	   }

       plbw->plc[0].apli[plbw->cItems] = (PLISTITEM)HeapMalloc(plbw->plc[0].hHeap, sizeof(LISTITEM));

		       /* Insert the item in the list based on the	*/
		       /* method of insertion				*/

       switch (	(LONG)(SHORT)SHORT1FROMMP(mp1) )
	   {
		       /* Method:  Add to end of list box		*/
	   case	LIT_END	:
		       /* Set the text of the item being inserted at	*/
		       /* the end of the list box list and check to see	*/
		       /* if the item is wider than the	rest in	which	*/
		       /* case the maximum item	width should be	updated	*/

	       if ( (lSetItemText(plbw->plc[0].hHeap, hWnd, plbw->plc[0].apli[plbw->cItems++],
				  PVOIDFROMMP(mp2)) > plbw->cxItem) && (plbw->flStyle &	LS_HORZSCROLL) )
		   plbw->cxItem	= plbw->plc[0].apli[plbw->cItems - 1]->cxItem;

	       RefreshList(plbw, plbw->cItems -	1);
	       if ( plbw->fFocus && (plbw->iFocus == LIT_NONE) )
		   SetFocus(plbw, 0L);

		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	       DosUnsetExceptionHandler(&xcptregr);
	       return(MRFROMLONG(plbw->cItems -	1));

		       /* Method:  Add to list box in ascending	order	*/
		       /* Method:  Add to list box in descending order	*/

	   case	LIT_SORTASCENDING :
	   case	LIT_SORTDESCENDING :

		       /* Set the text of the item being inserted at	*/
		       /* the end of the list box list and check to see	*/
		       /* if the item is wider than the	rest in	which	*/
		       /* case the maximum item	width should be	updated	*/

	       if ( (lSetItemText(plbw->plc[0].hHeap, hWnd, plbw->plc[0].apli[plbw->cItems],
				  PVOIDFROMMP(mp2)) > plbw->cxItem) && (plbw->flStyle &	LS_HORZSCROLL) )
		   plbw->cxItem	= plbw->plc[0].apli[plbw->cItems]->cxItem;

	       pli = plbw->plc[0].apli[plbw->cItems];

		       /* Determine the	type of	sorting	required	*/
	
	       if ( (LONG)LONGFROMMP(mp1) == LIT_SORTDESCENDING	)

		       /* Sort the list	in descending order		*/

		   SortDescending(plbw->plc[0].apli, 0,	plbw->cItems++);
	       else
		       /* Sort the list	in ascending order		*/

		   SortAscending(plbw->plc[0].apli, 0, plbw->cItems++);

	       for ( i = 0; i <	plbw->cItems; i++ )
		   if (	plbw->plc[0].apli[i] ==	pli )
		       break;
	       RefreshList(plbw, i);
	       if ( plbw->fFocus && (plbw->iFocus == LIT_NONE) )
		   SetFocus(plbw, 0L);
	       else
		   if (	plbw->iFocus !=	LIT_NONE )
		       for ( n = 0; n <	plbw->cItems; n++ )
			   if (	plbw->plc[0].apli[n]->fl & (LI_FOCUS | LI_SELECTED) )
			       {
			       plbw->iSelected = plbw->iFocus =	n;
			       break;
			       }

		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	       DosUnsetExceptionHandler(&xcptregr);
	       return(MRFROMLONG(i));

		       /* Method: Add to list box in position indicated	*/
	   default :
		       /* Check	to make	sure that the item being	*/
		       /* inserted can be inserted in the position	*/
		       /* requested					*/

	       if ( (LONGFROMMP(mp1) >=	0L) && (LONGFROMMP(mp1)	<= plbw->cItems) )
		   {
		       /* Check	to make	sure that the item position is	*/
		       /* not the next logical entry at	the end	of the	*/
		       /* list						*/

		   if (	LONGFROMMP(mp1)	!= plbw->cItems	)
		       {
		       /* Inserting the	item somewhere within the list,	*/
		       /* therefore open up the	position for the entry	*/
		       /* in the correct location shifting the other	*/
		       /* entries down a position			*/

		       for ( i = plbw->cItems++; i > LONGFROMMP(mp1); i-- )
			   plbw->plc[0].apli[i]	= plbw->plc[0].apli[i -	1];
		       memset(plbw->plc[0].apli[LONGFROMMP(mp1)], 0, sizeof(LISTITEM));
		       }
		   else
		       /* Entry	position essentially the last one, just	*/
		       /* bump the entries count since there is	no need	*/
		       /* to shuffle things around			*/

		       plbw->cItems++;

		       /* Set the text of the item being inserted at	*/
		       /* the end of the list box list and check to see	*/
		       /* if the item is wider than the	rest in	which	*/
		       /* case the maximum item	width should be	updated	*/

		   if (	(lSetItemText(plbw->plc[0].hHeap, hWnd,	plbw->plc[0].apli[LONGFROMMP(mp1)],
				      PVOIDFROMMP(mp2))	> plbw->cxItem)	&& (plbw->flStyle & LS_HORZSCROLL) )
		       plbw->cxItem = plbw->plc[0].apli[LONGFROMMP(mp1)]->cxItem;

		   RefreshList(plbw, (LONG)LONGFROMMP(mp1));
		   if (	plbw->fFocus &&	(plbw->iFocus == LIT_NONE) )
		       SetFocus(plbw, 0L);

		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

		   DosUnsetExceptionHandler(&xcptregr);
		   return(MRFROMLONG(LONGFROMMP(mp1)));
		   }
	       else
		   {
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

		   DosUnsetExceptionHandler(&xcptregr);
		   return(MRFROMLONG(LIT_ERROR));
		   }
	   }

   /*********************************************************************/
   /*  Standard	message:   LM_QUERYITEMCOUNT				*/
   /*			   mp1 = 0L;					*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	LM_QUERYITEMCOUNT :
       return(MRFROMLONG(plbw->cItems));

   /*********************************************************************/
   /*  Standard	message:   LM_QUERYITEMHANDLE				*/
   /*			   mp1 = MPFROMLONG(iItem);			*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	LM_QUERYITEMHANDLE :
		       /* Check	to see that items within the list box	*/
		       /* and that the item requested is within	the	*/
		       /* range	of items				*/

       if ( plbw->cItems && (LONGFROMMP(mp1) < plbw->cItems) )
	   return(MRFROMLONG(plbw->plc[0].apli[LONGFROMMP(mp1)]->ulHandle));
      break;

   /*********************************************************************/
   /*  Standard	message:   LM_QUERYITEMTEXT				*/
   /*			   mp1 = MPFROM2SHORT(iItem, cch);		*/
   /*			   mp2 = MPFROMP(pszText);			*/
   /*********************************************************************/

   case	LM_QUERYITEMTEXT :
		       /* Check	to see that items within the list box	*/
		       /* and that the item requested is within	the	*/
		       /* range	of items				*/

       if ( plbw->cItems && ((LONG)SHORT1FROMMP(mp1) < plbw->cItems) &&
	    ((LONG)SHORT2FROMMP(mp1) > 0L) )
	   {
	   memcpy(PVOIDFROMMP(mp2), plbw->plc[0].apli[SHORT1FROMMP(mp1)]->pszText,
		  (plbw->plc[0].apli[SHORT1FROMMP(mp1)]->cText + 1) < SHORT2FROMMP(mp1)	?
		  (plbw->plc[0].apli[SHORT1FROMMP(mp1)]->cText + 1) :
		  SHORT2FROMMP(mp1));
	   return(MRFROMLONG(plbw->plc[0].apli[SHORT1FROMMP(mp1)]->cText < SHORT2FROMMP(mp1) ?
			     plbw->plc[0].apli[SHORT1FROMMP(mp1)]->cText :
			     SHORT2FROMMP(mp1)));
	   }
       else
	   return(0L);

   /*********************************************************************/
   /*  Standard	message:   LM_QUERYITEMTEXTLENGTH			*/
   /*			   mp1 = MPFROMLONG(iItem);			*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	LM_QUERYITEMTEXTLENGTH :

		       /* Check	to see that items within the list box	*/
		       /* and that the item requested is within	the	*/
		       /* range	of items				*/

       if ( plbw->cItems && (LONGFROMMP(mp1) < plbw->cItems) )
	   return(MRFROMLONG(plbw->plc[0].apli[LONGFROMMP(mp1)]->cText));
       break;

   /*********************************************************************/
   /*  Standard	message:   LM_QUERYSELECTION				*/
   /*			   mp1 = MPFROMLONG(iItemPrev);			*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	LM_QUERYSELECTION :
       if ( plbw->cItems )
	   if (	plbw->flStyle &	(LS_MULTIPLESEL	| LS_EXTENDEDSEL) )
	       {
	       if ( (LONG)(SHORT)SHORT1FROMMP(mp1) == LIT_FIRST	)
		   {
		   for ( i = 0;	i < plbw->cItems; i++ )
		       if ( plbw->plc[0].apli[i]->fl & LI_SELECTED )
			   return(MRFROMLONG(i));

		   return(MRFROMLONG(LIT_NONE));
		   }
	       else
		   if (	(LONG)(SHORT)SHORT1FROMMP(mp1) >= 0L )
		       {
		       for ( i = (LONG)(SHORT)SHORT1FROMMP(mp1)	+ 1; i < plbw->cItems; i++ )
			   if (	plbw->plc[0].apli[i]->fl & LI_SELECTED )
			       return(MRFROMLONG(i));

		       return(MRFROMLONG(LIT_NONE));
		       }
		   else
		       return(MRFROMLONG(LIT_NONE));
	       }
	   else
	       return(MRFROMLONG(plbw->iSelected));
       else
	   return(MRFROMLONG(LIT_NONE));

   /*********************************************************************/
   /*  Standard	message:   LM_QUERYTOPINDEX				*/
   /*			   mp1 = 0L;					*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	LM_QUERYTOPINDEX :
       if ( plbw->cItems )
	   return(MRFROMLONG(plbw->iVertScroll));
       else
	   return(MRFROMLONG(LIT_NONE));

   /*********************************************************************/
   /*  Standard	message:   LM_SEARCHSTRING				*/
   /*			   mp1 = MPFROM2SHORT(usCmd, iItem);		*/
   /*			   mp2 = MPFROMP(pszSearch);			*/
   /*********************************************************************/

   case	LM_SEARCHSTRING	:

		       /* Check	to see that items within the list box	*/
		       /* and that the item requested is within	the	*/
		       /* range	of items				*/

       if ( plbw->cItems && ((LONG)(SHORT)SHORT2FROMMP(mp1) < plbw->cItems) )
	   {
		       /* Check	to make	sure that valid	values have	*/
		       /* been provided					*/

	   if (	(((ULONG)SHORT1FROMMP(mp1) & (LSS_PREFIX | LSS_SUBSTRING)) == (LSS_PREFIX | LSS_SUBSTRING)) ||
		((PSZ)PVOIDFROMMP(mp2) == NULL)	)
	       return(MRFROMLONG(LIT_ERROR));

		       /* Set the starting index point			*/

	   if (	(LONG)(SHORT)SHORT2FROMMP(mp1) == LIT_FIRST )
	       n = 0;
	   else
	       n = (LONG)(SHORT)SHORT2FROMMP(mp1) + 1;

		       /* Check	to see if the search is	for the	prefix	*/
		       /* portion of the list box item			*/

	   if (	(ULONG)SHORT1FROMMP(mp1) & LSS_PREFIX )
	       {
	       usLen = strlen((PSZ)PVOIDFROMMP(mp2));

	       if ( (ULONG)SHORT1FROMMP(mp1) & LSS_CASESENSITIVE )
		   {
		   for ( i = n;	i < plbw->cItems; i++ )
		       if ( strncmp(plbw->plc[0].apli[i]->pszText, (PSZ)PVOIDFROMMP(mp2), usLen) == 0 )
			   return(MRFROMLONG(i));

		   for ( i = 0;	i < n; i++ )
		       if ( strncmp(plbw->plc[0].apli[i]->pszText, (PSZ)PVOIDFROMMP(mp2), usLen) == 0 )
			   return(MRFROMLONG(i));
		   }
	       else
		   {
		   for ( i = n;	i < plbw->cItems; i++ )
		       if ( strnicmp(plbw->plc[0].apli[i]->pszText, (PSZ)PVOIDFROMMP(mp2), usLen) == 0 )
			   return(MRFROMLONG(i));

		   for ( i = 0;	i < n; i++ )
		       if ( strnicmp(plbw->plc[0].apli[i]->pszText, (PSZ)PVOIDFROMMP(mp2), usLen) == 0 )
			   return(MRFROMLONG(i));
		   }
	       return(MRFROMLONG(LIT_NONE));
	       }
	   else
		       /* Check	to see if the search is	for a substring	*/
		       /* within the list box items			*/

	       if ( (ULONG)SHORT1FROMMP(mp1) & LSS_SUBSTRING )
		   {
		   for ( i = n;	i < plbw->cItems; i++ )
		       if ( strstr(plbw->plc[0].apli[i]->pszText, (PSZ)PVOIDFROMMP(mp2)) )
			   return(MRFROMLONG(i));

		   for ( i = 0;	i < n; i++ )
		       if ( strstr(plbw->plc[0].apli[i]->pszText, (PSZ)PVOIDFROMMP(mp2)) )
			   return(MRFROMLONG(i));

		   return(MRFROMLONG(LIT_NONE));
		   }
	       else
		       /* Check	to see if the search is	case senstive	*/

		   if (	(ULONG)SHORT1FROMMP(mp1) & LSS_CASESENSITIVE )
		       {
		       for ( i = n; i <	plbw->cItems; i++ )
			   if (	strcmp(plbw->plc[0].apli[i]->pszText, (PSZ)PVOIDFROMMP(mp2)) ==	0 )
			       return(MRFROMLONG(i));

		       for ( i = 0; i <	n; i++ )
			   if (	strcmp(plbw->plc[0].apli[i]->pszText, (PSZ)PVOIDFROMMP(mp2)) ==	0 )
			       return(MRFROMLONG(i));

		       return(MRFROMLONG(LIT_NONE));
		       }
		   else
		       /* Search is case insensitive			*/
		       {
		       for ( i = n; i <	plbw->cItems; i++ )
			   if (	stricmp(plbw->plc[0].apli[i]->pszText, (PSZ)PVOIDFROMMP(mp2)) == 0 )
			       return(MRFROMLONG(i));

		       for ( i = 0; i <	n; i++ )
			   if (	stricmp(plbw->plc[0].apli[i]->pszText, (PSZ)PVOIDFROMMP(mp2)) == 0 )
			       return(MRFROMLONG(i));

		       return(MRFROMLONG(LIT_NONE));
		       }
	   }
       else
	   return(MRFROMLONG(LIT_ERROR));

   /*********************************************************************/
   /*  Standard	message:   LM_SELECTITEM				*/
   /*			   mp1 = MPFROMLONG(iItem);			*/
   /*			   mp2 = MPFROMLONG(fSelect);			*/
   /*********************************************************************/

   case	LM_SELECTITEM :
       if ( (LONG)LONGFROMMP(mp1) == LIT_NONE )
	   {
	   for ( i = 0;	i < plbw->cItems; i++ )
	       if (plbw->plc[0].apli[i]->fl & LI_SELECTED )
		   {
		   SaveSelectState(0L, i);
		   plbw->plc[0].apli[i]->fl &= ~LI_SELECTED;

		       /* Invert the rectangle of the entry to return	*/
		       /* it back to its normal	display	colour		*/

		   DrawSelection(plbw, i);
		   }
	   return(MRFROMLONG(TRUE));
	   }
       else
		       /* Check	to see that items within the list box	*/
		       /* and that the item requested is within	the	*/
		       /* range	of items				*/

	   if (	LONGFROMMP(mp1)	< plbw->cItems )
	       {
	       if ( plbw->plc[0].apli[LONGFROMMP(mp1)]->fl & LI_SELECTED )
		   fSelected = TRUE;
	       else
		   fSelected = FALSE;

		       /* Check	to see if the list box allows for	*/
		       /* multiple selections				*/

	       if ( plbw->flStyle & (LS_MULTIPLESEL | LS_EXTENDEDSEL) )
		   {
		       /* Multiple selections allowed, select or	*/
		       /* deselected the requested item			*/

		   SaveSelectState(0L, (LONG)LONGFROMMP(mp1));
		   if (	(BOOL)LONGFROMMP(mp2) )
		       {
		       RemoveFocus(plbw);
		       plbw->plc[0].apli[LONGFROMMP(mp1)]->fl |= (LI_SELECTED |	LI_FOCUS);
		       FocusChange(plbw, plbw->iFocus =	(LONG)LONGFROMMP(mp1), TRUE);
		       if ( plbw->iSelected == -1 )
			   plbw->iAnchor = plbw->iSelected = plbw->iFocus;
		       }
		   else
		       {
		       plbw->plc[0].apli[LONGFROMMP(mp1)]->fl &= ~LI_SELECTED;
		       for ( i = (LONG)LONGFROMMP(mp1),	plbw->iSelected	= -1; i	>= 0; i-- )
			   if (	plbw->plc[0].apli[i]->fl & LI_SELECTED )
			       {
			       plbw->iSelected = i;
			       break;
			       }

		       if ( plbw->iSelected == -1 )
			   for ( i = (LONG)LONGFROMMP(mp1); i <	plbw->cItems; i++ )
			       if ( plbw->plc[0].apli[i]->fl & LI_SELECTED )
				   {
				   plbw->iSelected = i;
				   break;
				   }
		       }
		       /* Invert the rectangle of the entry to return	*/
		       /* it back to its normal	display	colour		*/

		   DrawSelection(plbw, (LONG)LONGFROMMP(mp1));
		   }
	       else
		   {
		       /* Single selection only	allowed	for the	list	*/
		       /* box, deselect	the previous item and then	*/
		       /* select the desired one			*/

		   if (	(BOOL)LONGFROMMP(mp2) )
		       {
		       if ( plbw->iSelected != LIT_NONE	)
			   {
			   SaveSelectState(0L, plbw->iSelected);
			   plbw->plc[0].apli[plbw->iSelected]->fl &= ~(LI_SELECTED | LI_FOCUS);

		       /* Invert the rectangle of the entry to return	*/
		       /* it back to its normal	display	colour		*/

			   DrawSelection(plbw, plbw->iSelected);
			   }
		       SaveSelectState(0L, (LONG)LONGFROMMP(mp1));
		       plbw->plc[0].apli[plbw->iFocus =	plbw->iSelected	= (LONG)LONGFROMMP(mp1)]->fl |=	(LI_SELECTED | LI_FOCUS);
		       }
		   else
		       {
		       SaveSelectState(0L, (LONG)LONGFROMMP(mp1));
		       plbw->plc[0].apli[LONGFROMMP(mp1)]->fl &= ~(LI_SELECTED | LI_FOCUS);
		       plbw->iSelected = plbw->iFocus =	LIT_NONE;
		       }
		       /* Determine if the selected item should	be	*/
		       /* centred within the list making sure that	*/
		       /* this only occurs when	item is	not in the last	*/
		       /* page of the list box or when the list	box is	*/
		       /* less than a full page				*/

		   if (	plbw->iFocus !=	LIT_NONE )
		       {
		       if ( plbw->cItems < plbw->cLinesPage )
			   iVertScroll = plbw->iVertScroll;
		       else
			   if (	(iVertScroll = plbw->iFocus - plbw->cLinesPage / 2L) < 0L )
			       iVertScroll = 0L;
			   else
			       if ( iVertScroll	> (plbw->cItems	- plbw->cLinesPage) )
				   iVertScroll = plbw->cItems -	plbw->cLinesPage;

		       if ( plbw->iVertScroll != iVertScroll )
			   {
			   RefreshList(plbw, plbw->iVertScroll = iVertScroll);
			   mrNotifyOwner(plbw, LN_SCROLL);
			   }
		       else
			   if (	plbw->iSelected	!= LIT_NONE )
			       DrawSelection(plbw, plbw->iSelected);
		       }
		   else
		       DrawSelection(plbw, (LONG)LONGFROMMP(mp1));

		   if (	LONGFROMMP(mp1)	)
		       FocusChange(plbw, (LONG)LONGFROMMP(mp1),	TRUE);
		   }

	       if ( fSelected != (BOOL)(BOOL)LONGFROMMP(mp2) )
		   mrNotifyOwner(plbw, LN_SELECT);
	       return(MRFROMLONG(TRUE));
	       }
	   else
	       return(MRFROMLONG(FALSE));

   /*********************************************************************/
   /*  Standard	message:   LM_SETITEMHANDLE				*/
   /*			   mp1 = MPFROMLONG(iItem);			*/
   /*			   mp2 = MPFROMLONG(ulHandle);			*/
   /*********************************************************************/

   case	LM_SETITEMHANDLE :

		       /* Check	to see that items within the list box	*/
		       /* and that the item requested is within	the	*/
		       /* range	of items				*/

       if ( plbw->cItems && (LONGFROMMP(mp1) < plbw->cItems) )
	   {
	   plbw->plc[0].apli[LONGFROMMP(mp1)]->ulHandle	= (ULONG)LONGFROMMP(mp2);
	   return(MRFROMLONG(TRUE));
	   }
       break;

   /*********************************************************************/
   /*  Standard	message:   LM_SETITEMHEIGHT				*/
   /*			   mp1 = MPFROMLONG(lHeight);			*/
   /*			   mp2 = 0L					*/
   /*********************************************************************/

   case	LM_SETITEMHEIGHT :
       if ( LONGFROMMP(mp1) > 0L )
	   {
	   plbw->cyItem	= (LONG)LONGFROMMP(mp1);
	   SizeListBox(plbw);
	   return(MRFROMLONG(TRUE));
	   }
       break;

   /*********************************************************************/
   /*  Standard	message:   LM_SETITEMTEXT				*/
   /*			   mp1 = MPFROMLONG(iItem);			*/
   /*			   mp2 = MPFROMP(pszText);			*/
   /*********************************************************************/

   case	LM_SETITEMTEXT :
		       /* Check	to see that items within the list box	*/
		       /* and that the item requested is within	the	*/
		       /* range	of items				*/

       if ( plbw->cItems && (LONGFROMMP(mp1) < plbw->cItems) &&	PVOIDFROMMP(mp2) )
	   {
		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

	   xcptregr.prev_structure   = NULL;
	   xcptregr.ExceptionHandler = &ListBoxExceptionHandler;

	   DosSetExceptionHandler(&xcptregr);

	   if (	setjmp(jBuf) )
	       {
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	       DosUnsetExceptionHandler(&xcptregr);
	       return(MRFROMLONG(TRUE));
	       }
	   HeapFree(plbw->plc[0].hHeap,	plbw->plc[0].apli[LONGFROMMP(mp1)]->pszText);
	   if (	(lSetItemText(plbw->plc[0].hHeap, hWnd,	plbw->plc[0].apli[LONGFROMMP(mp1)],
			      PVOIDFROMMP(mp2))	> plbw->cxItem)	&& (plbw->flStyle & LS_OWNERDRAW) )
	       plbw->cxItem = plbw->plc[0].apli[LONGFROMMP(mp1)]->cxItem;

		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	   DosUnsetExceptionHandler(&xcptregr);
	   return(MRFROMLONG(TRUE));
	   }
       break;

   /*********************************************************************/
   /*  Standard	message:   LM_SETTOPINDEX				*/
   /*			   mp1 = MPFROMLONG(item);			*/
   /*			   mp2 = 0L					*/
   /*********************************************************************/

   case	LM_SETTOPINDEX :
		       /* Check	to see that items within the list box	*/
		       /* and that the item requested is within	the	*/
		       /* range	of items				*/

       if ( plbw->cItems && (LONGFROMMP(mp1) > 0) )
	   {
	   if (	LONGFROMMP(mp1)	< plbw->cItems	)
	       if ( plbw->cItems > plbw->cLinesPage )
		   if (	LONGFROMMP(mp1)	> (plbw->cItems	- plbw->cLinesPage) )
		       plbw->iVertScroll = plbw->cItems	- plbw->cLinesPage;
		   else
		       if ( plbw->cItems <= plbw->cLinesPage )
			   plbw->iVertScroll = 0L;
		       else
			   plbw->iVertScroll = (LONG)LONGFROMMP(mp1);
	       else
		   plbw->iVertScroll = 0L;
	   else
	       plbw->iVertScroll = 0L;

	   return(MRFROMLONG(TRUE));
	   }

       break;
   }
return(0L);
}
#pragma	subtitle("   List Manager - Extended List Box Messages Procedure")
#pragma	page( )

/* --- mrExtendedListHandler --------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	process	the extended messages for the	*/
/*     list box	control	window.						*/
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
/*     mrExtendedListHandler = Message Handling	Result			*/
/*									*/
/* --------------------------------------------------------------------	*/

MRESULT	EXPENTRY mrExtendedListHandler(HWND hWnd, ULONG	msg, MPARAM mp1, MPARAM	mp2)

{
HHEAPMEM     hHeap;		   /* Heap Handle			*/
PLISTITEM    albi;		   /* List Box Item Input Pointer	*/
PLISTBOXWIN  plbw;		   /* List Box Internal	Data Pointer	*/
PSZ	     *apsz;		   /* String Array Pointer		*/
PULONG	     aul;		   /* Handle Array Pointer		*/
register INT i,	n;		   /* Loop Counter			*/
EXCEPTIONREGISTRATIONRECORD xcptregr;	   /* Exception	Record		*/

plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

switch ( msg )
   {

   /*********************************************************************/
   /*  Extended	message:   LMX_ADDARRAY					*/
   /*			   mp1 = MPFROMLONG(cItems);			*/
   /*			   mp2 = MPFROMP(apsz);				*/
   /*********************************************************************/

   case	LMX_ADDARRAY :
       if ( (LONGFROMMP(mp1) > 0L) && PVOIDFROMMP(mp2) )
	   {
		       /* Check	to see if any items within the list box	*/
		       /* which	if there are none, there is nothing to	*/
		       /* delete and there is no memory	to release	*/

	   if (	plbw->cItems ==	0 )
	       {
		       /* Starting off the list	box from scratch,	*/
		       /* create the new heap that is to be used for	*/
		       /* list box					*/

	       hHeap = HeapAlloc(HALLOC_DEFAULT, HALLOC_DEFAULT);

		       /* Allocate the memory needed for the control	*/
		       /* information and save the heap	handle within	*/
		       /* it						*/

	       plbw->plc = (PLISTCOL)HeapMalloc(hHeap, sizeof(LISTCOL));

		       /* Allocate memory for the initial start	of the	*/
		       /* list.	 The list is allocated in groups of 8,	*/
		       /* hence	whenever the list total	is a multiple	*/
		       /* of 8,	the current list is full and needs to	*/
		       /* be expanded.					*/

	       plbw->plc[0].apli = (PLISTITEM *)HeapMalloc(plbw->plc[0].hHeap =	hHeap,
							   (((LONGFROMMP(mp1) /	CBLK_LIST) + 1UL) * CBLK_LIST) *
							   sizeof(PLISTITEM *));
	       }
	   else
	       plbw->plc[0].apli = (PLISTITEM *)HeapRealloc(plbw->plc[0].hHeap,	plbw->plc[0].apli,
							    ((((LONGFROMMP(mp1)	+ plbw->cItems)	/ CBLK_LIST) + 1UL) * CBLK_LIST) *
							    sizeof(PLISTITEM *));

		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

	   xcptregr.prev_structure   = NULL;
	   xcptregr.ExceptionHandler = &ListBoxExceptionHandler;

	   DosSetExceptionHandler(&xcptregr);

	   if (	setjmp(jBuf) )
	       {
	       HeapRelease(hHeap);

		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	       DosUnsetExceptionHandler(&xcptregr);
	       return(MRFROMLONG(TRUE));
	       }
		       /* Get the pointer to the item array		*/

	   apsz	= (PSZ *)PVOIDFROMMP(mp2);

		       /* Loop through the list	of items and place each	*/
		       /* one within the list box.  If the list	box	*/
		       /* contains any items, append the new items to	*/
		       /* the end of the list.				*/

	   for ( i = 0,	n = plbw->cItems; i < LONGFROMMP(mp1); i++, n++	)
	       {
	       plbw->plc[0].apli[n] = (PLISTITEM)HeapMalloc(hHeap, sizeof(LISTITEM));
	       if ( apsz[i] == NULL )
		   lSetItemText(plbw->plc[0].hHeap, hWnd, plbw->plc[0].apli[n],	"") ;
	       else
		   if (	(lSetItemText(plbw->plc[0].hHeap, hWnd,	plbw->plc[0].apli[n],
				      apsz[i]) > plbw->cxItem) && (plbw->flStyle & LS_HORZSCROLL) )
		       plbw->cxItem = plbw->plc[0].apli[n]->cxItem;
	       }
		       /* Save the list	box length and refresh the	*/
		       /* list box					*/

	   plbw->cItems	+= (LONG)LONGFROMMP(mp1);
	   RefreshList(plbw, 0L);

		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	   DosUnsetExceptionHandler(&xcptregr);
	   }
       else
	   return(MRFROMLONG(TRUE));
       break;

   /*********************************************************************/
   /*  Extended	message:   LMX_CALCSIZE					*/
   /*			   mp1 = MPFROMLONG(cLines);			*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	LMX_CALCSIZE :
       if ( LONGFROMMP(mp1) > 0L )
	   if (	plbw->flStyle &	LS_NOADJUSTPOS )
	       return(MRFROMLONG(MAKELONG(plbw->cx, plbw->cy)));
	   else
	       return(MRFROMLONG(MAKELONG(plbw->cx,
					  LONGFROMMP(mp1) * plbw->cyItem + 2L +
					  (plbw->flStyle & LS_HORZSCROLL ? plbw->cyScroll : 2L))));
       break;

   /*********************************************************************/
   /*  Extended	message:   LMX_CALCLINECOUNT				*/
   /*			   mp1 = MPFROMLONG(lHeight);			*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	LMX_CALCLINECOUNT :
       if ( LONGFROMMP(mp1) > 0L )
	   return(MRFROMLONG((LONGFROMMP(mp1) -	(2L +
			      (plbw->flStyle & LS_HORZSCROLL ? plbw->cyScroll :	2L))) /
			      plbw->cyItem));
       break;

   /*********************************************************************/
   /*  Extended	message:   LMX_SETITEMCOUNT				*/
   /*			   mp1 = MPFROMLONG(cItems);			*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	LMX_SETITEMCOUNT :
       if ( LONGFROMMP(mp1) > 0L )
	   {
		       /* Check	to see if any items within the list box	*/
		       /* which	if there are none, there is nothing to	*/
		       /* delete and there is no memory	to release	*/

	   if (	plbw->cItems )
	       {
		       /* Release the heap associated with the list box	*/
		       /* items	and set	the item count to zero		*/

	       HeapRelease(plbw->plc[0].hHeap);

	       plbw->cHorzScroll =
	       plbw->cVertScroll =
	       plbw->iHorzScroll =
	       plbw->iVertScroll = 0L;
	       plbw->cItems = 0UL;
	       plbw->iFocus = plbw->iSelected =	LIT_NONE;
	       }
		       /* Starting off the list	box from scratch,	*/
		       /* create the new heap that is to be used for	*/
		       /* list box					*/

	   hHeap = HeapAlloc(HALLOC_DEFAULT, HALLOC_DEFAULT);

		       /* Allocate the memory needed for the control	*/
		       /* information and save the heap	handle within	*/
		       /* it						*/

	   plbw->plc = (PLISTCOL)HeapMalloc(hHeap, sizeof(LISTCOL));

		       /* Allocate memory for the initial start	of the	*/
		       /* list.	 The list is allocated in groups of 8,	*/
		       /* hence	whenever the list total	is a multiple	*/
		       /* of 8,	the current list is full and needs to	*/
		       /* be expanded.					*/

	   plbw->plc[0].apli = (PLISTITEM *)HeapMalloc(plbw->plc[0].hHeap = hHeap,
						       (((LONGFROMMP(mp1) / CBLK_LIST) + 1UL) *	CBLK_LIST) *
						       sizeof(PLISTITEM));

	   for ( i = 0;	i < (INT)LONGFROMMP(mp1); i++ )
	       plbw->plc[0].apli[i] = (PLISTITEM)HeapMalloc(hHeap, sizeof(LISTITEM));

	   plbw->cItems	= (LONG)LONGFROMMP(mp1);
	   RefreshList(plbw, 0L);
	   }
       else
	   return(MRFROMLONG(TRUE));
       break;

   /*********************************************************************/
   /*  Extended	message:   LMX_SETARRAY					*/
   /*			   mp1 = MPFROMLONG(cItems);			*/
   /*			   mp2 = MPFROMP(apsz);				*/
   /*********************************************************************/

   case	LMX_SETARRAY :
       if ( (LONGFROMMP(mp1) > 0L) && PVOIDFROMMP(mp2) )
	   {
		       /* Check	to see if any items within the list box	*/
		       /* which	if there are none, there is nothing to	*/
		       /* delete and there is no memory	to release	*/

	   if (	plbw->cItems )
	       {
		       /* Release the heap associated with the list box	*/
		       /* items	and set	the item count to zero		*/

	       HeapRelease(plbw->plc[0].hHeap);

	       FocusChange(plbw, plbw->iFocus, FALSE);
	       plbw->iFocus = plbw->iSelected =	LIT_NONE;
	       plbw->cHorzScroll =
	       plbw->cVertScroll =
	       plbw->iHorzScroll =
	       plbw->iVertScroll = 0L;
	       plbw->cItems = 0UL;
	       }
		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

	   xcptregr.prev_structure   = NULL;
	   xcptregr.ExceptionHandler = &ListBoxExceptionHandler;

	   DosSetExceptionHandler(&xcptregr);

	   if (	setjmp(jBuf) )
	       {
	       HeapRelease(hHeap);

		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	       DosUnsetExceptionHandler(&xcptregr);
	       return(MRFROMLONG(TRUE));
	       }
		       /* Starting off the list	box from scratch,	*/
		       /* create the new heap that is to be used for	*/
		       /* list box					*/

	   hHeap = HeapAlloc(HALLOC_DEFAULT, HALLOC_DEFAULT);

		       /* Allocate the memory needed for the control	*/
		       /* information and save the heap	handle within	*/
		       /* it						*/

	   plbw->plc = (PLISTCOL)HeapMalloc(hHeap, sizeof(LISTCOL));

		       /* Allocate memory for the initial start	of the	*/
		       /* list.	 The list is allocated in groups of 8,	*/
		       /* hence	whenever the list total	is a multiple	*/
		       /* of 8,	the current list is full and needs to	*/
		       /* be expanded.					*/

	   plbw->plc[0].apli = (PLISTITEM *)HeapMalloc(plbw->plc[0].hHeap = hHeap,
						       (((LONGFROMMP(mp1) / CBLK_LIST) + 1UL) *	CBLK_LIST) *
						       sizeof(PLISTITEM	*));

		       /* Get the pointer to the item array		*/

	   apsz	= (PSZ *)PVOIDFROMMP(mp2);

		       /* Loop through the list	of items and place each	*/
		       /* one within the list box.  If the list	box	*/
		       /* contains any items, append the new items to	*/
		       /* the end of the list.				*/

	   for ( i = 0;	i < LONGFROMMP(mp1); i++ )
	       {
	       plbw->plc[0].apli[i] = (PLISTITEM)HeapMalloc(hHeap, sizeof(LISTITEM));
	       if ( apsz[i] == NULL )
		   lSetItemText(plbw->plc[0].hHeap, hWnd, plbw->plc[0].apli[i],	"") ;
	       else
		   if (	(lSetItemText(plbw->plc[0].hHeap, hWnd,	plbw->plc[0].apli[i],
				      apsz[i]) > plbw->cxItem) && (plbw->flStyle & LS_HORZSCROLL) )
		       plbw->cxItem = plbw->plc[0].apli[i]->cxItem;
	       }
		       /* Save the list	box length and refresh the	*/
		       /* list box					*/

	   plbw->cItems	+= (LONG)LONGFROMMP(mp1);
	   RefreshList(plbw, 0L);

		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	   DosUnsetExceptionHandler(&xcptregr);
	   }
       else
	   return(MRFROMLONG(TRUE));
       break;

   /*********************************************************************/
   /*  Extended	message:   LMX_SETARRAY					*/
   /*			   mp1 = MPFROMLONG(cItems);			*/
   /*			   mp2 = MPFROMP(aul);				*/
   /*********************************************************************/

   case	LMX_SETARRAYHANDLES :
       if ( (LONGFROMMP(mp1) > 0L) && PVOIDFROMMP(mp2) )
	   {
	   if (	LONGFROMMP(mp1)	> plbw->cItems )
	       return(MRFROMLONG(TRUE));

		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

	   xcptregr.prev_structure   = NULL;
	   xcptregr.ExceptionHandler = &ListBoxExceptionHandler;

	   DosSetExceptionHandler(&xcptregr);

	   if (	setjmp(jBuf) )
	       {
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	       DosUnsetExceptionHandler(&xcptregr);
	       return(MRFROMLONG(TRUE));
	       }
		       /* Get the pointer to the item array		*/

	   aul = (PULONG)PVOIDFROMMP(mp2);

		       /* Loop through the list	of items and place each	*/
		       /* one within the list box.  If the list	box	*/
		       /* contains any items, append the new items to	*/
		       /* the end of the list.				*/

	   for ( i = 0;	i < LONGFROMMP(mp1); i++ )
	       plbw->plc[0].apli[i]->ulHandle =	aul[i];

		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	   DosUnsetExceptionHandler(&xcptregr);
	   }
       else
	   return(MRFROMLONG(TRUE));
       break;

   /*********************************************************************/
   /*  Extended	message:   LMX_SETARRAYITEMS				*/
   /*			   mp1 = MPFROMLONG(cItems);			*/
   /*			   mp2 = MPFROMP(lbi);				*/
   /*********************************************************************/

   case	LMX_SETARRAYITEMS :
       if ( (LONGFROMMP(mp1) > 0L) && PVOIDFROMMP(mp2) )
	   {
		       /* Check	to see if any items within the list box	*/
		       /* which	if there are none, there is nothing to	*/
		       /* delete and there is no memory	to release	*/

	   if (	plbw->cItems )
	       {
		       /* Release the heap associated with the list box	*/
		       /* items	and set	the item count to zero		*/

	       HeapRelease(plbw->plc[0].hHeap);

	       FocusChange(plbw, plbw->iFocus, FALSE);
	       plbw->iFocus = plbw->iSelected =	LIT_NONE;
	       plbw->cHorzScroll =
	       plbw->cVertScroll =
	       plbw->iHorzScroll =
	       plbw->iVertScroll = 0L;
	       plbw->cItems = 0UL;
	       }
		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

	   xcptregr.prev_structure   = NULL;
	   xcptregr.ExceptionHandler = &ListBoxExceptionHandler;

	   DosSetExceptionHandler(&xcptregr);

	   if (	setjmp(jBuf) )
	       {
	       HeapRelease(hHeap);

		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	       DosUnsetExceptionHandler(&xcptregr);
	       return(MRFROMLONG(TRUE));
	       }
		       /* Starting off the list	box from scratch,	*/
		       /* create the new heap that is to be used for	*/
		       /* list box					*/

	   hHeap = HeapAlloc(HALLOC_DEFAULT, HALLOC_DEFAULT);

		       /* Allocate the memory needed for the control	*/
		       /* information and save the heap	handle within	*/
		       /* it						*/

	   plbw->plc = (PLISTCOL)HeapMalloc(hHeap, sizeof(LISTCOL));

		       /* Allocate memory for the initial start	of the	*/
		       /* list.	 The list is allocated in groups of 8,	*/
		       /* hence	whenever the list total	is a multiple	*/
		       /* of 8,	the current list is full and needs to	*/
		       /* be expanded.					*/

	   plbw->plc[0].apli = (PLISTITEM *)HeapMalloc(plbw->plc[0].hHeap = hHeap,
						       (((LONGFROMMP(mp1) / CBLK_LIST) + 1UL) *	CBLK_LIST) *
						       sizeof(PLISTITEM	*));

		       /* Get the pointer to the item array		*/

	   albi	= (PLISTITEM)PVOIDFROMMP(mp2);

		       /* Loop through the list	of items and place each	*/
		       /* one within the list box.  If the list	box	*/
		       /* contains any items, append the new items to	*/
		       /* the end of the list.				*/

	   for ( i = 0;	i < LONGFROMMP(mp1); i++ )
	       {
	       plbw->plc[0].apli[i] = (PLISTITEM)HeapMalloc(hHeap, sizeof(LISTITEM));

	       if ( albi[i].pszText == NULL )
		   lSetItemText(plbw->plc[0].hHeap, hWnd, plbw->plc[0].apli[i],	"") ;
	       else
		   if (	(lSetItemText(plbw->plc[0].hHeap, hWnd,	plbw->plc[0].apli[i],
				      albi[i].pszText) > plbw->cxItem) && (plbw->flStyle & LS_HORZSCROLL) )
		       plbw->cxItem = plbw->plc[0].apli[i]->cxItem;

	       plbw->plc[0].apli[i]->ulHandle =	albi[i].ulHandle;
	       }
		       /* Save the list	box length and refresh the	*/
		       /* list box					*/

	   plbw->cItems	+= (LONG)LONGFROMMP(mp1);
	   RefreshList(plbw, 0L);

		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	   DosUnsetExceptionHandler(&xcptregr);
	   }
       else
	   return(MRFROMLONG(TRUE));
       break;

   /*********************************************************************/
   /*  Extended	message:   LMX_SORT					*/
   /*			   mp1 = MPFROMLONG(lDirection);		*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	LMX_SORT :
       switch (	LONGFROMMP(mp1)	)
	   {
	   case	LIT_SORTASCENDING :
	       if ( plbw->cItems )

		       /* Sort the list	in ascending order		*/

		   SortAscending(plbw->plc[0].apli, 0, plbw->cItems - 1);
	       break;

	   case	LIT_SORTDESCENDING :
	       if ( plbw->cItems )

		       /* Sort the list	in descending order		*/

		   SortDescending(plbw->plc[0].apli, 0,	plbw->cItems - 1);
	       break;

	   default :
	       return(MRFROMLONG(TRUE));
	   }
       break;

   /*********************************************************************/
   /*  Extended	message:   LMX_SELECTALL				*/
   /*			   mp1 = 0L;					*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	LMX_SELECTALL :
       if ( plbw->cItems && (plbw->flStyle & (LS_MULTIPLESEL | LS_EXTENDEDSEL))	)
	   {
	   for ( i = 0;	i < plbw->cItems; i++ )
	       if ( (plbw->plc[0].apli[i]->fl &	LI_SELECTED) ==	0UL )
		   {
		   SaveSelectState(0L, i);
		   plbw->plc[0].apli[i]->fl |= LI_SELECTED;
		   }
	   mrNotifyOwner(plbw, LN_SELECT);
	   RefreshList(plbw, 0L);
	   }
       else
	   return(MRFROMLONG(TRUE));
       break;

   /*********************************************************************/
   /*  Extended	message:   LMX_QUERYITEMCOUNT				*/
   /*			   mp1 = 0L;					*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	LMX_QUERYITEMCOUNT :
       return(MRFROMLONG(plbw->cItems));
   }
return(0L);
}
#pragma	subtitle("   List Manager - Extended List Box Messages Procedure")
#pragma	page( )

/* --- mrChkBoxHandler --------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	process	the extended messages for the	*/
/*     list box	control	window check box support.			*/
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
/*     mrChkBoxHandler = Message Handling Result			*/
/*									*/
/* --------------------------------------------------------------------	*/

MRESULT	EXPENTRY mrChkBoxHandler(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)

{
PBOOL	     afl;		   /* Flags Array			*/
PLISTBOXWIN  plbw;		   /* List Box Internal	Data Pointer	*/
register INT i,	n;		   /* Loop Counter			*/
EXCEPTIONREGISTRATIONRECORD xcptregr;	   /* Exception	Record		*/

plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

switch ( msg )
   {

   /*********************************************************************/
   /*  Extended	message:   LMX_ADDARRAY					*/
   /*			   mp1 = MPFROMLONG(iItem);			*/
   /*			   mp2 = MPFROMLONG(fSelect);			*/
   /*********************************************************************/

   case	LMX_SETCHECK :
       if ( (LONG)LONGFROMMP(mp1) == LIT_NONE )
	   {
	   for ( i = 0;	i < plbw->cItems; i++ )
	       if (plbw->plc[0].apli[i]->fl & LI_CHECKED )
		   {
		   SaveSelectState(0L, i);
		   plbw->plc[0].apli[i]->fl &= ~LI_CHECKED;

		       /* Invert the rectangle of the entry to return	*/
		       /* it back to its normal	display	colour		*/

		   DrawSelection(plbw, i);
		   }
	   return(MRFROMLONG(TRUE));
	   }
       else
		       /* Check	to see that items within the list box	*/
		       /* and that the item requested is within	the	*/
		       /* range	of items				*/

	   if (	(LONG)LONGFROMMP(mp1) <	plbw->cItems )
	       {
	       if ( plbw->plc[0].apli[LONGFROMMP(mp1)]->fl & LI_CHECKED	)
		   plbw->plc[0].apli[LONGFROMMP(mp1)]->fl &= ~LI_CHECKED;
	       else
		   plbw->plc[0].apli[LONGFROMMP(mp1)]->fl |= LI_CHECKED;

		       /* Draw the updated item				*/

	       DrawSelection(plbw, (LONG)LONGFROMMP(mp1));
	       mrNotifyOwner(plbw, LNX_CHECKED);
	       return(MRFROMLONG(TRUE));
	       }
	   else
	       return(MRFROMLONG(FALSE));

   /*********************************************************************/
   /*  Extended	message:   LMX_QUERYCHECK				*/
   /*			   mp1 = MPFROMLONG(iItemPrev);			*/
   /*			   mp2 = 0L;					*/
   /*********************************************************************/

   case	LMX_QUERYCHECK :
       if ( plbw->cItems )
	   {
	   if (	(LONG)(SHORT)SHORT1FROMMP(mp1) == LIT_FIRST )
	       {
	       for ( i = 0; i <	plbw->cItems; i++ )
		   if (	plbw->plc[0].apli[i]->fl & LI_CHECKED )
		       return(MRFROMLONG(i));

	       return(MRFROMLONG(LIT_NONE));
	       }
	   else
	       if ( (LONG)(SHORT)SHORT1FROMMP(mp1) >= 0L )
		   {
		   for ( i = (LONG)(SHORT)SHORT1FROMMP(mp1) + 1; i < plbw->cItems; i++ )
		       if ( plbw->plc[0].apli[i]->fl & LI_CHECKED )
			   return(MRFROMLONG(i));

		   return(MRFROMLONG(LIT_NONE));
		   }
	       else
		   return(MRFROMLONG(LIT_NONE));
	   }
       else
	   return(MRFROMLONG(LIT_NONE));

   /*********************************************************************/
   /*  Extended	message:   LMX_SETCHECKARRAY				*/
   /*			   mp1 = MPFROMLONG(cItems);			*/
   /*			   mp2 = MPFROMP(afl);				*/
   /*********************************************************************/

   case	LMX_SETCHECKARRAY :
       if ( (LONGFROMMP(mp1) > 0L) && PVOIDFROMMP(mp2) )
	   {
		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

	   xcptregr.prev_structure   = NULL;
	   xcptregr.ExceptionHandler = &ListBoxExceptionHandler;

	   DosSetExceptionHandler(&xcptregr);

	   if (	setjmp(jBuf) )
	       {
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	       DosUnsetExceptionHandler(&xcptregr);
	       return(0L);
	       }
		       /* Get the pointer to the item array		*/

	   afl = (PBOOL)PVOIDFROMMP(mp2);

	   if (	plbw->cItems < LONGFROMMP(mp1) )
	       n = plbw->cItems;
	   else
	       n = (LONG)LONGFROMMP(mp1);

		       /* Loop through the list	of items and place each	*/
		       /* one within the list box.  If the list	box	*/
		       /* contains any items, append the new items to	*/
		       /* the end of the list.				*/

	   for ( i = 0;	i < n; i++ )
	       if ( afl[i] )
		   plbw->plc[0].apli[i]->fl |= LI_CHECKED;
	       else
		   plbw->plc[0].apli[i]->fl &= ~LI_CHECKED;

	   RefreshList(plbw, 0L);

		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	   DosUnsetExceptionHandler(&xcptregr);
	   return(MRFROMLONG(n));
	   }
       else
	   return(0L);

   /*********************************************************************/
   /*  Extended	message:   LMX_QUERYCHECKARRAY				*/
   /*			   mp1 = MPFROMLONG(cItems);			*/
   /*			   mp2 = MPFROMP(afl);				*/
   /*********************************************************************/

   case	LMX_QUERYCHECKARRAY :
       if ( (LONGFROMMP(mp1) > 0L) && PVOIDFROMMP(mp2) )
	   {
		       /* Register exception handler that is used for	*/
		       /* the lazy pointer validation tests		*/

	   xcptregr.prev_structure   = NULL;
	   xcptregr.ExceptionHandler = &ListBoxExceptionHandler;

	   DosSetExceptionHandler(&xcptregr);

	   if (	setjmp(jBuf) )
	       {
		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	       DosUnsetExceptionHandler(&xcptregr);
	       return(0L);
	       }
		       /* Get the pointer to the item array		*/

	   afl = (PBOOL)PVOIDFROMMP(mp2);

	   if (	plbw->cItems < LONGFROMMP(mp1) )
	       n = plbw->cItems;
	   else
	       n = (LONG)LONGFROMMP(mp1);

		       /* Loop through the list	of items and place each	*/
		       /* one within the list box.  If the list	box	*/
		       /* contains any items, append the new items to	*/
		       /* the end of the list.				*/

	   for ( i = 0;	i < n; i++ )
	       if ( plbw->plc[0].apli[i]->fl & LI_CHECKED )
		   afl[i] = TRUE;
	       else
		   afl[i] = FALSE;

		       /* Remove the exception handle since the	lazy	*/
		       /* pointer evaluation is	done			*/

	   DosUnsetExceptionHandler(&xcptregr);
	   return(MRFROMLONG(n));
	   }
       else
	   return(0L);
   }
return(0L);
}
#pragma	subtitle("   List Manager   - Exception Handler Routine")
#pragma	page( )

/* --- ListBoxExceptionHandler ------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used process exceptions	under 2.0 and to,	*/
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
/*     ListBoxExceptionHandler = Exception Handling Value		*/
/*									*/
/* --------------------------------------------------------------------	*/

APIRET APIENTRY	ListBoxExceptionHandler(PEXCEPTIONREPORTRECORD pxcptrepr,
					PEXCEPTIONREGISTRATIONRECORD pxcptregr,
					PCONTEXTRECORD pcr, PVOID sysinfo)

{
#pragma	info(nopar)

if ( EH_EXIT_UNWIND & pxcptrepr->fHandlerFlags )
   return(XCPT_CONTINUE_SEARCH);

if ( EH_UNWINDING & pxcptrepr->fHandlerFlags )
   return(XCPT_CONTINUE_SEARCH);

if ( EH_NESTED_CALL & pxcptrepr->fHandlerFlags )
   return(XCPT_CONTINUE_SEARCH);

if ( pxcptrepr->ExceptionNum ==	XCPT_ACCESS_VIOLATION )
   longjmp((INT	*)jBuf,	1);

return(XCPT_CONTINUE_SEARCH);
}
