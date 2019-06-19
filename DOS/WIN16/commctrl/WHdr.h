
/*  WHdr.h
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

#ifndef __WHEADER_H__
#define __WHEADER_H__

#define WHDR_ERR_GENERAL				-1
#define WHDR_ERR_REGISTERCLASS		-2
#define WHDR_ERR_GLOBALALLOC			-3
#define WHDR_ERR_GLOBALLOCK			-4
#define	WHDR_ERR_LISTCOUNT			-5
#define WHDR_ERR_LISTLOCK			-6
#define WHDR_ERR_OUTOFRANGE			-7
#define WHDR_ERR_LISTCREATE			-8
#define WHDR_ERR_NULLPOINTER			-9
#define WHDR_ERR_NOSIZE				-10
#define WHDR_ERR_LOCALALLOC			-11
#define WHDR_ERR_LOCALLOCK			-12
#define WHDR_ERR_OUTOFBOUNDS			-13
#define WHDR_ERR_LISTINITIALIZE		-14
#define WHDR_ERR_LISTINSERT			-15
#define WHDR_ERR_LISTDESTROY			-16
#define WHDR_ERR_GETDC				-17
#define WHDR_ERR_DRAWTEXT			-18
#define WHDR_ERR_HEADERINSERT		-19
#define WHDR_ERR_LISTDELETE			-20
#define WHDR_ERR_GETWINPROP			-21
#define WHDR_ERR_CREATEWINDOW		-22
#define WHDR_ERR_MEMORYLOCK			-23
#define WHDR_ERR_MEMORYALLOC			-24

#define TVLBOXID                    100
#define MAXSTRING                   96
#define HLVITEM                     long               

#if defined _WINDOWS
#define WHD_CLASSNAME                   "WSysHeader32"
#else
#define WHD_CLASSNAME                   "SysHeader32"
#endif

#endif /* #ifdef __WHEADER_H__ */

typedef struct
{
    HD_ITEM     data;
    RECT        rc;
    char        text[MAXSTRING];
} HEADERITEM, *PHEADERITEM;


typedef struct 
{
    int                 enDrag; /* for dragging */
    POINT               ptDrag;

    BOOL                bShow;
    UINT         		uStyles;
    int                 iCount;
    HFONT       		hFont;
    COLORREF            BkColor;
    BOOL                bRecalc;
    int                 eMouse;
    int                 iCurSel;
    HCURSOR             hDivider;
    HCURSOR             hDivOpen;
    HCURSOR             hArrow;
    WLIST               Items;
} HEADERDATA, *PHEADERDATA;




static BOOL
Headr_OnGetItem
( 
    HWND                hWnd, 
    HEADERDATA*         pthis,
    int                 iItem,
    HD_ITEM*            lphdi
);

static int
Headr_OnHitTest
( 
    HWND                hWnd, 
    HEADERDATA*         pthis,
    HD_HITTESTINFO*     lpht
);

static BOOL
Headr_OnSetItem
( 
    HWND                hWnd, 
    HEADERDATA*         pthis,
    int                 iItem,
    HD_ITEM*            lphdi
);
