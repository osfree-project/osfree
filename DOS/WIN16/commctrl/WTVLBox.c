/*  
	WTVLBox.c	1.13 
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

#ifdef _WINDOWS
#include <windows.h>
#include <commctrl.h>
#elif defined (macintosh)
#include "WCommCtrl.h"
#include "WinMalloc.h"
#else
#include "WCommCtrl.h"

#endif
#include "windowsx.h"
#include "assert.h"
#include <math.h>
#include <string.h>

#include "WLists.h"
#include "WTView.h"
#include "WTVLBox.h"
#include "WImgList.h"

#ifndef _WINDOWS
#define  GetTextExtentPoint32   GetTextExtentPoint
#endif
#define X_OFFSET    5
#define Y_OFFSET    4
#define EDITWIDTH_MIN 30

static VOID FastRect(HDC , int , int , int , int );

LRESULT WINAPI 
TVEdit_WndProc
(
	HWND				hWnd,
    UINT				uMessage,
    WPARAM				wParam,
    LPARAM				lParam
);


void
TVLBox_IGetIndentPoint 
(   
    LPRECT              lprcItm, 
    int                 nLevel, 
    int                 nIndent, 
    LPPOINT             lppt 
)
{
    lppt->x = nIndent * nLevel + nIndent / 2 + XBMPOFFSET;
    lppt->y = lprcItm->top + ( lprcItm->bottom - lprcItm->top ) / 2;
    
}

/* lppt is the center of the item button rectangle, x & y are mouse pos.
 */
BOOL
TVLBox_IPtInBtn 
(   
    LPPOINT             lppt, 
    int                 x, 
    int                 y 
)
{
    return ( x >= lppt->x - X_OFFSET && 
             x <= lppt->x + X_OFFSET && 
             y >= lppt->y - Y_OFFSET && 
             y <= lppt->y + Y_OFFSET );

}

/*
 *  Creates the objects used while drawing the tree.  This may be called
 *  repeatedly in the event of a WM_SYSCOLORCHANGED message.
 *
 *  WARNING: the Tree icons bitmap is assumed to be a 16 color DIB!
 */

void
TVLBox_IDrawItmBtn
(
    HDC         hDC,
    int         x,
    int         y,
    BOOL        bOpened
)
{

    Rectangle ( hDC, x - X_OFFSET, y - Y_OFFSET, x + X_OFFSET, y + Y_OFFSET );
    MoveToEx ( hDC, x - ( X_OFFSET - 2 ),y, NULL );
    LineTo ( hDC, x + ( X_OFFSET - 2 ), y );
    if ( !bOpened )
    {
        MoveToEx ( hDC, x,y - ( Y_OFFSET - 2 ), NULL );
        LineTo ( hDC, x, y + ( Y_OFFSET - 2 ) );
    }
}

