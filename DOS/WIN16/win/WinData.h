/*
	@(#)WinData.h	1.7
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
 
/* mask bits for the DRVWINDOWDATA components */
#define	DWD_FRAME	0x0001
#define	DWD_CLIENT	0x0002
#define	DWD_HSCROLL	0x0004
#define	DWD_VSCROLL	0x0008

/* functions for PWSH_SHOWWINDOW */
#define	DSW_HIDE	0x0000
#define	DSW_SHOW	0x0001

/* functions for TWIN_GenerateExpose */
#define EXPOSE_HIDE	0x0001
#define EXPOSE_MOVE	0x0002
#define EXPOSE_SIZE	0x0003
#define EXPOSE_SIZEMOVE	0x0004

typedef struct tagWINCREATESTRUCT
{
    /* this is the common portion */
    LPCSTR	lpszName;
    DWORD	dwStyle;
    DWORD	dwExStyle;
    int		nBorder;
    LPVOID	lpIconXORData;  /* Drv image data for icon XOR bits */
    LPVOID	lpIconANDData;  /* Drv image data for icon AND mask */
    LPVOID	Cursor;		/* Drv cursor data */
    /* this is per component */
    HWND	hWndFrame;
    HWND	hWndClient;
    RECT	rcFrame;
} WINCREATESTRUCT;
typedef WINCREATESTRUCT *LPWINCREATESTRUCT;
