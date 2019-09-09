/*  WTView.h	1.5
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

#ifndef __WTREEVIEW_H__
#define __WTREEVIEW_H__

#define WTV_ERR_GENERAL				-1
#define WTV_ERR_REGISTERCLASS		-2
#define WTV_ERR_GLOBALALLOC			-3
#define WTV_ERR_GLOBALLOCK			-4
#define	WTV_ERR_LISTCOUNT			-5
#define WTV_ERR_LISTLOCK			-6
#define WTV_ERR_OUTOFRANGE			-7
#define WTV_ERR_LISTCREATE			-8
#define WTV_ERR_NULLPOINTER			-9
#define WTV_ERR_NOSIZE				-10
#define WTV_ERR_LOCALALLOC			-11
#define WTV_ERR_LOCALLOCK			-12
#define WTV_ERR_OUTOFBOUNDS			-13
#define WTV_ERR_LISTINITIALIZE		-14
#define WTV_ERR_LISTINSERT			-15
#define WTV_ERR_LISTDESTROY			-16
#define WTV_ERR_GETDC				-17
#define WTV_ERR_DRAWTEXT			-18
#define WTV_ERR_HEADERINSERT		-19
#define WTV_ERR_LISTDELETE			-20
#define WTV_ERR_GETWINPROP			-21
#define WTV_ERR_CREATEWINDOW		-22
#define WTV_ERR_MEMORYLOCK			-23
#define WTV_ERR_MEMORYALLOC			-24


#define TVLBOXID                    100
#define MAXSTRING                   96

#define WC_TVTIPTEXT				"WSysTViewTip32"

#ifdef _WINDOWS
#define WWC_TREEVIEW				"WSysTreeView32"
#else /* #ifdef _WINDOWS */
#define WWC_TREEVIEW				WC_TREEVIEW
#endif /* #ifdef _WINDOWS #else */

#endif /* #ifdef __WTRACKBAR_H__ */

typedef struct {
    char        szText[MAXSTRING];
    LPARAM      lParam;
    HTREEITEM   hItem;
    int         Ord;  /* for sorting without atually swapping key and szText */
} TVSORTITEM, *PTVSORTITEM; 

typedef struct {
    LPARAM      lParam;
} TVLBOXITEM, *PTVLBOXITEM; 

typedef struct {
    TV_ITEM     tvi;         
    int         nLevel;
    DWORD       dwConnect; /* bit array for drawing vertical line */
    BOOL        bOpened;
    HTREEITEM   Parent,
                FChild,   /* address of first child */
                LChild,
                NSibling,
                PSibling;
} TVITEMDATA, *PTVITEMDATA; 

typedef struct 
{
	char		Label[MAXSTRING];
    int         Next;
} TVSTR, *PTVSTR;


typedef struct 
{
    int         ItmHeight;
    int         ItmWidth;
    long        lCount;
    long        lDropSel;
    long        lCurSel;
    long        lCaret;
    long        lTopIndx;
    long        lOffset;
    int         iHExtent;
    BOOL        bFocus;
    WLIST       ItmList;
} TVLBOXDATA, *PTVLBOXDATA;


typedef struct {
	    UINT	    		uStyles;
        BOOL                bShow;
    	int         		nIndent;
        int                 enDrag; /* for dragging */
        POINT               ptDrag;
        TVLBOXDATA          psLBox;
    	HWND        		hwEdit;
        HWND                hwTipTxt;
    	unsigned long       lCount;
    	HFONT       		hFont;
    	HTREEITEM   		hEditItem; /* item that are being edited, 0 if none */
    	HTREEITEM   		hDeleted; /* array of TV Items deleted and availble for recycle */
        SIZE                ImgSize;
        SIZE                StImgSize;
   	    WLIST	    		ItmList;  /* array of TreeView Items */
    	WLIST       		StrList;
    	HIMAGELIST  		hImgList;
    	HIMAGELIST  		hStImgList;
} TREEVIEWDATA, *PTREEDATA;

HGLOBAL
TView_IGetResource ( HWND hwTree, PTREEDATA* ppTree, PTVITEMDATA* ppItmList, PTVSTR* ppStrList );
void
TView_IReleaseResource ( HGLOBAL hRsrc, PTREEDATA pTree, PTVITEMDATA* ppItmList, PTVSTR* ppStrList );
void 
TView_OnKeyDown (HWND hWnd, int VKey, int cRepeat, UINT keyFlags);
void
TView_OnDrawItem ( HWND hwnd, TREEVIEWDATA* pthis, int iIndx );