void
TVLBox_OnHScroll
(
    HWND                    hWnd,
    int                     iCode,
    int                     iPos
)
{
    RECT                    rc;
    int                     iNewPos;
    int                     iPageSize;
    int                     iLineSize;
    HDC                     hDC         = GetDC ( hWnd );
	HGLOBAL		            hRsrc   	= ( HGLOBAL )NULL;
	TREEVIEWDATA*	        pthis		= ( TREEVIEWDATA* )NULL;
    BOOL                    bRedraw     = FALSE;

    assert ( hRsrc = TView_IGetResource ( hWnd, &pthis, ( PTVITEMDATA* )NULL, ( PTVSTR* )NULL ) );
    if ( IsWindowVisible ( pthis->hwEdit ) )
        TreeView_EndEditLabelNow ( hWnd, TRUE );    
    GetClientRect ( hWnd, &rc );
#if 0
    GetCharWidth ( hDC, 'A'-'a', 'Z'- 'a', &iLineSize );
#else
    iLineSize = 8;
#endif
    ReleaseDC ( hWnd, hDC );
    iPageSize = ( rc.right - rc.left ) / iLineSize;
    switch ( iCode )
    {
        case    SB_LINELEFT:    if ( pthis->psLBox.lOffset < 0 )
                              {
                                pthis->psLBox.lOffset += iLineSize;
                                if ( pthis->psLBox.lOffset > 0 )
                                    pthis->psLBox.lOffset = 0;
                                iNewPos = 0 - pthis->psLBox.lOffset;
                                bRedraw = TRUE;
                              }
                              break;
        case    SB_LINERIGHT:  if ( pthis->psLBox.lOffset >
                                    ( rc.right - pthis->psLBox.iHExtent ) )
                              {
                                pthis->psLBox.lOffset -= iLineSize;
                                if ( pthis->psLBox.lOffset <
                                     ( rc.right - pthis->psLBox.iHExtent ) )
                                    pthis->psLBox.lOffset = rc.right - pthis->psLBox.iHExtent;
                                iNewPos = 0 - pthis->psLBox.lOffset;
                                bRedraw = TRUE;
                              }
                              break;
        case    SB_PAGEUP:    if ( pthis->psLBox.lOffset < 0 )
                              {
                                pthis->psLBox.lOffset += iPageSize;
                                if ( pthis->psLBox.lOffset > 0 )
                                  pthis->psLBox.lOffset = 0;
                                iNewPos = 0 - pthis->psLBox.lOffset;
                                bRedraw = TRUE;
                              }
                              break;
        case    SB_PAGEDOWN:  if ( pthis->psLBox.lOffset >
                                    ( rc.right - pthis->psLBox.iHExtent ) )
                              {
                                pthis->psLBox.lOffset -= iPageSize;
                                if ( pthis->psLBox.lOffset <
                                     ( rc.right - pthis->psLBox.iHExtent ) )
                                  pthis->psLBox.lOffset = rc.right - pthis->psLBox.iHExtent;
                                iNewPos = 0 - pthis->psLBox.lOffset;
                                bRedraw = TRUE;
                              }
                              break;
        case    SB_THUMBPOSITION:
        case    SB_THUMBTRACK: pthis->psLBox.lOffset = 0 - iPos;
                               iNewPos = iPos;
                               bRedraw = TRUE;
                               break;

    }
    TView_IReleaseResource ( hRsrc, pthis, ( PTVITEMDATA* )NULL, ( PTVSTR* )NULL );
    if ( bRedraw )
    {
        InvalidateRect ( hWnd, ( HWND )NULL, TRUE );
        SetScrollPos ( hWnd, SB_HORZ, iNewPos, TRUE );
    }

}


void
TVLBox_OnVScroll
(
    HWND                    hWnd,
    int                     iCode,
    int                     iPos
)
{
    RECT                    rc;
    int                     iNewPos;
    int                     iPageSize;
	HGLOBAL		            hRsrc   	= ( HGLOBAL )NULL;
	TREEVIEWDATA*	        pthis		= ( TREEVIEWDATA* )NULL;
    BOOL                    bRedraw     = FALSE;

    assert ( hRsrc = TView_IGetResource ( hWnd, &pthis, ( PTVITEMDATA* )NULL, ( PTVSTR* )NULL ) );
    GetClientRect ( hWnd, &rc );
    iPageSize = rc.bottom / pthis->psLBox.ItmHeight;
    switch ( iCode )
    {
        case    SB_LINEUP:    if ( pthis->psLBox.lTopIndx > 0 )
                              {
                                ( pthis->psLBox.lTopIndx )--;
                                iNewPos = pthis->psLBox.lTopIndx;
                                bRedraw = TRUE;
                              }
                              break;
        case    SB_LINEDOWN:  if ( ( pthis->psLBox.lCount - pthis->psLBox.lTopIndx ) *
                                   pthis->psLBox.ItmHeight > rc.bottom )
                              {
                                ( pthis->psLBox.lTopIndx )++;
                                iNewPos = pthis->psLBox.lTopIndx;
                                bRedraw = TRUE;
                              }
                              break;
        case    SB_PAGEUP:    if ( pthis->psLBox.lTopIndx > 0 )
                              {
                                pthis->psLBox.lTopIndx -= iPageSize;
                                if ( pthis->psLBox.lTopIndx < 0 )
                                  pthis->psLBox.lTopIndx = 0;
                                iNewPos = pthis->psLBox.lTopIndx;
                                bRedraw = TRUE;
                              }
                              break;
        case    SB_PAGEDOWN:  if ( ( pthis->psLBox.lCount - pthis->psLBox.lTopIndx ) *
                                   pthis->psLBox.ItmHeight > rc.bottom )
                              {
                                pthis->psLBox.lTopIndx += iPageSize;
                                if ( pthis->psLBox.lTopIndx >= pthis->psLBox.lCount - iPageSize )
                                  pthis->psLBox.lTopIndx = pthis->psLBox.lCount - iPageSize;
                                if ( pthis->psLBox.lTopIndx < 0 )
                                  pthis->psLBox.lTopIndx = 0;
                                iNewPos = pthis->psLBox.lTopIndx;
                                bRedraw = TRUE;
                              }
                              break;
        case    SB_THUMBPOSITION:
        case    SB_THUMBTRACK: iNewPos = pthis->psLBox.lTopIndx = iPos;
                               bRedraw = TRUE;
                               break;

    }
    TView_IReleaseResource ( hRsrc, pthis, ( PTVITEMDATA* )NULL, ( PTVSTR* )NULL );
    if ( bRedraw )
    {
        InvalidateRect ( hWnd, ( HWND )NULL, TRUE );
        SetScrollPos ( hWnd, SB_VERT, iNewPos, TRUE );
    }
}

