/*
	@(#)Button.h	2.8
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
 
#ifndef Button__h
#define Button__h

#include "WindowBytes.h"
/* Private definitions for the DefBUTTONProc */

/* Private item action flags */
#define	ODA_TEXT	0x0100
#define	ODA_BACK	0x0200

/* Private window data offsets */
#define	BWD_STATUS	0x0000
#define	BWD_HFONT	0x0002
#define	BWD_GROUPID	0x0004

/* Private button status flags */
#define	BSF_CHECKED	0x0001
#define	BSF_GRAYED	0x0002
#define	BSF_SELECTED	0x0004
#define	BSF_FOCUS	0x0008
#define	BSF_DISABLED	0x0010
#define	BSF_BUTTONDOWN	0x0040
#define	BSF_CAPTUREACTIVE 0x0080
#define	BSF_CHECKMASK	(BSF_CHECKED|BSF_GRAYED)
#define	BSF_STATEMASK	(BSF_CHECKED|BSF_GRAYED|BSF_SELECTED|BSF_FOCUS)

/* Macros */
#define	GetButtonStatus(h) GetWindowByte(h,BWD_STATUS)
#define SetButtonStatus(h,m) SetWindowByte(h,BWD_STATUS,m)
#define OrButtonStatus(h,m) OrWindowByte(h,BWD_STATUS,m)
#define	TestButtonStatus(h,m) TestWindowByte(h,BWD_STATUS,m)
#define ClearButtonStatus(h,m) AndWindowByte(h,BWD_STATUS,m)
#define ToggleButtonStatus(h,m) XorWindowByte(h,BWD_STATUS,m)

extern void TWIN_Cycle3StateButton(HWND hWnd);

#endif /* Button__h */
