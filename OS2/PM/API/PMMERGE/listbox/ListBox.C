#pragma	title("List Box Replacement  --  Version 1.2 -- (ListBox.C)")
#pragma	subtitle("   List Box Control - Interface Definitions")

/* Program name: Listbox.C    Title: A List Box	Replacement		*/
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

#define	INCL_GPI		   /* Include OS/2 PM GPI Interface	*/
#define	INCL_WIN		   /* Include OS/2 PM Windows Interface	*/

#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "entryfld.h"
#include "listbox.h"

/* This	module contains	the common routines used by the	list box along	*/
/* with	the window procedure for the list box.				*/
/*									*/
/* Equivalent command line invocation of each module using the		*/
/* IBM C Set++ Compiler	Version	2.0 is:					*/
/*									*/
/*     Icc -G3e- -O+ -Rn -C -W3	-FoListBox ListBox.C			*/

/* Filename:   ListBox.C						*/

/*  Version:   1.2							*/
/*  Created:   1993-10-14						*/
/*  Revised:   1994-07-05						*/

/* Routines:   BOOL fRegisterListBox(HAB hAB);				*/
/*	       VOID UpdateScrollBars(PLISTBOXWIN plbw);			*/
/*	       VOID DrawSelection(HWND hWnd, PRECTL prcl,		*/
/*				  PLISTBOXWIN plbw, LONG iItem);	*/
/*	       VOID DrawItem(PLISTBOXWIN plbw, LONG iItem, HPS hPS,	*/
/*			     PRECTL prcl, PPOINTL pptl);		*/
/*	       BOOL fItemVisible(PLISTBOXWIN plbw, LONG	iItem);		*/
/*	       LONG lItemFromPoint(PLISTBOXWIN plbw, LONG y);		*/
/*	       MRESULT EXPENTRY	ScrollBarWndProc(HWND hWnd, ULONG msg,	*/
/*						 MPARAM	mp1,		*/
/*						 MPARAM	mp2);		*/
/*	       MRESULT EXPENTRY	ListBoxWndProc(HWND hWnd, ULONG	msg,	*/
/*					       MPARAM mp1, MPARAM mp2);	*/


/* --------------------------------------------------------------------	*/

/************************************************************************/
/*									*/
/* Standard List Box Styles						*/
/*									*/
/*     LS_MULTIPLESEL	   0x00000001L					*/
/*     LS_OWNERDRAW	   0x00000002L					*/
/*     LS_NOADJUSTPOS	   0x00000004L					*/
/*     LS_HORZSCROLL	   0x00000008L					*/
/*     LS_EXTENDEDSEL	   0x00000010L					*/
/*									*/
/* Standard List Box notification messages				*/
/*									*/
/*     LN_SELECT		  1					*/
/*     LN_SETFOCUS		  2					*/
/*     LN_KILLFOCUS		  3					*/
/*     LN_SCROLL		  4					*/
/*     LN_ENTER			  5					*/
/*									*/
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

/************************************************************************/
/*									*/
/* Module Variable Definitions						*/
/*									*/
/************************************************************************/

HMODULE	hmodDLL;		   /* DLL Module Handle			*/

/************************************************************************/
/*									*/
/* Module Prototype Definitions						*/
/*									*/
/************************************************************************/

static VOID DrawItem(PLISTBOXWIN plbw, LONG iItem, HPS hPS, PRECTL prcl, PPOINTL pptl);

/************************************************************************/
/************************************************************************/
/*									*/
/* DLL Initialization Functions						*/
/*									*/
/************************************************************************/
/************************************************************************/

#if defined(__IBMC__) || defined(__IBMCPP__)

#pragma	subtitle("   List Box Control - DLL Initialization/Termination Procedure")
#pragma	page( )

/* --- _Dll_InitTerm ----------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	provide	the DLL	initialization and	*/
/*     termination.  The function is called by the C startup code	*/
/*     and allows the control to register itself and provide any	*/
/*     necessary startup.						*/
/*									*/
/*     This function is	designed for IBM C Set/2 Version 1.0 and	*/
/*     IBM C Set++ Version 2.x.						*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     ULONG hModule; =	DLL Module Handle				*/
/*     ULONG fl;      =	Startup	/ Termination Flag			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     _Dll_InitTerm =	0 : Error Return				*/
/*		     =	1 : Successful Startup / Termination		*/
/*									*/
/* --------------------------------------------------------------------	*/

ULONG _System _Dll_InitTerm(ULONG hModule, ULONG fl)

{
		       /* Determine if in startup or termination mode	*/
if ( fl	== 0 )
   {
		       /* DLL being initialized, save the DLL module	*/
		       /* handle to allow the bitmap loading routines	*/
		       /* routines a means of loading the default	*/
		       /* bitmaps when required				*/
   hmodDLL = hModule;
   }

return(1UL);
}

#else
#if defined(__WATCOMC__)

extern INT __hmodule;

#pragma	subtitle("   List Box Control - DLL Initialization Procedure")
#pragma	page( )

/* --- __dll_initialize	-------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	provide	the DLL	initialization and	*/
/*     termination.  The function is called by the C startup code	*/
/*     and allows the control to register itself and provide any	*/
/*     necessary startup.						*/
/*									*/
/*     This function is	designed for WATCOM C 386 Version 9.0 and for	*/
/*     WATCOM C/C++üý Version 9.5.					*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     Nothing								*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     __dll_initialize	=  0 : Error Return				*/
/*			=  1 : Successful Startup			*/
/*									*/
/* --------------------------------------------------------------------	*/

INT __dll_initialize(VOID)

{
		       /* DLL being initialized, save the DLL module	*/
		       /* handle to allow the bitmap loading routines	*/
		       /* routines a means of loading the default	*/
		       /* bitmaps when required				*/
hmodDLL	= __hmodule;

return(1);
}
#pragma	subtitle("   Image Button Control - DLL Initialization Procedure")
#pragma	page( )

/* --- __dll_terminate --------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	provide	the DLL	initialization and	*/
/*     termination.  The function is called by the C startup code	*/
/*     and allows the control to register itself and provide any	*/
/*     necessary startup.						*/
/*									*/
/*     This function is	designed for WATCOM C 386 Version 9.0 and for	*/
/*     WATCOM C/C++üý Version 9.5.					*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     Nothing								*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     __dll_terminate =  0 : Error Return				*/
/*		       =  1 : Successful Startup			*/
/*									*/
/* --------------------------------------------------------------------	*/

INT __dll_terminate(VOID)

{

return(1);

}

#else
#if defined(__BCPLUSPLUS__) || defined(__BORLANDC__)

#pragma	subtitle("   List Box Control - DLL Initialization/Termination Procedure")
#pragma	page( )

/* --- _dllmain	---------------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	provide	the DLL	initialization and	*/
/*     termination.  The function is called by the C startup code	*/
/*     and allows the control to register itself and provide any	*/
/*     necessary startup.						*/
/*									*/
/*     This function is	designed for Borland C++ for OS/2 Version 1.x.	*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     ULONG fl;      =	Startup	/ Termination Flag			*/
/*     ULONG hModule; =	DLL Module Handle				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     _dllmain	=  0 : Error Return					*/
/*		=  1 : Successful Startup / Termination			*/
/*									*/
/* --------------------------------------------------------------------	*/

ULONG _dllmain(ULONG fl, ULONG hModule)

{
		       /* Determine if in startup or termination mode	*/
if ( fl	== 0 )
   {
		       /* DLL being initialized, save the DLL module	*/
		       /* handle to allow the bitmap loading routines	*/
		       /* routines a means of loading the default	*/
		       /* bitmaps when required				*/
   hmodDLL = hModule;
   }

return(1UL);
}

#endif
#endif
#endif

/************************************************************************/
/************************************************************************/
/*									*/
/* Module Private Functions						*/
/*									*/
/************************************************************************/
/************************************************************************/

#pragma	subtitle("   List Box Control - Item Draw Routine")
#pragma	page( )

/* --- DrawItem	---------------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	draw a specified item within the list	*/
/*     box.  All drawing of items contained within the list box	MUST	*/
/*     use this	routine	since it insures that list boxes that are	*/
/*     owner drawn are properly	handled.				*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw;  = List Box Data Pointer			*/
/*     LONG	   iItem; = Item to Draw				*/
/*     HPS	   hPS;	  = Presentation Space Handle			*/
/*     PRECTL	   prcl;  = Clipping Rectangle				*/
/*     PPOINTL	   pptl;  = Starting Point				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

static VOID DrawItem(PLISTBOXWIN plbw, LONG iItem, HPS hPS, PRECTL prcl, PPOINTL pptl)

{
BOOL	  fDraw	= TRUE;		   /* Draw Item	Flag			*/
OWNERITEM oi;			   /* List Box Item Pointer		*/
POINTL	  ptl;			   /* Display Point Holder		*/
RECTL	  rcl;			   /* Display Rectangle			*/
LONG	  lPSid;		   /* PS ID				*/

if ( (plbw->plc[0].apli[iItem]->fl & LI_FOCUS) && plbw->fFocusShown )
   WinShowCursor(plbw->hWnd, FALSE);

		       /* Check	to see if the list box is owner	drawn	*/
		       /* in which case, the OWNERITEM package needs to	*/
		       /* be completed and sent	off to the list	box	*/
		       /* owner	to allow it to draw the	specified item	*/

if ( plbw->flStyle & LS_OWNERDRAW )
   {
		       /* Fill in the owner draw structure to before	*/
		       /* sending off the message to the owner of the	*/
		       /* list box					*/

   memset(&oi, 0, sizeof(OWNERITEM));
   oi.hwnd	     = plbw->hWnd;
   oi.hps	     = hPS;

   if (	plbw->plc[0].apli[iItem]->fl & LI_SELECTED )
       oi.fsState    = TRUE;

   if (	plbw->plc[0].apli[iItem]->fl & LI_SELECTEDPREV )
       {
       oi.fsStateOld = TRUE;
       plbw->plc[0].apli[iItem]->fl &= ~LI_SELECTEDPREV;
       }

   oi.rclItem	     = *prcl;
   --oi.rclItem.xRight;

   if (	plbw->iHorzScroll )
       oi.rclItem.xLeft	+= 1L -	(plbw->iHorzScroll * plbw->xChar);
   else
       oi.rclItem.xLeft	+= 1L;
   oi.idItem	     = iItem;
   oi.hItem	     = plbw->plc[0].apli[iItem]->ulHandle;

		       /* Clear	the display rectangle before giving the	*/
		       /* owner	item to	the list box owner		*/
   rcl = *prcl;
   rcl.xRight =	rcl.xLeft + 1L;
   WinFillRect(hPS, &rcl, plbw->lClrList);
   rcl.xLeft = prcl->xRight - 1L;
   rcl.xRight =	prcl->xRight;
   WinFillRect(hPS, &rcl, plbw->lClrList);

   lPSid = GpiSavePS(hPS);

		       /* Place	the colour table back into index mode	*/
		       /* for the owner	draw portion			*/

   GpiCreateLogColorTable(hPS, LCOL_RESET, LCOLF_INDRGB, 0L, 0L, (PLONG)NULL);
   GpiSetColor(hPS, plbw->lClrTextIndex);
   GpiSetBackColor(hPS,	plbw->lClrListIndex);

		       /* Send of the draw item	message	to the list box	*/
		       /* owner.  If the owner returns a value of FALSE	*/
		       /* the owner has	indicated that it wants	the	*/
		       /* drawing of the item to be performed by the	*/
		       /* list box.  A return value of TRUE indicates	*/
		       /* that the owner fully handled the drawing of	*/
		       /* the item.					*/

   if (	WinSendMsg(plbw->hwndOwner, WM_DRAWITEM,
		   MPFROMLONG(plbw->id), MPFROMP(&oi)) )
       {
       if ( oi.fsState )
	   {
	   rcl = *prcl;
	   ++rcl.xLeft;
	   --rcl.xRight;
	   WinInvertRect(hPS, &rcl);
	   }
       fDraw = FALSE;
       }
   GpiRestorePS(hPS, lPSid);

		       /* Reset	the colour back	into RGB mode		*/

   GpiCreateLogColorTable(hPS, 0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
   }

if ( fDraw )
   {
		       /* Depending on the selection status of the item	*/
		       /* being	drawn, set the colours for the item	*/

   if (	plbw->plc[0].apli[iItem]->fl & LI_SELECTED )
       {
		       /* Selected item	being drawn, use the		*/
		       /* appropriate colours for the selected item	*/

       GpiSetColor(hPS,	plbw->lClrHilite);
       GpiSetBackColor(hPS, plbw->lClrHiliteBackground);
       WinFillRect(hPS,	prcl, plbw->lClrHiliteBackground);
       }
   else
       {
		       /* Normal item being drawn, use the appropriate	*/
		       /* colours for the selected item			*/

       GpiSetColor(hPS,	plbw->lClrText);
       GpiSetBackColor(hPS, plbw->lClrList);
       WinFillRect(hPS,	prcl, plbw->lClrList);
       }

   if (	plbw->flStyleExt & LSXS_CHECKBOX )
       {
		       /* When a predefined point specified, the	*/
		       /* starting point for the text has been defined,	*/
		       /* therefore only need to draw the text in the	*/
		       /* position specified				*/
       if ( pptl )
	   ptl.x = pptl->x;
       else
		       /* No predefined	point specified, need to	*/
		       /* calculate the	starting point for the text	*/

	   ptl.x = prcl->xLeft + 2L;

       ptl.y = (((prcl->yTop - prcl->yBottom) /	2L) + prcl->yBottom) - plbw->cyHalfBitmap;
       WinDrawBitmap(hPS, plbw->hbm,
		     (plbw->plc[0].apli[iItem]->fl & LI_CHECKED) ? &plbw->rclChecked : &plbw->rclUnChecked,
		     &ptl, 0L, 0L, DBM_IMAGEATTRS | DBM_NORMAL);
       ptl.x +=	plbw->xTextOffset;
       ptl.y = prcl->yTop - plbw->yAscender;
       }
   else
		       /* When a predefined point specified, the	*/
		       /* starting point for the text has been defined,	*/
		       /* therefore only need to draw the text in the	*/
		       /* position specified				*/
       if ( pptl )
	   ptl = *pptl;
       else
		       /* No predefined	point specified, need to	*/
		       /* calculate the	starting point for the text	*/
	   {
	   ptl.x = prcl->xLeft + 2L;
	   ptl.y = prcl->yTop -	plbw->yAscender;
	   }
		       /* For the extreme right	edge of	the rectangle,	*/
		       /* clear	a 1 pel	verticle strip such that it	*/
		       /* provides visual separation between the item	*/
		       /* and the vertical scroll bar			*/
   rcl = *prcl;
   rcl.xLeft = prcl->xRight - 1L;
   WinFillRect(hPS, &rcl, plbw->lClrList);
   GpiCharStringPosAt(hPS, &ptl, prcl, CHS_CLIP,
		      (LONG)plbw->plc[0].apli[iItem]->cText,
		      plbw->plc[0].apli[iItem]->pszText, (PLONG)NULL);
   }
		       /* When the list	box item has focus, draw the	*/
		       /* focus	rectangle around the item		*/

if ( (plbw->plc[0].apli[iItem]->fl & LI_FOCUS) && plbw->fFocusShown )
   WinShowCursor(plbw->hWnd, TRUE);

plbw->plc[0].apli[iItem]->fl &=	~LI_SELECTEDPREV;

}
#pragma	subtitle("   List Box Control - Scroll Bar Procedure")
#pragma	page( )