void 
TVLBox_OnDrawItem
(
    HWND                    hWnd,
    LPTVIEWDRAWITEMSTRUCT   lptvdi
)
{
    POINT                   pt;
    WORD                    wIndent, wTopBitmap, wTopText;
    RECT                    rcTemp, rcBound;
    int                     nLevel;
    HFONT                   hOFont;
	TREEVIEWDATA*	        pthis   = (TREEVIEWDATA*)lptvdi->pTree;
    HDC                     hDC     = GetDC ( hWnd );


    GetClientRect ( hWnd, &rcBound );
    CopyRect(&rcTemp, &lptvdi->rcItem);

    if ( pthis->uStyles & TVS_HASBUTTONS )
        nLevel = lptvdi->nLevel + 1;
    else
        nLevel = lptvdi->nLevel;

    TVLBox_IGetIndentPoint ( &rcTemp, nLevel-1, pthis->nIndent, &pt ); 

    wIndent = rcTemp.left + ((int)(nLevel) * pthis->nIndent) + XBMPOFFSET;
    rcTemp.left = wIndent + pthis->nIndent;
    rcTemp.right = rcTemp.left + lptvdi->TextSize.cx;
    wTopText = rcTemp.top + (rcTemp.bottom - rcTemp.top) / 2 - lptvdi->nTextHeight / 2;
    wTopBitmap = rcTemp.top + (rcTemp.bottom - rcTemp.top) / 2 - pthis->ImgSize.cy / 2;

#if 0
    if (lptvdi->itemAction == ODA_FOCUS)
        goto DrawFocus;
    else if (lptvdi->itemAction == ODA_SELECT)
        goto DrawSelection;
#endif


    if ( ( pthis->uStyles & TVS_HASLINES ) && nLevel )
    {
        DWORD    dwMask = 1;
        DWORD    dwConnect = lptvdi->dwConnect;
        int      nTempLevel;
        int      x,y;

        /* draw lines in text color */
        SetBkColor(hDC,GetSysColor(COLOR_WINDOWTEXT));

        /* Draw a series of | lines for outer levels */

        x = lptvdi->rcItem.left + pthis->nIndent / 2 + XBMPOFFSET;
        dwMask = (DWORD)pow ( 2, lptvdi->nLevel );
        for ( nTempLevel = 0; nTempLevel < ( lptvdi->nLevel - 0) ; 
                              nTempLevel++, x += pthis->nIndent, dwMask /= 2)
          {
              if ( nTempLevel == 0 && !( pthis->uStyles & TVS_LINESATROOT ) )
                  continue;
              if ( dwConnect & dwMask )
                FastRect(hDC,x,rcTemp.top,1,rcTemp.bottom - rcTemp.top);
          }


        /* Draw the short vert line up towards the parent */
        if ( lptvdi->nLevel > 0 ||
             ( lptvdi->nLevel == 0 && pthis->uStyles & TVS_LINESATROOT )
           )
        {
            x = lptvdi->rcItem.left + pt.x;
            y = pt.y;

            if ( !lptvdi->bLastKid )
                y = rcTemp.bottom;

            FastRect(hDC,x,rcTemp.top,1,y-rcTemp.top);

            /*Draw short horiz bar to right*/
            FastRect(hDC, x, pt.y, pthis->nIndent / 2,1);
        }
    }
      
    /* draw +/- button */
    if ( lptvdi->bHasChild && ( pthis->uStyles & TVS_HASBUTTONS ) )
    {
        TVLBox_IDrawItmBtn ( hDC, lptvdi->rcItem.left + pt.x, 
                                rcTemp.bottom - lptvdi->nLineHeight / 2,
                                lptvdi->bOpened ); 
    }

    /* Draw the state item icon */
    if ( pthis->hStImgList && lptvdi->nStImage != -1 )
    {
        ImageList_Draw ( pthis->hStImgList, lptvdi->nStImage, hDC, 
                         wIndent, wTopBitmap, ILD_NORMAL ); 
        wIndent += pthis->StImgSize.cx;
    }

    /* Draw the image icon */
#if 1
    if ( pthis->hImgList )
        ImageList_Draw ( pthis->hImgList, lptvdi->nImage, hDC, 
                         wIndent, wTopBitmap, ILD_NORMAL ); 
#else
    if ( lptvdi->hImgL )
        ImageList_Draw ( lptvdi->hImgL, lptvdi->nImage, hDC, 
                         wIndent, wTopBitmap, ILD_NORMAL | INDEXTOOVERLAYMASK (1) ); 
#endif


/*DrawSelection:*/
    {
        int iBkColor, iTxtColor, iTxtLen;
        
        iBkColor = COLOR_WINDOW;
        if (lptvdi->itemState & ODS_SELECTED)
        {
            iBkColor = pthis->psLBox.bFocus ? COLOR_HIGHLIGHT :
                       ( pthis->uStyles & TVS_SHOWSELALWAYS )? COLOR_BTNFACE :
                       COLOR_WINDOW;
        }

        iTxtColor = iBkColor == COLOR_WINDOW ? COLOR_WINDOWTEXT : 
                                               COLOR_HIGHLIGHTTEXT;

        SetBkColor ( hDC, GetSysColor ( iBkColor ) );
        SetTextColor ( hDC, GetSysColor ( iTxtColor ) );
    
        if ( ( iTxtLen = lstrlen(lptvdi->pszText ) ) > 0 )
        {
            hOFont = SelectObject ( hDC, pthis->hFont );
            /*rcTemp.left += pthis->ImgSize.cx;*/
            OffsetRect ( &rcTemp, pthis->ImgSize.cx, 0 );
            if ( rcTemp.right >= rcBound.right )
                rcTemp.right = rcBound.right - 1;
            if ( rcTemp.top <= rcBound.top )
                rcTemp.top = rcBound.top + 1;
            if ( rcTemp.bottom >= rcBound.bottom )
                rcTemp.bottom = rcBound.bottom - 1;
            ExtTextOut(hDC, rcTemp.left + 1, wTopText + 1, ETO_CLIPPED | ETO_OPAQUE,
                       &rcTemp,lptvdi->pszText, iTxtLen, NULL);
            SelectObject ( hDC, hOFont );
        }
    }

    if (lptvdi->itemState & ODS_FOCUS && lptvdi->itemAction != ODA_SELECT) 
    {
/*DrawFocus:*/
        DrawFocusRect(hDC, &rcTemp);
    }

    ReleaseDC ( hWnd, hDC );
}

