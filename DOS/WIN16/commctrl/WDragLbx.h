
/* WDragLbx.h	DRAGLISTBOX CONTROL 
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

#ifndef __WDRAGLISTBOX_H__
#define __WDRAGLISTBOX_H__

#define WDB_ERR_GENERAL					-1
#define WDB_ERR_REGISTERCLASS			-2
#define WDB_ERR_GLOBALALLOC				-3
#define WDB_ERR_GLOBALLOCK				-4
#define	WDB_ERR_LISTCOUNT				-5
#define WDB_ERR_LISTLOCK				-6
#define WDB_ERR_OUTOFRANGE				-7
#define WDB_ERR_LISTCREATE				-8
#define WDB_ERR_NULLPOINTER				-9
#define WDB_ERR_NOSIZE					-10
#define WDB_ERR_LOCALALLOC				-11
#define WDB_ERR_LOCALLOCK				-12
#define WDB_ERR_OUTOFBOUNDS				-13
#define WDB_ERR_LISTINITIALIZE			-14
#define WDB_ERR_LISTINSERT				-15
#define WDB_ERR_LISTDESTROY				-16
#define WDB_ERR_GETDC					-17
#define WDB_ERR_DRAWTEXT				-18
#define WDB_ERR_HEADERINSERT			-19
#define WDB_ERR_LISTDELETE				-20
#define WDB_ERR_CREATEWINDOW			-22
#define WDB_ERR_GETWINPROP			    -23
#define WDB_ERR_SETWINDOWLONG		    -24

#ifndef _WINDOWS
#define WMakeDragList			MakeDragList
#define WLBItemFromPt			LBItemFromPt
#define WDrawInsert			DrawInsert
#else
BOOL WINAPI WMakeDragList ( HWND hwLB );
int WINAPI WLBItemFromPt ( HWND hwLB, POINT pt,BOOL bAutoScroll	);
void WINAPI WDrawInsert ( HWND hwParent, HWND hwLB, int nItem );
#endif

#endif