/* --- ScrollBarWndProc	-------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	handle the button down message of the	*/
/*     subclassed scroll bar window to make sure that the focus	is	*/
/*     placed back to the list box in the proper manner.  The vertical	*/
/*     and horizontal scrolls of the list box are subclassed to	allow	*/
/*     the detection of	the mouse clicks which will be used to allow	*/
/*     the proper display of the focus indicator.			*/
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
/*     ScrollBarWndProc	= Message Handling Result			*/
/*									*/
/* --------------------------------------------------------------------	*/

MRESULT	EXPENTRY ScrollBarWndProc(HWND hWnd, ULONG msg,	MPARAM mp1, MPARAM mp2)

{
PFNWP pfnScrollBarProc;		   /* Original Scroll Bar Function	*/

switch ( msg )
   {

   /*********************************************************************/
   /*  Button click							*/
   /*********************************************************************/

   case	WM_BUTTON1DOWN :
   case	WM_BUTTON2DOWN :
   case	WM_BUTTON3DOWN :
       WinFocusChange(HWND_DESKTOP, WinQueryWindow(hWnd, QW_OWNER), 0UL);
       break;
   }
pfnScrollBarProc = (PFNWP)WinQueryWindowULong(hWnd, QWL_USER);
return((*pfnScrollBarProc)(hWnd, msg, mp1, mp2));
}

/************************************************************************/
/************************************************************************/
/*									*/
/* Module Public Functions						*/
/*									*/
/************************************************************************/
/************************************************************************/

#pragma	subtitle("   List Box Control - List Box Registration Procedure")
#pragma	page( )

/* --- fRegisterListBox	-------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	register the list box for the		*/
/*     application with	PM.						*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HAB hAB;	= Anchor Block Handle					*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     fRegisterListBox	=  TRUE	: List Box Registration	Successfull	*/
/*			= FALSE	: List Box Registration	Failed		*/
/*									*/
/* --------------------------------------------------------------------	*/

BOOL EXPENTRY fRegisterListBox(HAB hAB)

{
		       /* Register the main program window class	*/

if ( fRegisterEntryField(hAB) )
   if (	WinRegisterClass(hAB, "ListBoxWindow", (PFNWP)ListBoxWndProc,
			 CS_SYNCPAINT |	CS_SIZEREDRAW |	CS_PARENTCLIP |	CS_CLIPCHILDREN,
			 USER_RESERVED)	)
       return(TRUE);
   else
       return(FALSE);
else
   return(FALSE);
}
#pragma	subtitle("   List Box Control - Scroll Bar Update Procedure")
#pragma	page( )

/* --- UpdateScrollBars	-------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	update the scroll bars for the		*/
/*     list box	control.						*/
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

VOID UpdateScrollBars(PLISTBOXWIN plbw)

{
register INT n;			   /* Loop Counters			*/

		       /* Get the list box information pointer from	*/
		       /* the reserved memory of the window		*/

if ( plbw->cItems > plbw->cLinesPage )
   {
   WinEnableWindow(plbw->hwndScrollRight, TRUE);

		       /* Reset	the scroll bars	to take	into		*/
		       /* consideration	the visible number of lines	*/
		       /* now contained	within the window		*/

   WinSendMsg(plbw->hwndScrollRight, SBM_SETSCROLLBAR,
	      MPFROMSHORT(plbw->iVertScroll),
	      MPFROM2SHORT(0, (((n = plbw->cItems - plbw->cLinesPage) <	0) ?
			       (plbw->iVertScroll = plbw->cVertScroll =	0) :
			       (plbw->cVertScroll = n))));
   WinSendMsg(plbw->hwndScrollRight, SBM_SETTHUMBSIZE,
	      MPFROM2SHORT(plbw->cLinesPage, plbw->cItems), 0L);
   }
else
   {
   if (	plbw->iVertScroll )
       plbw->iVertScroll = 0;
   WinEnableWindow(plbw->hwndScrollRight, FALSE);
   }
		       /* Get the list box information pointer from	*/
		       /* the reserved memory of the window		*/

if ( plbw->flStyle & LS_HORZSCROLL )
   if (	plbw->cxItem > (plbw->rcl.xRight - plbw->rcl.xLeft) )
       {
       WinEnableWindow(plbw->hwndScrollBottom, TRUE);

		       /* Reset	the scroll bars	to take	into		*/
		       /* consideration	the visible number of lines	*/
		       /* now contained	within the window		*/

       WinSendMsg(plbw->hwndScrollBottom, SBM_SETSCROLLBAR,
		  MPFROMSHORT(plbw->iHorzScroll),
		  MPFROM2SHORT(0, (((n = ((plbw->cxItem	- (plbw->rcl.xRight - plbw->rcl.xLeft))	/ plbw->xChar +	1)) < 0) ?
				   (plbw->iHorzScroll =	plbw->iHorzScroll = 0) :
				   (plbw->cHorzScroll =	n))));
       WinSendMsg(plbw->hwndScrollBottom, SBM_SETTHUMBSIZE,
		  MPFROM2SHORT(plbw->cCharsPage, (plbw->cxItem / plbw->xChar) +	1), 0L);
       }
   else
       {
       if ( plbw->iHorzScroll )
	   plbw->iHorzScroll = 0;
       WinEnableWindow(plbw->hwndScrollBottom, FALSE);
       }

plbw->fDirty = FALSE;
}
#pragma	subtitle("   List Box Control - Selected Item Draw Routine")
#pragma	page( )

/* --- DrawSelection ----------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	draw the specified item	as a		*/
/*     selected	item within the	list box.				*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     HWND	   hWnd;  = List Box Window Handle			*/
/*     PRECTL	   prcl;  = Display and	Clipping Rectangle		*/
/*     PLISTBOXWIN plbw;  = List Box Data Pointer			*/
/*     LONG	   iItem; = Item to Check				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID DrawSelection(PLISTBOXWIN plbw, LONG iItem)

{
HPS    hPS;			   /* Presentation Space Handle		*/
POINTL ptl;			   /* Text Display Point		*/
RECTL  rcl;			   /* Display Rectangle			*/

		       /* Check	to see if the item requested is	visible	*/

if ( fItemVisible(plbw,	iItem) )

		       /* Get the presentation space that is to	be used	*/
		       /* to draw the text				*/

   if (	(hPS = WinGetPS(plbw->hWnd)) !=	(HPS)NULL )
       {
		       /* Current entry	viewable, calculate the	actual	*/
		       /* rectangle which the entry resides in		*/
       rcl = plbw->rcl;
       rcl.yTop	= plbw->rcl.yTop - (iItem - plbw->iVertScroll) *
		  plbw->cyItem;
       rcl.yBottom = rcl.yTop -	plbw->cyItem;

		       /* Form the starting point for the text		*/

       ptl.x = rcl.xLeft + 2L -	(plbw->iHorzScroll * plbw->xChar);
       ptl.y = rcl.yTop	- plbw->yAscender;

		       /* Set the colour table to RGB mode		*/

       GpiCreateLogColorTable(hPS, 0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);

		       /* Draw the text	of the list box	item		*/

       DrawItem(plbw, iItem, hPS, &rcl,	&ptl);

		       /* Release the presentation space		*/
       WinReleasePS(hPS);
       }
}
#pragma	subtitle("   Traffic Manager - Focus Draw Routine")
#pragma	page( )

/* --- DrawItemSelection ------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	draw the focus indicator around	a	*/
/*     list box	item.							*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw;  = List Box Data Pointer			*/
/*     LONG	   iItem; = Item to Check				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     Nothing								*/
/*									*/
/* --------------------------------------------------------------------	*/

VOID DrawItemSelection(PLISTBOXWIN plbw, LONG iItem)

{
HPS    hPS;			   /* Presentation Space Handle		*/
POINTL ptl;			   /* Text Display Point		*/
RECTL  rcl;			   /* Drawing Rectangle			*/

		       /* Get the presentation space for the list box	*/

if ( (hPS = WinGetPS(plbw->hWnd)) != (HPS)NULL )
   {
		       /* Form the display rectangle for the item	*/
   rcl = plbw->rcl;

   rcl.yBottom = (rcl.yTop = plbw->rcl.yTop - (iItem - plbw->iVertScroll) *
			     plbw->cyItem) - plbw->cyItem;

		       /* Form the display point for the text to be	*/
		       /* drawn						*/

   ptl.x = rcl.xLeft + 2L;
   ptl.y = rcl.yTop - plbw->yAscender;

		       /* Place	the colour table into RGB mode and draw	*/
		       /* the text item					*/

   GpiCreateLogColorTable(hPS, 0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);
   DrawItem(plbw, iItem, hPS, &rcl, &ptl);

		       /* Release the presentation space handle		*/
   WinReleasePS(hPS);
   }
}
#pragma	subtitle("   List Box Control - Item Visibility Routine")
#pragma	page( )

/* --- fItemVisible -----------------------------------	[ Private ] ---	*/
/*									*/
/*     This function is	used to	determine if the item is within	the	*/
/*     visible portion of the list box.					*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw;  = List Box Data Pointer			*/
/*     LONG	   iItem; = Item to Check				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     fItemVisible =  TRUE : Item is Visible				*/
/*		    = FALSE : Item is not Visible			*/
/*									*/
/* --------------------------------------------------------------------	*/

BOOL fItemVisible(PLISTBOXWIN plbw, LONG iItem)

{
		       /* Check	to see that the	item specified is	*/
		       /* currently visible within the list box.  An	*/
		       /* item is visible when it is within the	range	*/
		       /* of the top and bottom	lines that are		*/
		       /* displayed.					*/

if ( (iItem >= plbw->iVertScroll) &&
     (iItem < (plbw->iVertScroll + plbw->cLinesPage)) )

		       /* Item within the visible range, return	with	*/
		       /* visible flag					*/
   return(TRUE);
else
		       /* Item not within the visible range, return	*/
		       /* with hidden flag				*/
   return(FALSE);
}
#pragma	subtitle("   List Box Control - Descending Insert Item Locate Routine")
#pragma	page( )

/* --- lItemFromPoint ---------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	determine the index of the item	within	*/
/*     the list	box using a vertical point within the list box.	 The	*/
/*     calculation is based on the point falling within	a horizontal	*/
/*     band which is the height	of the item.  This initial index is	*/
/*     combined	with the scroll	bar position to	determine the actual	*/
/*     item index.  All	calculations are base on premise of the	first	*/
/*     item in the list	box is the topmost item.			*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PLISTBOXWIN plbw; = List	Box Internal Data Pointer		*/
/*     LONG	   y;	 = Vertical Point				*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     lItemFromPoint =	Index of Item					*/
/*									*/
/* --------------------------------------------------------------------	*/