/* draw a solid color rectangle quickly */
static VOID FastRect(HDC hDC, int x, int y, int cx, int cy)
{
    RECT rc;

    rc.left = x;
    rc.right = x+cx;
    rc.top = y;
    rc.bottom = y+cy;
    ExtTextOut(hDC,x,y,ETO_OPAQUE,&rc,NULL,0,NULL);
}


long
TVLBox_ItemFromPoint 
(
    HWND                hwTree,
    PTVLBOXDATA         pLBox,
    int                 x,
    int                 y
)
{
    RECT                rcLBox;
    int                 nCount;
    int                 nIndx;
    int                 nItmHeight;
    int                 nHitItem = -1;
    WORD                hWord    = 1;  /*assume point is outside of treeview */
    WORD                lWord    = 0;

    GetClientRect ( hwTree, &rcLBox );
    if ( x >= rcLBox.left && x <= rcLBox.right &&
         y >= rcLBox.top  && y <= rcLBox.bottom )
    {
        nCount = TVLBox_GetCount ( pLBox );
        nIndx = TVLBox_GetTopIndex ( pLBox );
        nItmHeight = TVLBox_GetItemHeight ( pLBox );
#if 1
        lWord = nHitItem = nIndx + ( y - rcLBox.top ) / nItmHeight;
        if ( nHitItem >= nCount )
        {
            lWord = nHitItem = -1;
        }
        else
            hWord = 0;    /* point is inside treeview */
#else
        while ( rcLBox.top < rcLBox.bottom  && nIndx < nCount )
        {
            if ( y >= rcLBox.top && y < rcLBox.top + nItmHeight )
            {
                hWord = 0;
                lWord = nIndx;
                break;
            }
            nIndx++;
            rcLBox.top += nItmHeight;
        }
#endif
    }

    return ( MAKELONG ( lWord, hWord ) );
}

