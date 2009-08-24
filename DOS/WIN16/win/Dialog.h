/*
	@(#)Dialog.h	2.5
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
 
#ifndef Dialog__h
#define Dialog__h

/* first 3 longwords are reserved for Windows-defined offsets:
	DWL_MSGRESULT	0
	DWL_DLGPROC	DWL_MSGRESULT+sizeof(LRESULT)
	DWL_USER	DWL_DLGPROC+sizeof(LRESULT)
*/
#define DWL_LPARAM	DWL_USER+sizeof(LRESULT)
#define	DWW_STATUS	DWL_LPARAM+sizeof(LRESULT)
#define DWW_PARENT	DWW_STATUS+sizeof(WORD)
#define DWW_RESULT	DWW_PARENT+sizeof(WORD)
#define DWW_HFONT	DWW_RESULT+sizeof(WORD)
#define DWW_DEFID	DWW_HFONT+sizeof(WORD)

typedef struct {
	LPSTR	lpszCaption;
	LPSTR	lpszText;
	HICON	hIcon;
} SHELLABOUTDATA;

typedef struct {
	void	      *cf_lpStruct;
    	char 	      cf_cwd[256];
	int           cf_state;
} COMMONFILEINFO;

typedef struct {
	LPSTR	text;
	LPSTR	caption;
	RECT	crect;
	DWORD	lParam;
	WORD	flag;
} MESSAGEBOXDATA;

/* prototypes */

HWND InternalCreateDialogIndirectParam(HINSTANCE,const VOID *,HWND,DLGPROC,LPARAM);
int InternalDialog(HWND);

#endif /* Dialog__h */