LONG lItemFromPoint(PLISTBOXWIN	plbw, LONG y)

{
LONG iSelected;			   /* Selected Item Index		*/

		       /* Calculate the	item from the vertical point in	*/
		       /* the list box.	 The formula used is by	taking	*/
		       /* the current top index	position (which		*/
		       /* corresponds to the vertical scroll bar	*/
		       /* position) and	adding this to a value formed	*/
		       /* by subtracting the point from	the top	edge	*/
		       /* point, which results in the distance from the	*/
		       /* top edge and then taking this	value and	*/
		       /* dividing it by the height of each item which	*/
		       /* results in a zero based index	from the top	*/
		       /* item shown.					*/

		       /*						*/
		       /*	      plbw->rcl.yTop			*/
		       /*  ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄ¿		*/
		       /*  ³		   plbw->iVertScroll³^³		*/
		       /*  ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄ´		*/
		       /*  ³				    ³ ³		*/
		       /*  ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´ ³		*/
		       /*  ³		  ž y		    ³ ³		*/
		       /*  ÃÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄ´		*/
		       /*  ³		       plbw->cyItem ³v³		*/
		       /*  ÃÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÅÄÙ		*/
		       /*  ³<³				  ³>³		*/
		       /*  ÀÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÙ		*/

iSelected = (plbw->rcl.yTop - y) / plbw->cyItem	+ plbw->iVertScroll;

		       /* When the point is on a boundary between two	*/
		       /* items	(the height of the item	divides	evenly	*/
		       /* in point), need to adjust the	index value to	*/
		       /* apply	to the item before it			*/

return((LONG)((plbw->rcl.yTop -	y) % plbw->cyItem ? iSelected :	iSelected - 1L));
}
#pragma	subtitle("   List Box Control - List Box Window Procedure")
#pragma	page( )

/* --- ListBoxWndProc ---------------------------------- [ Public ] ---	*/
/*									*/
/*     This function is	used to	process	the messages for the list box	*/
/*     control window.							*/
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
/*     ListBoxWndProc =	Message	Handling Result				*/
/*									*/
/* --------------------------------------------------------------------	*/

MRESULT	EXPENTRY ListBoxWndProc(HWND hWnd, ULONG msg, MPARAM mp1, MPARAM mp2)