BOOL
TVEdit_Subclass
(
    HWND                hwLBox
)
{
    WNDPROC             lpEdWndProc = ( WNDPROC )NULL;
    
    lpEdWndProc = ( WNDPROC )SetWindowLong ( hwLBox, GWL_WNDPROC, 
                                        ( LPARAM )( WNDPROC )TVEdit_WndProc ); 
    if ( !lpEdWndProc )
        return TRUE;
    return FALSE;
}


LRESULT WINAPI 
TVEdit_WndProc
(
	HWND				hWnd,
    UINT				uMessage,
    WPARAM				wParam,
    LPARAM				lParam
)
{
    LRESULT             lResult             = 0;
    BOOL                bCallOrigWndProc    = FALSE;
	switch (uMessage)
	{
        case WM_ERASEBKGND:
            {
                RECT    rc;
                HDC     hdc = ( HDC )wParam;
                GetClientRect ( hWnd, &rc );
                Rectangle ( hdc, rc.left, rc.top, rc.right, rc.bottom );
                return -1;
            }
        case WM_GETDLGCODE:
			return ( LRESULT )DLGC_WANTARROWS | DLGC_WANTALLKEYS;
		
		case WM_KEYDOWN:
            CallWindowProc( ( FARPROC )GetClassLong ( hWnd, GCL_WNDPROC ),
                                   hWnd, uMessage, wParam, lParam );
            if ( ( int )wParam == VK_ESCAPE || ( int )wParam == VK_RETURN )
		        TView_OnKeyDown ( GetParent ( hWnd ), (UINT)( wParam ), 
                              ( int )( short )LOWORD ( lParam ), 
			                  ( UINT )HIWORD ( lParam ) );
            else
            {
                RECT    rect, rcDad;                      
                char    szText[MAXSTRING];
                SIZE    size;
                HFONT   hFont;
                HWND    hwParent = GetParent ( hWnd );
                HDC     hDC = GetDC ( hWnd );
                GetWindowRect ( hWnd, &rect );
                MapWindowPoints ( ( HWND )NULL, hwParent, ( LPPOINT )&rect.left, 2 ); 
                GetClientRect ( hwParent, &rcDad );
                GetWindowText ( hWnd, szText, MAXSTRING );
                hFont = ( HFONT )SendMessage ( hWnd, WM_GETFONT, 0, 0L ); 
                hFont = SelectObject ( hDC, hFont );
                GetTextExtentPoint32 ( hDC, szText, lstrlen ( szText ), &size ); 
                SelectObject ( hDC, hFont );
                if ( size.cx < EDITWIDTH_MIN )
                    size.cx = EDITWIDTH_MIN;
                if ( rect.left + size.cx + 5 < rcDad.right  &&
                     size.cx + 5 > EDITWIDTH_MIN )
                    SetWindowPos ( hWnd, ( HWND )NULL, 0, 0, 
                                   size.cx + 5, rect.bottom - rect.top,
                                   SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOZORDER );
                ReleaseDC ( hWnd, hDC );
            }

            break;

        default:
			return CallWindowProc( ( FARPROC )GetClassLong ( hWnd, GCL_WNDPROC ),
                                   hWnd, uMessage, wParam, lParam );
	}
    
	if ( bCallOrigWndProc )
        lResult = CallWindowProc( ( FARPROC )GetClassLong ( hWnd, GCL_WNDPROC ),
                                   hWnd, uMessage, wParam, lParam );
    return lResult;
}

