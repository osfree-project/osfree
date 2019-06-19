
/*  WProgBar.h	1.1 PROGRESSBAR CONTROL 
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

#ifndef __WPROGRESSBAR_H__
#define __WPROGRESSBAR_H__

#define WPB_ERR_GENERAL					-1
#define WPB_ERR_REGISTERCLASS			-2
#define WPB_ERR_GLOBALALLOC				-3
#define WPB_ERR_GLOBALLOCK				-4
#define	WPB_ERR_LISTCOUNT				-5
#define WPB_ERR_LISTLOCK				-6
#define WPB_ERR_OUTOFRANGE				-7
#define WPB_ERR_LISTCREATE				-8
#define WPB_ERR_NULLPOINTER				-9
#define WPB_ERR_NOSIZE					-10
#define WPB_ERR_LOCALALLOC				-11
#define WPB_ERR_LOCALLOCK				-12
#define WPB_ERR_OUTOFBOUNDS				-13
#define WPB_ERR_LISTINITIALIZE			-14
#define WPB_ERR_LISTINSERT				-15
#define WPB_ERR_LISTDESTROY				-16
#define WPB_ERR_GETDC					-17
#define WPB_ERR_DRAWTEXT				-18
#define WPB_ERR_HEADERINSERT			-19
#define WPB_ERR_LISTDELETE				-20
#define WPB_ERR_GETWINPROP			-21
#define WPB_ERR_CREATEWINDOW			-22
#define WPB_ERR_MEMORYLOCK			-23
#define WPB_ERR_MEMORYALLOC			-24


#ifdef _WINDOWS
#define WC_PROGRESSBAR				"Wmsctls_progress32"
#else /* #ifdef _WINDOWS */
#define WC_PROGRESSBAR				PROGRESS_CLASS
#endif /* #ifdef _WINDOWS #else */

void WINAPI ProgBar_Terminate ( HINSTANCE );
int WINAPI ProgBar_Initialize ( HINSTANCE );

#endif /* #ifdef __WTRACKBAR_H__ */


