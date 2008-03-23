
/*  WLVLBox.h	1.1 
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
#if 0
typedef struct {
    int         nIndent;
    int         nLevel;
    int         nImage;
    int         nBitmapHeight;
    int         nBitmapWidth;
    int         nTextHeight;
    int         nLineHeight;
    DWORD       dwConnect;
    LPSTR       pszText;
    SIZE        TextSize;
    UINT        uStyles;
    BOOL        bHasChild;
    BOOL        bLastKid;
    BOOL        bOpened;
    HIMAGELIST  hImgL;
    HFONT       hFont;
} TVIEWDRAWITEMSTRUCT, *LPTVIEWDRAWITEMSTRUCT;
#endif

#if 0
/*
// Interface functions
*/
long
TVListBox_ItemFromPoint 
(
    HWND                hwTree,
    PTVLBOXDATA         pLBox,
    int                 x,
    int                 y
);

/*
// Support functions
*/
static VOID  near FastRect(HDC hDC, int x, int y, int cx, int cy);
#endif

void LVLBox_OnHScroll ( HWND, int, int);

void LVLBox_OnVScroll ( HWND, int, int);


BOOL LVEdit_Subclass ( HWND );