long
TVLBox_AddStringLP
(
    HWND            hWnd,
    PTVLBOXDATA     pLBox,
    LPARAM          lParam
)
{
    TVLBOXITEM      LBItm;
    RECT            rc;
    long            lIndx       = -1;

    LBItm.lParam = lParam;
    if ( WLInsert ( &pLBox->ItmList, pLBox->lCount, ( LPVOID )&LBItm ) == 0 )
    {
        lIndx = ( pLBox->lCount )++;
        GetClientRect ( hWnd, &rc );
        if ( pLBox->ItmHeight * pLBox->lCount > rc.bottom )
        {
            SetScrollRange ( hWnd, SB_VERT, 0, pLBox->lCount - rc.bottom / pLBox->ItmHeight, TRUE );
            ShowScrollBar ( hWnd, SB_VERT, TRUE );
        }
    }
    
    return lIndx;
}

long
TVLBox_DeleteStringLP
(
    HWND            hWnd,
    PTVLBOXDATA     pLBox,
    long            lIndx
)
{
    RECT            rc;
    long            lMaxTopIndx;
    long            lRemains    = -1;

    if ( WLDelete ( &pLBox->ItmList, lIndx ) == 0 )
    {
        lRemains = -- ( pLBox->lCount );
        GetClientRect ( hWnd, &rc );
        lMaxTopIndx = pLBox->lCount - rc.bottom / pLBox->ItmHeight;
        if ( lMaxTopIndx < 0 )
            lMaxTopIndx = 0;
        if ( lMaxTopIndx < pLBox->lTopIndx )
            pLBox->lTopIndx = lMaxTopIndx;
        if ( pLBox->ItmHeight * pLBox->lCount < rc.bottom )
        {
            ShowScrollBar ( hWnd, SB_VERT, FALSE );
        }
        else
            SetScrollRange ( hWnd, SB_VERT, 0, lMaxTopIndx, TRUE );
    }
    
    return lRemains;
}


/* this is made to simulate listbox LB_FINDSTRING where the listbox doesn't have
 * LBS_HASSTRING
 */
long
TVLBox_FindStringLP
(
    PTVLBOXDATA     pLBox,
    long            indxStart,
    LPARAM          lParam
)
{
    long            i, lCheck;
    long            lCount      = pLBox->lCount;
    long            lReturn     = -1;
    PTVLBOXITEM     pItmList    = ( PTVLBOXITEM )NULL; 

    if ( lCount <= 0 )
        return lReturn;
    if ( indxStart == -1 || indxStart >= lCount )
        indxStart = 0;
    if ( WLLock ( &pLBox->ItmList, ( LPVOID )&pItmList ) != 0 )
        return lReturn;
    for ( i = indxStart, lCheck = indxStart+1;; lCheck++ )
    {
        if ( lParam == ( pItmList + i )->lParam )
        {
            lReturn = i;
            break;
        }

        if ( ( i = lCheck%lCount ) == indxStart )
            break;
    }
    WLUnlock ( &pLBox->ItmList, ( LPVOID )&pItmList );
    
    return lReturn;
}

long
TVLBox_GetCurSel
(
    PTVLBOXDATA     pLBox
)
{
    
    return pLBox->lCurSel;
}

long
TVLBox_GetTopIndex
(
    PTVLBOXDATA     pLBox
)
{
    
    return pLBox->lTopIndx;
}

long
TVLBox_GetCaretIndex
(
    PTVLBOXDATA     pLBox
)
{
    
    return pLBox->lCaret;
}


long
TVLBox_GetCount
(
    PTVLBOXDATA     pLBox
)
{
    
    return pLBox->lCount;
}

int
TVLBox_GetItemHeight
(
    PTVLBOXDATA     pLBox
)
{
    
    return pLBox->ItmHeight;
}

