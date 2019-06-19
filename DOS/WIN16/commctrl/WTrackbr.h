
/*  WTrackbr.h	1.2 TRACKBAR CONTROL 
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

#ifndef __WTRACKBAR_H__
#define __WTRACKBAR_H__

#define WTB_ERR_GENERAL					-1
#define WTB_ERR_REGISTERCLASS			-2
#define WTB_ERR_GLOBALALLOC				-3
#define WTB_ERR_GLOBALLOCK				-4
#define	WTB_ERR_LISTCOUNT				-5
#define WTB_ERR_LISTLOCK				-6
#define WTB_ERR_OUTOFRANGE				-7
#define WTB_ERR_LISTCREATE				-8
#define WTB_ERR_NULLPOINTER				-9
#define WTB_ERR_NOSIZE					-10
#define WTB_ERR_LOCALALLOC				-11
#define WTB_ERR_LOCALLOCK				-12
#define WTB_ERR_OUTOFBOUNDS				-13
#define WTB_ERR_LISTINITIALIZE			-14
#define WTB_ERR_LISTINSERT				-15
#define WTB_ERR_LISTDESTROY				-16
#define WTB_ERR_GETDC					-17
#define WTB_ERR_DRAWTEXT				-18
#define WTB_ERR_HEADERINSERT			-19
#define WTB_ERR_LISTDELETE				-20
#define WTB_ERR_GETWINPROP			-21
#define WTB_ERR_CREATEWINDOW			-22
#define WTB_ERR_MEMORYLOCK			-23
#define WTB_ERR_MEMORYALLOC			-24


#ifdef _WINDOWS

#define WC_TRACKBAR				"Wmsctls_trackbar32"

#else /* #ifdef _WINDOWS */

#define WC_TRACKBAR				TRACKBAR_CLASS
#endif /* #ifdef _WINDOWS #else */


#endif /* #ifdef __WTRACKBAR_H__ */


