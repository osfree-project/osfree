/*  WToolbar.c	1.3 TOOLBAR CONTROL 
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

#ifndef __WTOOLBARCONTROL_H__
#define __WTOOLBARCONTROL_H__

#define WTLB_ERR_GENERAL					-1
#define WTLB_ERR_REGISTERCLASS			-2
#define WTLB_ERR_GLOBALALLOC				-3
#define WTLB_ERR_GLOBALLOCK				-4
#define	WTLB_ERR_LISTCOUNT				-5
#define WTLB_ERR_LISTLOCK				-6
#define WTLB_ERR_OUTOFRANGE				-7
#define WTLB_ERR_LISTCREATE				-8
#define WTLB_ERR_NULLPOINTER				-9
#define WTLB_ERR_NOSIZE					-10
#define WTLB_ERR_LOCALALLOC				-11
#define WTLB_ERR_LOCALLOCK				-12
#define WTLB_ERR_OUTOFBOUNDS				-13
#define WTLB_ERR_LISTINITIALIZE			-14
#define WTLB_ERR_LISTINSERT				-15
#define WTLB_ERR_LISTDESTROY				-16
#define WTLB_ERR_GETDC					-17
#define WTLB_ERR_DRAWTEXT				-18
#define WTLB_ERR_HEADERINSERT			-19
#define WTLB_ERR_LISTDELETE				-20
#define WTLB_ERR_CREATEWINDOW			-22
#define WTLB_ERR_GETWINPROP			-23
#define WTLB_ERR_GETNAVIGATOR			-24

#ifdef _WINDOWS

#define WTOOLBARCLASSNAME			"WToolbarWindow32"

#else /* ifdef _WINDOWS */
#define WTOOLBARCLASSNAME			TOOLBARCLASSNAME
#define WCreateToolbarEx			CreateToolbarEx
#endif

#endif
