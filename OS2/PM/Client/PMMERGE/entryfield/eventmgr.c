#pragma	title("List Box Replacement  --  Version 1.1 -- (EventMgr.C)")
#pragma	subtitle("   Event Manager - Interface Definitions")

/* Program name: Listbox.C    Title: A List Box	Replacement		*/
/*									*/
/* OS/2	Developer Magazine, Issue:  Jan	'94, page 66                    */
/* Author:  Mark Benge	   IBM Corp.					*/
/*	    Matt Smith	   Prominare Inc.				*/
/* Description:	 Replacement for OS/2 List Box,	first of a series.	*/
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

#define	INCL_WIN		   /* Include OS/2 PM Windows Interface	*/

#include <os2.h>

#include "entryfld.h"

/* This	module contains	the routines that handle the event management	*/
/* for the list	box.  All notifications	to the owner window are	handled	*/
/* through these routines.						*/
/*									*/
/* Equivalent command line invocation of each module using the		*/
/* IBM C Set++ Compiler	Version	2.0 is:					*/
/*									*/
/*     Icc -G3e- -O+ -Rn -C -W3	-FoEventMgr EventMgr.C			*/

/* Filename:   EventMgr.C						*/

/*  Version:   1.1							*/
/*  Created:   1994-03-01						*/
/*  Revised:   1994-03-01						*/

/* Routines:   MRESULT mrNotifyOwner(PENTRYFWIN	pefw,		       */
/*				     ULONG ulNotification);		*/

/* --------------------------------------------------------------------	*/

#pragma	subtitle("   Event Manager - Owner Notification Procedure")
#pragma	page( )

/* --- mrNotifyOwner ----------------------------------- [ Public } ---	*/
/*									*/
/*     This function is	used to	send the notification message to the	*/
/*     window owning the list box control.  The	notification message	*/
/*     is one of the defined messages for the control.			*/
/*									*/
/* Standard List Box notification messages				*/
/*									*/
/*     EN_SETFOCUS		  0x0001				*/
/*     EN_KILLFOCUS		  0x0002				*/
/*     EN_CHANGE		  0x0004				*/
/*     EN_SCROLL		  0x0008				*/
/*     EN_MEMERROR		  0x0010				*/
/*     EN_OVERFLOW		  0x0020				*/
/*     EN_INSERTMODETOGGLE	  0x0040				*/
/*									*/
/*     Upon Entry:							*/
/*									*/
/*     PENTRYFWIN pefw;		  = List Box Internal Data Pointer     */
/*     ULONG	   ulNotification; = Notification Value			*/
/*									*/
/*     Upon Exit:							*/
/*									*/
/*     mrNotifyOwner = Notification Result				*/
/*									*/
/* --------------------------------------------------------------------	*/

MRESULT	mrNotifyOwner(PENTRYFWIN pefw, ULONG ulNotification)

{

return(WinSendMsg(pefw->hwndOwner, WM_CONTROL,
		  MPFROM2SHORT(pefw->id, (USHORT)ulNotification),
		  MPFROMHWND((pefw->hWnd))));
}