{
BITMAPINFOHEADER bmp;		   /* Bitmap Information Holder		*/
BOOL	      fItemSelect;	   /* Select Item Flag			*/
BOOL	      fItemValid;	   /* Valid Item Flag			*/
BOOL	      fScroll;		   /* Scroll List Flag			*/
FONTMETRICS   fm;		   /* Font Metrics			*/
HHEAPMEM      hHeap;		   /* Heap Handle			*/
HPS	      hPS;		   /* Presentation Space Handle		*/
HWND	      hwndHelp;		   /* Help Window Handle		*/
PCREATESTRUCT pcrst;		   /* Create Structure Pointer		*/
PLISTBOXCDATA plbwcd;		   /* List Box Window Data Pointer	*/
PLISTBOXWIN   plbw;		   /* List Box Internal	Data Pointer	*/
POINTL	      ptl;		   /* Display Point			*/
POINTL	      rgptl[TXTBOX_COUNT]; /* Text Box Point Array		*/
PSWP	      pswp;		   /* Size Window Position Pointer	*/
PSZ	      pszText;		   /* Temporary	Text Pointer		*/
RECTL	      rcl;		   /* Display Rectangle			*/
RECTL	      rclPaint;		   /* Display Rectangle			*/
ULONG	      fl;		   /* Flags				*/
ULONG	      flStyle;		   /* Style Holder			*/
ULONG	      ulID;		   /* Presentation Parameter ID		*/
register INT i,	n;		   /* Loop Counter			*/

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
		       /* as well as a small heap			*/

       plbw = (PLISTBOXWIN)HeapMalloc(hHeap = HeapAlloc(4096UL,	4096UL), sizeof(LISTBOXWIN));
       plbw->hHeap = hHeap;

		       /* Save the address of the internal control data	*/
		       /* in the control's reserved memory to allow it  */
		       /* to be	referenced as required by the control	*/

       WinSetWindowPtr(hWnd, QUCWP_WNDP, (PVOID)plbw);

		       /* Get the control's creation structure address  */
		       /* to copy the relevant information such	as the	*/
		       /* size,	position and text of the control into	*/
		       /* the internal control data			*/

       pcrst = (PCREATESTRUCT)PVOIDFROMMP(mp2);

		       /* Save the owner and parent of the control so	*/
		       /* notification messages	can be sent back to	*/
		       /* the proper locations within the owning	*/
		       /* application					*/

       plbw->hAB	= WinQueryAnchorBlock(hWnd);
       plbw->hWnd	= hWnd;
       plbw->hwndOwner	= pcrst->hwndOwner;
       plbw->hwndParent	= pcrst->hwndParent;
       plbw->id		= pcrst->id;

		       /* Save the size	of the list box	along with the	*/
		       /* current style					*/

       plbw->cxWindow	= pcrst->cx;
       plbw->cyWindow	= pcrst->cy;
       plbw->flStyle	= pcrst->flStyle;

		       /* Set the current selected index		*/

       plbw->iFocus = plbw->iSelected =	LIT_NONE;

		       /* Set column count				*/

       plbw->cColumns =	1L;

		       /* Get the default colours for the list box	*/

       plbw->lClrText		  = lGetPresParam(hWnd,	PP_FOREGROUNDCOLOR,
						  PP_FOREGROUNDCOLORINDEX,
						  SYSCLR_OUTPUTTEXT);
       plbw->lClrList		  = lGetPresParam(hWnd,	PP_BACKGROUNDCOLOR,
						  PP_BACKGROUNDCOLORINDEX,
						  SYSCLR_ENTRYFIELD);
       plbw->lClrBorder		  = lGetPresParam(hWnd,	PP_BORDERCOLOR,
						  PP_BORDERCOLORINDEX,
						  SYSCLR_BUTTONDARK);
       plbw->lClrBackground	  = lGetPresParam(hWnd,	PP_BACKGROUNDCOLOR,
						  PP_BACKGROUNDCOLORINDEX,
						  SYSCLR_FIELDBACKGROUND);
       plbw->lClrHilite		  = lGetPresParam(hWnd,	PP_HILITEFOREGROUNDCOLOR,
						  PP_HILITEFOREGROUNDCOLORINDEX,
						  SYSCLR_HILITEFOREGROUND);
       plbw->lClrHiliteBackground = lGetPresParam(hWnd,	PP_HILITEBACKGROUNDCOLOR,
						  PP_HILITEBACKGROUNDCOLORINDEX,
						  SYSCLR_HILITEBACKGROUND);
       if ( !WinQueryPresParam(hWnd, PP_FOREGROUNDCOLORINDEX, 0UL, &ulID, sizeof(LONG),	(PVOID)&plbw->lClrTextIndex,
			       QPF_NOINHERIT | QPF_ID1COLORINDEX) )
	   plbw->lClrTextIndex = SYSCLR_OUTPUTTEXT;
       if ( !WinQueryPresParam(hWnd, PP_BACKGROUNDCOLORINDEX, 0UL, &ulID, sizeof(LONG),	(PVOID)&plbw->lClrListIndex,
			       QPF_NOINHERIT | QPF_ID1COLORINDEX) )
	   plbw->lClrListIndex = SYSCLR_ENTRYFIELD;

		       /* Determine the	size of	the scroll bars		*/

       plbw->cxScroll =	WinQuerySysValue(HWND_DESKTOP, SV_CXVSCROLL);
       plbw->cyScroll =	WinQuerySysValue(HWND_DESKTOP, SV_CYHSCROLL);
       plbw->hptr     =	WinQuerySysPointer(HWND_DESKTOP, SPTR_ARROW, FALSE);

		       /* Get the font metrics to determine the	height	*/
		       /* and width of a character			*/

       GpiQueryFontMetrics(hPS = WinGetPS(hWnd), sizeof(FONTMETRICS), &fm);
       plbw->hbm = GpiLoadBitmap(hPS, hmodDLL, IDB_CHECKMARKS, 0L, 0L);

       WinReleasePS(hPS);

       GpiQueryBitmapParameters(plbw->hbm, &bmp);

       plbw->rclChecked.xRight = bmp.cx	- (bmp.cx / 4L);
       plbw->rclChecked.xLeft  = bmp.cx	/ 2L;

       plbw->rclUnChecked.xRight = bmp.cx / 4L;
       plbw->rclChecked.yTop = plbw->rclUnChecked.yTop = bmp.cy	/ 2L;
       plbw->cyHalfBitmap = bmp.cy / 4L;

       plbw->xTextOffset = (bmp.cx / 4L) + fm.lAveCharWidth / 2L;

       plbw->yAscender = fm.lMaxAscender;
       plbw->xChar     = fm.lAveCharWidth;

		       /* Create the vertical scroll bar that is placed	*/
		       /* on the right side of the list	box		*/

       if ( (plbw->hwndScrollRight = WinCreateWindow(hWnd, WC_SCROLLBAR, (PSZ)NULL,
						     SBS_VERT |	WS_VISIBLE,
						     pcrst->cx - plbw->cxScroll, 0L,
						     plbw->cxScroll, pcrst->cy,
						     hWnd, HWND_TOP,
						     0x0000c001UL, (PVOID)NULL,
						     (PVOID)NULL)) != (HWND)NULL )
	   {
		       /* Sub-class the	scroll bar to allow the		*/
		       /* monitoring of	the button up events which will	*/
		       /* allow	the proper refocusing to the list box	*/

	   WinSetWindowULong(plbw->hwndScrollRight, QWL_USER,
			     (ULONG)WinSubclassWindow(plbw->hwndScrollRight, (PFNWP)ScrollBarWndProc));	
	   WinQueryWindowPos(plbw->hwndScrollRight, &plbw->aswp[SWP_VERT]);
	   WinEnableWindow(plbw->hwndScrollRight, FALSE);
	   }
       else
	   return(MRFROMLONG(TRUE));

		       /* When a horizontal scroll bar is requested,	*/
		       /* create the horizontal	scroll bar at the	*/
		       /* bottom of the	list box			*/

       if ( (pcrst->flStyle & LS_HORZSCROLL) &&	!fAddHorzScroll(hWnd, plbw) )
	   return(MRFROMLONG(TRUE));

		       /* Check	to see if the list box is owner	drawn	*/
		       /* in which case	need to	get the	item height to	*/
		       /* allow	for the	proper calculation of the	*/
		       /* drawing rectangle that will be passed	to the	*/
		       /* owner	when an	item needs to be drawn		*/

       MeasureItem(plbw, fm.lMaxBaselineExt);

		       /* Calculate the	placement of the various	*/
		       /* elements of the list box			*/

       if ( (pcrst->cx > 0L) &&	(pcrst->cy > 0L) )
	   SizeListBox(plbw);

		       /* Try to load the sound	support	DLL which is	*/
		       /* used to interface with the MMPM/2 sound	*/
		       /* system					*/

       LoadSoundSupport(plbw);

		       /* Get the address of the CTLDATA structure that	*/
		       /* may contain the listbox items	that the	*/
		       /* control can use during its creation instead	*/
		       /* of using messages to set the listbox items	*/

       if ( (plbwcd = (PLISTBOXCDATA)PVOIDFROMMP(mp1)) != NULL )
	   if (	plbwcd->ulVersion == LBV_100 )
	       SetControlDataList(plbw,	plbwcd->vdata.lbcd1_0.cItems, plbwcd->vdata.lbcd1_0.abList);
	   else
	       if ( plbwcd->ulVersion == LBV_110 )
		   {
		   SetControlDataSounds(plbw, plbwcd->vdata.lbcd1_1.cItems,
					plbwcd->vdata.lbcd1_1.cSounds,
					plbwcd->vdata.lbcd1_1.abList);

		   if (	plbwcd->vdata.lbcd1_1.flExtStyles )
		       if ( (plbw->flStyleExt =	plbwcd->vdata.lbcd1_1.flExtStyles) & LSXS_CHECKBOX )
			   plbw->fEnableCheckboxes = TRUE;
		   }
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
		       /* Get the list box information pointer from	*/
		       /* the reserved memory of the window		*/

       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* When the window is being activated or		*/
		       /* deactivated, make sure that the control	*/
		       /* responds properly to help requests when it	*/
		       /* has the focus					*/

       if ( SHORT1FROMMP(mp2) )
	   {
		       /* Notify the owner of the list box that	that	*/
		       /* list box is gaining focus			*/

	   mrNotifyOwner(plbw, LN_SETFOCUS);

	   if (	plbw->cItems )
	       {
		       /* When the focus is being received and none of	*/
		       /* the list box items has previously received	*/
		       /* the focus, set the first item	in the list box	*/
		       /* as receiving the focus			*/

	       if ( !plbw->fFocus )
		   {
		   if (	plbw->iFocus ==	LIT_NONE )
		       plbw->iFocus = 0L;

		   WinCreateCursor(plbw->hWnd, plbw->rcl.xLeft,
				   (plbw->rcl.yTop - (plbw->iFocus - plbw->iVertScroll)	*
						      plbw->cyItem) - plbw->cyItem,
				   plbw->rcl.xRight - 1L - plbw->rcl.xLeft, plbw->cyItem,
				   CURSOR_FRAME	| CURSOR_HALFTONE, NULL);
		   }
		       /* Show the focus indicator on the item that is	*/
		       /* deemed to have the focus			*/

	       if ( !plbw->fFocusShown )
		   FocusChange(plbw, plbw->iFocus, TRUE);
	       }
	   else
	       if ( !plbw->fFocus )
		   WinCreateCursor(plbw->hWnd, 0L, 0L,
				   plbw->rcl.xRight - 1L - plbw->rcl.xLeft, plbw->cyItem,
				   CURSOR_FRAME	| CURSOR_HALFTONE, NULL);

		       /* Set the focus	flag				*/

	   plbw->fFocus	= TRUE;
	   }
       else
		       /* Focus	being lost, only update	internal flags	*/
		       /* if the control has received focus previously	*/

	   if (	plbw->fFocus )
	       {
		       /* Notify the owner of the list box that	that	*/
		       /* list box is losing focus			*/

	       mrNotifyOwner(plbw, LN_KILLFOCUS);

		       /* Remove the focus indicator if	one was	being	*/
		       /* used						*/

	       if ( plbw->cItems )
		   RemoveFocus(plbw);
	       WinDestroyCursor(hWnd);

		       /* Clear	the focus flag.	 NOTE:	the flag MUST	*/
		       /* NOT be cleared before	the focus indicator is	*/
		       /* removed since	the RemoveFocus	routine	checks	*/
		       /* to see if the	control	has focus and will only	*/
		       /* remove the indicator if the control does	*/
		       /* indeed have focus.				*/

	       plbw->fFocus = plbw->fFocusShown	= FALSE;
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
	   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
	   plbw->cxWindow = pswp->cx;
	   plbw->cyWindow = pswp->cy;
	   SizeListBox(plbw);
	   if (	plbw->fFocus )
	       {
	       WinCreateCursor(plbw->hWnd, 0L, 0L,
			       plbw->rcl.xRight	- 1L - plbw->rcl.xLeft,	plbw->cyItem,
			       CURSOR_FRAME | CURSOR_HALFTONE, NULL);
	       WinCreateCursor(plbw->hWnd, plbw->rcl.xLeft,
			       (plbw->rcl.yTop - (plbw->iFocus - plbw->iVertScroll) *
						  plbw->cyItem)	- plbw->cyItem,
			       plbw->rcl.xRight	- 1L - plbw->rcl.xLeft,	plbw->cyItem,
			       CURSOR_FRAME | CURSOR_HALFTONE, NULL);
	       }
		       /* Reset	the scroll bars	to take	into		*/
		       /* consideration	the visible number of lines	*/
		       /* now contained	within the window		*/

	   UpdateScrollBars(plbw);
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

       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
       plbw->cxWindow =	(LONG)(SHORT)SHORT1FROMMP(mp2);
       plbw->cyWindow =	(LONG)(SHORT)SHORT2FROMMP(mp2);
       SizeListBox(plbw);
       if ( plbw->fFocus )
	   {
	   WinDestroyCursor(plbw->hWnd);
	   WinCreateCursor(plbw->hWnd, plbw->rcl.xLeft,
			   (plbw->rcl.yTop - (plbw->iFocus - plbw->iVertScroll)	*
					      plbw->cyItem) - plbw->cyItem,
			   plbw->rcl.xRight - 1L - plbw->rcl.xLeft, plbw->cyItem,
			   CURSOR_FRAME	| CURSOR_HALFTONE, NULL);
	   }
		       /* Reset	the scroll bars	to take	into		*/
		       /* consideration	the visible number of lines	*/
		       /* now contained	within the window		*/

       UpdateScrollBars(plbw);
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
		       /* Get the list box information pointer from	*/
		       /* the reserved memory of the window		*/

       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Make sure that the list box has focus		*/

       if ( !plbw->fFocus )
	   WinSetFocus(HWND_DESKTOP, hWnd);

		       /* Check	to see that there are items contained	*/
		       /* within the list box				*/

       if ( plbw->cItems )
	   {
	   if (	(plbw->flStyleExt & LSXS_EDITABLE) &&
		((ULONG)WinGetKeyState(HWND_DESKTOP, VK_ALT) & 0x00008000UL) )
	       if ( fCreateEditArea(plbw, mp1) )
		   return(MRFROMLONG(TRUE));

		       /* Save the extended information	for undo	*/

	   SaveExtendedState(plbw);

	   if (	plbw->flStyleExt & LSXS_CHECKBOX )
	       if ( SHORT1FROMMP(mp1) <	plbw->xTextOffset )
		   if (	fDecodeChkSelect(plbw, mp1) )
		       {
		       /* Capture the mouse to allow proper tracking of	*/
		       /* group	selections where the mouse is dragged	*/
		       /* over more than one item			*/

		       WinSetCapture(HWND_DESKTOP, hWnd);
		       plbw->fCapture =	TRUE;

		       /* Play any associated sounds for the double	*/
		       /* click	event					*/

		    /* PlaySound(plbw, LSND_SINGLECLICK); */
		       return(MRFROMLONG(TRUE));
		       }

		       /* Check	to see if the list box is using	the	*/
		       /* extended selection interface in which	case	*/
		       /* need to check	for extended selection mouse	*/
		       /* clicks					*/

	   if (	plbw->flStyle &	LS_EXTENDEDSEL )
	       {
		       /* Check	for Shift+Button 1 selection which is	*/
		       /* used to select all items from	the anchor	*/
		       /* point	to the item selected			*/

	       if ( (ULONG)WinGetKeyState(HWND_DESKTOP,	VK_SHIFT) & 0x00008000UL )
		   fItemValid =	fDecodeExtendedMouse(plbw, mp1,	TRUE);
	       else
		       /* Check	for Ctrl+Button	1 selection which is	*/
		       /* used to select or deselect an	item selected	*/

		   if (	(ULONG)WinGetKeyState(HWND_DESKTOP, VK_CTRL) & 0x00008000UL )
		       {
		       if ( (fItemValid	= fDecodeExtendedMouse(plbw, mp1, FALSE)) == TRUE )

		       /* Check	to see if the item was previously	*/
		       /* selected and if not, set this	item as	the	*/
		       /* new anchor point				*/

			   if (	!(plbw->plc[0].apli[plbw->iFocus]->fl &	LI_SELECTEDOLD)	)
			       plbw->iAnchor = plbw->iFocus;
		       }
		   else
		       /* No extended keyboard selection being used,	*/
		       /* process the click such that when the mouse is	*/
		       /* over a selected item,	only the focus changes	*/
		       /* whereas when it is over an unselected	item,	*/
		       /* all items that were selected are unselected	*/
		       /* the the new item is the only item selected	*/

		       if ( (fItemValid	= fDecodeMouse(plbw, mp1)) == TRUE )
			   if (	!(plbw->plc[0].apli[plbw->iFocus]->fl &	LI_SELECTEDOLD)	)

		       /* Check	to see if the item was previously	*/
		       /* selected and if not, set this	item as	the	*/
		       /* new anchor point				*/

			       plbw->iAnchor = plbw->iFocus;
	       }
	   else
		       /* No extended keyboard selection being used,	*/
		       /* process the click such that when the mouse is	*/
		       /* over a selected item,	only the focus changes	*/
		       /* whereas when it is over an unselected	item,	*/
		       /* all items that were selected are unselected	*/
		       /* the the new item is the only item selected	*/

	       fItemValid = fDecodeMouse(plbw, mp1);

		       /* Check	to see if the item was valid in	which	*/
		       /* case need to set the mouse capture for the	*/
		       /* list box to insure that all mouse movements	*/
		       /* are properly processed until the user		*/
		       /* releases button 1 on the mouse		*/

	   if (	fItemValid )
	       {
		       /* Capture the mouse to allow proper tracking of	*/
		       /* group	selections where the mouse is dragged	*/
		       /* over more than one item			*/

	       WinSetCapture(HWND_DESKTOP, hWnd);
	       plbw->fCapture =	TRUE;

		       /* Play any associated sounds for the double	*/
		       /* click	event					*/

	       PlaySound(plbw, LSND_SINGLECLICK);
	       }
	   }
       return(MRFROMLONG(TRUE));

   /*********************************************************************/
   /*  Button 1	double click						*/
   /*********************************************************************/

   case	WM_BUTTON1DBLCLK :
		       /* Get the list box information pointer from	*/
		       /* the reserved memory of the window		*/

       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see that there are items contained	*/
		       /* within the list box				*/

       if ( plbw->cItems )
	   {
		       /* Check	to see if the mouse point within the	*/
		       /* list box item	area				*/

	   ptl.x = (LONG)SHORT1FROMMP(mp1);
	   ptl.y = (LONG)SHORT2FROMMP(mp1);
	   if (	WinPtInRect(plbw->hAB, &plbw->rcl, &ptl) )

		       /* Point	clicked	is within the list box item	*/
		       /* area,	now check to see that the point	is over	*/
		       /* a valid item					*/

	       if ( lItemFromPoint(plbw, (LONG)SHORT2FROMMP(mp1)) >= plbw->cItems )

		       /* Point	clicked	on does	not contain a valid	*/
		       /* entry, make sure that	the item is deselected	*/

		   return(MRFROMLONG(TRUE));
	       else
		   {
		       /* Point	clicked	over a valid item, set the	*/
		       /* double click flag which will be checked when	*/
		       /* the final mouse processing (button 1 up)	*/
		       /* determines type of notification that should	*/
		       /* be sent to the owning	window			*/

		   plbw->fDblClk = TRUE;

		       /* Play any associated sounds for the double	*/
		       /* click	event					*/

		   PlaySound(plbw, LSND_DOUBLECLICK);
		   }
	       }
       return(MRFROMLONG(TRUE));

   /*********************************************************************/
   /*  Button 1	released						*/
   /*********************************************************************/

   case	WM_BUTTON1UP :
		       /* Get the list box information pointer from	*/
		       /* the reserved memory of the window		*/

       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see that there are items contained	*/
		       /* within the list box				*/

       if ( plbw->cItems )
	   {

		       /* Check	to see if the capture is currently	*/
		       /* active					*/

	   if (	WinQueryCapture(HWND_DESKTOP) == hWnd )
	       {
		       /* Capture still	active,	release	the mouse	*/
		       /* capture and clear the	internal capture flag	*/

	       WinSetCapture(HWND_DESKTOP, (HWND)NULL);
	       plbw->fCapture =	FALSE;
	       }
		       /* Check	to see if the timer active in which	*/
		       /* case need to turn it off			*/

	   if (	plbw->fTimer )
	       {
	       plbw->fTimer = FALSE;
	       WinStopTimer(plbw->hAB, hWnd, TID_SCROLL);
	       }
		       /* Check	to see if the mouse was	double clicked	*/
		       /* which	means that the owner should be notified	*/
		       /* using	the appropriate	notification message	*/

	   if (	plbw->fDblClk )
	       {
	       mrNotifyOwner(plbw, LN_ENTER);
	       plbw->fDblClk = FALSE;
	       }
	   }
       return(MRFROMLONG(TRUE));

   /*********************************************************************/
   /*  Button 2	clicked							*/
   /*  Button 3	clicked							*/
   /*********************************************************************/

   case	WM_BUTTON2DOWN :
   case	WM_BUTTON3DOWN :
       WinSetActiveWindow(HWND_DESKTOP,	hWnd);
       break;

#define	DEBUG_LISTBOX

#if defined(DEBUG_LISTBOX)

   case	WM_BUTTON2DBLCLK :
       {
       PSZ pszBuffer;		   /* Buffer Pointer			*/

		       /* Get the list box information pointer from	*/
		       /* the reserved memory of the window		*/

       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
       DosAllocMem((PPVOID)(PVOID)&pszBuffer, 4096UL, PAG_READ | PAG_WRITE | PAG_COMMIT);
       sprintf(pszBuffer, "iFocus = %d\niSelected = %d\nfFocus = %d\nfFocusShown = %d\nCursor Level = %d",
			  plbw->iFocus,	plbw->iSelected, plbw->fFocus, plbw->fFocusShown,
			  WinQuerySysValue(HWND_DESKTOP, SV_CURSORLEVEL));
       WinMessageBox(HWND_DESKTOP, HWND_DESKTOP, pszBuffer, "List Box Data", 0UL,
		     MB_OK);
       DosFreeMem((PVOID)pszBuffer);
       HeapDisplayStatus(plbw->plc[0].hHeap);
       }
       break;
#endif

   /*********************************************************************/
   /* Process mouse movement over the list box				*/
   /*********************************************************************/

   case	WM_MOUSEMOVE :
		       /* Get the list box information pointer from	*/
		       /* the reserved memory of the window		*/

       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see if the mouse is in capture mode	*/
		       /* which	means that the user has	not released	*/
		       /* Button 1 yet and is probably in a drag	*/
		       /* operation					*/

       if ( (WinQueryCapture(HWND_DESKTOP) == hWnd) && plbw->fCapture )

		       /* User dragging	the mouse while	pressing Button	*/
		       /* 1, check to see if the mouse pointer inside	*/
		       /* or outside the list box area and where	*/
		       /* appropriate, change the item selected	based	*/
		       /* on that position				*/

	   if (	(LONG)(SHORT)SHORT2FROMMP(mp1) > plbw->rcl.yTop	)
	       {
		       /* User has dragged the mouse pointer above the	*/
		       /* top item of the list box, move up a line and	*/
		       /* start	the scroll timer if it isn't already    */
		       /* running					*/

	       DragUp(plbw);
	       if ( !plbw->fTimer )
		   {
		   plbw->fTimer	= TRUE;
		   WinStartTimer(plbw->hAB, hWnd, TID_SCROLL, 50UL);
		   }
	       }
	   else
	       if ( (LONG)(SHORT)SHORT2FROMMP(mp1) < plbw->rcl.yBottom )
		   {
		       /* User has dragged the mouse pointer below the	*/
		       /* bottom item of the list box, move up a line	*/
		       /* and start the	scroll timer if	it isn't        */
		       /* already running				*/

		   DragDown(plbw);
		   if (	!plbw->fTimer )
		       {
		       plbw->fTimer = TRUE;
		       WinStartTimer(plbw->hAB,	hWnd, TID_SCROLL, 50UL);
		       }
		   }
	       else
		       /* The mouse pointer position is	vertically	*/
		       /* within the list box limits although it may be	*/
		       /* horizontally outside of the list box.	 Take	*/
		       /* the vertical position	and using a "canned"	*/
		       /* horizontal position, process the point to see	*/
		       /* if this changes the selected items within the	*/
		       /* list box.					*/

		   if (	fDecodeMouse(plbw,
				     MPFROMLONG(MAKEULONG((USHORT)(plbw->rcl.xLeft + 1L),
						SHORT2FROMMP(mp1)))) )
		       {
		       /* Valid	position, stop the timer if it was set	*/
		       /* since	any vertical movement can be easily	*/
		       /* monitored and	the list box does not need to	*/
		       /* be scrolled					*/

		       if ( plbw->fTimer )

			   {
			   plbw->fTimer	= FALSE;
			   WinStopTimer(plbw->hAB, hWnd, TID_SCROLL);
			   }
		       }
		   else
		       /* Invalid position, start the timer since any	*/
		       /* vertical movement cannot be easily monitored	*/
		       /* monitored and	the list box needs to be	*/
		       /* scrolled					*/

		       if ( !plbw->fTimer )
			   {
			   plbw->fTimer	= TRUE;
			   WinStartTimer(plbw->hAB, hWnd, TID_SCROLL, 50UL);
			   }

		       /* Send the WM_CONTROLPOINTER message to	the	*/
		       /* owner	of the control to allow	the owner of	*/
		       /* control to change the	pointer	shape from the	*/
		       /* current defined arrow	shape			*/

       WinSetPointer(HWND_DESKTOP,
		     (HPOINTER)WinSendMsg(plbw->hwndOwner,
					  WM_CONTROLPOINTER,
					  MPFROMLONG(plbw->id),
					  MPFROMLONG(plbw->hptr)));
       return(MRFROMLONG(TRUE));

   /*********************************************************************/
   /* Process timer notification					*/
   /*********************************************************************/

   case	WM_TIMER :
		       /* Check	to make	sure that the timer is the	*/
		       /* scroll timer for the list box	when mouse	*/
		       /* capture has been started during the drag	*/
		       /* operation of the mouse and the mouse pointer	*/
		       /* is outside the list box display area		*/

       if ( SHORT1FROMMP(mp1) == TID_SCROLL )
	   {
		       /* Get the address of the control info from the	*/
		       /* control's reserved memory                     */

	   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Get the current mouse	pointer	position	*/

	   WinQueryPointerPos(HWND_DESKTOP, &ptl);

		       /* Convert the mouse pointer position, which is	*/
		       /* in desktop co-ordinates to the list box	*/
		       /* co-ordinates					*/

	   WinMapWindowPoints(HWND_DESKTOP, hWnd, &ptl,	1L);

		       /* Check	to see if the mouse pointer is inside	*/
		       /* the display rectangle	in which case the timer	*/
		       /* should be stopped to allow the normal		*/
		       /* mouse	handling to be performed		*/

	   if (	WinPtInRect(plbw->hAB, &plbw->rcl, &ptl) )
	       {
	       plbw->fTimer = FALSE;
	       WinStopTimer(plbw->hAB, hWnd, TID_SCROLL);
	       }
	   else
		       /* When the mouse pointer is above the list box,	*/
		       /* scroll upwards				*/

	       if ( ptl.y > plbw->rcl.yTop )
		   DragUp(plbw);
	       else
		       /* When the mouse pointer is below the list box,	*/
		       /* scroll downwards				*/

		   if (	ptl.y <	plbw->rcl.yBottom )
		       DragDown(plbw);
	   }
       break;

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	4: Keyboard interface						*/
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

       if ( SHORT1FROMMP(mp1) &	KC_KEYUP )
	   break;

       if ( SHORT1FROMMP(mp1) &	KC_CHAR	)
	   {
		       /* Get the list box data	pointer	from the window	*/

	   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
	   fItemSelect = FALSE;
	   if (	plbw->iSelected	!= LIT_NONE )
	       {
	       if ( (n = (CHAR1FROMMP(mp2) & 223)) <
		    (i = (plbw->plc[0].apli[plbw->iSelected]->pszText[0] & 223)) )
		   {
		   if (	plbw->iSelected	)
		       for ( i = plbw->iSelected - 1L; i >= 0; i-- )
			   if (	(plbw->plc[0].apli[i]->pszText[0] & 223) == (CHAR1FROMMP(mp2) &	223) )
			       {
			       fItemSelect = TRUE;
			       break;
			       }
		   }
	       else
		   for ( i = plbw->iSelected + 1L; i < plbw->cItems; i++ )
		       if ( (plbw->plc[0].apli[i]->pszText[0] &	223) ==	(CHAR1FROMMP(mp2) & 223) )
			   {
			   fItemSelect = TRUE;
			   break;
			   }
	       }
	   else
	       for ( i = 0; i <	plbw->cItems; i++ )
		   if (	(plbw->plc[0].apli[i]->pszText[0] & 223) == (CHAR1FROMMP(mp2) &	223) )
		       {
		       fItemSelect = TRUE;
		       break;
		       }

	   if (	fItemSelect )
	       {
	       if ( i <	plbw->iVertScroll )
		   {
		   if (	(plbw->iVertScroll = plbw->iSelected = i) >
			plbw->cVertScroll )
		       plbw->iVertScroll = plbw->cVertScroll;

		       /* Update the scroll bar	positioning and	force	*/
		       /* the repainting of the	list			*/

		   WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
			      MPFROMSHORT(plbw->iVertScroll), 0L);
		   WinInvalidateRect(hWnd, (PRECTL)NULL, FALSE);
		   }
	       else
		   if (	i <= (plbw->iVertScroll	+ plbw->cLinesPage) )
		       {
		       /* Determine if the currently selected item	*/
		       /* within the list is displayed in the viewable	*/
		       /* area of the window in	which case the entry	*/
		       /* needs	to be changed back to normal		*/

		       if ( (plbw->iSelected >=	plbw->iVertScroll) &&
			    (plbw->iSelected <=	(plbw->iVertScroll + plbw->cLinesPage))	)
			   fSelectItem(plbw, 0L, plbw->iSelected, FALSE);

		       fSelectItem(plbw, 0L, plbw->iSelected = i, TRUE);
		       }
		   else
		       {
		       if ( (plbw->iVertScroll = plbw->iSelected = i) >
			    plbw->cVertScroll )
			   plbw->iVertScroll = plbw->cVertScroll;

		       /* Update the scroll bar	positioning and	force	*/
		       /* the repainting of the	list			*/

		       WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
				  MPFROMSHORT(plbw->iVertScroll), 0L);
		       WinInvalidateRect(hWnd, (PRECTL)NULL, FALSE);
		       }
	       return(MRFROMLONG(TRUE));
	       }
	   }
		       /* Check	to make	sure the key is	decodeable key	*/

       if ( (CHARMSG(&msg)->fs & KC_CTRL) && (CHARMSG(&msg)->chr == '\\') )
	   {
		       /* Ctrl+\ : Deselect all				*/
		       /* Get the list box data	pointer	from the window	*/

	   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
	   if (	plbw->flStyle &	(LS_MULTIPLESEL	| LS_EXTENDEDSEL) )
	       {
	       if ( plbw->iFocus != LIT_NONE )
		   if (	plbw->plc[0].apli[plbw->iFocus]->fl & LI_SELECTEDOLD )
		       fItemSelect = TRUE;
		   else
		       fItemSelect = FALSE;
	       else
		   fItemSelect = FALSE;

	       SaveExtendedState(plbw);
	       for ( i = 0; i <	plbw->cItems; i++ )
		   if (	i == plbw->iFocus )
		       {
		       if ( !fItemSelect &&
			     (plbw->plc[0].apli[i]->fl & LI_SELECTED) )
			   {
			   SaveSelectState(0L, i);
			   plbw->plc[0].apli[i]->fl &= ~LI_SELECTED;
			   }
		       }
		   else
		       if ( (plbw->plc[0].apli[i]->fl &	LI_SELECTED) )
			   {
			   SaveSelectState(0L, i);
			   plbw->plc[0].apli[i]->fl &= ~LI_SELECTED;
			   }
	       WinInvalidateRect(hWnd, (PRECTL)NULL, FALSE);
	       mrNotifyOwner(plbw, LN_SELECT);
	       return(MRFROMLONG(TRUE));
	       }
	   }
       else
	   if (	(CHARMSG(&msg)->fs & KC_CTRL) && (CHARMSG(&msg)->chr ==	'/') )
	       {
		       /* Ctrl+/ : Select all				*/

		       /* Get the list box data	pointer	from the window	*/

	       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
	       if ( plbw->flStyle & (LS_MULTIPLESEL | LS_EXTENDEDSEL) )
		   {
		   SaveExtendedState(plbw);
		   for ( i = 0;	i < plbw->cItems; i++ )
		       if ( !(plbw->plc[0].apli[i]->fl & LI_SELECTED) )
			   {
			   SaveSelectState(0L, i);
			   plbw->plc[0].apli[i]->fl |= LI_SELECTED;
			   }

		   plbw->iSelected = plbw->iFocus;
		   WinInvalidateRect(hWnd, (PRECTL)NULL, FALSE);
		   mrNotifyOwner(plbw, LN_SELECT);
		   return(MRFROMLONG(TRUE));
		   }
	       }
	   else
		       /* Check	to make	sure the key is	decodeable key	*/

	       if ( CHARMSG(&msg)->fs &	KC_VIRTUALKEY )
		   switch ( CHARMSG(&msg)->vkey	)
		       {
		       /* Alt+Backspace	: Undo				*/

		       case VK_BACKSPACE :

		       /* Get the list box data	pointer	from the window	*/

			   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
			   if (	(CHARMSG(&msg)->fs & KC_ALT) &&	(plbw->flStyle & LS_EXTENDEDSEL) )
			       {
			       RestoreExtendedState(plbw);
			       WinInvalidateRect(hWnd, (PRECTL)NULL, FALSE);
			       }
			   return(MRFROMLONG(TRUE));

		       /* Enter	key pressed				*/

		       case VK_ENTER :
		       case VK_NEWLINE :

		       /* Get the list box data	pointer	from the window	*/

			   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see if there	is a valid selection	*/
		       /* and if not, ignore the keystroke.  When valid	*/
		       /* send a message back to the window that will	*/
		       /* cause	the item to be selected.		*/

			   if (	plbw->iSelected	== LIT_NONE )
			       return(0L);
			   else
			       if ( plbw->fFocus )
				   mrNotifyOwner(plbw, LN_ENTER);

			   return(MRFROMLONG(TRUE));

		       /* Space	bar pressed				*/

		       case VK_SPACE :

		       /* Get the list box data	pointer	from the window	*/

			   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see if the focus has	been placed on	*/
		       /* the list box and if it is visible remove the	*/
		       /* focus	indicator				*/

			   if (	plbw->iFocus > 0L )

		       /* Check	to see if the list box is in single	*/
		       /* select mode which means that the item	which	*/
		       /* contains the focus also is the current item	*/
		       /* selected					*/

			       if ( plbw->flStyle & (LS_MULTIPLESEL | LS_EXTENDEDSEL) )
				   {
				   if (	plbw->flStyle &	LS_MULTIPLESEL )
				       fSelectMultipleItem(plbw, 0L, plbw->iFocus,
							   (BOOL)(plbw->plc[0].apli[plbw->iFocus]->fl &	LI_SELECTED ? FALSE : TRUE));
				   else
				       if ( !(plbw->plc[0].apli[plbw->iFocus]->fl & LI_SELECTED) )
					   fSelectMultipleItem(plbw, 0L, plbw->iFocus,
							       TRUE);
				   if (	!fItemVisible(plbw, plbw->iFocus) )
				       {
				       if ( (plbw->iVertScroll = plbw->iFocus -	plbw->cLinesPage / 2L) < 0L )
					   plbw->iVertScroll = 0L;

				       mrNotifyOwner(plbw, LN_SCROLL);
				       WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
						  MPFROMSHORT(plbw->iVertScroll), 0L);
				       WinInvalidateRect(hWnd, (PRECTL)NULL, FALSE);
				       }
				   mrNotifyOwner(plbw, LN_SELECT);
				   }
			   return(MRFROMLONG(TRUE));

		       /* Up cursor key	selected			*/

		       case VK_UP :

		       /* Get the list box data	pointer	from the window	*/

			   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

			   if (	(plbw->flStyle & LS_EXTENDEDSEL) &&
				(CHARMSG(&msg)->fs & KC_SHIFT) )
			       if ( plbw->iFocus > plbw->iAnchor )
				   fSelectMultipleItem(plbw, 0L, plbw->iFocus, FALSE);
			       else
				   if (	plbw->iFocus )
				       fSelectMultipleItem(plbw, 0L, plbw->iFocus - 1L,	TRUE);

		       /* Get the list box data	pointer	from the window	*/

			   LineUp(plbw);
			   return(MRFROMLONG(TRUE));

		       case VK_DOWN :

		       /* Get the list box data	pointer	from the window	*/

			   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

			   if (	(plbw->flStyle & LS_EXTENDEDSEL) &&
				(CHARMSG(&msg)->fs & KC_SHIFT) )
			       if ( plbw->iFocus < plbw->iAnchor )
				   fSelectMultipleItem(plbw, 0L, plbw->iFocus, FALSE);
			       else
				   if (	plbw->iFocus < (plbw->cItems - 1L) )
				       fSelectMultipleItem(plbw, 0L, plbw->iFocus + 1L,	TRUE);

		       /* Get the list box data	pointer	from the window	*/

			   LineDown(plbw);
			   return(MRFROMLONG(TRUE));

		       case VK_RIGHT :

		       /* Get the list box data	pointer	from the window	*/

			   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Calculate the	proper position	of the scroll	*/
		       /* bar such that	it falls within	a valid	range	*/
		       /* and set the new scroll bar position and force	*/
		       /* the repainting of the	output window		*/

			   if (	plbw->iHorzScroll < plbw->cHorzScroll )
			       {

		       /* Send the scroll notification to the list box	*/
		       /* owner						*/

			       mrNotifyOwner(plbw, LN_SCROLL);

		       /* Remove the focus indicator since it will	*/
		       /* show the right edge repeatedly along the line	*/
		       /* while	scrolling				*/

			       RemoveFocus(plbw);

		       /* Update the scroll bar	position and then cause	*/
		       /* the list area	to scroll horizontally before	*/
		       /* forcing the list box to repaint the area	*/
		       /* invalidated					*/

			       WinSendMsg(plbw->hwndScrollBottom, SBM_SETPOS, MPFROMSHORT(++plbw->iHorzScroll),	0L);
			       WinScrollWindow(hWnd, -plbw->xChar, 0L,
					       (PRECTL)&plbw->rcl, (PRECTL)&plbw->rcl,
					       (HRGN)NULL, (PRECTL)NULL, SW_INVALIDATERGN);
			       WinUpdateWindow(hWnd);

		       /* Redisplay the	focus since the	text has been	*/
		       /* scrolled properly				*/

			       SetFocus(plbw, plbw->iFocus);
			       }
			   return(MRFROMLONG(TRUE));

		       case VK_LEFT :

		       /* Get the list box data	pointer	from the window	*/

			   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Calculate the	proper position	of the scroll	*/
		       /* bar such that	it falls within	a valid	range	*/
		       /* and set the new scroll bar position and force	*/
		       /* the repainting of the	output window		*/

			   if (	plbw->iHorzScroll > 0L )
			       {
		       /* Send the scroll notification to the list box	*/
		       /* owner						*/

			       mrNotifyOwner(plbw, LN_SCROLL);

		       /* Remove the focus indicator since it will	*/
		       /* show the right edge repeatedly along the line	*/
		       /* while	scrolling				*/

			       RemoveFocus(plbw);

		       /* Update the scroll bar	position and then cause	*/
		       /* the list area	to scroll horizontally before	*/
		       /* forcing the list box to repaint the area	*/
		       /* invalidated					*/

			       WinSendMsg(plbw->hwndScrollBottom, SBM_SETPOS, MPFROMSHORT(--plbw->iHorzScroll),	0L);
			       WinScrollWindow(hWnd, plbw->xChar, 0L,
					       (PRECTL)&plbw->rcl, (PRECTL)&plbw->rcl,
					       (HRGN)NULL, (PRECTL)NULL, SW_INVALIDATERGN);
			       WinUpdateWindow(hWnd);

		       /* Redisplay the	focus since the	text has been	*/
		       /* scrolled properly				*/

			       SetFocus(plbw, plbw->iFocus);
			       }
			   return(MRFROMLONG(TRUE));

		       case VK_PAGEUP :
		       case VK_F7 :

		       /* Get the list box data	pointer	from the window	*/

			   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see if the focus has	been placed on	*/
		       /* the list box and if it is visible remove the	*/
		       /* focus	indicator				*/

			   if (	(plbw->iFocus != LIT_NONE) && (plbw->iVertScroll != 0L)	)
			       if ( (plbw->flStyle & (LS_MULTIPLESEL | LS_EXTENDEDSEL))	== 0UL )
				   {
				   SaveSelectState(0L, plbw->iFocus);
				   plbw->plc[0].apli[plbw->iFocus]->fl &= ~(LI_FOCUS | LI_SELECTED);
				   if (	fItemVisible(plbw, plbw->iFocus) )
				       DrawItemSelection(plbw, plbw->iFocus);

		       /* Check	to see if the new position calculated	*/
		       /* will be after	the list bottom	display	item,	*/
		       /* in which case, the list will need to be	*/
		       /* shifted downwards and	the list redisplayed	*/

				   if (	(plbw->iFocus -= (plbw->cLinesPage - 1L)) < plbw->iVertScroll )
				       {

		       /* Check	to see that the	movement doesn't cause  */
		       /* the bottom of	the list to be greater than the	*/
		       /* scroll limits	in which case, the scroll index	*/
		       /* should be set	to this	limit			*/

				       if ( (plbw->iVertScroll -= (plbw->cLinesPage - 1L)) < 0L	)
					   plbw->iVertScroll = 0L;

		       /* Check	to make	sure that the selected item	*/
		       /* due to the page up is	not invalid by being	*/
		       /* outside the limits of	the array		*/		

				       if ( plbw->iFocus < 0L )
					   plbw->iFocus	= 0L;

		       /* Update the scroll bar	positioning and	force	*/
		       /* the repainting of the	list			*/

				       mrNotifyOwner(plbw, LN_SCROLL);
				       SaveSelectState(0L, plbw->iFocus);
				       plbw->plc[0].apli[plbw->iSelected = plbw->iFocus]->fl |=	(LI_FOCUS | LI_SELECTED);
				       WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
						  MPFROMSHORT(plbw->iVertScroll), 0L);
				       WinInvalidateRect(hWnd, (PRECTL)NULL, FALSE);
				       }
				   else
				       {
				       SaveSelectState(0L, plbw->iFocus);
				       plbw->plc[0].apli[plbw->iSelected = plbw->iFocus]->fl |=	(LI_FOCUS | LI_SELECTED);
				       DrawItemSelection(plbw, plbw->iFocus);
				       }
				   }
			       else
				   {
				   RemoveFocus(plbw);

		       /* Check	to see if the new position calculated	*/
		       /* will be after	the list bottom	display	item,	*/
		       /* in which case, the list will need to be	*/
		       /* shifted downwards and	the list redisplayed	*/

				   if (	(plbw->iFocus -= (plbw->cLinesPage - 1L)) < plbw->iVertScroll )
				       {

		       /* Check	to see that the	movement doesn't cause  */
		       /* the bottom of	the list to be greater than the	*/
		       /* scroll limits	in which case, the scroll index	*/
		       /* should be set	to this	limit			*/

				       if ( (plbw->iVertScroll -= (plbw->cLinesPage - 1L)) < 0L	)
					   plbw->iVertScroll = 0L;

		       /* Check	to make	sure that the selected item	*/
		       /* due to the page up is	not invalid by being	*/
		       /* outside the limits of	the array		*/		

				       if ( plbw->iFocus < 0L )
					   plbw->iFocus	= 0L;

		       /* Update the scroll bar	positioning and	force	*/
		       /* the repainting of the	list			*/

				       mrNotifyOwner(plbw, LN_SCROLL);
				       plbw->plc[0].apli[plbw->iFocus]->fl |= LI_FOCUS;
				       WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
						  MPFROMSHORT(plbw->iVertScroll), 0L);
				       WinInvalidateRect(hWnd, (PRECTL)NULL, FALSE);
				       }
				   else
				       SetFocus(plbw, plbw->iFocus);
				   }
			   return(MRFROMLONG(TRUE));

		       case VK_F8 :
		       case VK_PAGEDOWN	:

		       /* Get the list box data	pointer	from the window	*/

			   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see if the focus has	been placed on	*/
		       /* the list box and if it is visible remove the	*/
		       /* focus	indicator				*/

			   if (	(plbw->iFocus != LIT_NONE) && (plbw->iVertScroll != plbw->cVertScroll) )
			       if ( (plbw->flStyle & (LS_MULTIPLESEL | LS_EXTENDEDSEL))	== 0UL )
				   {
				   SaveSelectState(0L, plbw->iFocus);
				   plbw->plc[0].apli[plbw->iFocus]->fl &= ~(LI_FOCUS | LI_SELECTED);
				   if (	fItemVisible(plbw, plbw->iFocus) )
				       DrawItemSelection(plbw, plbw->iFocus);

		       /* Check	to see if the new position calculated	*/
		       /* will be a after the current list bottom	*/
		       /* display item,	in which case, the list	will	*/
		       /* need to be shifted downwards and the list	*/
		       /* redisplayed					*/

				   if (	(plbw->iFocus += (plbw->cLinesPage - 1L)) >=
					(plbw->iVertScroll + plbw->cLinesPage) )
				       {

		       /* Check	to see that the	movement doesn't cause  */
		       /* the bottom of	the list to be greater than the	*/
		       /* scroll limits	in which case, the scroll index	*/
		       /* should be set	to this	limit			*/

				       if ( (plbw->iVertScroll += (plbw->cLinesPage - 1L)) > plbw->cVertScroll )
					   plbw->iVertScroll = plbw->cVertScroll;

		       /* Check	to make	sure that the selected item	*/
		       /* due to the page up is	not invalid by being	*/
		       /* outside the limits of	the array		*/		

				       if ( plbw->iFocus >= plbw->cItems )
					   plbw->iFocus	= plbw->cItems - 1L;

		       /* Update the scroll bar	positioning and	force	*/
		       /* the repainting of the	list			*/

				       SaveSelectState(0L, plbw->iFocus);
				       plbw->plc[0].apli[plbw->iSelected = plbw->iFocus]->fl |=	(LI_FOCUS | LI_SELECTED);
				       mrNotifyOwner(plbw, LN_SCROLL);
				       WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
						  MPFROMSHORT(plbw->iVertScroll), 0L);
				       WinInvalidateRect(hWnd, (PRECTL)NULL, FALSE);
				       }
				   else
				       {
				       SaveSelectState(0L, plbw->iFocus);
				       plbw->plc[0].apli[plbw->iSelected = plbw->iFocus]->fl |=	(LI_FOCUS | LI_SELECTED);
				       DrawItemSelection(plbw, plbw->iFocus);
				       }
				   }
			       else
				   {
				   RemoveFocus(plbw);

		       /* Check	to see if the new position calculated	*/
		       /* will be a after the current list bottom	*/
		       /* display item,	in which case, the list	will	*/
		       /* need to be shifted downwards and the list	*/
		       /* redisplayed					*/

				   if (	(plbw->iFocus += (plbw->cLinesPage - 1L)) >=
					(plbw->iVertScroll + plbw->cLinesPage) )
				       {

		       /* Check	to see that the	movement doesn't cause  */
		       /* the bottom of	the list to be greater than the	*/
		       /* scroll limits	in which case, the scroll index	*/
		       /* should be set	to this	limit			*/

				       if ( (plbw->iVertScroll += (plbw->cLinesPage - 1L)) > plbw->cVertScroll )
					   plbw->iVertScroll = plbw->cVertScroll;

		       /* Check	to make	sure that the selected item	*/
		       /* due to the page up is	not invalid by being	*/
		       /* outside the limits of	the array		*/		

				       if ( plbw->iFocus >= plbw->cItems )
					   plbw->iFocus	= plbw->cItems - 1L;

		       /* Update the scroll bar	positioning and	force	*/
		       /* the repainting of the	list			*/

				       plbw->plc[0].apli[plbw->iFocus]->fl |= LI_FOCUS;
				       mrNotifyOwner(plbw, LN_SCROLL);
				       WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
						  MPFROMSHORT(plbw->iVertScroll), 0L);
				       WinInvalidateRect(hWnd, (PRECTL)NULL, FALSE);
				       }
				  else
				       SetFocus(plbw, plbw->iFocus);
				   }
			   return(MRFROMLONG(TRUE));

		       case VK_HOME :

		       /* Get the list box data	pointer	from the window	*/

			   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Set the current item selected	to the top of	*/
		       /* the list and update the scroll bar to		*/
		       /* reflect this change before forcing the window	*/
		       /* to be	repainted				*/

			   if (	plbw->iVertScroll )
			       {
			       if ( plbw->iFocus > plbw->cLinesPage )
				   fScroll = TRUE;
			       else
				   fScroll = FALSE;

			       if ( (plbw->flStyle & (LS_MULTIPLESEL | LS_EXTENDEDSEL))	== 0UL )
				   {
				   SaveSelectState(0L, plbw->iFocus);
				   plbw->plc[0].apli[plbw->iFocus]->fl &= ~(LI_FOCUS | LI_SELECTED);
				   plbw->plc[0].apli[plbw->iSelected = plbw->iFocus = 0L]->fl |= (LI_FOCUS | LI_SELECTED);
				   }
			       else
				   {
				   plbw->plc[0].apli[plbw->iFocus]->fl &= ~LI_FOCUS;
				   plbw->plc[0].apli[plbw->iFocus = 0L]->fl |= LI_FOCUS;
				   }
			
			       if ( fScroll )
				   {
				   mrNotifyOwner(plbw, LN_SCROLL);
				   WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
					      MPFROMSHORT(plbw->iVertScroll = 0), 0L);
				   WinInvalidateRect(hWnd, (PRECTL)NULL, FALSE);
				   }
			       }
			   return(MRFROMLONG(TRUE));

		       case VK_END :

		       /* Get the list box data	pointer	from the window	*/

			   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Set the current item selected	to the bottom	*/
		       /* of the list and update the scroll bar	to	*/
		       /* reflect this change before forcing the window	*/
		       /* to be	repainted				*/

			   if (	(plbw->cItems >	plbw->cLinesPage) &&
				(plbw->iVertScroll != plbw->cVertScroll) )
			       {
			       if ( plbw->iFocus < plbw->cVertScroll )
				   fScroll = TRUE;
			       else
				   fScroll = FALSE;

			       if ( (plbw->flStyle & (LS_MULTIPLESEL | LS_EXTENDEDSEL))	== 0UL )
				   {
				   SaveSelectState(0L, plbw->iFocus);
				   plbw->plc[0].apli[plbw->iFocus]->fl &= ~(LI_FOCUS | LI_SELECTED);
				   plbw->plc[0].apli[plbw->iSelected = plbw->iFocus = plbw->cItems - 1L]->fl |=	(LI_FOCUS | LI_SELECTED);
				   }
			       else
				   {
				   plbw->plc[0].apli[plbw->iFocus]->fl &= ~LI_FOCUS;
				   plbw->plc[0].apli[plbw->iFocus = plbw->cItems - 1L]->fl |= LI_FOCUS;
				   }

			       if ( fScroll )
				   {
				   mrNotifyOwner(plbw, LN_SCROLL);
				   WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS,
					      MPFROMSHORT(plbw->iVertScroll = plbw->cVertScroll), 0L);
				   WinInvalidateRect(hWnd, (PRECTL)NULL, FALSE);
				   }
			       }
			   return(MRFROMLONG(TRUE));

		       case VK_TAB :

		       /* Get the list box data	pointer	from the window	*/

			   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* TAB key pressed, determine which control is	*/
		       /* the next tab stop and	set the	focus on that	*/
		       /* control					*/

			   WinSetFocus(HWND_DESKTOP,
				       WinEnumDlgItem(plbw->hwndParent,
						      hWnd,
						      EDI_NEXTTABITEM));
			   return(MRFROMLONG(TRUE));

		       case VK_BACKTAB :

		       /* Get the list box data	pointer	from the window	*/

			   plbw	= (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Shift+TAB key	pressed, determine which	*/
		       /* control is the previous tab stop and set the	*/
		       /* focus	on that	control				*/

			   WinSetFocus(HWND_DESKTOP,
				       WinEnumDlgItem(plbw->hwndParent,
						      hWnd,
						      EDI_PREVTABITEM));
			   return(MRFROMLONG(TRUE));
		       }
       return(WinDefWindowProc(hWnd, msg, mp1, mp2));

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	5: Entry Field interface					*/
/*									*/
/************************************************************************/
/************************************************************************/

   case	WM_CONTROL :
		       /* Get the list box data	pointer	from the window	*/

       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
       if ( HWNDFROMMP(mp2) == plbw->hwndEdit )
	   switch ( SHORT2FROMMP(mp1) )
	       {
	       case EN_KILLFOCUS :
		   pszText = (PSZ)HeapMalloc(plbw->plc[0].hHeap,
					     (ULONG)(n = WinQueryWindowTextLength(plbw->hwndEdit) + 1));
		   WinQueryWindowText(plbw->hwndEdit, n, pszText);
		   WinSendMsg(hWnd, LM_SETITEMTEXT,
			      MPFROMLONG(plbw->iEditing), MPFROMP(pszText));
		   HeapFree(plbw->plc[0].hHeap,	pszText);
		   mrNotifyOwner(plbw, LNX_EDITED);
		   WinPostMsg(hWnd, WM_DESTROYEDIT, 0L,	0L);
		   break;
	       }
       break;

   case	WM_DESTROYEDIT :
       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);
       WinDestroyWindow(plbw->hwndEdit);
       break;

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	6: Scroll bar interface						*/
/*									*/
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Horizontal scroll bar						*/
   /*********************************************************************/

   case	WM_HSCROLL :
		       /* Get the list box data	pointer	from the window	*/

       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Save the current scroll position		*/

       i = plbw->iHorzScroll;

		       /* Determine the	scroll event			*/

       switch (	HIUSHORT(mp2) )
	   {
		       /* Line up request through up scroll button	*/

	   case	SB_LINELEFT :
	       plbw->iHorzScroll--;
	       break;
		       /* Line down request through down scroll	button	*/

	   case	SB_LINERIGHT :
	       plbw->iHorzScroll++;
	       break;
		       /* Page up request through scroll bar selection	*/

	   case	SB_PAGELEFT :
	       plbw->iHorzScroll -= plbw->cCharsPage;
	       break;
		       /* Page down request through scroll bar		*/
		       /* selection					*/

	   case	SB_PAGERIGHT :
	       plbw->iHorzScroll += plbw->cCharsPage;
	       break;
		       /* Slider is being dragged			*/

	   case	SB_SLIDERTRACK :
	       plbw->iHorzScroll = SHORT1FROMMP(mp2);
	       break;
		       /* Ignore all other scroll bar events		*/
	   default :
	       return(0L);
	   }
		       /* Calculate the	proper position	of the scroll	*/
		       /* bar such that	it falls within	a valid	range	*/
		       /* and set the new scroll bar position and force	*/
		       /* the repainting of the	output window		*/

       if (  (plbw->iHorzScroll	= max(0, min(plbw->iHorzScroll,	plbw->cHorzScroll))) - i )
	   {
		       /* Send the scroll notification to the list box	*/
		       /* owner						*/

	   mrNotifyOwner(plbw, LN_SCROLL);

		       /* Remove the focus indicator since it will	*/
		       /* show the right edge repeatedly along the line	*/
		       /* while	scrolling				*/

	   RemoveFocus(plbw);

		       /* Update the scroll bar	position and then cause	*/
		       /* the list area	to scroll horizontally before	*/
		       /* forcing the list box to repaint the area	*/
		       /* invalidated					*/

	   WinSendMsg(plbw->hwndScrollBottom, SBM_SETPOS, MPFROMSHORT(plbw->iHorzScroll), 0L);
	   rcl = plbw->rcl;
	   --rcl.xRight;
	   WinScrollWindow(hWnd, -(plbw->iHorzScroll - i) * plbw->xChar, 0L,
			   (PRECTL)&rcl, (PRECTL)&plbw->rcl,
			   (HRGN)NULL, (PRECTL)NULL, SW_INVALIDATERGN);
	   WinUpdateWindow(hWnd);

		       /* Redisplay the	focus since the	text has been	*/
		       /* scrolled properly				*/

	   SetFocus(plbw, plbw->iFocus);
	   }
       break;

   /*********************************************************************/
   /*  Vertical	scroll bar						*/
   /*********************************************************************/

   case	WM_VSCROLL :
		       /* Get the list box data	pointer	from the window	*/

       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Save the current scroll position		*/

       i = plbw->iVertScroll;

		       /* Determine the	scroll event			*/

       switch (	HIUSHORT(mp2) )
	   {
		       /* Line up request through up scroll button	*/

	   case	SB_LINEUP :
	       plbw->iVertScroll--;
	       break;
		       /* Line down request through down scroll	button	*/

	   case	SB_LINEDOWN :
	       plbw->iVertScroll++;
	       break;
		       /* Page up request through scroll bar selection	*/

	   case	SB_PAGEUP :
	       plbw->iVertScroll -= (plbw->cLinesPage -	1);
	       break;
		       /* Page down request through scroll bar		*/
		       /* selection					*/

	   case	SB_PAGEDOWN :
	       plbw->iVertScroll += (plbw->cLinesPage -	1);
	       break;
		       /* Slider is being dragged			*/

	   case	SB_SLIDERTRACK :
	       plbw->iVertScroll = SHORT1FROMMP(mp2);
	       break;
		       /* Ignore all other scroll bar events		*/
	   default :
	       return(0L);
	   }
		       /* Calculate the	proper position	of the scroll	*/
		       /* bar such that	it falls within	a valid	range	*/

       if ( (plbw->iVertScroll = max(0,	min(plbw->iVertScroll, plbw->cVertScroll))) - i	)
	   {
		       /* Set the new scroll bar position and force the	*/
		       /* repainting of	the output window		*/

	   mrNotifyOwner(plbw, LN_SCROLL);
	   WinSendMsg(plbw->hwndScrollRight, SBM_SETPOS, MPFROMSHORT(plbw->iVertScroll), 0L);
	   WinScrollWindow(hWnd, 0L, (plbw->iVertScroll	- i) * plbw->cyItem,
			   (PRECTL)&plbw->rcl, (PRECTL)&plbw->rcl,
			   (HRGN)NULL, (PRECTL)NULL, SW_INVALIDATERGN);
	   WinUpdateWindow(hWnd);
	   }
       break;

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	7: Control messages						*/
/*									*/
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Base messages							*/
   /*********************************************************************/

   case	LM_DELETEALL :
   case	LM_DELETEITEM :
   case	LM_INSERTITEM :
   case	LM_QUERYITEMCOUNT :
   case	LM_QUERYITEMHANDLE :
   case	LM_QUERYITEMTEXT :
   case	LM_QUERYITEMTEXTLENGTH :
   case	LM_QUERYSELECTION :
   case	LM_QUERYTOPINDEX :
   case	LM_SEARCHSTRING	:
   case	LM_SELECTITEM :
   case	LM_SETITEMHANDLE :
   case	LM_SETITEMHEIGHT :
   case	LM_SETITEMTEXT :
   case	LM_SETTOPINDEX :
       return(mrBaseListHandler(hWnd, msg, mp1,	mp2));

   /*********************************************************************/
   /*  Extended	messages						*/
   /*********************************************************************/

   case	LMX_ADDARRAY :
   case	LMX_CALCLINECOUNT :
   case	LMX_CALCSIZE :
   case	LMX_COPY :
   case	LMX_CUT	:
   case	LMX_PASTE :
   case	LMX_QUERYITEMCOUNT :
   case	LMX_QUERYITEMRECT :
   case	LMX_QUERYSELECTLIST :
   case	LMX_RESETLIST :
   case	LMX_SELECTALL :
   case	LMX_SELECTITEMBITMAP :
   case	LMX_SETARRAY :
   case	LMX_SETARRAYHANDLES :
   case	LMX_SETARRAYITEMS :
   case	LMX_SETITEMBITMAPS :
   case	LMX_SETITEMCOUNT :
   case	LMX_SORT :
       return(mrExtendedListHandler(hWnd, msg, mp1, mp2));

   /*********************************************************************/
   /*  Extended	messages						*/
   /*********************************************************************/

   case	LMX_SETCHECK :
   case	LMX_QUERYCHECK :
   case	LMX_SETCHECKARRAY :
   case	LMX_QUERYCHECKARRAY :
       return(mrChkBoxHandler(hWnd, msg, mp1, mp2));

   /*********************************************************************/
   /*  Extended	sound messages						*/
   /*********************************************************************/

   case	LMXM_SETSOUNDEVENT :
   case	LMXM_QUERYSOUNDEVENT :
       return(mrSoundHandler(hWnd, msg,	mp1, mp2));

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	8: Painting and	display						*/
/*									*/
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  System colour/Presentation parameter change			*/
   /*********************************************************************/

   case	WM_SYSCOLORCHANGE :
   case	WM_PRESPARAMCHANGED :

		       /* Get the list box data	pointer	from the window	*/

       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Presentation parameters or system colours	*/
		       /* have changed,	get the	new values for the	*/
		       /* colours					*/

       plbw->lClrText		  = lGetPresParam(hWnd,	PP_FOREGROUNDCOLOR,
						  PP_FOREGROUNDCOLORINDEX,
						  SYSCLR_OUTPUTTEXT);
       plbw->lClrList		  = lGetPresParam(hWnd,	PP_BACKGROUNDCOLOR,
						  PP_BACKGROUNDCOLORINDEX,
						  SYSCLR_ENTRYFIELD);
       plbw->lClrBorder		  = lGetPresParam(hWnd,	PP_BORDERCOLOR,
						  PP_BORDERCOLORINDEX,
						  SYSCLR_BUTTONDARK);
       plbw->lClrBackground	  = lGetPresParam(hWnd,	PP_BACKGROUNDCOLOR,
						  PP_BACKGROUNDCOLORINDEX,
						  SYSCLR_FIELDBACKGROUND);
       plbw->lClrHilite		  = lGetPresParam(hWnd,	PP_HILITEFOREGROUNDCOLOR,
						  PP_HILITEFOREGROUNDCOLORINDEX,
						  SYSCLR_HILITEFOREGROUND);
       plbw->lClrHiliteBackground = lGetPresParam(hWnd,	PP_HILITEBACKGROUNDCOLOR,
						  PP_HILITEBACKGROUNDCOLORINDEX,
						  SYSCLR_HILITEBACKGROUND);
       if ( !WinQueryPresParam(hWnd, PP_FOREGROUNDCOLORINDEX, 0UL, &ulID, sizeof(LONG),	(PVOID)&plbw->lClrTextIndex,
			       QPF_NOINHERIT | QPF_ID1COLORINDEX) )
	   plbw->lClrTextIndex = SYSCLR_OUTPUTTEXT;
       if ( !WinQueryPresParam(hWnd, PP_BACKGROUNDCOLORINDEX, 0UL, &ulID, sizeof(LONG),	(PVOID)&plbw->lClrListIndex,
			       QPF_NOINHERIT | QPF_ID1COLORINDEX) )
	   plbw->lClrListIndex = SYSCLR_ENTRYFIELD;

		       /* Since	there may be a different font being	*/
		       /* used,	determine the new size of the list	*/
		       /* box horizontal scroll	bar			*/

       if ( (hPS = WinGetPS(hWnd)) != (HPS)NULL	)
	   {
	   if (	plbw->cItems )
	       for ( i = 0, plbw->cxItem = 0L; i < plbw->cItems; i++ )
		   {
		   GpiQueryTextBox(hPS,	(LONG)plbw->plc[0].apli[i]->cText,
				   plbw->plc[0].apli[i]->pszText, 5L, rgptl);
		   if (	(plbw->plc[0].apli[i]->cxItem =	rgptl[TXTBOX_CONCAT].x -
						      rgptl[TXTBOX_BOTTOMLEFT].x) > plbw->cxItem )
		       plbw->cxItem = plbw->plc[0].apli[i]->cxItem;
		   }
		       /* Get the font metrics for the current font to	*/
		       /* allow	the sizing of the list box to be	*/
		       /* properly calculated				*/

	   GpiQueryFontMetrics(hPS, sizeof(FONTMETRICS), &fm);
	   plbw->yAscender = fm.lMaxAscender;
	   MeasureItem(plbw, fm.lMaxBaselineExt);
	   plbw->xChar	= fm.lAveCharWidth;

		       /* Initialize the values	for the	list box	*/
		       /* pertaining to	the number of items in the list	*/
		       /* box, display line count, etc.			*/

	   SizeListBox(plbw);

		       /* Release the presentation space handle	and	*/
		       /* force	the repainting of the list area	of the	*/
		       /* list box					*/

	   WinReleasePS(hPS);
	   }
       if ( WinIsWindowShowing(hWnd) )
	   WinInvalidateRect(hWnd, &plbw->rcl, FALSE);
       break;

   /*********************************************************************/
   /*  Paint control							*/
   /*********************************************************************/

   case	WM_PAINT :
		       /* Get the address of the control info from the	*/
		       /* control's reserved memory                     */

       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Check	to see if the style of the list	box has	*/
		       /* changed since	it was last painted and	if the	*/
		       /* case,	check to see how it should be changed	*/
		       /* in terms of its component parts		*/

       if ( ((flStyle =	WinQueryWindowULong(hWnd, QWL_STYLE)) &	0x0000ffffUL) != (plbw->flStyle	& 0x0000ffffUL)	)
	   if (	(plbw->flStyle & LS_HORZSCROLL)	&& ((flStyle & LS_HORZSCROLL) == 0) )
	       {
		       /* Scroll bar being removed, delete the		*/
		       /* horizontal scroll bar	and re-jigg the	list	*/
		       /* box to allow for the additional space		*/

	       WinDestroyWindow(plbw->hwndScrollBottom);
	       plbw->flStyle = flStyle;
	       SizeListBox(plbw);
	       }
	   else
	       if ( flStyle & LS_HORZSCROLL )
		   {
		       /* Horizontal scroll bar	being added, create the	*/
		       /* scroll bar and re-jigg the list box to allow	*/
		       /* for the reduction in space for the displayed	*/
		       /* list						*/

		   if (	fAddHorzScroll(hWnd, plbw) )
		       {
		       hPS = WinGetPS(hWnd);

		       if ( plbw->cItems )
			   for ( i = 0,	plbw->cxItem = 0L; i < plbw->cItems; i++ )
			       {
			       GpiQueryTextBox(hPS, (LONG)plbw->plc[0].apli[i]->cText,
					       plbw->plc[0].apli[i]->pszText, 5L, rgptl);
			       if ( (plbw->plc[0].apli[i]->cxItem = rgptl[TXTBOX_CONCAT].x -
								  rgptl[TXTBOX_BOTTOMLEFT].x) >	plbw->cxItem )
				   plbw->cxItem	= plbw->plc[0].apli[i]->cxItem;
			       }
		       GpiQueryFontMetrics(hPS,	sizeof(FONTMETRICS), &fm);
		       WinReleasePS(hPS);
		       MeasureItem(plbw, fm.lMaxBaselineExt);
		       SizeListBox(plbw);
		       UpdateScrollBars(plbw);
		       }
		   }
	       else
		       /* General style	change,	refresh	the list box	*/
		       /* sizes	in case	the list box has changed from	*/
		       /* no adjust position to	the opposite		*/
		   {
		   plbw->flStyle = flStyle;
		   SizeListBox(plbw);
		   UpdateScrollBars(plbw);
		   }
		       /* Check	to see if delayed update in which case	*/
		       /* update the limits and	scroll bars		*/

       if ( plbw->fDirty )
	   UpdateScrollBars(plbw);

		       /* Set the colour table to RGB mode		*/

       if ( (hPS = WinBeginPaint(hWnd, (HPS)NULL, &rclPaint)) != (HPS)NULL )
	   {
	   GpiCreateLogColorTable(hPS, 0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);

		       /* Check	to see if the style of the list	box	*/
		       /* includes a horizontal	scroll bar in which	*/
		       /* case need to paint the corner	left over where	*/
		       /* the two scroll bars meet			*/

	   if (	plbw->flStyle &	LS_HORZSCROLL )
	       WinFillRect(hPS,	&plbw->rclCorner, plbw->lClrBackground);

		       /* Check	to see if the style of the list	box	*/
		       /* allows for the height	of the list box	to be	*/
		       /* adjusted to allow only the display of		*/
		       /* complete lines within	the list box in	which	*/
		       /* case,	need to	paint the left over area at the	*/
		       /* top of the window				*/

	   if (	((plbw->flStyle	& LS_NOADJUSTPOS) != LS_NOADJUSTPOS) &&
		(plbw->rclAdjusted.yTop	> plbw->rclAdjusted.yBottom) )
	       WinFillRect(hPS,	&plbw->rclAdjusted, plbw->lClrBackground);

		       /* Now, draw the	borders	of the non-scroll bar	*/
		       /* edges.  First	set the	colour of the border	*/
		       /* and then draw	the inside edge	of the border.	*/
		       /* The border follows the typical 3D effect of	*/
		       /* one line being dark and the second line being	*/
		       /* light.					*/

	   GpiSetColor(hPS, RGB_WHITE);
	   GpiMove(hPS,	plbw->aptlInside);
	   GpiPolyLine(hPS, plbw->cptl,	&plbw->aptlInside[1]);

		       /* Having drawn the inside edge,	now draw the	*/
		       /* outside edge of the border			*/

	   GpiSetColor(hPS, plbw->lClrBorder);
	   GpiMove(hPS,	plbw->aptlOutside);
	   GpiPolyLine(hPS, plbw->cptl,	&plbw->aptlOutside[1]);

		       /* Check	to see if the list box contains	any	*/
		       /* items	in which case they need	to be drawn	*/

	   if (	plbw->cItems )
	       {
		       /* Initialize the base rectangle			*/

	       rcl = plbw->rcl;

		       /* Using	the paint rectangle, determine the last	*/
		       /* item that requires to	be painted		*/

	       n = plbw->iVertScroll + ((plbw->rcl.yTop	- rclPaint.yBottom) /
		   plbw->cyItem);

	       if ( plbw->rcl.yTop < rclPaint.yTop )
		   i = 0;
	       else
		   i = (plbw->rcl.yTop - rclPaint.yTop)	/ plbw->cyItem;

		       /* Using	the paint rectangle, determine the	*/
		       /* first	item that requires to be painted and	*/
		       /* also form the	final rectangle	for the	text	*/

	       rcl.yBottom = (rcl.yTop = plbw->rcl.yTop	-
			     (i	* plbw->cyItem)) - plbw->cyItem;

		       /* Form the starting display point for the	*/
		       /* drawing of the text				*/

	       ptl.x = plbw->rcl.xLeft + 2L - (plbw->iHorzScroll * plbw->xChar);
	       ptl.y = rcl.yTop	- plbw->yAscender;

		       /* Since	the entire window is sub-divided	*/
		       /* logically into lines,	paint each of the lines	*/
		       /* within the window				*/

	       for ( i += plbw->iVertScroll; i <= n; i++ )
		   if (	i >= plbw->cItems )
		       {
		       /* No more stream information available,	paint	*/
		       /* the remainder	of the window below the	last	*/
		       /* logical line in the background colour	and	*/
		       /* break	out of the loop				*/

		       if ( (rcl.yBottom = plbw->rcl.yBottom) <	rcl.yTop )
			   WinFillRect(hPS, &rcl, plbw->lClrList);
		       break;
		       }
		   else
		       {
		       if ( (fl	= plbw->plc[0].apli[i]->fl) & LI_SELECTED )
			   plbw->plc[0].apli[i]->fl |= LI_SELECTEDPREV;

		       /* Draw the text	for the	list box item		*/

		       DrawItem(plbw, i, hPS, &rcl, &ptl);
		       plbw->plc[0].apli[i]->fl	= fl & ~LI_SELECTEDPREV;

		       /* Shift	the line positioning down ready	for the	*/
		       /* next line to be drawn				*/

		       rcl.yTop	= rcl.yBottom;
		       rcl.yBottom -= plbw->cyItem;
		       ptl.y -=	plbw->cyItem;
		       }
	       }
	   else
		       /* No items contained within the	list box, just	*/
		       /* fill it with the list	box background colour	*/

	       WinFillRect(hPS,	&plbw->rcl, plbw->lClrList);

	   WinEndPaint(hPS);
	   FocusChange(plbw, plbw->iFocus, TRUE);
	   }
       break;

