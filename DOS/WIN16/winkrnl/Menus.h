/*
	@(#)Menus.h	2.5
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
 
#ifndef Menus__h
#define Menus__h

#define	CTLCOLOR_MENU	0x0010

/* ChangeItem action bits */
#define	LCA_GET		0x0000
#define	LCA_SET		0x8000
#define	LCA_CONTENTS	0x0001
#define	LCA_CHECKBMP	0x0002
#define	LCA_UNCHECKBMP	0x0004
#define	LCA_LEFTINDENT	0x0008
#define	LCA_RIGHTINDENT	0x0010
#define	LCA_FLAGS	0x0020
#define	LCA_RECT	0x0040
#define	LCA_ITEMID	0x0080
#define	LCA_ITEMCOUNT	0x0100
#define	LCA_FONT	0x0200
#define	LCA_SELECTION	0x0400
#define	LCA_STATE	0x0800

#define	LCA_BITMAPS	(LCA_CHECKBMP | LCA_UNCHECKBMP)
#define	LCA_INDENTS	(LCA_LEFTINDENT | LCA_RIGHTINDENT)
#define	LCA_ALL		(LCA_INDENTS | LCA_BITMAPS | LCA_FLAGS | \
			LCA_CONTENTS | LCA_ITEMID)

#define LBA_CREATE	0
#define LBA_DESTROY	1
#define LBA_MODIFYITEM	2
#define LBA_INSERTITEM	3
#define LBA_APPENDITEM	4
#define LBA_DELETEITEM	5
#define LBA_REMOVEITEM	6
#define LBA_GETDATA	7
#define LBA_SETDATA	8

typedef struct tagMENUITEMSTRUCT
{
    WORD	wPosition;		/* position to insert */
    WORD	wIDNewItem;		/* command ID/menu handle */
    WORD	wAction;		/* action bits for change */
    HBITMAP	hCheckedBmp;		/* checkmark bitmap */
    HBITMAP	hUncheckedBmp;		/* unchecked bitmap */
    WORD	wLeftIndent;		/* left indent */
    WORD	wRightIndent;		/* right indent */
    WORD	wItemFlags;		/* item flags */
    LPSTR	lpItemData;		/* item contents */
} MENUITEMSTRUCT;

typedef MENUITEMSTRUCT *LPMENUITEMSTRUCT;

typedef struct tagMENUCREATESTRUCT
{
    HFONT	hFont;
    DWORD	dwStyle;
    DWORD	dwIndents;
} MENUCREATESTRUCT;

typedef MENUCREATESTRUCT *LPMENUCREATESTRUCT;

typedef struct tagTRACKPOPUPSTRUCT
{
    HMENU	hMenu;
    UINT	uiFlags;
    int		x;
    int		y;
    BOOL	bSystemMenu;
    HWND	hWndOwner;
    RECT	*lprc;
    HWND	hPopups[5];	/* this should be a linked list */
    WORD	wPopupFlags[5];
    int		nPopups;	/* number of popups */
} TRACKPOPUPSTRUCT;

typedef TRACKPOPUPSTRUCT *LPTRACKPOPUPSTRUCT;

#define	TP_STATUS	0
#define	TP_LPTPS	2

#define TP_TRACKPOPUP   0x1000
#define TP_MENUBAR	0x2000

#define	PSF_POPUPACTIVE		0x0001
#define	PSF_POPSELKILLED	0x0002
#define	PSF_BUTTONDOWN		0x0004

/* private menu message */
#define MM_MENUINIT 	WM_USER+1 /* wParam -- selection, lParam = 0 */

#endif /* Menus__h */
