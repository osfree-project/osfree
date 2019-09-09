/*  WTVLBox.h	1.5 
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

#define XBMPOFFSET  2

typedef struct {
    UINT        itemID; 
    UINT        itemAction; 
    UINT        itemState; 
    UINT        itemData; 
    RECT        rcItem;
    LPVOID      pTree;

    int         nLevel;
    int         nImage;
    int         nStImage;
    int         nTextHeight;
    int         nLineHeight;
    DWORD       dwConnect;
    LPSTR       pszText;
    SIZE        TextSize;
    BOOL        bHasChild;
    BOOL        bLastKid;
    BOOL        bOpened;
} TVIEWDRAWITEMSTRUCT, *LPTVIEWDRAWITEMSTRUCT;

BOOL
TVEdit_Subclass ( HWND hwLBox );


long
TVLBox_DeleteStringLP ( HWND hWnd, PTVLBOXDATA pLBox, long lIndx );

long
TVLBox_FindStringLP
(
    PTVLBOXDATA     pLBox,
    long            indxStart,
    LPARAM          lParam
);

long
TVLBox_GetCaretIndex ( PTVLBOXDATA pLBox );

long
TVLBox_GetCount ( PTVLBOXDATA pLBox );

long
TVLBox_GetCurSel ( PTVLBOXDATA pLBox );

int
TVLBox_GetHorzExtent ( PTVLBOXDATA pLBox );

int
TVLBox_GetItemHeight ( PTVLBOXDATA pLBox );

long
TVLBox_GetItemRect ( HWND hwTree, PTVLBOXDATA pLBox, long lIndx, LPRECT lprc );

long
TVLBox_GetItemData ( PTVLBOXDATA pLBox, long indx );

long
TVLBox_GetTopIndex ( PTVLBOXDATA pLBox );

void
TVLBox_IGetIndentPoint 
(   
    LPRECT              lprcItm, 
    int                 nLevel, 
    int                 nIndent, 
    LPPOINT             lppt 
);

long
TVLBox_InsertStringLP ( HWND hWnd, PTVLBOXDATA pLBox, long lIndx, LPARAM lParam );

BOOL
TVLBox_IPtInBtn ( LPPOINT lppt, int x, int y );

long
TVLBox_ItemFromPoint ( HWND hwTree, PTVLBOXDATA pLBox, int x, int y );

void 
TVLBox_OnDrawItem(HWND  hwnd, LPTVIEWDRAWITEMSTRUCT  lptvdi );

void
TVLBox_OnHScroll ( HWND hWnd, int iCode, int iPos );

void
TVLBox_OnVScroll ( HWND hWnd, int iCode, int iPos );

BOOL
TVLBox_SetCurSel ( HWND hWnd, TREEVIEWDATA*	pthis, long indx, UINT uAction, LPPOINT ppt );

int
TVLBox_SetDropHilite ( HWND hWnd, TREEVIEWDATA*	pthis, long indx );

void
TVLBox_SetHorzExtent ( HWND hWnd, PTVLBOXDATA pLBox, int extent );

long
TVLBox_SetTopIndex ( PTVLBOXDATA pLBox, long lTopIndx );