long
TVLBox_GetItemRect
(
    HWND            hwTree,
    PTVLBOXDATA     pLBox,
    long            lIndx,
    LPRECT          lprc
)
{
    RECT            rc;
    long            lOff;
    long            lReturn     = -1;

    if ( lIndx < 0 || lIndx >= pLBox->lCount || lIndx < pLBox->lTopIndx || !lprc )
        return lReturn;
    GetClientRect ( hwTree, &rc );
    lOff = lIndx - pLBox->lTopIndx;
    if ( rc.bottom > lOff * pLBox->ItmHeight )
    {
        SetRect ( lprc, rc.left + pLBox->lOffset, lOff * pLBox->ItmHeight, rc.right, ( lOff + 1 ) * pLBox->ItmHeight );
        lReturn = 0;
    }
    return lReturn;
}

long
TVLBox_InsertStringLP
(
    HWND            hWnd,
    PTVLBOXDATA     pLBox,
    long            lIndx,
    LPARAM          lParam
)
{
    TVLBOXITEM      LBItm;
    RECT            rc;

    if ( lIndx > pLBox->lCount )
        lIndx = pLBox->lCount;
    LBItm.lParam = lParam;
    if ( WLInsert ( &pLBox->ItmList, lIndx, ( LPVOID )&LBItm ) == 0 )
    {
        ( pLBox->lCount )++;
        GetClientRect ( hWnd, &rc );
        if ( pLBox->ItmHeight * pLBox->lCount > rc.bottom )
        {
            SetScrollRange ( hWnd, SB_VERT, 0, pLBox->lCount - rc.bottom / pLBox->ItmHeight, TRUE );
            ShowScrollBar ( hWnd, SB_VERT, TRUE );
        }
    }
    else
        lIndx = -1;
    
    return lIndx;
}


long
TVLBox_GetItemData
(
    PTVLBOXDATA     pLBox,
    long            indx
)
{
    long            dwReturn    = -1;
    PTVLBOXITEM     pItmList    = ( PTVLBOXITEM )NULL; 

    if ( indx < pLBox->lCount &&
         WLLock ( &pLBox->ItmList, ( LPVOID )&pItmList ) == 0 )
        dwReturn = ( pItmList + indx )->lParam;
    WLUnlock ( &pLBox->ItmList, ( LPVOID )&pItmList );
     
    return dwReturn;
}

BOOL
TVLBox_SetCurSel
(
    HWND            hWnd,
    TREEVIEWDATA*	pthis,
    long            indx,
    UINT            uAction,
    LPPOINT         ppt
)
{
    long            lPrevSel = pthis->psLBox.lCurSel;
    NM_TREEVIEW     nmtv;
    PTVITEMDATA     pItmList    = ( PTVITEMDATA )NULL;
    PTVITEMDATA     pItem       = ( PTVITEMDATA )NULL;

    
    if ( indx == lPrevSel ||
         ( uAction == TVC_BYMOUSE && ppt == ( LPPOINT )NULL ) )
        return FALSE;
    if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItmList ) )
        return FALSE;
    memset ( &nmtv, 0, sizeof ( nmtv ) );
    pItem = pItmList + TVLBox_GetItemData ( &pthis->psLBox, indx );
    nmtv.itemNew = pItem->tvi;
    if ( lPrevSel != -1 )
    {
        pItem = pItmList + TVLBox_GetItemData ( &pthis->psLBox, lPrevSel );
        nmtv.itemOld = pItem->tvi;
    }
    nmtv.hdr.hwndFrom = hWnd;
    nmtv.hdr.idFrom = GetDlgCtrlID ( hWnd );
    nmtv.hdr.code = TVN_SELCHANGING;
    if ( ppt )
    {
        nmtv.ptDrag.x = ppt->x;
        nmtv.ptDrag.y = ppt->y;
    }
    nmtv.action = uAction;
    WLUnlock ( &pthis->ItmList, ( LPVOID )NULL );

    /* send TVN_SELCHANGING notification message */
    if ( SendMessage ( GetParent ( hWnd ), WM_NOTIFY, 
                       nmtv.hdr.idFrom, ( LPARAM )&nmtv ) == FALSE )
    {
        pthis->psLBox.lCurSel = indx;
        if ( pthis->psLBox.bFocus )
            pthis->psLBox.lCaret = indx;
        if ( lPrevSel != -1 )
            TView_OnDrawItem ( hWnd, pthis, lPrevSel );
        if ( pthis->psLBox.lCurSel != -1 )
            TView_OnDrawItem ( hWnd, pthis, pthis->psLBox.lCurSel );
    }
    
    nmtv.hdr.code = TVN_SELCHANGED;
    /* send TVN_SELCHANGED notification message */
    SendMessage ( GetParent ( hWnd ), WM_NOTIFY, 
                  nmtv.hdr.idFrom, ( LPARAM )&nmtv );

    return TRUE;

}

