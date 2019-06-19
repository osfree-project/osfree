
/*  WLView.h	1.1 
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

#ifndef __WLISTVIEW_H__
#define __WLISTVIEW_H__

#define WLV_ERR_GENERAL				-1
#define WLV_ERR_REGISTERCLASS		-2
#define WLV_ERR_GLOBALALLOC			-3
#define WLV_ERR_GLOBALLOCK			-4
#define	WLV_ERR_LISTCOUNT			-5
#define WLV_ERR_LISTLOCK			-6
#define WLV_ERR_OUTOFRANGE			-7
#define WLV_ERR_LISTCREATE			-8
#define WLV_ERR_NULLPOINTER			-9
#define WLV_ERR_NOSIZE				-10
#define WLV_ERR_LOCALALLOC			-11
#define WLV_ERR_LOCALLOCK			-12
#define WLV_ERR_OUTOFBOUNDS			-13
#define WLV_ERR_LISTINITIALIZE		-14
#define WLV_ERR_LISTINSERT			-15
#define WLV_ERR_LISTDESTROY			-16
#define WLV_ERR_GETDC				-17
#define WLV_ERR_DRAWTEXT			-18
#define WLV_ERR_HEADERINSERT		-19
#define WLV_ERR_LISTDELETE			-20
#define WLV_ERR_GETWINPROP			-21
#define WLV_ERR_CREATEWINDOW		-22
#define WLV_ERR_MEMORYLOCK			-23
#define WLV_ERR_MEMORYALLOC			-24


#define TVLBOXID                    100
#define MAXSTRING                   96
#define HLVITEM                     long               

#ifdef _WINDOWS
#define WWC_LISTVIEW				"WSysListView32"
#else /* #ifdef _WINDOWS */
#define WWC_LISTVIEW				WC_LISTVIEW
#endif /* #ifdef _WINDOWS #else */

#endif /* #ifdef __WLISTVIEW_H__ */


typedef struct 
{
    long        lCount;
    long        lDropSel;

    long        lCaret;
    long        lOffset;
    BOOL        bFocus;

    SIZE        Extent;
    POINT       ptOrg;
    int         ItmHeight;
    int         ItmWidth;
    long        lCurSel;
    long        lSelCount;
    long        lTopIndx;
    int         iStartSel;
    int         eSelMode;
    RECT        rcSelect;
} TVLBOXDATA, *PTVLBOXDATA;

typedef struct {
    LPARAM      lParam;
    HLVITEM     hItm;
} LVSORTITEM, *PLVSORTITEM; 

typedef struct
{
    int         tmAvCharWidth;
    int         tmHeight;
    int         cMaxText;
    UINT        itemState;
    RECT        rcText;
    RECT        rcBound;
    RECT        rcPaint;
    HDC         hDC;
    HFONT       hFont;
    HIMAGELIST  hImgL;
} LVITEMLAYOUT, *PLVITEMLAYOUT;

typedef struct
{
    HLVITEM     NSItm; /* index of subitem in next col of same row */    
    HLVITEM     NSCol;
    LPSTR       pszText;
} LVCOLITEM, *PLVCOLITEM;

typedef struct
{
    BOOL        bDeleted;
    int         ColWidth;
    WLIST       ColItms;
} LVIEWCOL, *PLVIEWCOL;

typedef struct
{
    WLIST       Columns;
    long        lCount;
} LVIEWCOLS, *PLVIEWCOLS;

typedef struct
{
    LVCOLITEM   firstCol;
    LV_ITEM     lvItm;
    RECT        rcItem;

    int         Above,
                Below,
                Left,
                Right;
} LVIEWROW, *PLVIEWROW;

typedef struct
{
    int         ColWidth; /* col 0 width */
    WLIST       RowItms; /* column 0 */
    WLIST       SortedRows;
    int         iDeleted; /* for items recycling */
    long        lCount;
} LVIEWROWS, *PLVIEWROWS;


typedef struct
{
    long        lCol;
    long        lRow;
    PLVIEWCOL   lpCol; /* NULL if lCol = 0 */
    PLVIEWROW   lpRow;
    PLVCOLITEM  lpSubItm;
}LVITEMPATH, *PLVITEMPATH;

typedef struct {
    BOOL                bShow;
    int                 enDrag; /* for dragging */
    POINT               ptDrag;
    int            		iEditItem; /* item that are being edited, 0 if none */
    int            		iDeleted;  /* array of TV Items deleted and availble for recycle */

    UINT         		uStyles;
    COLORREF            BkColor;
    COLORREF            TxtColor;
    COLORREF            TxtBkColor;
    UINT                CallBMask;
    TVLBOXDATA          psLBox;
    int                 iCount;
    BOOL                bReArrange;
    SIZE                BmpSize;
    HFONT       		hFont;
    LVIEWROWS           Rows;
    LVIEWCOLS           Cols;
    HWND        		hwEdit;
    HWND        		hwHeader;
    HIMAGELIST  		hImgList;  /* for normal icons */
    HIMAGELIST  		hSImgList; /* for small icons */
    HIMAGELIST  		hStImgList; /* for state icons */
} LISTVIEWDATA, *PLISTVDATA;

HGLOBAL
LView_IGetResource
(
    HWND,
    PLISTVDATA*         
);

BOOL
LView_OnScroll
( 
    HWND, 
    LISTVIEWDATA*,
    int,
    int
);
void
LView_IReleaseResource
(
    HGLOBAL,
    PLISTVDATA          
);
