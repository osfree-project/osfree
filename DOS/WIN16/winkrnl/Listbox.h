/*
	@(#)Listbox.h	2.11
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
 
#ifndef Listbox__h
#define Listbox__h

#include "ObjEngine.h"

/* Listbox item data struct */
typedef struct tagLISTBOXITEM
  {
    struct tagLISTBOXITEM *lpNextItem;	/* ptr to the next */
    WORD	wSlotIndex;		/* slot index */
    WORD	wItemFlags;		/* per-item state flags */
    WORD	wItemLeftIndent;	/* item left indent value */
    WORD	wItemRightIndent;	/* item right indent value */
    POINT	pt;			/* item position in parent coords */
    WORD	wItemWidth;		/* item width */
    WORD	wItemHeight;		/* item height */
    HBITMAP	hCheckedBmp;		/* checkmark bitmap */
    HBITMAP	hUncheckedBmp;		/* unchecked bitmap */
    HMENU	hPopupMenu;		/* popup menu/command ID */
    HWND	hWndPopup;		/* Associated popup menu window */
    DWORD	dwItemData;		/* item-associated data */
    LPSTR	lpItemString;		/* ptr to the item string */
  } LISTBOXITEM;

typedef LISTBOXITEM *LPLISTBOXITEM;

/* Listbox info data structure */
typedef struct tagLISTBOXINFO
  {
    OBJHEAD	ObjHead;		/* generic object header */
    HWND	hWnd;			/* Listbox window handle */
    HMENU	hListBox;		/* menu handle for LBS_PRELOADED */
    HWND	hWndParent;		/* Listbox parent/owner */
    WORD	wLBoxStyle;		/* Listbox style */
    WORD	wState;			/* State flags */
    HFONT	hFont;			/* Listbox font handle */
    WORD	wItemHeight;		/* Item height for fixed */
    WORD	wLeftIndent;		/* left indent */
    WORD	wRightIndent;		/* right indent */
    WORD	wTopIndex;		/* visual frame top */
    WORD	wFrameSize;		/* visual frame size */
    WORD	wColumnWidth;		/* column width for MULTICOLUMN */
    WORD	wHorzExtent;		/* horizontal extent for scrolling */
    WORD	wHorzOffset;		/* horizontal offset for scrolling */
    BOOL	bPosChanged;		/* dirty flag */
    BOOL	bFrameChanged;		/* dirty flag */
    WORD	wItemCount;		/* count of items */    
    WORD	wSelCount;		/* selections count (multi)*/    
    WORD	wSelection;		/* current selection (single) */
    WORD	wSlotCount;		/* available slots */
    LPLISTBOXITEM *lpItemList;		/* ptr to the list of items */
    LPLISTBOXITEM lpListHead;		/* item list head */
    LPLISTBOXITEM lpListTail;		/* item list tail */
  } LISTBOXINFO;

typedef LISTBOXINFO *LPLISTBOXINFO;

typedef LISTBOXINFO LBOXINFO;
typedef LISTBOXINFO *LPLBOXINFO;

/* Private LISTBOX window data */
#define	LWD_HMENU	0x0000
#define	LWD_LPMENUDATA	0x0002

/* Slot space realloc granularity */
#define	LPD_SLOTG	16

/* Private LISTBOX state bits */
#define	LSF_DISABLED		ODS_DISABLED 	/* 0x0004 */
#define	LSF_FOCUS		ODS_FOCUS	/* 0x0010 */
#define	LSF_REDRAW		0x0100
#define	LSF_BUTTONDOWN		0x0200
#define	LSF_CAPTUREACTIVE	0x0400
#define	LSF_POPUPACTIVE		0x0800
#define	LSF_SCROLLENABLED	0x1000
#define	LSF_HSCROLLDISABLED	0x4000
#define	LSF_VSCROLLDISABLED	0x8000

/* Private item state flags */
#define	LIF_ENABLED	MF_ENABLED	/* 0x0000 */
#define	LIF_GRAYED	MF_GRAYED	/* 0x0001 */
#define	LIF_DISABLED	MF_DISABLED	/* 0x0002 */
#define	LIF_STRING	MF_STRING	/* 0x0000 */
#define	LIF_BITMAP	MF_BITMAP	/* 0x0004 */
#define	LIF_UNCHECKED	MF_UNCHECKED	/* 0x0000 */
#define	LIF_CHECKED	MF_CHECKED	/* 0x0008 */
#define	LIF_POPUP	MF_POPUP	/* 0x0010 */
#define	LIF_MENUBARBREAK MF_MENUBARBREAK /* 0x0020 */
#define	LIF_MENUBREAK	MF_MENUBREAK	/* 0x0040 */
#define	LIF_HILITE	MF_HILITE	/* 0x0080 */
#define	LIF_OWNERDRAW	MF_OWNERDRAW	/* 0x0100 */
#define	LIF_USECHECKBITMAPS MF_USECHECKBITMAPS /* 0x0200 */
#define	LIF_BYCOMMAND	MF_BYCOMMAND	/* 0x0000 */
#define	LIF_BYPOSITION	MF_BYPOSITION	/* 0x0400 */
#define	LIF_SEPARATOR	MF_SEPARATOR	/* 0x0800 */

#define	MF_BYSLOTINDEX	0x8000

/* AddItem functions */
#define	LAF_APPEND	0x0000
#define	LAF_INSERT	0x0001

#define	LBS_OWNERDRAWMASK (LBS_OWNERDRAWFIXED | LBS_OWNERDRAWVARIABLE)

#define	BOWNDATA(l) (((l)->wLBoxStyle & LBS_OWNERDRAWMASK)\
			&& !((l)->wLBoxStyle & LBS_HASSTRINGS))

#define	BOWNERDRAW(l) ((l)->wLBoxStyle & LBS_OWNERDRAWMASK)

#define	BSINGLESEL(l) (!((l)->wLBoxStyle & \
			(LBS_MULTIPLESEL|LBS_EXTENDEDSEL)))

#define	LBoxSetState(lpLBInfo,wMask) lpLBInfo->wState |= (wMask)
#define	LBoxClearState(lpLBInfo,wMask) lpLBInfo->wState &= ~(wMask)
#define	LBoxTestState(lpLBInfo,wMask) (lpLBInfo->wState & (wMask))

#endif /* Listbox__h */