int
TVLBox_SetDropHilite
(
    HWND            hWnd,
    TREEVIEWDATA*	pthis,
    long            indx
)
{
    int             iResult = 0;
    long            lDropSel = pthis->psLBox.lDropSel;

    pthis->psLBox.lDropSel = indx;
    if ( lDropSel != -1 )
        TView_OnDrawItem ( hWnd, pthis, lDropSel );
    if ( pthis->psLBox.lDropSel != -1 )
        TView_OnDrawItem ( hWnd, pthis, pthis->psLBox.lDropSel );
    return iResult;
}


int
TVLBox_GetHorzExtent
(
    PTVLBOXDATA     pLBox
)
{
    return pLBox->iHExtent;
}

void
TVLBox_SetHorzExtent
(
    HWND            hWnd,
    PTVLBOXDATA     pLBox,
    int             extent
)
{
    if ( extent > pLBox->iHExtent )
    {
        RECT    rc;
        GetClientRect ( hWnd, &rc );
        pLBox->iHExtent = extent;
        SetScrollRange ( hWnd, SB_HORZ, 0, extent - rc.right, TRUE );
    }
}



long
TVLBox_SetTopIndex
(
    PTVLBOXDATA     pLBox,
    long            lTopIndx
)
{
    
    
    pLBox->lTopIndx = lTopIndx;
    return lTopIndx;
}

LRESULT WINAPI 
TViewTip_WndProc
(
	HWND				hWnd,
    UINT				uMessage,
    WPARAM				wParam,
    LPARAM				lParam
)
{
    LRESULT             lResult             = 0;
    BOOL                bCallOrigWndProc    = FALSE;
    HWND                hwTree;
    POINT               pt;

    hwTree   = ( HWND )GetWindowLong ( hWnd, GWL_USERDATA );
	switch (uMessage)
	{
        case WM_GETDLGCODE:
			return ( LRESULT )DLGC_WANTARROWS | DLGC_WANTALLKEYS;
		
		case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
            pt.x = LOWORD ( lParam );
            pt.y = HIWORD ( lParam );
            MapWindowPoints ( hWnd, hwTree, &pt, 1 ); 
            SendMessage ( hwTree, uMessage, wParam, MAKELONG ( pt.x, pt.y ) ); 
            break;

        case WM_MOUSEMOVE:
            {
                RECT    rc;
                pt.x = LOWORD ( lParam );
                pt.y = HIWORD ( lParam );
                GetClientRect ( hWnd, &rc );
                if ( !PtInRect ( &rc, pt ) )
                    goto HideTipText;
                MapWindowPoints ( hWnd, hwTree, &pt, 1 ); 
                GetClientRect ( hwTree, &rc );
                if ( !PtInRect ( &rc, pt ) )
                {
HideTipText:
                    SetWindowPos ( hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_HIDEWINDOW | SWP_NOACTIVATE );
                    ReleaseCapture ();
                }
                break;
            }

        case WM_SETFONT:
            SetWindowLong ( hWnd, 0, wParam );
            break;

        case WM_SETTEXT:
            {
                RECT    rc;
                HFONT   hFont = ( HFONT )GetWindowLong ( hWnd, 0 );
                HDC     hDC = GetDC ( hWnd );
                GetClientRect ( hWnd, &rc );
                hFont = SelectObject ( hDC, hFont );
                DrawText ( hDC, ( LPSTR )lParam, lstrlen( ( LPSTR )lParam ),
                     &rc, DT_SINGLELINE | DT_CENTER | DT_VCENTER );
                SelectObject ( hDC, hFont );
                ReleaseDC ( hWnd, hDC );
                break;
            }
        default:
			return DefWindowProc( hWnd, uMessage, wParam, lParam );
	}
    
	if ( bCallOrigWndProc )
        lResult = DefWindowProc( hWnd, uMessage, wParam, lParam );
    return lResult;
}
