/*  WPropsht.h	1.2 TAB CONTROL 
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

#ifndef __WPROPSHEET_H__
#define __WPROPSHEET_H__

#define WPS_ERR_GENERAL					-1
#define WPS_ERR_REGISTERCLASS			-2
#define WPS_ERR_GLOBALALLOC				-3
#define WPS_ERR_GLOBALLOCK				-4
#define	WPS_ERR_LISTCOUNT				-5
#define WPS_ERR_LISTLOCK				-6
#define WPS_ERR_OUTOFRANGE				-7
#define WPS_ERR_LISTCREATE				-8
#define WPS_ERR_NULLPOINTER				-9
#define WPS_ERR_NOSIZE					-10
#define WPS_ERR_LOCALALLOC				-11
#define WPS_ERR_LOCALLOCK				-12
#define WPS_ERR_OUTOFBOUNDS				-13
#define WPS_ERR_LISTINITIALIZE			-14
#define WPS_ERR_LISTINSERT				-15
#define WPS_ERR_LISTDESTROY				-16
#define WPS_ERR_GETDC					-17
#define WPS_ERR_DRAWTEXT				-18
#define WPS_ERR_HEADERINSERT			-19
#define WPS_ERR_LISTDELETE				-20
#define WPS_ERR_CREATEWINDOW			-22
#define WPS_ERR_GETWINPROP			-23

#ifndef _WINDOWS
#define WPropertySheet				PropertySheet
#define WCreatePropertySheetPage		CreatePropertySheetPage	
#define WDestroyPropertySheetPage		DestroyPropertySheetPage	
#endif

#endif