/************************************************************************/
/************************************************************************/
/*									*/
/* Part	9: Control destruction coding					*/
/*									*/
/************************************************************************/
/************************************************************************/

   /*********************************************************************/
   /*  Control being destroyed,	perform	clean-up			*/
   /*********************************************************************/

   case	WM_DESTROY :
		       /* Get the address of the control info from the	*/
		       /* control's reserved memory                     */

       plbw = (PLISTBOXWIN)WinQueryWindowPtr(hWnd, QUCWP_WNDP);

		       /* Release the column heaps			*/

       if ( plbw->cItems )
	   for ( i = 0;	i < plbw->cColumns; i++	)
	       HeapRelease(plbw->plc[i].hHeap);

		       /* Destroy the scroll bars			*/

       if ( plbw->hwndScrollLeft )
	   WinDestroyWindow(plbw->hwndScrollLeft);
       if ( plbw->hwndScrollRight )
	   WinDestroyWindow(plbw->hwndScrollRight);
       if ( plbw->hwndScrollTop	)
	   WinDestroyWindow(plbw->hwndScrollTop);
       if ( plbw->hwndScrollBottom )
	   WinDestroyWindow(plbw->hwndScrollBottom);

		       /* Close	the open .WAV files and	release	the	*/
		       /* sound	support	DLL				*/

       UnloadSoundSupport(plbw);

		       /* Delete the check mark	bitmaps			*/

       GpiDeleteBitmap(plbw->hbm);

		       /* Release the heap				*/

       HeapRelease(plbw->hHeap);
       break;
		       /* Default message processing			*/
   default :
       return(WinDefWindowProc(hWnd, msg, mp1, mp2));
   }
return(0L);
}
