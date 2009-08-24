/********************************************************************
	@(#)Scroll.h	2.8
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

********************************************************************/
 
#ifndef Scroll__h
#define Scroll__h

#include "ObjEngine.h"

typedef struct tagTWINSCROLLINFO
{
    OBJHEAD	ObjHead;		/* generic object header */
    DWORD	dwStyle;		/* copy of the window style */
    HWND	hWnd;			/* my window */
    WORD	nMin;			/* min position */
    WORD	nMax;			/* max position */
    WORD	nPos;			/* current position */
    WORD	nTrackingPos;		/* current tracking pos */
    WORD	wState;			/* state flags */
    WORD	wArrowWidth;		/* width of the arrow */
    WORD	wArrowHeight;		/* height of the arrow */
    WORD	wThumbOffset;		/* offset of the thumb in pixels */
    WORD	wTrackingOffset;	/* tracking offset of the thumb */
} TWINSCROLLINFO;

typedef TWINSCROLLINFO *LPTWINSCROLLINFO;


void ScrollbarSendItemDraw(LPTWINSCROLLINFO,WORD,WORD);
void ScrollbarDraw(LPTWINSCROLLINFO,LPDRAWITEMSTRUCT);

/* additional styles */
#define	SBS_OWNERDRAW	0x0100

/* additional owner-draw type */
#define	ODT_SCROLLBAR	5

/* Scrollbar ownerdraw action bits */
#define	ODA_LEFTARROW	0x0100
#define	ODA_UPARROW	0x0100
#define	ODA_RIGHTARROW	0x0200
#define	ODA_DOWNARROW	0x0200
#define	ODA_THUMB	0x0400
#define	ODA_THUMBTRACK	0x0800
#define	ODA_LEFTFIELD	0x1000
#define	ODA_UPFIELD	0x1000
#define	ODA_RIGHTFIELD	0x2000
#define	ODA_DOWNFIELD	0x2000

/* state flags */
#define	SSF_NOTIFYSENT	0x0001
#define	SSF_HIDDEN	0x0002
#define	SSF_MOUSEOUT	0x0004
#define	SSF_UPARROW	0x0008
#define	SSF_LEFTARROW	0x0008
#define	SSF_DOWNARROW	0x0010
#define	SSF_RIGHTARROW	0x0010
#define	SSF_UPFIELD	0x0020
#define	SSF_LEFTFIELD	0x0020
#define	SSF_DOWNFIELD	0x0040
#define	SSF_RIGHTFIELD	0x0040
#define	SSF_THUMB	0x0080

#define	SSF_THUMBTRACK	0x0100

#define	SSF_LEFTARROWGRAYED	0x1000
#define	SSF_UPARROWGRAYED	0x1000
#define	SSF_RIGHTARROWGRAYED	0x2000
#define	SSF_DOWNARROWGRAYED	0x2000
#define	SSF_GRAYED	(SSF_LEFTARROWGRAYED|SSF_RIGHTARROWGRAYED)

#define	SSF_CLICKED (SSF_LEFTARROW|SSF_RIGHTARROW|SSF_LEFTFIELD|\
		     SSF_RIGHTFIELD|SSF_THUMB)

/* Private SCROLLBAR window data */
#define	SWD_LPSCROLLINFO	0

/* Timer IDs */
#define	IDT_FIRSTCLICK	500
#define	IDT_HOLDCLICK	501

#define	CTICKS_FIRSTCLICK	400
#define	CTICKS_HOLDCLICK	50

/* Hittest events */
#define	SBH_NULL		0
#define	SBH_HITLEFTARROW	1
#define	SBH_HITUPARROW		1
#define	SBH_HITRIGHTARROW	2
#define	SBH_HITDOWNARROW	2
#define	SBH_HITLEFTFIELD	3
#define	SBH_HITUPFIELD		3
#define	SBH_HITRIGHTFIELD	4
#define	SBH_HITDOWNFIELD	4
#define	SBH_HITTHUMB		5

#define	SBTSTATE(l,m) ((l)->wState & (m))
#define	SBCSTATE(l,m) (l)->wState&=~(m)
#define	SBSSTATE(l,m) (l)->wState|=(m)
#define	SBTSTYLE(l,m) ((l)->dwStyle & (m))

/* Motif-look-and-feel bitmaps */
#define OBM_M_UPARROW	32713
#define OBM_M_DNARROW	32712
#define OBM_M_RGARROW	32711
#define OBM_M_LFARROW	32710
#define OBM_M_UPARROWD	32703
#define OBM_M_DNARROWD	32702
#define OBM_M_RGARROWD	32701
#define OBM_M_LFARROWD	32700

#endif /* Scroll__h */
