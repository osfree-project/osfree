/*    
	CommdlgIT.c	1.5
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

*/


#define NOAPIPROTO
#include "windows.h"
#include "Resources.h"
#include "ModTable.h"
#include "Segment.h"
#include "Log.h"

extern long int Trap();

extern long int IT_1LPOF();
extern long int IT_2LP1UI();
extern long int IT_PRINTDLG();
extern long int IT_CHOOSECOLOR();
extern long int IT_CHOOSEFONT();
extern long int IT_FINDREPLACE();
extern long int IT_V();

	/* These function prototypes conflict with commdlg.h (included from
	 * windows.h) unless NOCOMMDLG is defined.
	 */
extern long int GetOpenFileName();
extern long int GetSaveFileName();
extern long int GetFileTitle();
extern long int PrintDlg();
extern long int ChooseColor();
extern long int ChooseFont();
extern long int FindText();
extern long int ReplaceText();
extern long int CommDlgExtendedError();

 /* Interface Segment Image COMMDLG: */

static long int (*seg_image_COMMDLG_0[])() =
{	/* nil */	0, 0,
	/* 001 */	IT_1LPOF,  GetOpenFileName,
	/* 002 */	IT_1LPOF,  GetSaveFileName,
	/* 003 */	Trap, 0,
	/* 004 */	Trap, 0,
	/* 005 */	IT_CHOOSECOLOR, ChooseColor,
	/* 006 */	Trap, 0,
	/* 007 */	Trap, 0,
	/* 008 */	Trap, 0,
	/* 009 */	Trap, 0,
	/* 00a */	Trap, 0,
	/* 00b */	IT_FINDREPLACE, FindText,
	/* 00c */	IT_FINDREPLACE, ReplaceText,
	/* 00d */	Trap, 0,
	/* 00e */	Trap, 0,
	/* 00f */	IT_CHOOSEFONT, ChooseFont,
	/* 010 */	Trap, 0,
	/* 011 */	Trap, 0,
	/* 012 */	Trap, 0,
	/* 013 */	Trap, 0,
	/* 014 */	IT_PRINTDLG, PrintDlg,
	/* 015 */	Trap, 0,
	/* 016 */	Trap, 0,
	/* 017 */	Trap, 0,
	/* 018 */	Trap, 0,
	/* 019 */	Trap, 0,
	/* 01a */	IT_V, CommDlgExtendedError,
	/* 01b */	IT_2LP1UI, GetFileTitle,
	0
};

 /* Segment Table COMMDLG: */

SEGTAB SegmentTableCOMMDLG[] =
{	{ (char *) seg_image_COMMDLG_0, 216, TRANSFER_CALLBACK, 216, 0, 0 },
	/* end */	{ 0, 0, 0, 0, 0, 0 }
};

extern void TWIN_InitCommdlg();

long int
Commdlg_LibMain()
{
	TWIN_InitCommdlg ();
	return(0);
}
