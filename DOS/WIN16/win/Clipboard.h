/*
	@(#)Clipboard.h	1.7
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
 
/*
**   MiD   26-SEP-1995   Added HBITMAP field in CLIPTABLE to keep track of bitmaps
**                       as they are copied/pasted to/from the clipboard.
*/


#ifndef Clipboard__h
#define Clipboard__h

#define NHANDLES  8

typedef struct tagCLIPENTRY
{
   HANDLE  wFormat;
   int     offset;
   int     length;
   DWORD   atom;
} CLIPENTRY, *LPCLIPENTRY;

typedef struct tagDRVCLIPINFO
{
   DWORD        winOwner;          /* X-window (cast to Window)     */
   DWORD        ClipboardTime;     /* MUST CAST TO (Time) in Driver */
   DWORD        CutBufferTime;     /* MUST CAST TO (Time) in Driver */
} DRVCLIPINFO, *LPDRVCLIPINFO;
 
typedef struct tagCLIPTABLE
{
   HWND         hwndOpen;          /* who opened?                  */
   HWND         hwndOwner;         /* who is the owner?            */
   HBITMAP      hbmpClip;          /* handle of last bitmap, put to the clipboard */
   HWND        *ViewerList;
   int          ViewerCount;
   LPDRVCLIPINFO       lpDrvData;
} CLIPTABLE, *LPCLIPTABLE;

#define GETCOUNT(lp)      GETDWORD(lp)
#define GETFORMAT(lp, i)  GETDWORD(lp + i * sizeof(DWORD))
#define GETLENGTH(lp, i)  GETDWORD(lp + i * sizeof(DWORD) + sizeof(DWORD))

#endif /* Clipboard__h */
