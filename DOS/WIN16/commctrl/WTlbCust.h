/*  WTlbCust.h	1.1
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

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/

#ifndef _WTOOLBAR_CUSTOMIZE_H
#define _WTOOLBAR_CUSTOMIZE_H

#define MAX_BTNTXTLEN   40

typedef struct 
{
	int		iCmdID;
	char	Text[MAX_BTNTXTLEN];
} TBCDLGBTN;

#endif

LRESULT CALLBACK 
Toolbr_CustomizeWndProc
(
	HWND				hWnd,
    UINT				uMessage,
    WPARAM				wParam,
    LPARAM				lParam
);
