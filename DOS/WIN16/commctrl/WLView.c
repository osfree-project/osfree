/*  
	WLView.c	
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
#else
#include "WCommCtrl.h"
#endif
#include "assert.h"
#include "string.h"
#include "math.h"
#include "WLists.h"
#include "WLView.h"
#include "WLVLBox.h"
#include "WImgList.h"
#include "WLVHelpr.h"
#include "WLVLBox.h"

#define ROWS        4
#define COLS        3  
#define IDTVLIST    101 
#define IDLVEDIT    102 

#define STARTEDIT_TIMER     16
#define DEF_HEADERHEIGHT    20
#define DEF_ITEMHEIGHT      20
#define ICONOFFSET_CY       2
#define SMALLICON_ITEMWIDTH 150
#define DEF_SBITEXT_INDENT  5
#define DEF_ICON_SPACING    15
#define MAXLABELLENGTH      256

#ifndef _WINDOWS
#define  GetTextExtentPoint32   GetTextExtentPoint
#define  SetWindowOrgEx(x,y,z,h)    SetWindowOrg(x,y,z)
#endif

enum { eNoDrag = 0, eEndDrag = eNoDrag, eCancelDrag,
       eLBtnDown, eRBtnDown, eBeginLDrag, eBeginRDrag, eDragging };

enum { eNoSel, eSingleSel, eControlSel, eShiftSel };

static char LView_PROP[] = "LView_Data";	

#define GETTHISDATA(hwnd) \
   GetProp(hwnd, LView_PROP )
#define SETTHISDATA(hwnd, pthisNew) \
   SetProp(hwnd, LView_PROP, pthisNew)
#define REMOVETHISDATA(hwnd) \
   RemoveProp(hwnd, LView_PROP )


/*prototypes*/
static BOOL
LView_OnGetItemRect
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iItem,
    LPRECT              lpRect
);

static BOOL
LView_OnSetItemPos
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iItem,
    int                 x,
    int                 y
);

static int
LView_OnHitTest
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    LV_HITTESTINFO*     lpht
);

static int
LView_OnGetNextItem
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iStart,
    UINT                flags
);

static int
LView_OnGetItemText
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iItem,
    LV_ITEM*            lplvi
);

static BOOL
LView_OnGetItemPos
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iItem,
    LPPOINT             lppt
);

static BOOL
LView_OnGetItem
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    LV_ITEM*            lplvi
);

static BOOL 
LView_OnEndEditLabelNow 
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    BOOL                fCancel 
); 

static BOOL
LView_IUpdateSubItm 
( 
    PLVITEMPATH         pSubItmPath,
    LV_ITEM*            lplvi 
);

static BOOL
LView_IOpenSubItm
( 
    LISTVIEWDATA*       pthis,
    PLVITEMPATH         pSubItmPath,
    BOOL                bCreate
);

static void
LView_IInferItmState ( HWND, PLISTVDATA, int, PLVIEWROW, PLVITEMLAYOUT );

static int LView_OnEraseBkground ( HWND hWnd, HDC hDC );

static LRESULT  
LView_LVMWndProc ( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam );

/* revised */
static int
LView_IAllocItmHandle 
(
    LISTVIEWDATA*       pthis
)
{
    int                 iItem       = 0;
    PLVIEWROW           pItmList    = ( PLVIEWROW )NULL;
    PLVIEWROW           pDeleted    = ( PLVIEWROW )NULL;
    PLVIEWROWS          pRows       = &pthis->Rows;

    if ( pRows->iDeleted != -1 )
    {
        iItem = pRows->iDeleted;
        if ( !WLLock ( &pRows->RowItms, ( LPVOID )&pItmList ) )
        {
            pDeleted = pItmList + iItem;
            pRows->iDeleted = pDeleted->Below;

            WLUnlock ( &pRows->RowItms, ( LPVOID )&pItmList );
            ( pRows->lCount )++;
        }
    }
    else
    {
     	LVIEWROW          LVItm;
        memset ( &LVItm, 0, sizeof ( LVIEWROW ) );
       	if ( !WLInsert ( &pRows->RowItms, pRows->lCount, &LVItm ) )
            iItem = ( int )( pRows->lCount )++;
    }
   
    pthis->iCount = pRows->lCount; /* need to be looked at */
    return iItem;
}


/* HeapSort implementation */
static void
LView_IDownHeapCB
(
    PLVSORTITEM         pSortAr,
    int                 N,
    int                 k,
    PFNTVCOMPARE        lpfnCmp,
    LPARAM              lParamSort
)
{
    int                 j;
    LVSORTITEM          v;

    v = pSortAr[k];
    while ( k <= N / 2 )
    {
        j = k+k;
        if ( j < N && 
             ( *lpfnCmp )( pSortAr[j].lParam, 
                           pSortAr[j+1].lParam, lParamSort ) < 0 )
            j++;
        if ( ( *lpfnCmp ) ( v.lParam, pSortAr[j].lParam, lParamSort ) >= 0 )
            break;
        pSortAr[k] = pSortAr[j];
        k = j;
    }
    pSortAr[k] = v; 
}

/* revised Heap Sort */
static void
LView_IDoHeapSort
(
    PLVSORTITEM         pSortAr,
    int                 Count,
    PFNLVCOMPARE        pfnCompare,
    LPARAM              lParamSort
)
{
    int k;
    LVSORTITEM v;
    for ( k = Count/2; k >= 0; k-- )
        LView_IDownHeapCB ( pSortAr, Count, k, pfnCompare, lParamSort );

    while ( Count > 0 )
    {
        v = pSortAr[0];
        pSortAr[0] = pSortAr[Count];
        pSortAr[Count] = v;
        LView_IDownHeapCB ( pSortAr, --Count, 0, pfnCompare, lParamSort );
    }
}

/* in progress */
static void
LView_IDrawSubItmLIcon
(
    HWND                    hWnd, 
	LISTVIEWDATA*	        pthis,
    PLVITEMPATH             pItmPath,
    PLVITEMLAYOUT           pLayout
)
{
    LV_DISPINFO             lvdi;
    int                     iImage;
    LPSTR                   lpText;
    HFONT                   hOldFont;

    iImage = pItmPath->lpRow->lvItm.iImage;
    if ( pItmPath->lCol == 0 ) /* draw icon */
    {
        if ( iImage == I_IMAGECALLBACK )
        {
            int idCtrl =  GetDlgCtrlID ( hWnd );
            lvdi.hdr.code = LVN_GETDISPINFO;
            lvdi.hdr.hwndFrom = hWnd;
            lvdi.hdr.idFrom = idCtrl;
            lvdi.item = pItmPath->lpRow->lvItm;
            lvdi.item.mask = LVIF_IMAGE;
            SendMessage ( GetParent ( hWnd ), WM_NOTIFY, idCtrl, ( LPARAM )&lvdi );
            iImage = lvdi.item.iImage;
        }

        ImageList_Draw ( pLayout->hImgL, iImage, pLayout->hDC, 
                         pLayout->rcBound.left + DEF_ICON_SPACING, 
                         pLayout->rcBound.top + ICONOFFSET_CY, ILD_TRANSPARENT );

        /* the state of the item, if any, should be displayed here - need revisit */
    }

    if ( ( lpText = pItmPath->lpSubItm->pszText ) == LPSTR_TEXTCALLBACK )
    {
        int idCtrl =  GetDlgCtrlID ( hWnd );
        lvdi.hdr.code = LVN_GETDISPINFO;
        lvdi.hdr.hwndFrom = hWnd;
        lvdi.hdr.idFrom = idCtrl;
        lvdi.item = pItmPath->lpRow->lvItm;
        lvdi.item.mask = LVIF_TEXT;
        lvdi.item.iSubItem = pItmPath->lCol;
        SendMessage ( GetParent ( hWnd ), WM_NOTIFY, idCtrl, ( LPARAM )&lvdi );
        lpText = lvdi.item.pszText;
        if ( lvdi.item.mask & LVIF_DI_SETITEM )
            LView_IUpdateSubItm ( pItmPath, &lvdi.item );  
    }
    /*need revisit: add 3 dots if length of text bigger than width of rc */
    if ( lpText )
    {
        if ( pLayout->itemState & LVIS_SELECTED && 
             ( pthis->psLBox.bFocus || ( pthis->uStyles & LVS_SHOWSELALWAYS ) ) 
           )
        {
            SetBkColor ( pLayout->hDC,GetSysColor(COLOR_HIGHLIGHT) );
            SetTextColor ( pLayout->hDC,GetSysColor(COLOR_HIGHLIGHTTEXT) );
            ExtTextOut ( pLayout->hDC, pLayout->rcText.left, pLayout->rcText.top, 
                         ETO_OPAQUE, &pLayout->rcText, NULL, 0, NULL);
        }
        else
        {
            SetBkColor ( pLayout->hDC, pthis->TxtBkColor );
            SetTextColor ( pLayout->hDC, pthis->TxtColor );
        }

        hOldFont = SelectObject ( pLayout->hDC, pLayout->hFont );
        DrawText ( pLayout->hDC, lpText, lstrlen( lpText ),
                     &pLayout->rcText, DT_CENTER | DT_WORDBREAK );
        SelectObject ( pLayout->hDC, hOldFont );

        if ( pLayout->itemState & LVIS_FOCUSED )
            DrawFocusRect ( pLayout->hDC, &pLayout->rcText );     
    }
}

/* new */
static void
LView_IDrawSubItmRP
(
    HWND                    hWnd, 
	LISTVIEWDATA*	        pthis,
    PLVITEMPATH             pItmPath,
    PLVITEMLAYOUT           pLayout,
    UINT                    uStyle
)
{
    LV_DISPINFO             lvdi;
    int                     iImage;
    LPSTR                   lpText;
    HFONT                   hOldFont;
    RECT                    rcText;

    iImage = pItmPath->lpRow->lvItm.iImage;
    if ( pItmPath->lCol == 0 ) /* draw icon */
    {
        if ( iImage == I_IMAGECALLBACK )
        {
            int idCtrl =  GetDlgCtrlID ( hWnd );
            lvdi.hdr.code = LVN_GETDISPINFO;
            lvdi.hdr.hwndFrom = hWnd;
            lvdi.hdr.idFrom = idCtrl;
            lvdi.item = pItmPath->lpRow->lvItm;
            lvdi.item.mask = LVIF_IMAGE;
            SendMessage ( GetParent ( hWnd ), WM_NOTIFY, idCtrl, ( LPARAM )&lvdi );
            iImage = lvdi.item.iImage;
        }

       ImageList_Draw ( pLayout->hImgL, iImage, pLayout->hDC, 
                        pLayout->rcBound.left+1, 
                        pLayout->rcBound.top + ICONOFFSET_CY , ILD_TRANSPARENT );

        /* the state of the item, if any, should be displayed here - need revisit */
    }

    if ( ( lpText = pItmPath->lpSubItm->pszText ) == LPSTR_TEXTCALLBACK )
    {
        int idCtrl =  GetDlgCtrlID ( hWnd );
        lvdi.hdr.code = LVN_GETDISPINFO;
        lvdi.hdr.hwndFrom = hWnd;
        lvdi.hdr.idFrom = idCtrl;
        lvdi.item = pItmPath->lpRow->lvItm;
        lvdi.item.mask = LVIF_TEXT;
        lvdi.item.iSubItem = pItmPath->lCol;
        SendMessage ( GetParent ( hWnd ), WM_NOTIFY, idCtrl, ( LPARAM )&lvdi );
        lpText = lvdi.item.pszText;
        if ( lvdi.item.mask & LVIF_DI_SETITEM )
            LView_IUpdateSubItm ( pItmPath, &lvdi.item );  
    }
    /*need revisit: add 3 dots if length of text bigger than width of rc */
    if ( lpText )
    {
        char buf[MAXLABELLENGTH];
        SIZE size;
        int pixLen;
        int iLen = lstrlen ( lpText );
        hOldFont = SelectObject ( pLayout->hDC, pLayout->hFont );
        GetTextExtentPoint32 ( pLayout->hDC, lpText, iLen, &size );
        CopyRect ( &rcText, &pLayout->rcText );
        if ( uStyle != LVS_REPORT )
            rcText.right = rcText.left + size.cx;    
        pixLen = rcText.right - rcText.left;

        if ( iLen > MAXLABELLENGTH )
            iLen = MAXLABELLENGTH;
        if ( pixLen < size.cx )
        {
            int ifmtLen;
            iLen -= ( size.cx - pixLen ) / pLayout->tmAvCharWidth;
            ifmtLen = iLen > 3? iLen - 3 : 0;
            if ( ifmtLen > 0 )
	    {
                lstrcpyn ( buf, lpText, ifmtLen + 1 );
		buf[ifmtLen + 1] = 0;
	    }
            else
                *buf = 0;
            lstrcat ( buf, "..." );
            lpText = buf;
        }

        rcText.right = rcText.left + min ( pixLen, size.cx );    

        if ( pItmPath->lCol == 0 && pLayout->itemState & LVIS_SELECTED &&
             ( pthis->psLBox.bFocus || pthis->uStyles & LVS_SHOWSELALWAYS ) 
           )
        {
            SetBkColor ( pLayout->hDC,GetSysColor(COLOR_HIGHLIGHT) );
            SetTextColor ( pLayout->hDC,GetSysColor(COLOR_HIGHLIGHTTEXT) );
            ExtTextOut ( pLayout->hDC, rcText.left, rcText.top-1, 
                         ETO_OPAQUE, &rcText, NULL, 0, NULL);
        }
        else
        {
            SetBkColor ( pLayout->hDC, pthis->TxtBkColor );
            SetTextColor ( pLayout->hDC, pthis->TxtColor );
        }

        if ( uStyle == LVS_LIST )
        {
            if ( iLen > pLayout->cMaxText )
            {
                pLayout->cMaxText = iLen;
                pthis->psLBox.ItmWidth = pthis->BmpSize.cx + size.cx + DEF_SBITEXT_INDENT + 10; 
            }
        }
        DrawText ( pLayout->hDC, lpText, iLen,
                   &rcText, DT_LEFT );
        SelectObject ( pLayout->hDC, hOldFont );

        if ( pLayout->itemState & LVIS_FOCUSED && pItmPath->lCol == 0 )
            DrawFocusRect ( pLayout->hDC, &rcText );     
    }
}

static BOOL
LView_IArrangeIcons
(
    HWND                hwLView,
    PLISTVDATA          pthis

)
{
    RECT                rc;
    int                 nItmsPerLine;
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;
    int                 iItem       = pthis->psLBox.lTopIndx;

    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) ||
         WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs )
       )
       return FALSE;
    
    if ( iItem >= pthis->iCount )
       return FALSE;

    GetClientRect ( hwLView, &rc );
    pIndx = pIndxs + iItem;
    pItm = pItms + ( int )pIndx->hItm;
    nItmsPerLine = ( rc.right - rc.left ) /  ( pItm->rcItem.right - pItm->rcItem.left );
    while ( TRUE )
    {
        if ( pthis->psLBox.Extent.cx < pItm->rcItem.right )
            pthis->psLBox.Extent.cx = pItm->rcItem.right;
        if ( pthis->psLBox.Extent.cy < pItm->rcItem.bottom )
            pthis->psLBox.Extent.cy = pItm->rcItem.bottom;
        if ( pItm->Left == -1 && iItem % nItmsPerLine != 0 )
            pItm->Left = iItem - 1;
        if ( pItm->Right == -1 &&
             ( iItem % nItmsPerLine != nItmsPerLine - 1 ) &&
             iItem + 1 < pthis->iCount )
            pItm->Right = iItem + 1;
        if ( pItm->Above == -1 && iItem - nItmsPerLine >= 0 )
            pItm->Above = iItem - nItmsPerLine;
        if ( pItm->Below && iItem + nItmsPerLine < pthis->iCount )
            pItm->Below = iItem + nItmsPerLine;
        if ( ++iItem >= pthis->iCount )
            break;
        pItm = pItms + ( pIndxs + iItem )->hItm;
    }

    WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )&pItms );
    WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs );
    return TRUE;
}

static BOOL
LView_IIsItmVisible
(
    HWND                hwLView,
    PLISTVDATA          pthis,
    RECT                rcClip,
    RECT                rcItem  
)
{
    int                 dx;
/*    RECT                rc;*/
    RECT                rcDummy;

    /*GetClientRect ( hwLView, &rc );*/
    if ( ( pthis->uStyles & LVS_TYPEMASK ) == LVS_REPORT )
    {
        dx = 0;
        if ( !( pthis->uStyles & LVS_NOCOLUMNHEADER ) && rcClip.top <= 0 )
            rcClip.top = DEF_HEADERHEIGHT;
    }
    else
        dx = pthis->psLBox.ptOrg.x;
    OffsetRect ( &rcClip, dx, pthis->psLBox.ptOrg.y );
    return IntersectRect ( &rcDummy, &rcClip, &rcItem );
}

static void
LView_IOwnerDraw
(
    HWND                    hWnd, 
	LISTVIEWDATA*	        pthis,
    PLVITEMPATH             pItmPath,
    PLVITEMLAYOUT           pLayout,
    UINT                    uStyle
)
{
    DRAWITEMSTRUCT          dis;

    memset ( &dis, 0, sizeof ( DRAWITEMSTRUCT ) );
    dis.CtlType = ODT_LISTVIEW;
    dis.CtlID = GetDlgCtrlID ( hWnd );
    dis.hwndItem = hWnd;
    dis.hDC = pLayout->hDC;
    dis.itemData = pItmPath->lpRow->lvItm.lParam;
    dis.itemID = pItmPath->lpRow->lvItm.iItem;
    dis.itemState = ( pLayout->itemState & LVIS_SELECTED )? ODS_SELECTED : 0; 
    LView_OnGetItemRect ( hWnd, pthis, pItmPath->lpRow->lvItm.iItem, &dis.rcItem ); 
    SendMessage ( GetParent ( hWnd ), WM_DRAWITEM, dis.CtlID, ( LPARAM )&dis );
}

static void
LView_ISetItemRect
(
    HWND                hwLView,
    PLISTVDATA          pthis,
    int                 iItem,
    PLVIEWROW           pItm,
    PLVITEMLAYOUT       pItmLayout  
)
{
    int                 cx, cy;
    int                 left, top;
    UINT                uStyleMask;
    RECT                rc;

    GetClientRect ( hwLView, &rc );
    ImageList_GetIconSize ( pItmLayout->hImgL, &cx, &cy );
    uStyleMask = pthis->uStyles & LVS_TYPEMASK;
    if ( uStyleMask == LVS_REPORT || uStyleMask == LVS_LIST )
    {
        int y = uStyleMask == LVS_REPORT ? DEF_HEADERHEIGHT : 0;
        int cItms = rc.bottom / pthis->psLBox.ItmHeight;
        left = 0;
        if ( uStyleMask == LVS_LIST )
        {
            left = iItem / cItms * pthis->psLBox.ItmWidth;
            y = ( iItem % cItms ) * pthis->psLBox.ItmHeight;
        }
        else
            y += iItem*pthis->psLBox.ItmHeight;
        SetRect ( &pItmLayout->rcBound, left, y, 
                  left + pthis->Rows.ColWidth, y + cy + 2 * ICONOFFSET_CY );
        SetRect ( &pItmLayout->rcText, left + cx + DEF_SBITEXT_INDENT, y + ICONOFFSET_CY,
                  left+pthis->Rows.ColWidth, pItmLayout->rcBound.bottom ); 
    }
    else
    if ( uStyleMask == LVS_ICON || uStyleMask == LVS_SMALLICON )
    {
        if ( pItm->rcItem.left == -1 || pthis->bReArrange )
        {
            int rItms = rc.right / pthis->psLBox.ItmWidth;
            top = iItem / rItms * pthis->psLBox.ItmHeight;
            left = ( iItem % rItms ) * pthis->psLBox.ItmWidth;
            SetRect ( &pItmLayout->rcBound, left, top,
                                            left + pthis->psLBox.ItmWidth, 
                                            top + pthis->psLBox.ItmHeight );
        }
        else
            CopyRect ( &pItmLayout->rcBound, &pItm->rcItem );

        if ( uStyleMask == LVS_ICON )
        {
            SetRect ( &pItmLayout->rcText, pItmLayout->rcBound.left+1, 
                      pItmLayout->rcBound.top + cy + 5, 
                      pItmLayout->rcBound.left + cx + 2 * DEF_ICON_SPACING, 
                      pItmLayout->rcBound.top + cy + 2 * pItmLayout->tmHeight + 5 ); 
        }
        else
        {
            pItmLayout->rcText.left = pItmLayout->rcBound.left + cx + DEF_SBITEXT_INDENT;
            pItmLayout->rcText.top = pItmLayout->rcBound.top + ICONOFFSET_CY;
            pItmLayout->rcText.right = pItmLayout->rcText.left + SMALLICON_ITEMWIDTH;
            pItmLayout->rcText.bottom = pItmLayout->rcText.top + pItmLayout->tmHeight;
        }
    
        CopyRect ( &pItm->rcItem, &pItmLayout->rcBound ); 

    }

    if ( pthis->psLBox.Extent.cy < pItmLayout->rcBound.bottom ) 
        pthis->psLBox.Extent.cy = pItmLayout->rcBound.bottom;
    if ( pthis->psLBox.Extent.cx < pItmLayout->rcBound.right ) 
        pthis->psLBox.Extent.cx = pItmLayout->rcBound.right;

}

static BOOL
LView_IGetFirstVisibleItm
(
    HWND                hwLView,
    PLISTVDATA          pthis,
    PLVITEMPATH         pItmPath,
    PLVITEMLAYOUT       pItmLayout  
)
{
    int                 cx, cy;
    RECT                rc;
    UINT                uStyleMask;
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWCOL           pCols       = ( PLVIEWCOL )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;
    int                 iItem       = 0;
    BOOL                bResetTopIndx   = FALSE;

    if ( pthis->iCount <= 0 )
        return FALSE;

    if ( !pItmLayout )
        return FALSE;                    

    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) ||
         WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs ) ||
         WLLock ( &pthis->Cols.Columns, ( LPVOID )&pCols )
       )
       return FALSE;

    ImageList_GetIconSize ( pItmLayout->hImgL, &cx, &cy );
    uStyleMask = pthis->uStyles & LVS_TYPEMASK;
    if ( uStyleMask == LVS_REPORT || uStyleMask == LVS_LIST )
    {
        pthis->psLBox.ItmHeight = cy + 2 * ICONOFFSET_CY;
        pthis->psLBox.ItmWidth = pthis->Rows.ColWidth;
    } 
    else
    {
        pthis->psLBox.ItmWidth = uStyleMask == LVS_ICON ? cx + 2 * DEF_ICON_SPACING : SMALLICON_ITEMWIDTH;
        pthis->psLBox.ItmHeight = uStyleMask == LVS_ICON ? 
                                  cy + 2 * pItmLayout->tmHeight + 5 : 
                                  cy + 2 * ICONOFFSET_CY;
    }

    while ( iItem < pthis->iCount )
    {
        pIndx = pIndxs + iItem;
        pItm = pItms + ( int )pIndx->hItm;
        LView_ISetItemRect ( hwLView, pthis, iItem, pItm, pItmLayout );
        if ( !bResetTopIndx &&
             LView_IIsItmVisible ( hwLView, pthis, rc, pItmLayout->rcBound )  )
        {
            pthis->psLBox.lTopIndx = iItem;
            bResetTopIndx = TRUE;
        }
        if ( LView_IIsItmVisible ( hwLView, pthis, pItmLayout->rcPaint, pItmLayout->rcBound )  )
            break;
        iItem++;
    }

    pItmPath->lCol = 0;
    pItmPath->lRow = iItem;
    pItmPath->lpCol = ( PLVIEWCOL )NULL;
    pItmPath->lpRow = pItm;
    pItmPath->lpSubItm = &pItm->firstCol;

    pItmLayout->itemState = 0;
    LView_IInferItmState ( hwLView, pthis, iItem, pItm, pItmLayout );

    return TRUE;
}

#if 0
/* new */
/* used expressly in small/large icon listview */
static int
LView_IGetItmFromPt
(
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    LPPOINT             lppt
)
{
    RECT                rc;
    POINT               pt;
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;
    int                 iItem;      = pthis->psLBox.iAnchorItm;
    int                 iHitItm     = -1;

    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) ||
         WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs )
       )
       return -1;
    if ( iItem >= pthis->iCount )
       return -1;

    pt.x = lppt->x + pthis->psLBox.ptOrg.x;
    pt.y = lppt->y + pthis->psLBox.ptOrg.y;

    GetClientRect ( hWnd, &rc );
    pIndx = pIndxs + iItem;
    pItm = pItms + ( int )pIndx->hItm;
    
    while ( TRUE )
    {
        if ( pItm->rcItem.right < pt.x && pItm->Right != -1 )
        {
            iItem = pItm->Right;
            pItm = pItms + ( pIndxs + iItem )->hItm;
            continue;
        }

        if ( pItm->rcItem.bottom < pt.y && pItm->Below != -1 )
        {
            iItem = pItm->Below;
            pItm = pItms + ( pIndxs + iItem )->hItm;
            continue;
        }
        else
        if ( pItm->rcItem.top > pt.y && pItm->Above != -1 )
        {
            iItem = pItm->Above;
            pItm = pItms + ( pIndxs + iItem )->hItm;
            continue;
        }


        break;
    }

    if ( PtInRect ( &pItm->rcItem, pt ) )
        iHitItm = iItem;

    WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )&pItms );
    WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs );
    return iHitItm;
}
#else
/* new */
/* used expressly in small/large icon listview */
static int
LView_IGetItmFromPt
(
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    LPPOINT             lppt
)
{
    RECT                rc;
    POINT               pt;
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;
    int                 iItem       ; /*= pthis->psLBox.iAnchorItm;*/
    int                 iHitItm     = -1;

    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) ||
         WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs )
       )
       return -1;
    if ( iItem >= pthis->iCount )
       return -1;

    pt.x = lppt->x + pthis->psLBox.ptOrg.x;
    pt.y = lppt->y + pthis->psLBox.ptOrg.y;

    GetClientRect ( hWnd, &rc );
    
    for ( iItem = 0; iItem < pthis->iCount; iItem++ )
    {
        pIndx = pIndxs + iItem;
        pItm = pItms + ( int )pIndx->hItm;
        if ( PtInRect ( &pItm->rcItem, pt ) )
        {
            iHitItm = iItem;
            break;        
        }
    }

    WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )&pItms );
    WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs );
    return iHitItm;
}
#endif

#if 0
/* new */
/* used expressly in small/large icon listview */
static int
LView_IGetNearestItmFromPt
(
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    LPPOINT             lppt
)
{
    RECT                rc;
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;
    int                 iItem       = pthis->psLBox.iAnchorItm;
    /*int                 iHitItm     = -1;*/

    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) ||
         WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs )
       )
       return -1;
    if ( iItem >= pthis->iCount )
       return -1;

    GetClientRect ( hWnd, &rc );
    pIndx = pIndxs + iItem;
    pItm = pItms + ( int )pIndx->hItm;
    
    while ( TRUE )
    {
        if ( pItm->rcItem.right < lppt->x && pItm->Right != -1 )
        {
            iItem = pItm->Right;
            pItm = pItms + ( pIndxs + iItem )->hItm;
            continue;
        }

        if ( pItm->rcItem.bottom < lppt->y && pItm->Below != -1 )
        {
            iItem = pItm->Below;
            pItm = pItms + ( pIndxs + iItem )->hItm;
            continue;
        }

        break;
    }

    WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )&pItms );
    WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs );
    return iItem;
}
#else
/* new */
/* used expressly in small/large icon listview */
static int
LView_IGetNearestItmFromPt
(
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    LPPOINT             lppt
)
{
    RECT                rc;
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;
    int                 iItem       = 0; /*pthis->psLBox.iAnchorItm;*/

    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) ||
         WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs )
       )
       return -1;
    if ( iItem >= pthis->iCount )
       return -1;

    GetClientRect ( hWnd, &rc );
    pIndx = pIndxs + iItem;
    pItm = pItms + ( int )pIndx->hItm;
    
    while ( TRUE )
    {
        if ( pItm->rcItem.right < lppt->x && pItm->Right != -1 )
        {
            iItem = pItm->Right;
            pItm = pItms + ( pIndxs + iItem )->hItm;
            continue;
        }

        if ( pItm->rcItem.bottom < lppt->y && pItm->Below != -1 )
        {
            iItem = pItm->Below;
            pItm = pItms + ( pIndxs + iItem )->hItm;
            continue;
        }

        break;
    }

    WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )&pItms );
    WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs );
    return iItem;
}
#endif

/* new */
static BOOL
LView_IGetItmText
(
    HWND                hWnd,
    LV_ITEM*            pLVItem,
    LPSTR*              lppText  
)
{
    LV_DISPINFO             lvdi;
    lvdi.hdr.code = LVN_GETDISPINFO;
    lvdi.hdr.hwndFrom = hWnd;
    lvdi.hdr.idFrom = GetDlgCtrlID ( hWnd );
    lvdi.item = *pLVItem;
    lvdi.item.mask = LVIF_TEXT;
/*    lvdi.item.iSubItem = 0;*/
    SendMessage ( GetParent ( hWnd ), WM_NOTIFY, lvdi.hdr.idFrom, ( LPARAM )&lvdi );
    *lppText = lvdi.item.pszText;
    return TRUE;
}

static BOOL
LView_IGetNextSubItm
(
    PLISTVDATA          pthis,
    PLVITEMPATH         pItmPath,
    PLVITEMLAYOUT       pItmLayout  
)
{
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWCOL           pCols       = ( PLVIEWCOL )NULL;
    PLVIEWCOL           pCol        = ( PLVIEWCOL )NULL;
    PLVCOLITEM          pColItms    = ( PLVCOLITEM )NULL;
    int                 iNxtCol, iCurCol;

    pItms = pthis->Rows.RowItms.lpData;
    pIndxs = pthis->Rows.SortedRows.lpData;
    pCols = pthis->Cols.Columns.lpData;

    if ( ( iNxtCol = pItmPath->lpSubItm->NSCol ) == 0 )
    {
#if 0
        WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )NULL );
        WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )NULL );
        WLUnlock ( &pthis->Cols.Columns, ( LPVOID )NULL );
#endif
        return FALSE;
    }

    iCurCol = pItmPath->lCol;
    if ( pthis->uStyles & LVS_REPORT )
    {
        pItmLayout->rcText.left = pItmLayout->rcText.right;
        while ( ++iCurCol < iNxtCol )
        {
            pCol = pCols + iCurCol;
            if ( pCol->bDeleted ) continue;
            pItmLayout->rcText.left += pCol->ColWidth; /*pCol->ColInfo.cx;*/
        }
        pItmLayout->rcText.right = pItmLayout->rcText.left + ( pCols + iCurCol )->ColWidth; /*ColInfo.cx;*/
        pItmLayout->rcText.left += DEF_SBITEXT_INDENT;
    }

    {
    int iCol = pItmPath->lpSubItm->NSCol;
    pCol = pCols + iCol;
    WLLock ( &pCol->ColItms, ( LPVOID )&pColItms );
    pItmPath->lpSubItm = pColItms + pItmPath->lpSubItm->NSItm;
    pItmPath->lCol = iCol;
    pItmPath->lpCol = pCol;
    }

    if ( pthis->psLBox.Extent.cy < pItmLayout->rcText.bottom ) 
        pthis->psLBox.Extent.cy = pItmLayout->rcText.bottom;
    if ( pthis->psLBox.Extent.cx < pItmLayout->rcText.right ) 
        pthis->psLBox.Extent.cx = pItmLayout->rcText.right;

    return TRUE;
}


static BOOL
LView_IGetNextVisibleItm
(
    HWND                hwLView,
    PLISTVDATA          pthis,
    PLVITEMPATH         pItmPath,
    PLVITEMLAYOUT       pItmLayout  
)
{
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWCOL           pCols       = ( PLVIEWCOL )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVIEWROW           pPrevItm    = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;
    int                 iItem       = pItmPath->lRow + 1;

    pItms = pthis->Rows.RowItms.lpData;
    pIndxs = pthis->Rows.SortedRows.lpData;
    pCols = pthis->Cols.Columns.lpData;

    while ( iItem < pthis->iCount )
    {
        pIndx = pIndxs + iItem;
        pItm = pItms + ( int )pIndx->hItm;
        LView_ISetItemRect ( hwLView, pthis, iItem, pItm, pItmLayout );
        if ( LView_IIsItmVisible ( hwLView, pthis, pItmLayout->rcPaint, pItmLayout->rcBound )  )
            break;
        iItem++;
    }

    if ( iItem >= pthis->iCount )
    {
        WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )NULL );
        WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )NULL );
        WLUnlock ( &pthis->Cols.Columns, ( LPVOID )NULL );
        return FALSE;
    }

    pItmPath->lCol = 0;
    pItmPath->lRow = iItem; 
    pItmPath->lpCol = ( PLVIEWCOL )NULL;  
    pPrevItm = pItmPath->lpRow;
    pItmPath->lpRow = pItm;
    pItmPath->lpSubItm = &pItm->firstCol;
    
    
    pItmLayout->itemState = 0;
    LView_IInferItmState ( hwLView, pthis, iItem, pItm, pItmLayout );

    return TRUE;
}


HGLOBAL
LView_IGetResource
(
    HWND                hwTree,
    PLISTVDATA*         ppTree
)
{
    HGLOBAL             hRsrc = GETTHISDATA ( hwTree );

    if ( hRsrc &&
         ( *ppTree = ( PLISTVDATA )GlobalLock ( hRsrc ) )
       )
        return hRsrc;

    return ( HGLOBAL )NULL;
}


/* new */
static BOOL
LView_ICloseSubItm
( 
    LISTVIEWDATA*       pthis,
    PLVITEMPATH         pSubItmPath
)
{
    if ( pSubItmPath->lpCol > (PLVIEWCOL)0 )
        WLUnlock ( &pSubItmPath->lpCol->ColItms, ( LPVOID )NULL ); 

    WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )NULL );
    WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )NULL );
    WLUnlock ( &pthis->Cols.Columns, ( LPVOID )NULL );
    return TRUE;
}

static void
LView_IInferItmState
(
    HWND                hWnd,
    PLISTVDATA          pthis,
    int                 iItem,
    PLVIEWROW           pItm,
    PLVITEMLAYOUT       pItmLayout  
)
{
    pItm->lvItm.state &= ~LVIS_FOCUSED;
    if ( iItem == pthis->psLBox.lCurSel && pthis->psLBox.bFocus )
        pItm->lvItm.state |= LVIS_FOCUSED;

    if ( !IsRectEmpty ( &pthis->psLBox.rcSelect ) /*&&
         //pthis->psLBox.eSelMode == eShiftSel*/ 
       )
    {
        RECT    rcDummy;
        RECT    rcItem;
        rcItem.left = LVIR_SELECTBOUNDS;
        LView_OnGetItemRect ( hWnd, pthis, iItem, &rcItem );
        if ( IntersectRect ( &rcDummy, &rcItem, &pthis->psLBox.rcSelect ) )
            pItmLayout->itemState = LVIS_SELECTED;
    }
    else       
    if ( pthis->psLBox.eSelMode == eShiftSel &&
         iItem <= max ( pthis->psLBox.iStartSel, pthis->psLBox.lCurSel ) &&
         iItem >= min ( pthis->psLBox.iStartSel, pthis->psLBox.lCurSel ) )
         pItm->lvItm.state |= LVIS_SELECTED;
    else
    if ( pthis->psLBox.eSelMode != eControlSel &&
         iItem != pthis->psLBox.lCurSel )
    {
        pItm->lvItm.state &= ~LVIS_SELECTED;
    }
    pItmLayout->itemState |= pItm->lvItm.state;
}

static void
LView_ISelectItm
(
    HWND                hWnd,
    LISTVIEWDATA*       pthis,
    int                 iSel
)
{
    LVITEMPATH          ItmPath;
    RECT                rcItem;

    ItmPath.lCol = 0;
    ItmPath.lRow = iSel;
    if ( !LView_IOpenSubItm ( pthis, &ItmPath, FALSE ) )
        return;
    
    if ( pthis->uStyles & LVS_SINGLESEL )
        goto SingleSelect;
    if ( pthis->psLBox.eSelMode == eControlSel )
    {
        ItmPath.lpRow->lvItm.state ^= LVIS_SELECTED;
        if ( ItmPath.lpRow->lvItm.state & LVIS_SELECTED )
            pthis->psLBox.lSelCount++;
        else
            pthis->psLBox.lSelCount--;
        InvalidateRect ( hWnd, &ItmPath.lpRow->rcItem, TRUE );
        if ( !IsRectEmpty ( &pthis->psLBox.rcSelect ) )
        {
            InvalidateRect ( hWnd, &pthis->psLBox.rcSelect, TRUE );
            SetRectEmpty ( &pthis->psLBox.rcSelect );
        }
    }
    else
    if ( pthis->psLBox.eSelMode == eShiftSel )
    {
        InvalidateRect ( hWnd, &pthis->psLBox.rcSelect, TRUE );
        SetRectEmpty ( &pthis->psLBox.rcSelect );
        if ( pthis->psLBox.iStartSel == -1 )
            pthis->psLBox.iStartSel = pthis->psLBox.lCurSel;
        pthis->psLBox.lSelCount = abs ( pthis->psLBox.iStartSel - iSel ) + 1;
#if 0
        if ( ( pthis->uStyles & LVS_TYPEMASK ) == LVS_ICON ||
             ( pthis->uStyles & LVS_TYPEMASK ) == LVS_SMALLICON )
#endif
        {
            RECT    rcItm, rcItmSel;
            rcItm.left = LVIR_BOUNDS;
            rcItmSel.left = LVIR_BOUNDS;
            LView_OnGetItemRect ( hWnd, pthis, pthis->psLBox.iStartSel, &rcItm ); 
            LView_OnGetItemRect ( hWnd, pthis, iSel, &rcItmSel ); 
            SetRect ( &pthis->psLBox.rcSelect, min ( rcItm.left, rcItmSel.left ),
                      min ( rcItm.top, rcItmSel.top ), max ( rcItm.right, rcItmSel.right ),
                      max ( rcItm.bottom, rcItmSel.bottom ) );
            InvalidateRect ( hWnd, &pthis->psLBox.rcSelect, TRUE );
            
        }
    }
    else
    {
SingleSelect:
        if ( pthis->psLBox.lSelCount > 1 )
        {
            InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
            SetRectEmpty ( &pthis->psLBox.rcSelect );
            pthis->psLBox.lSelCount = 1;
            pthis->psLBox.iStartSel = -1;
        }
        ItmPath.lpRow->lvItm.state |= LVIS_SELECTED;
    }

    LView_ICloseSubItm ( pthis, &ItmPath );
    
    if ( pthis->psLBox.lSelCount > 1 )
        InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE ); 
    else
    {
        rcItem.left = (pthis->uStyles & LVS_TYPEMASK)==LVS_REPORT ? 
                      LVIR_LABEL : LVIR_BOUNDS;
        if ( LView_OnGetItemRect ( hWnd, pthis, pthis->psLBox.lCurSel, &rcItem ) )
            InvalidateRect ( hWnd, &rcItem, TRUE );
        rcItem.left = (pthis->uStyles & LVS_TYPEMASK)==LVS_REPORT ? 
                      LVIR_LABEL : LVIR_BOUNDS;
        if ( LView_OnGetItemRect ( hWnd, pthis, iSel, &rcItem ) )
            InvalidateRect ( hWnd, &rcItem, TRUE );
    }
    pthis->psLBox.lCurSel = iSel;

}

/* new */
static BOOL
LView_IOpenSubItm
( 
    LISTVIEWDATA*       pthis,
    PLVITEMPATH         pSubItmPath,
    BOOL                bCreate
)
{
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWCOL           pCols       = ( PLVIEWCOL )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;
    int                 iItem       = ( int )pSubItmPath->lRow;
    int                 iSubItem    = ( int )pSubItmPath->lCol;

    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) ||
         WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs ) ||
         WLLock ( &pthis->Cols.Columns, ( LPVOID )&pCols )
       )
       return FALSE;
    if ( iItem >= pthis->iCount )
       iItem = pthis->iCount - 1; 
    pIndx = pIndxs + iItem;
    pItm = pItms + ( int )pIndx->hItm;

    pSubItmPath->lCol = iSubItem;
    pSubItmPath->lRow = pIndx->hItm;
    pSubItmPath->lpCol = ( PLVIEWCOL )NULL;  /*&pItm->firstCol;*/
    pSubItmPath->lpRow = pItm;
    pSubItmPath->lpSubItm = &pItm->firstCol;

    if ( iSubItem == 0 )
        return TRUE;
    if ( iSubItem >= pthis->Cols.lCount )
        return FALSE;

    return LVCols_LocateSubItem ( pCols, pSubItmPath, bCreate );  
}

#if 0
static BOOL
LView_IReposition
(
    HWND                hWnd,
    PLISTVDATA          pthis,
    int                 iItem,
    int                 x,
    int                 y
)
{
    int                 iItemUsher;
    SIZE                Offset;
    POINT               pt;
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVIEWROW           pItmLeft    = ( PLVIEWROW )NULL;
    PLVIEWROW           pItmRight   = ( PLVIEWROW )NULL;
    PLVIEWROW           pItmAbove   = ( PLVIEWROW )NULL;
    PLVIEWROW           pItmBelow   = ( PLVIEWROW )NULL;
    PLVIEWROW           pItmUsher   = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;
    int                 itmHeight   = pthis->psLBox.ItmHeight;
    int                 itmWidth    = pthis->psLBox.ItmWidth;

    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) ||
         WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs )
       )
       return FALSE;
    if ( iItem >= pthis->iCount )
       return FALSE;

    pIndx = pIndxs + iItem;
    pItm = pItms + ( int )pIndx->hItm;
    pt.x = x; pt.y = y;
    Offset.cx = x - pItm->rcItem.right;
    Offset.cy = y - pItm->rcItem.bottom;
    if ( abs ( Offset.cx )  <= itmWidth &&
         abs ( Offset.cy )  <= itmHeight )
         goto EndRePos;

    if ( pItm->Left != -1 )
        pItmLeft = pItms + (int)( pIndxs + pItm->Left )->hItm;
    if ( pItm->Right != -1 )
        pItmRight = pItms + (int)( pIndxs + pItm->Right )->hItm;
    if ( pItm->Above != -1 )
        pItmAbove = pItms + (int)( pIndxs + pItm->Above )->hItm;
    if ( pItm->Below != -1 )
        pItmBelow = pItms + (int)( pIndxs + pItm->Below )->hItm;

    if ( abs ( Offset.cx ) > itmWidth ||
         abs ( Offset.cy ) > itmHeight )
    {
        iItemUsher = LView_IGetNearestItmFromPt ( hWnd, pthis, &pt );
        if ( pItmLeft )
        {
            pItmLeft->Right = pItm->Above;
            if ( pItmLeft->Right == -1 )
                pItmLeft->Right = pItm->Below;
            if ( pItmLeft->Right == -1 )
                pItmLeft->Right = pItm->Right;
        }
        if ( pItmRight )
        {
            pItmRight->Left = pItm->Above;
            if ( pItmRight->Left == -1 )
                pItmRight->Left = pItm->Below;
            if ( pItmRight->Left == -1 )
                pItmRight->Left = pItm->Left;
        }
        if ( pItmAbove )
        {
            pItmAbove->Below = pItm->Left;
            if ( pItmAbove->Below == -1 )
                pItmAbove->Below = pItm->Right;
            if ( pItmAbove->Below == -1 )
                pItmAbove->Below = pItm->Below;

        }
        if ( pItmBelow )
        {
            pItmBelow->Above = pItm->Right;
            if ( pItmBelow->Above == -1 )
                pItmBelow->Above = pItm->Left;
            if ( pItmBelow->Above == -1 )
                pItmBelow->Above = pItm->Above;
        }
    }

    if ( iItemUsher == -1 ) 
        goto EndRePos;

    if ( iItem == pthis->psLBox.iAnchorItm )
        pthis->psLBox.iAnchorItm = pItm->Right;
    pItmUsher = pItms + (int)( pIndxs + iItemUsher )->hItm;
    pItm->Left = pItm->Right = pItm->Above = pItm->Below = -1;
    if ( pItmUsher->rcItem.right < x )
    {
        pItmUsher->Right = iItem; 
        if ( pItmUsher->rcItem.top > y && pItmUsher->Above != -1 )
            iItemUsher = pItmUsher->Above;
        pItm->Left = iItemUsher;
    }
    else
    if ( pItmUsher->rcItem.right >= x && 
         pItmUsher->rcItem.left  <= x )
    {
        if ( pItmUsher->rcItem.bottom <= y )
        {
            pItm->Above = iItemUsher;
            pItmUsher->Below = iItem; 
        }
        else
        {
            pItm->Below = iItemUsher;
            if ( pItmUsher->Above != -1 )
            {
                iItemUsher = pItmUsher->Above;
                pItmAbove = pItms + (int)( pIndxs + iItemUsher )->hItm;
                pItmAbove->Below = iItem;
                pItm->Above = iItemUsher;
            }
            pItmUsher->Above = iItem; 
        }
    }
    else
    if ( pItmUsher->rcItem.left > x )
    {
        if ( pItmUsher->rcItem.bottom + itmHeight < y )
        {
            pItmUsher->Below = iItem; 
            pItm->Above = iItemUsher;
        }
        else
/*        if ( pItmUsher->rcItem.top < y )*/
        {
            pItm->Right = iItemUsher;
            if ( pItmUsher->Left != -1 )
            {
                iItemUsher = pItmUsher->Left;
                pItmLeft = pItms + (int)( pIndxs + iItemUsher )->hItm;
                pItmLeft->Right = iItem;
                pItm->Left = iItemUsher;
            }
            pItmUsher->Left = iItem; 
        }
        if ( pItmUsher->rcItem.top > y && pItmUsher->Above != -1 )
            iItemUsher = pItmUsher->Above;
        pItm->Right = iItemUsher;
    }

EndRePos:
    WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )&pItms );
    WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs );
}
#else
static BOOL
LView_IReposition
(
    HWND                hWnd,
    PLISTVDATA          pthis,
    int                 iItem,
    int                 x,
    int                 y
)
{
    int                 iItemUsher;
    SIZE                Offset;
    POINT               pt;
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVIEWROW           pItmLeft    = ( PLVIEWROW )NULL;
    PLVIEWROW           pItmRight   = ( PLVIEWROW )NULL;
    PLVIEWROW           pItmAbove   = ( PLVIEWROW )NULL;
    PLVIEWROW           pItmBelow   = ( PLVIEWROW )NULL;
    PLVIEWROW           pItmUsher   = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;
    int                 itmHeight   = pthis->psLBox.ItmHeight;
    int                 itmWidth    = pthis->psLBox.ItmWidth;

    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) ||
         WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs )
       )
       return FALSE;
    if ( iItem >= pthis->iCount )
       return FALSE;

    pIndx = pIndxs + iItem;
    pItm = pItms + ( int )pIndx->hItm;
    pt.x = x; pt.y = y;
    Offset.cx = x - pItm->rcItem.right;
    Offset.cy = y - pItm->rcItem.bottom;
    if ( abs ( Offset.cx )  <= itmWidth &&
         abs ( Offset.cy )  <= itmHeight )
         goto EndRePos;

    if ( pItm->Left != -1 )
        pItmLeft = pItms + (int)( pIndxs + pItm->Left )->hItm;
    if ( pItm->Right != -1 )
        pItmRight = pItms + (int)( pIndxs + pItm->Right )->hItm;
    if ( pItm->Above != -1 )
        pItmAbove = pItms + (int)( pIndxs + pItm->Above )->hItm;
    if ( pItm->Below != -1 )
        pItmBelow = pItms + (int)( pIndxs + pItm->Below )->hItm;

    if ( abs ( Offset.cx ) > itmWidth ||
         abs ( Offset.cy ) > itmHeight )
    {
        iItemUsher = LView_IGetNearestItmFromPt ( hWnd, pthis, &pt );
        if ( pItmLeft )
        {
            pItmLeft->Right = pItm->Above;
            if ( pItmLeft->Right == -1 )
                pItmLeft->Right = pItm->Below;
            if ( pItmLeft->Right == -1 )
                pItmLeft->Right = pItm->Right;
        }
        if ( pItmRight )
        {
            pItmRight->Left = pItm->Above;
            if ( pItmRight->Left == -1 )
                pItmRight->Left = pItm->Below;
            if ( pItmRight->Left == -1 )
                pItmRight->Left = pItm->Left;
        }
        if ( pItmAbove )
        {
            pItmAbove->Below = pItm->Left;
            if ( pItmAbove->Below == -1 )
                pItmAbove->Below = pItm->Right;
            if ( pItmAbove->Below == -1 )
                pItmAbove->Below = pItm->Below;

        }
        if ( pItmBelow )
        {
            pItmBelow->Above = pItm->Right;
            if ( pItmBelow->Above == -1 )
                pItmBelow->Above = pItm->Left;
            if ( pItmBelow->Above == -1 )
                pItmBelow->Above = pItm->Above;
        }
    }

    if ( iItemUsher == -1 ) 
        goto EndRePos;

    pItmUsher = pItms + (int)( pIndxs + iItemUsher )->hItm;
    pItm->Left = pItm->Right = pItm->Above = pItm->Below = -1;
    if ( pItmUsher->rcItem.right < x )
    {
        pItmUsher->Right = iItem; 
        if ( pItmUsher->rcItem.top > y && pItmUsher->Above != -1 )
            iItemUsher = pItmUsher->Above;
        pItm->Left = iItemUsher;
    }
    else
    if ( pItmUsher->rcItem.right >= x && 
         pItmUsher->rcItem.left  <= x )
    {
        if ( pItmUsher->rcItem.bottom <= y )
        {
            pItm->Above = iItemUsher;
            pItmUsher->Below = iItem; 
        }
        else
        {
            pItm->Below = iItemUsher;
            if ( pItmUsher->Above != -1 )
            {
                iItemUsher = pItmUsher->Above;
                pItmAbove = pItms + (int)( pIndxs + iItemUsher )->hItm;
                pItmAbove->Below = iItem;
                pItm->Above = iItemUsher;
            }
            pItmUsher->Above = iItem; 
        }
    }
    else
    if ( pItmUsher->rcItem.left > x )
    {
        if ( pItmUsher->rcItem.bottom + itmHeight < y )
        {
            pItmUsher->Below = iItem; 
            pItm->Above = iItemUsher;
        }
        else
/*        if ( pItmUsher->rcItem.top < y )*/
        {
            pItm->Right = iItemUsher;
            if ( pItmUsher->Left != -1 )
            {
                iItemUsher = pItmUsher->Left;
                pItmLeft = pItms + (int)( pIndxs + iItemUsher )->hItm;
                pItmLeft->Right = iItem;
                pItm->Left = iItemUsher;
            }
            pItmUsher->Left = iItem; 
        }
        if ( pItmUsher->rcItem.top > y && pItmUsher->Above != -1 )
            iItemUsher = pItmUsher->Above;
        pItm->Right = iItemUsher;
    }

EndRePos:
    WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )&pItms );
    WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs );

    return TRUE;
}
#endif

static BOOL
LView_ISetColWidth
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iCol,
    int                 iWidth
)
{
    PLVIEWCOL           pCols       = ( PLVIEWCOL )NULL;

    if ( WLLock ( &pthis->Cols.Columns, ( LPVOID )&pCols ) )
        return FALSE;
    if ( iCol >= pthis->Cols.lCount )
        return FALSE;
    ( pCols + iCol )->ColWidth = iWidth;
    if ( iCol == 0 )
        pthis->Rows.ColWidth = iWidth;
    WLUnlock ( &pthis->Cols.Columns, ( LPVOID )NULL );
    return TRUE;
}


/* new */
static BOOL
LView_IUpdateSubItm 
( 
/*    LISTVIEWDATA*       pthis,*/
    PLVITEMPATH         pSubItmPath,
    LV_ITEM*            lplvi 
)
{
    PLVCOLITEM          pSubItm     = ( PLVCOLITEM )NULL;

    pSubItm = pSubItmPath->lpSubItm;

    if ( !pSubItm )
        return FALSE;
    if ( pSubItm->pszText > (LPSTR)0 && pSubItm->pszText != LPSTR_TEXTCALLBACK )
        WinFree ( pSubItm->pszText );
    pSubItm->pszText = lplvi->pszText;
    if ( lplvi->pszText != ( LPSTR )NULL &&  
         lplvi->pszText != LPSTR_TEXTCALLBACK )
    {
        if ( !( pSubItm->pszText = 
		WinMalloc ( lstrlen ( lplvi->pszText ) + 1 ) ) )
            return FALSE;
        lstrcpy ( pSubItm->pszText, lplvi->pszText );
    }

    return TRUE;
}

void
LView_IResetScroll
(
    HWND                hWnd,
    PLISTVDATA          pthis
)
{
    RECT                rc;

    GetClientRect ( hWnd, &rc );
    if ( pthis->psLBox.Extent.cx > rc.right )
    {
        SetScrollRange ( hWnd, SB_HORZ, 0, pthis->psLBox.Extent.cx - rc.right, TRUE ); 
        ShowScrollBar ( hWnd, SB_HORZ, TRUE );        
    }
    else
        ShowScrollBar ( hWnd, SB_HORZ, FALSE );        
    if ( pthis->psLBox.Extent.cy > rc.bottom - rc.top )
    {
        SetScrollRange ( hWnd, SB_VERT, 0, pthis->psLBox.Extent.cy- rc.bottom, TRUE ); 
        ShowScrollBar ( hWnd, SB_VERT, TRUE );        
    }
    else
        ShowScrollBar ( hWnd, SB_VERT, FALSE );        

}

void
LView_IReleaseResource
(
    HGLOBAL             hRsrc,
    PLISTVDATA          pTree
)
{
    if ( pTree )
        GlobalUnlock ( hRsrc );
}

static BOOL 
LView_OnCreate 
(
	HWND			hWnd, 
	CREATESTRUCT*		pcs
) 
{
	RECT                	rc;
    	HGLOBAL			hListView	= ( HGLOBAL )NULL;
	LISTVIEWDATA*		pthis		= ( LISTVIEWDATA* )NULL;

	int		    	ErrorCode	= 0;

	if ( ! ( hListView = GlobalAlloc ( GHND, sizeof ( LISTVIEWDATA ) ) ) )
		ErrorCode = WLV_ERR_GLOBALALLOC;
	else
	if ( ! ( pthis = ( LISTVIEWDATA *) GlobalLock ( hListView ) ) )
		ErrorCode = WLV_ERR_GLOBALLOCK;
	else
	{
        	UINT    edtStyle = WS_CHILD | WS_BORDER;
        	UINT    hdrStyle = WS_CHILD | WS_BORDER | HDS_HORZ;
		SETTHISDATA(hWnd, hListView);
		pthis->uStyles = pcs->style;
		pthis->hFont = ( HFONT )GetStockObject ( ANSI_VAR_FONT );
        	GetClientRect ( hWnd, &rc );
        	pthis->psLBox.ItmWidth = rc.right - rc.left;
        	pthis->psLBox.ItmHeight = DEF_ITEMHEIGHT;
        	pthis->psLBox.lCurSel = -1;
        	pthis->psLBox.lDropSel = -1;
        	pthis->psLBox.lCaret = -1;
        	pthis->psLBox.ptOrg.x = pthis->psLBox.ptOrg.y = 0; 
/*        pthis->psLBox.iAnchorItm = 0; */
        	pthis->psLBox.iStartSel = -1; 
        	pthis->Rows.iDeleted = -1;
        	pthis->bReArrange = TRUE;
        	pthis->BkColor = CLR_NONE;
        	pthis->TxtColor = GetSysColor ( COLOR_BTNTEXT );
        	pthis->TxtBkColor = GetSysColor ( COLOR_WINDOW );

        /* is ES_WANTRETURN supported in Twin32 ? */
        	if ( ( pthis->uStyles & LVS_TYPEMASK ) == LVS_ICON )
            		edtStyle |= ES_CENTER | ES_WANTRETURN | ES_MULTILINE;
        	else
            		edtStyle |= ES_LEFT | ES_WANTRETURN;

        	pthis->hwEdit = CreateWindowEx((DWORD)0L,
                	 (LPSTR)"EDIT",(LPSTR)NULL,
                	 edtStyle,
                 	0, 0, 0, 0,
                 	hWnd,
                 	(HMENU)IDLVEDIT,
                 	pcs->hInstance,
                 	NULL);
        	LVEdit_Subclass ( pthis->hwEdit );
        	SendMessage ( pthis->hwEdit, WM_SETFONT, ( WPARAM )pthis->hFont, ( LPARAM )FALSE );

        	if ( !( pthis->uStyles & LVS_NOSORTHEADER ) )
            		hdrStyle |= HDS_BUTTONS;
#ifdef _WINDOWS
        pthis->hwHeader = CreateWindowEx ( ( DWORD )0L, 
                 "WSysHeader32", (LPSTR)NULL, 
                 hdrStyle,
                 0, 0, rc.right - rc.left, DEF_ITEMHEIGHT, 
                 hWnd, ( HMENU )NULL, 
                 pcs->hInstance,
                 NULL );
#else
#if 1
        pthis->hwHeader = CreateWindowEx ( ( DWORD )0L, 
                 WC_HEADER, (LPSTR)NULL, 
                 hdrStyle,
                 0, 0, 0, 0, 
                 hWnd, ( HMENU )NULL, 
                 pcs->hInstance,
                 NULL );
#endif
#endif

        LVRows_Init ( &pthis->Rows );
        LVCols_Init ( &pthis->Cols );

	}

	if ( pthis )
		GlobalUnlock ( hListView );
	return ( ErrorCode >= 0 );
}

void
LView_OnDrawItem
(
    HWND                    hWnd, 
    LISTVIEWDATA*	    pthis,
    PLVITEMPATH             pItmPath,
    PLVITEMLAYOUT           pLayout
)
{
    UINT                    lvStyles = pthis->uStyles & LVS_TYPEMASK;

    if ( lvStyles == LVS_ICON )
        LView_IDrawSubItmLIcon ( hWnd, pthis, pItmPath, pLayout );     
    else
    if ( lvStyles == LVS_REPORT && pthis->uStyles & LVS_OWNERDRAWFIXED )
        LView_IOwnerDraw ( hWnd, pthis, pItmPath, pLayout, lvStyles );     
    else
    {
        LView_IDrawSubItmRP ( hWnd, pthis, pItmPath, pLayout, lvStyles );     
        if ( lvStyles == LVS_REPORT )
        {
            while ( TRUE )
            {        
                if ( !LView_IGetNextSubItm ( pthis, pItmPath, pLayout ) )
                    break;
                LView_IDrawSubItmRP ( hWnd, pthis, pItmPath, pLayout, lvStyles );     
            }
        }
    }

}

void 
LView_OnKeyDown (HWND hWnd, int VKey, int cRepeat, UINT keyFlags) 
{
    LV_KEYDOWN      lkd;
    HWND            hwFocus     = GetFocus ();
	HGLOBAL		    hRsrc   	= ( HGLOBAL )NULL;
	LISTVIEWDATA*	pthis		= ( LISTVIEWDATA* )NULL;
    
    assert ( hRsrc = LView_IGetResource ( hWnd, &pthis ) );
    lkd.hdr.code = LVN_KEYDOWN;
    lkd.hdr.hwndFrom = hWnd;
    lkd.hdr.idFrom = GetDlgCtrlID ( hWnd );
    lkd.wVKey = VKey;
    SendMessage ( GetParent ( hWnd ), WM_NOTIFY, lkd.hdr.idFrom, ( LPARAM )&lkd ); 
    
    if ( ( VKey == VK_ESCAPE || VKey == VK_RETURN )
         && hwFocus == pthis->hwEdit )
    {
        BOOL bCancel = VKey == VK_ESCAPE ? TRUE : FALSE;
        LView_OnEndEditLabelNow ( hWnd, pthis, bCancel );
    }
    else
    if ( VKey == VK_TAB )
    {
        SetFocus ( GetNextDlgTabItem ( GetParent ( hWnd ), hWnd, FALSE ) );
    }
    else
    {
        int     iSel = -1;
        UINT    flags = VKey == VK_UP ? LVNI_ABOVE :
                        VKey == VK_DOWN ? LVNI_BELOW :
                        VKey == VK_RIGHT ? LVNI_TORIGHT :
                        VKey == VK_LEFT ? LVNI_TOLEFT : 0;
                        
        if ( flags )
            iSel = LView_OnGetNextItem ( hWnd, pthis, pthis->psLBox.lCurSel, flags );
        if ( iSel != -1 )
            LView_ISelectItm ( hWnd, pthis, iSel );
    }


    LView_IReleaseResource ( hRsrc, pthis );

}

/* to be deleted later */
static void 
LView_OnMButtonDown (HWND hWnd, int x, int y, UINT keyFlags) 
{
    HGLOBAL			hListView	= ( HGLOBAL )NULL;
	LISTVIEWDATA*	pthis		= ( LISTVIEWDATA* )NULL;
	
	int			    ErrorCode	= 0;

	if ( ! ( hListView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WLV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( LISTVIEWDATA *) GlobalLock ( hListView ) ) )
		ErrorCode = WLV_ERR_GLOBALLOCK;
	else
        LView_OnSetItemPos ( hWnd, pthis, pthis->psLBox.lCurSel, x, y );
	
	if ( pthis )
		GlobalUnlock ( hListView );
}

static void 
LView_OnLButtonDown 
(
    	HWND            hWnd, 
    	int             x, 
    	int             y, 
    	UINT            keyFlags,
    	BOOL            bLBtnDown
) 
{
    	HGLOBAL		hListView	= ( HGLOBAL )NULL;
	LISTVIEWDATA*	pthis		= ( LISTVIEWDATA* )NULL;
	
	int	    	ErrorCode	= 0;

	if ( ! ( hListView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WLV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( LISTVIEWDATA *) GlobalLock ( hListView ) ) )
		ErrorCode = WLV_ERR_GLOBALLOCK;
	else
	{
        LV_HITTESTINFO hti;
        int iSel;
        SetFocus ( hWnd );
        hti.pt.x = x; /* + pthis->psLBox.ptOrg.x;*/
        hti.pt.y = y; /* + pthis->psLBox.ptOrg.y;*/
        if ( ( iSel = LView_OnHitTest ( hWnd, pthis, &hti ) ) != -1 )
        {
            if ( pthis->psLBox.lCurSel == iSel && pthis->uStyles & LVS_EDITLABELS )
                SetTimer ( hWnd, STARTEDIT_TIMER, 500, NULL );
            else
            {
                if ( IsWindowVisible ( pthis->hwEdit ) )
                    SetWindowPos ( pthis->hwEdit, HWND_BOTTOM, 0, 0, 0, 0, 
                               SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE );
                pthis->psLBox.eSelMode = ( keyFlags & MK_CONTROL ) ? eControlSel :
                                         ( keyFlags & MK_SHIFT ) ? eShiftSel : eSingleSel; 
                LView_ISelectItm ( hWnd, pthis, iSel );
            }

            pthis->ptDrag.x = x;
            pthis->ptDrag.y = y;
            pthis->enDrag = bLBtnDown ? eLBtnDown : eRBtnDown;
        }

	}	
	
	if ( pthis )
		GlobalUnlock ( hListView );
}

static void 
LView_OnLButtonUp (HWND hWnd, int x, int y, UINT keyFlags) 
{
    	HGLOBAL			hListView	= ( HGLOBAL )NULL;
	LISTVIEWDATA*		pthis		= ( LISTVIEWDATA* )NULL;
	
	int				ErrorCode	= 0;

	if ( ! ( hListView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WLV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( LISTVIEWDATA *) GlobalLock ( hListView ) ) )
		ErrorCode = WLV_ERR_GLOBALLOCK;
    {
        if ( pthis->enDrag == eDragging && hWnd == GetCapture () )
            ReleaseCapture ( );
        pthis->enDrag = eNoDrag;
    }

	if ( pthis )
		GlobalUnlock ( hListView );

}

/*Need revisit this*/
static void 
LView_OnDestroy ( HWND hWnd )
{
	HGLOBAL			hListView	= ( HGLOBAL )NULL;
	LISTVIEWDATA*		pthis		= ( LISTVIEWDATA* )NULL;

	int		    	ErrorCode	= 0;

	if ( ! ( hListView = GlobalAlloc ( GHND, sizeof ( LISTVIEWDATA ) ) ) )
		ErrorCode = WLV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( LISTVIEWDATA *) GlobalLock ( hListView ) ) )
		ErrorCode = WLV_ERR_GLOBALLOCK;
	else
	{
        if ( pthis->hwEdit )
            DestroyWindow ( pthis->hwEdit );
        WLDestroy ( &pthis->Rows.RowItms );
        WLDestroy ( &pthis->Rows.SortedRows );
        /*LVRows_Destroy ( &pthis->Rows );*/
        LVCols_Destroy ( &pthis->Cols );
        if ( !( pthis->uStyles & LVS_SHAREIMAGELISTS ) && pthis->hImgList )
            ImageList_Destroy ( pthis->hImgList );
        if ( !( pthis->uStyles & LVS_SHAREIMAGELISTS ) && pthis->hSImgList )
            ImageList_Destroy ( pthis->hSImgList );

        if ( pthis )
            GlobalUnlock ( hListView );
        REMOVETHISDATA ( hWnd );
		GlobalFree ( hListView );
	}
}

static void 
LView_OnMouseMove 
(
	HWND			hWnd,
    	UINT            	uMessage,
	int			x, 
	int			y, 
	UINT			keyFlags
) 
{
    	POINT           	pt;
	HGLOBAL			hListView	= ( HGLOBAL )NULL;
	LISTVIEWDATA*		pthis		= ( LISTVIEWDATA* )NULL;
	int			ErrorCode	= 0;

	if ( ! ( hListView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WLV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( LISTVIEWDATA *) GlobalLock ( hListView ) ) )
		ErrorCode = WLV_ERR_GLOBALLOCK;
	else
    if ( ( pthis->enDrag == eLBtnDown ) ||
         ( pthis->enDrag == eRBtnDown ) )
	{
        /*bLBtnDwn = pthis->enDrag == eLBtnDown ? TRUE : FALSE;*/
        if ( abs( pthis->ptDrag.x - x ) > 5 ||
             abs( pthis->ptDrag.y - y ) > 5  
           )
           pthis->enDrag = pthis->enDrag == eLBtnDown ? eBeginLDrag : eBeginRDrag;
	}
    else
    if ( pthis->enDrag == eBeginLDrag || pthis->enDrag == eBeginRDrag )
    {
        NM_LISTVIEW nmlv;
        PLVIEWROW   pItms       = ( PLVIEWROW )NULL;
        PLVSORTITEM pIndxs      = ( PLVSORTITEM )NULL;
        PLVIEWROW   pItm        = ( PLVIEWROW )NULL;
        int         iSel        = pthis->psLBox.lCurSel;

        if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) == 0 &&
             WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs ) == 0
           )
        {
            memset ( &nmlv, 0, sizeof ( NM_LISTVIEW ) );
            pItm = pItms + ( pIndxs + iSel )->hItm;
            nmlv.iItem = iSel;
            nmlv.hdr.hwndFrom = hWnd;
            nmlv.hdr.idFrom = GetDlgCtrlID ( hWnd );
            nmlv.hdr.code = pthis->enDrag == eBeginLDrag ? LVN_BEGINDRAG : LVN_BEGINRDRAG;
            nmlv.ptAction.x = x;
            nmlv.ptAction.y = y;
            nmlv.lParam = pItm->lvItm.lParam;
            SendMessage ( GetParent ( hWnd ), WM_NOTIFY, nmlv.hdr.idFrom, ( LPARAM )&nmlv ); 
            
            WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )NULL );
            WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )NULL );
        }

        pthis->enDrag = eDragging;

    }
    else
    if ( pthis->enDrag == eDragging )
    {
        pt.x = x;
        pt.y = y;
        SendMessage ( GetParent ( hWnd ), uMessage , ( WPARAM )keyFlags, ( LPARAM )MAKELONG ( pt.x, pt.y ) );
    }
	
	if ( pthis )
		GlobalUnlock ( hListView );
}

static void 
LView_OnNotify 
(
	HWND			hWnd,
    	UINT            	uMessage,
	int		    	idControl, 
	LPNMHDR         	lpNotify 
) 
{
	HGLOBAL			hListView	= ( HGLOBAL )NULL;
	LISTVIEWDATA*		pthis		= ( LISTVIEWDATA* )NULL;
	int			ErrorCode	= 0;
    	HD_NOTIFY*      	lpHD        	= ( HD_NOTIFY* )NULL;

    if ( ! ( hListView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WLV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( LISTVIEWDATA *) GlobalLock ( hListView ) ) )
		ErrorCode = WLV_ERR_GLOBALLOCK;
	else
    if ( lpNotify->code == HDN_ITEMCLICK &&
         lpNotify->hwndFrom == pthis->hwHeader )
    {
         NM_LISTVIEW     nmlv;
         lpHD = ( HD_NOTIFY* )lpNotify;
         memset ( &nmlv, 0, sizeof ( NM_LISTVIEW ) );
         nmlv.iItem = -1;
         nmlv.iSubItem = lpHD->iItem;
         nmlv.hdr.code = LVN_COLUMNCLICK;
         nmlv.hdr.hwndFrom = hWnd;
         nmlv.hdr.idFrom = GetDlgCtrlID ( hWnd );
         SendMessage ( GetParent ( hWnd ), WM_NOTIFY, nmlv.hdr.idFrom, ( LPARAM )&nmlv );
    }
    else
    if ( lpNotify->code == HDN_ITEMCHANGED &&
         lpNotify->hwndFrom == pthis->hwHeader )
    {
        lpHD = ( HD_NOTIFY* )lpNotify;
        if ( lpHD->pitem->mask & HDI_WIDTH )
        {
            LView_ISetColWidth ( hWnd, pthis, lpHD->iItem, lpHD->pitem->cxy );
            InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
        }
    }
        
    if ( pthis )
		GlobalUnlock ( hListView );
}

#if 1
static int 
LView_OnEraseBkground 
(
	HWND			hWnd, 
	HDC			hDC 
)
{
    	HGLOBAL			hListView	= ( HGLOBAL )NULL;
	LISTVIEWDATA*		pthis		= ( LISTVIEWDATA* )NULL;
	RECT			rect;
	HPEN			hPen        	= GetStockObject ( BLACK_PEN );
	HBRUSH			hBrush	    	= ( HBRUSH )NULL;
	int			ErrorCode	= 0;
    	int             	Result      	= 0;

	if ( ! ( hListView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WLV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( LISTVIEWDATA *) GlobalLock ( hListView ) ) )
		ErrorCode = WLV_ERR_GLOBALLOCK;
	else
	if ( hDC )
	{
        if ( pthis->BkColor == CLR_NONE )
            hBrush = CreateSolidBrush ( GetSysColor ( COLOR_WINDOW ) );
        else
            hBrush = CreateSolidBrush ( pthis->BkColor );
		hPen = SelectObject ( hDC, hPen );
		hBrush = SelectObject ( hDC, hBrush );
		GetClientRect ( hWnd, &rect );
		Rectangle ( hDC, rect.left, rect.top,
					   rect.right, rect.bottom );
		hPen = SelectObject ( hDC, hPen );
		hBrush = SelectObject ( hDC, hBrush );
        DeleteObject ( hBrush );
		Result = 1;
	}

	if ( pthis )
		GlobalUnlock ( hListView );
	return Result;

}
#endif

#if 0
static void 
LView_OnMeasureItem
(   
    HWND                hWnd, 
    MEASUREITEMSTRUCT*  lpMeasureItem 
)
{
    TEXTMETRIC          tm;
    HDC                 hDC;
    PLISTVDATA          pthis       = ( PLISTVDATA )NULL;
    HGLOBAL             hRsrc       = ( HGLOBAL )NULL;

    assert ( hRsrc = LView_IGetResource ( hWnd, &pthis ) );
    hDC = GetDC ( hWnd );
    GetTextMetrics ( hDC, &tm );
    
    lpMeasureItem->itemHeight = max ( tm.tmHeight, DEF_ITEMHEIGHT );
    ReleaseDC ( hWnd, hDC );
    LView_IReleaseResource ( hRsrc, pthis );
}
#endif

static void 
LView_OnPaint 
(
	HWND			hWnd, 
	HDC		    	hPaintDC 
)
{
	PAINTSTRUCT         	ps;
    	LVITEMLAYOUT        	lviLayout;
    	LVITEMPATH          	lviPath;
    	BOOL                	bMoreItm    	= FALSE;
	HGLOBAL			hListView	= ( HGLOBAL )NULL;
	LISTVIEWDATA*		pthis		= ( LISTVIEWDATA* )NULL;
	
	int			ErrorCode	= 0;

	BeginPaint(hWnd, &ps);
	if ( !IsWindowVisible ( hWnd ) )
		goto EndLViewPaint;
	
	if ( ! ( hListView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WLV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( LISTVIEWDATA *) GlobalLock ( hListView ) ) )
		ErrorCode = WLV_ERR_GLOBALLOCK;
	else
	{
        int   Count;
        UINT  uStyleMask;
        HFONT hOldFont;
        TEXTMETRIC tm;
        RECT    rc;
        GetClientRect ( hWnd, &rc );
        
        Count = pthis->iCount;

        if ( rc.bottom > pthis->psLBox.Extent.cy )
            pthis->psLBox.ptOrg.y = 0;
        if ( rc.right > pthis->psLBox.Extent.cx )
            pthis->psLBox.ptOrg.x = 0;
        /* Start painting.*/
        
        hOldFont = SelectObject ( ps.hdc, pthis->hFont );
        GetTextMetrics ( ps.hdc, &tm );
        SelectObject ( ps.hdc, hOldFont );

        lviLayout.tmAvCharWidth = tm.tmAveCharWidth;
        lviLayout.tmHeight = tm.tmHeight;
        lviLayout.hDC = ps.hdc;
        CopyRect ( &lviLayout.rcPaint, &ps.rcPaint );
        lviLayout.hFont = pthis->hFont;
        lviLayout.cMaxText = 0;
        if ( ( uStyleMask = pthis->uStyles & LVS_TYPEMASK ) == LVS_ICON )
            lviLayout.hImgL = pthis->hImgList;
        else
            lviLayout.hImgL = pthis->hSImgList;
#if 0
        if ( ( pthis->uStyles & LVS_TYPEMASK ) == LVS_REPORT &&
             !( pthis->uStyles & LVS_NOCOLUMNHEADER ) )
            SetWindowPos ( pthis->hwHeader, HWND_TOP, 0 - pthis->psLBox.ptOrg.x,0, 
                       rc.right + pthis->psLBox.ptOrg.x, pthis->psLBox.ItmHeight, SWP_SHOWWINDOW );
	else
            SetWindowPos ( pthis->hwHeader, HWND_BOTTOM, 0, 0, 0, 0,
                           SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE );
#endif
        ImageList_GetIconSize ( lviLayout.hImgL, &pthis->BmpSize.cx, &pthis->BmpSize.cy ); 
        
        SetWindowOrgEx ( lviLayout.hDC, pthis->psLBox.ptOrg.x, pthis->psLBox.ptOrg.y, NULL );
        /* need enhancement to restrict drawing to only visible items */
        bMoreItm = LView_IGetFirstVisibleItm ( hWnd, pthis, &lviPath, &lviLayout );
        while ( bMoreItm )
        {
            LView_OnDrawItem ( hWnd, pthis, &lviPath, &lviLayout );
            bMoreItm = LView_IGetNextVisibleItm ( hWnd, pthis, &lviPath, &lviLayout );
        } 

        if ( pthis->bReArrange && 
            ( uStyleMask == LVS_ICON || uStyleMask == LVS_SMALLICON ) )
            LView_IArrangeIcons ( hWnd, pthis );

        pthis->bReArrange = FALSE;
        pthis->bShow = TRUE;

        LView_IResetScroll ( hWnd, pthis );
        
	if ( ( pthis->uStyles & LVS_TYPEMASK ) == LVS_REPORT &&
             !( pthis->uStyles & LVS_NOCOLUMNHEADER ) )
            SetWindowPos ( pthis->hwHeader, HWND_TOP, 0 - pthis->psLBox.ptOrg.x,0, 
                       rc.right + pthis->psLBox.ptOrg.x, pthis->psLBox.ItmHeight, SWP_SHOWWINDOW );
	else
            SetWindowPos ( pthis->hwHeader, HWND_BOTTOM, 0, 0, 0, 0,
                           SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE );
	}
	if ( pthis )
		GlobalUnlock ( hListView );
EndLViewPaint:
	EndPaint(hWnd, &ps);
}


static void 
LView_OnSetFocus ( HWND hWnd, BOOL bSetFocus ) 
{
	HGLOBAL			hRsrc	    	= ( HGLOBAL )NULL;
	LISTVIEWDATA*		pthis		= ( LISTVIEWDATA* )NULL;
	
    assert ( hRsrc = LView_IGetResource ( hWnd, &pthis ) );
    pthis->psLBox.bFocus = bSetFocus;
    LView_IReleaseResource ( hRsrc, pthis );
    InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
}

static void 
LView_OnTimer 
(
	HWND				hWnd, 
	int			    	idTimer 
)
{
	HGLOBAL			    hRsrc	    = ( HGLOBAL )NULL;
	LISTVIEWDATA*		pthis	    = ( LISTVIEWDATA* )NULL;

    int                 iSel;
    RECT                rcText;
    LRESULT             lStop;
    LV_DISPINFO         lvdi;


    LVITEMPATH          ItmPath;
    

    KillTimer ( hWnd, idTimer );
    assert ( hRsrc = LView_IGetResource ( hWnd, &pthis ) );
    iSel = pthis->psLBox.lCurSel;

    ItmPath.lCol = 0;
    ItmPath.lRow = pthis->psLBox.lCurSel;
    if ( !LView_IOpenSubItm ( pthis, &ItmPath, FALSE ) )
        return;

    lvdi.hdr.code = LVN_BEGINLABELEDIT;
    lvdi.hdr.hwndFrom = hWnd;
    lvdi.hdr.idFrom = GetDlgCtrlID ( hWnd );
    lvdi.item = ItmPath.lpRow->lvItm;
        
    lStop = SendMessage ( GetParent ( hWnd ), WM_NOTIFY, lvdi.hdr.idFrom,
                       ( LPARAM )&lvdi );

    rcText.left = LVIR_LABEL;
    if ( lStop == 0 && 
         LView_OnGetItemRect ( hWnd, pthis, iSel, &rcText ) != -1 )
    {
        LPSTR   lpText;
        if ( ( lpText = lvdi.item.pszText ) == LPSTR_TEXTCALLBACK )
            LView_IGetItmText ( hWnd, &lvdi.item, &lpText );
        SetWindowText ( pthis->hwEdit, lpText );
        SetWindowPos ( pthis->hwEdit, HWND_TOP, rcText.left, rcText.top, 
                       rcText.right - rcText.left + 5, 
                       rcText.bottom - rcText.top, 
                       SWP_SHOWWINDOW );
        SetFocus ( pthis->hwEdit );
#if 0
        SetWindowText ( pthis->hwEdit, lpText );
#endif
        SendMessage ( pthis->hwEdit, EM_SETSEL, 0, -1 );
    }

    
    LView_ICloseSubItm ( pthis, &ItmPath );
    LView_IReleaseResource ( hRsrc, pthis );
    ValidateRect ( hWnd, ( LPRECT )NULL );
    return;
}


/* new */
static HWND 
LView_OnEditLabel 
( 
    HWND            hWnd, 
	LISTVIEWDATA*	pthis,
    int             iItem 
) 
{
    HWND            hwEdit      = ( HWND )NULL;
	
    if ( iItem >= pthis->iCount )
        return hwEdit;
    pthis->psLBox.lCurSel = iItem;
    SetTimer ( hWnd, STARTEDIT_TIMER, 100, NULL );
    hwEdit = pthis->hwEdit;
    return hwEdit;
}

/* new */
/* if fCancel is TRUE, then pass a null string in item.pszText of a LV_DISPINFO 
 * in LVN_ENDLABELEDIT, otherwise pass the text in the edit box. It's up to the 
 * application to call LVM_SETITEM to update text of item. This handler doesn't 
 * update item text automatically on a FALSE value of fCancel.
 */
static BOOL 
LView_OnEndEditLabelNow 
( 
    HWND                hWnd, 
    LISTVIEWDATA*	pthis,
    BOOL                fCancel 
) 
{
    char                szText[MAXSTRING];
    LV_DISPINFO         lvdi;

    memset ( &lvdi, 0, sizeof ( LV_DISPINFO ) );

    lvdi.hdr.code = LVN_ENDLABELEDIT;
    lvdi.hdr.hwndFrom = hWnd;
    lvdi.hdr.idFrom = GetDlgCtrlID ( hWnd );
    lvdi.item.iItem = pthis->psLBox.lCurSel;
    lvdi.item.iSubItem = 0;
    lvdi.item.mask = LVIF_TEXT;
    lvdi.item.pszText = NULL;
    if ( !fCancel )
    {
        GetWindowText ( pthis->hwEdit, szText, MAXSTRING );
        lvdi.item.pszText = szText;
    }
    
    SendMessage ( GetParent ( hWnd ), WM_NOTIFY, lvdi.hdr.idFrom,
                       ( LPARAM )&lvdi );

    SetWindowPos ( pthis->hwEdit, HWND_BOTTOM, 0, 0, 0, 0,
                   SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE );
    SetFocus ( hWnd );
    return TRUE;
}

static BOOL
LView_OnEnsureVisible
(
    HWND            hWnd, 
    LISTVIEWDATA*   pthis,
    int             iItem,
    BOOL            bPartial
)
{
    RECT            rc, rcItem, rcTest;
    int             dx = 0, dy = 0;
    UINT            uStyleMask = pthis->uStyles & LVS_TYPEMASK;

    
    GetClientRect ( hWnd, &rc );
    if ( uStyleMask == LVS_REPORT )
        rc.top += DEF_HEADERHEIGHT;
    rcItem.left = LVIR_BOUNDS;
    LView_OnGetItemRect ( hWnd, pthis, iItem, &rcItem );
    if ( bPartial && IntersectRect ( &rcTest, &rc, &rcItem ) )
        return TRUE;
    if ( EqualRect ( &rcTest, &rcItem ) )
        return TRUE;
    
    if ( rcItem.left < 0 )
        dx = rcItem.left;
    else
    if ( rcItem.right > rc.right )
        dx = rcItem.right - rc.right ;

    if ( rcItem.top < rc.top )
        dy = rcItem.top - rc.top;
    else
    if ( rcItem.bottom > rc.bottom )
        dy = rcItem.bottom - rc.bottom ;
    if ( uStyleMask == LVS_REPORT )
    {
        if ( ( dy % pthis->psLBox.ItmHeight ) > 0 )
            dy += pthis->psLBox.ItmHeight;
        dy /= pthis->psLBox.ItmHeight;
    }
    LView_OnScroll ( hWnd, pthis, dx, dy );

    return TRUE;
}


/* new */
static void 
LView_OnStyleChanged 
(
	HWND			hWnd, 
    	WORD                	wStyleType,
    	LPSTYLESTRUCT       	lpStyle
)
{
	HGLOBAL			hListView	= ( HGLOBAL )NULL;
	LISTVIEWDATA*		pthis		= ( LISTVIEWDATA* )NULL;
	
	int			ErrorCode	= 0;

	if ( ! ( hListView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WLV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( LISTVIEWDATA *) GlobalLock ( hListView ) ) )
		ErrorCode = WLV_ERR_GLOBALLOCK;
	else
	{
        if ( IsWindowVisible ( pthis->hwEdit ) )
            SetWindowPos ( pthis->hwEdit, HWND_BOTTOM, 0, 0, 0, 0, 
                       SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE );
        if ( lpStyle->styleOld != lpStyle->styleNew )
        {
            UINT uStyleMask = lpStyle->styleNew & LVS_TYPEMASK;
            UINT edtStyle = GetWindowLong ( pthis->hwEdit, GWL_STYLE );
/*            pthis->psLBox.iAnchorItm = 0;*/
            pthis->psLBox.Extent.cx = pthis->psLBox.Extent.cy = 0;
            pthis->psLBox.ptOrg.x = pthis->psLBox.ptOrg.y = 0;
            pthis->uStyles = lpStyle->styleNew;
            pthis->bReArrange = TRUE;
            if ( ( uStyleMask == LVS_ICON && !( edtStyle & ES_MULTILINE ) ) ||
                 ( uStyleMask != LVS_ICON && edtStyle & ES_MULTILINE ) )
            {
                DestroyWindow ( pthis->hwEdit );
                if ( uStyleMask == LVS_ICON )
                    edtStyle = ES_CENTER | ES_WANTRETURN | ES_MULTILINE;
                else
                    edtStyle = ES_WANTRETURN;
                edtStyle |= WS_CHILD | WS_BORDER;
                pthis->hwEdit = CreateWindowEx((DWORD)0L,
                         (LPSTR)"EDIT",(LPSTR)NULL,
                         edtStyle,
                         0, 0, 0, 0,
                         hWnd,
                         (HMENU)IDLVEDIT,
                         ( HINSTANCE )GetWindowLong ( hWnd, GWL_HINSTANCE ),
                         NULL );
                SendMessage ( pthis->hwEdit, WM_SETFONT, ( WPARAM )pthis->hFont, ( LPARAM )FALSE );
                LVEdit_Subclass ( pthis->hwEdit );
            }
	    SetWindowPos ( pthis->hwHeader, HWND_BOTTOM, 0, 0, 0, 0,
			   SWP_HIDEWINDOW );
            InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
        }
	}
	if ( pthis )
		GlobalUnlock ( hListView );
}



static LRESULT WINAPI 
LView_WndProc
(
	HWND			hWnd,
        UINT			uMessage,
        WPARAM			wParam,
        LPARAM			lParam
)
{
	switch (uMessage)
	{
		case WM_CREATE:
			if ( ! LView_OnCreate ( hWnd, ( LPCREATESTRUCT )lParam ) )
				return ( LRESULT ) -1L;
			break;
		case WM_DESTROY:
			LView_OnDestroy ( hWnd );
			break;
#if 1
		case WM_ERASEBKGND:
			return ( LRESULT )LView_OnEraseBkground ( hWnd, ( HDC )wParam );
			break;
#endif
		case WM_GETDLGCODE:
			return ( LRESULT )DLGC_WANTALLKEYS;

        	case WM_HSCROLL:
            		LVLBox_OnHScroll ( hWnd, ( int )LOWORD ( wParam ), ( int )HIWORD ( wParam ) );
            		return FALSE;
        
        	case WM_VSCROLL:
            		LVLBox_OnVScroll ( hWnd, ( int )LOWORD ( wParam ), ( int )HIWORD ( wParam ) );
            		return FALSE;
#if 0
        	case WM_MEASUREITEM:
            		LView_OnMeasureItem(hWnd, (MEASUREITEMSTRUCT*)lParam );
            		return TRUE;
#endif	
		case WM_KEYDOWN:
			LView_OnKeyDown ( hWnd, (UINT)( wParam ), ( int )( short )LOWORD ( lParam ), 
				(UINT)HIWORD ( lParam ) );
			break;

        	case WM_KILLFOCUS:
            		LView_OnSetFocus ( hWnd, FALSE );
			break;

		case WM_LBUTTONDOWN:
			LView_OnLButtonDown ( hWnd, (int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )wParam, TRUE );
			break;

        	case WM_LBUTTONUP:
        	case WM_RBUTTONUP:
			LView_OnLButtonUp ( hWnd, (int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )wParam );
			break;

/* to be deleted later */
        	case WM_MBUTTONDOWN:
			LView_OnMButtonDown ( hWnd, (int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )wParam );
			break;

        	case WM_MOUSEMOVE:
			LView_OnMouseMove ( hWnd, uMessage, (int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )wParam );
			break;

		case WM_NOTIFY:
			LView_OnNotify ( hWnd, uMessage, ( int )wParam, ( LPNMHDR )lParam );
			break;
            
        	case WM_PAINT:
			LView_OnPaint( hWnd, ( HDC ) wParam  );
			break;

        	case WM_RBUTTONDOWN:
			LView_OnLButtonDown ( hWnd, (int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )wParam, FALSE );
			break;

        	case WM_SETFOCUS:
            		LView_OnSetFocus ( hWnd, TRUE );
			break;

        	case WM_STYLECHANGED:
            		LView_OnStyleChanged ( hWnd, wParam, ( LPSTYLESTRUCT )lParam );
			break;

        	case WM_TIMER:
		    	LView_OnTimer ( hWnd, ( int )wParam );
            		break;

		default:
		    if ( uMessage >= LVM_FIRST )
			    return LView_LVMWndProc(hWnd, uMessage, wParam, lParam);
		    return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}
    
	    return 0;
}

static HIMAGELIST
LView_OnCreateDragImage
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iItem,
    LPPOINT             lppt
)
{
    SIZE                size, sizeIcon;
    RECT                rc;
    HFONT               hOldFont;
    HBITMAP             hBmp        = ( HBITMAP )NULL;
    HBITMAP             hOldBmp     = ( HBITMAP )NULL;
    HDC                 hDC         = ( HDC )NULL;
    HDC                 hTopDC      = ( HDC )NULL;
    HWND                hwTop       = ( HWND )NULL;
    HIMAGELIST          hImgL       = ( HIMAGELIST )NULL;
    LV_ITEM             lvi;

    LView_OnGetItemPos ( hWnd, pthis, iItem, lppt );
    memset ( &lvi, 0, sizeof ( LV_ITEM ) );
    lvi.mask = LVIF_TEXT | LVIF_IMAGE;
    lvi.iItem = iItem;
    LView_OnGetItem ( hWnd, pthis, &lvi );
    size.cx = lvi.cchTextMax;
    ImageList_GetIconSize ( pthis->hImgList, &sizeIcon.cx, &sizeIcon.cy );
    size.cx += sizeIcon.cx;
    if ( size.cy < sizeIcon.cy )
        size.cy = sizeIcon.cy;
    hTopDC = GetDC ( ( hwTop = GetDesktopWindow ( ) ) );
    if ( ( hDC = CreateCompatibleDC ( hTopDC ) ) &&
         ( hBmp = CreateCompatibleBitmap ( hTopDC, size.cx, size.cy ) ) &&
         ( hImgL = ( HIMAGELIST )ImageList_Create ( size.cx, size.cy, ILC_COLOR, 10, 10 ) )
       )
    {
        int iImage = lvi.iImage;
        hOldBmp = SelectObject ( hDC, hBmp );
        hOldFont = SelectObject ( hDC, pthis->hFont );

        ImageList_Draw ( pthis->hImgList, iImage, hDC, 0, 0, ILD_NORMAL );
        SetRect ( &rc, sizeIcon.cx, 0, size.cx, size.cy );
        SetTextColor ( hDC, pthis->TxtColor );
        SetBkColor ( hDC, pthis->TxtBkColor );
        DrawText ( hDC, lvi.pszText, lvi.cchTextMax, &rc, DT_LEFT );
        SelectObject ( hDC, hOldFont );
        SelectObject ( hDC, hOldBmp );
        ImageList_Add ( hImgL, hBmp, ( HBITMAP )NULL );
    }

    if ( hDC )
        DeleteDC ( hDC );
    if ( hBmp )
        DeleteObject ( hBmp );
    ReleaseDC ( hwTop, hTopDC );
    return hImgL;
}

#if 0
static BOOL
LView_OnDeleteAllItems
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis
)
{
    NM_LISTVIEW         nmlv;
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;
    int                 i           = 0;

    WLDestroy ( &pthis->Rows.RowItms );
    WLDestroy ( &pthis->Rows.SortedRows );
    WLCreate ( &pthis->Rows.RowItms, sizeof ( LVIEWROW ) );
    WLCreate ( &pthis->Rows.SortedRows, sizeof ( LVSORTITEM ) );

    pthis->Rows.iDeleted = -1;
    pthis->iCount = pthis->Rows.lCount = 0;

    memset ( &nmlv, 0, sizeof ( NM_LISTVIEW ) );
    nmlv.hdr.code = LVN_DELETEALLITEMS;
    nmlv.hdr.hwndFrom = hWnd;
    nmlv.hdr.idFrom = GetDlgCtrlID ( hWnd );
    nmlv.iItem = -1;
    SendMessage ( GetParent ( hWnd ), WM_NOTIFY, nmlv.hdr.idFrom, ( LPARAM )&nmlv );

    InvalidateRect ( hWnd, (LPRECT)NULL, TRUE );
    return TRUE;
}
#else
static BOOL
LView_OnDeleteAllItems
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis
)
{
    NM_LISTVIEW         nmlv;
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    int                 i           = 0;
    UINT                Count       = 0;

    if ( WLCount ( &pthis->Rows.RowItms, &Count ) )
	return FALSE;
    if ( Count <= 0 )
	return TRUE;
    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) )
       return FALSE;
    pItm = pItms + i;
    pItm->Below = -1;
    for ( i = 1; i < Count; i++ )
    {
        pItm->Below = i;
        pItm = pItms + i;
        pItm->Below = -1;
    }

    WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )NULL );
    WLDestroy ( &pthis->Rows.SortedRows );
    WLCreate ( &pthis->Rows.SortedRows, sizeof ( LVSORTITEM ) );

    pthis->Rows.iDeleted = 0;
    pthis->iCount = pthis->Rows.lCount = 0;

    memset ( &nmlv, 0, sizeof ( NM_LISTVIEW ) );
    nmlv.hdr.code = LVN_DELETEALLITEMS;
    nmlv.hdr.hwndFrom = hWnd;
    nmlv.hdr.idFrom = GetDlgCtrlID ( hWnd );
    nmlv.iItem = -1;
    SendMessage ( GetParent ( hWnd ), WM_NOTIFY, nmlv.hdr.idFrom, ( LPARAM )&nmlv );

    InvalidateRect ( hWnd, (LPRECT)NULL, TRUE );
    return TRUE;
}
#endif

/* new */
static BOOL
LView_OnDeleteCol
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iCol
)
{
    PLVIEWCOL           pCols       = ( PLVIEWCOL )NULL;
    PLVIEWCOL           pCol        = ( PLVIEWCOL )NULL;

    if ( iCol >= pthis->Cols.lCount )
        return FALSE;
    if ( WLLock ( &pthis->Cols.Columns, ( LPVOID )&pCols ) )
       return FALSE;
    pCol = pCols + iCol;
    pCol->bDeleted = TRUE;
    --( pthis->Cols.lCount );
    WLUnlock ( &pthis->Cols.Columns, ( LPVOID )NULL );
    return TRUE;
}

/* new */
static BOOL
LView_OnDeleteItem
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iItem
)
{
    NM_LISTVIEW         nmlv;
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;

    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) ||
         WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs )
       )
       return FALSE;
    if ( iItem >= pthis->iCount )
       return FALSE;
    pIndx = pIndxs + iItem;
    pItm = pItms + ( int )pIndx->hItm;
    pItm->Below = -1;
    if ( pthis->Rows.iDeleted != -1 )
        pItm->Below = pthis->Rows.iDeleted;

    WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )NULL );
    WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )NULL );

    pthis->Rows.iDeleted = iItem;
    pthis->iCount = --( pthis->Rows.lCount );
    WLDelete ( &pthis->Rows.SortedRows, iItem );
    memset ( &nmlv, 0, sizeof ( NM_LISTVIEW ) );
    nmlv.hdr.code = LVN_DELETEITEM;
    nmlv.hdr.hwndFrom = hWnd;
    nmlv.hdr.idFrom = GetDlgCtrlID ( hWnd );
    nmlv.iItem = iItem;
    SendMessage ( GetParent ( hWnd ), WM_NOTIFY, nmlv.hdr.idFrom, ( LPARAM )&nmlv );
    return TRUE;
}

static int
LView_OnFindItem
(
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iStart,
    LV_FINDINFO*        lpFind
)
{
    int                 iItem;
    BOOL                bFound      = FALSE;
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;

    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) ||
         WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs ) 
       )
       return -1;
    if ( iStart >= pthis->iCount )
        iStart = 0;
    iItem = iStart;
    do
    {
        pIndx = pIndxs + iItem;
        pItm = pItms + ( int )pIndx->hItm;
        if ( lpFind->flags & LVFI_PARAM && 
             pItm->lvItm.lParam == lpFind->lParam )
            bFound = TRUE;
        else
        if ( lpFind->flags & LVFI_NEARESTXY &&
             LView_IGetNearestItmFromPt ( hWnd, pthis, &lpFind->pt ) != -1 )
            bFound = TRUE;
        else
        {
            LV_ITEM   lvi;
            lvi.mask = LVIF_TEXT;
            if ( LView_OnGetItemText ( hWnd, pthis, iItem, &lvi ) )
            {
                if ( lpFind->flags & LVFI_PARTIAL )
                {
                    LPSTR   lpText;
                    lpText = strstr ( lpFind->psz, lvi.pszText );
                    if ( lpText != ( LPSTR )NULL &&
                         lpText == lvi.pszText )
                         bFound = TRUE;
                }
                else
                if ( lstrcmp ( lpFind->psz, lvi.pszText ) == 0 )
                    bFound = TRUE;
            }
        }

        if ( ++iItem == pthis->iCount && lpFind->flags & LVFI_WRAP )
            iItem = 0;

    } while ( !bFound && iItem < pthis->iCount && iItem != iStart );

    WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )NULL );
    WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )NULL ); 

    return bFound ? iItem : -1;
}


/* new */
static BOOL
LView_OnGetCol
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iCol,
    LV_COLUMN*          lpLCol
)
{
    HD_ITEM             hdi;

    hdi.mask = 0;
    hdi.fmt = 0;        
    if ( lpLCol->mask & LVCF_WIDTH )
        hdi.mask |= HDI_WIDTH;    
    if ( lpLCol->mask & LVCF_TEXT )
    {
        hdi.mask |= HDI_TEXT;    
        hdi.pszText = lpLCol->pszText; 
        hdi.cchTextMax = lstrlen(lpLCol->pszText); 
        hdi.fmt |= HDF_STRING;
    }
    if ( lpLCol->mask & LVCF_FMT )
        hdi.mask |= HDI_FORMAT;    
 
    if ( !SendMessage( pthis->hwHeader, HDM_GETITEM, 
                           (WPARAM) iCol, (LPARAM) &hdi) )
        return FALSE;                 

    if ( lpLCol->mask & LVCF_WIDTH )
        lpLCol->cx = hdi.cxy; 
    if ( lpLCol->mask & LVCF_FMT )
    {
        lpLCol->fmt = hdi.fmt & HDF_CENTER ? LVCFMT_CENTER : 
                      hdi.fmt & HDF_LEFT ? LVCFMT_LEFT : LVCFMT_RIGHT; 
    }
    
    return TRUE;
}

/* new */
static int
LView_OnGetColWidth
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iCol
)
{
    LV_COLUMN           lvc;

    lvc.mask = LVCF_WIDTH;
    if ( LView_OnGetCol ( hWnd, pthis, iCol, &lvc ) )
        return lvc.cx;
    return 0;
}

static int
LView_OnGetCountPerPage
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis
)
{
    RECT                rc;
    int                 iCount = pthis->iCount;
    UINT                uStyleMask = pthis->uStyles & LVS_TYPEMASK;
    
    GetClientRect ( hWnd, &rc );
    if ( uStyleMask == LVS_REPORT || uStyleMask == LVS_LIST )
    {
        if ( uStyleMask == LVS_REPORT && !( pthis->uStyles & LVS_NOCOLUMNHEADER ) )
        {        
            RECT    rcHd;
            GetClientRect ( pthis->hwHeader, &rcHd );
            rc.top += rcHd.bottom;
        }
        iCount =  ( rc.bottom - rc.top ) / pthis->psLBox.ItmHeight;
        if ( uStyleMask == LVS_LIST )
            iCount *= ( rc.right / pthis->psLBox.ItmWidth );
    }

    return iCount;
}

static BOOL
LView_OnGetItem
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    LV_ITEM*            lplvi
)
{
    LVITEMPATH          ItmPath;
    int                 iLen;
    LPSTR               lpText  	= ( LPSTR )NULL;
    PLVIEWROW           pItms       	= ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      	= ( PLVSORTITEM )NULL;
    PLVIEWROW           pItm        	= ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       	= ( PLVSORTITEM )NULL;

    
    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) ||
         WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs ) )
        return FALSE;

    pIndx = pIndxs + lplvi->iItem;
    pItm = pItms + ( int )pIndx->hItm;
    lplvi->lParam = pItm->lvItm.lParam;
    
    ItmPath.lCol = lplvi->iSubItem;
    ItmPath.lRow = lplvi->iItem;
    if ( !LView_IOpenSubItm ( pthis, &ItmPath, FALSE ) )
        return 0;
    if ( lplvi->mask & LVIF_TEXT )
    {
        if ( ( lpText = ItmPath.lpSubItm->pszText ) == LPSTR_TEXTCALLBACK )
        {
            LV_ITEM         lvi = *lplvi;
            LView_IGetItmText ( hWnd, &lvi, &lpText );
        }

        if ( ( iLen = lstrlen ( lpText ) ) > lplvi->cchTextMax )
            iLen = lplvi->cchTextMax - 1;
        lstrcpyn ( lplvi->pszText, lpText, iLen );
        lplvi->pszText[iLen] = 0;
    }

    if ( lplvi->mask & LVIF_IMAGE )
    {
        lplvi->iImage = ItmPath.lpRow->lvItm.iImage;
        if ( lplvi->iImage == I_IMAGECALLBACK )
        {
            LV_DISPINFO lvdi;
            int idCtrl =  GetDlgCtrlID ( hWnd );
            lvdi.hdr.code = LVN_GETDISPINFO;
            lvdi.hdr.hwndFrom = hWnd;
            lvdi.hdr.idFrom = idCtrl;
            lvdi.item = ItmPath.lpRow->lvItm;
            lvdi.item.mask = LVIF_IMAGE;
            SendMessage ( GetParent ( hWnd ), WM_NOTIFY, idCtrl, ( LPARAM )&lvdi );
            lplvi->iImage = lvdi.item.iImage;
        }
    }
    if ( lplvi->mask & LVIF_PARAM )
        lplvi->lParam = ItmPath.lpRow->lvItm.lParam;
    if ( lplvi->mask & LVIF_STATE )
        lplvi->state = lplvi->stateMask & ItmPath.lpRow->lvItm.state;
    LView_ICloseSubItm ( pthis, &ItmPath );
    return TRUE;
}

static BOOL
LView_OnGetItemPos
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iItem,
    LPPOINT             lppt
)
{
    RECT                rc;

    rc.left = LVIR_BOUNDS;
    if ( LView_OnGetItemRect ( hWnd, pthis, iItem, &rc ) )
    {
        lppt->x = rc.left;
        lppt->y = rc.top;
        return TRUE;
    }

    return FALSE;
}
/* in progress */
static BOOL
LView_OnGetItemRect
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iItem,
    LPRECT              lpRect
)
{
    int                 iCode;
    UINT                uStyleMask;
    RECT                rcIcon, rcLabel;
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;

    
    if ( !lpRect || iItem >= pthis->iCount || iItem < 0 )
        return FALSE;
    iCode = lpRect->left;
    uStyleMask = pthis->uStyles & LVS_TYPEMASK;

    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) ||
         WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs ) )
        return FALSE;

    pIndx = pIndxs + iItem;
    pItm = pItms + ( int )pIndx->hItm;
    if ( uStyleMask == LVS_ICON || uStyleMask == LVS_SMALLICON )
    {
        CopyRect ( lpRect, &pItm->rcItem );
    }
    else
    {
        RECT    rc;
        int     iItmsPerCol, iCols;
        GetClientRect ( hWnd, &rc );
        CopyRect ( lpRect, &rc );
        if ( uStyleMask == LVS_REPORT )
            rc.top += DEF_HEADERHEIGHT;
        iItmsPerCol = ( rc.bottom - rc.top ) / pthis->psLBox.ItmHeight;
        if ( uStyleMask == LVS_LIST )
        {
            iCols = iItem / iItmsPerCol;
            iItem %= iItmsPerCol;
            lpRect->left = rc.left + iCols * pthis->psLBox.ItmWidth;
            lpRect->right = lpRect->left + pthis->psLBox.ItmWidth;
        }

        lpRect->top = rc.top + iItem * pthis->psLBox.ItmHeight;;
        lpRect->bottom = lpRect->top + pthis->psLBox.ItmHeight;
    }

    if ( iCode == LVIR_BOUNDS )
        goto GetItemRectReturn;

    if ( uStyleMask == LVS_ICON )
    {
        SetRect ( &rcIcon, lpRect->left, lpRect->top, 
                  lpRect->right, lpRect->top + pthis->BmpSize.cy );
    }
    else
    {
        SetRect ( &rcIcon, lpRect->left, lpRect->top, 
                  lpRect->left + pthis->BmpSize.cx, lpRect->bottom );
    }

    if ( iCode == LVIR_ICON )
    {
        CopyRect ( lpRect, &rcIcon );
        goto GetItemRectReturn;
    }
    
    if ( uStyleMask == LVS_ICON )
    {
        SetRect ( &rcLabel, lpRect->left, rcIcon.bottom, 
                  lpRect->right, lpRect->bottom );
    }
    else
    {
        LPSTR   lpText;
        int     iLen;
        if ( ( lpText = pItm->lvItm.pszText ) == LPSTR_TEXTCALLBACK )
            LView_IGetItmText ( hWnd, &pItm->lvItm, &lpText );
        iLen = ListView_GetStringWidth ( hWnd, lpText );
/* need revisit */
#if 0
        if ( uStyleMask == LVS_REPORT &&
             iLen + rcIcon.right - rcIcon.left > pthis->Rows.ColInfo.cx )
            iLen = pthis->Rows.ColInfo.cx - rcIcon.right + rcIcon.left;
#endif
        SetRect ( &rcLabel, rcIcon.right, lpRect->top, 
                  rcIcon.right + DEF_SBITEXT_INDENT + iLen, lpRect->bottom );
    }

    if ( iCode == LVIR_LABEL )
    {
        CopyRect ( lpRect, &rcLabel );
        goto GetItemRectReturn;
    }

    if ( iCode == LVIR_SELECTBOUNDS )
    {
        UnionRect ( lpRect, &rcIcon, &rcLabel );
        goto GetItemRectReturn;
    }

GetItemRectReturn:    
    OffsetRect ( lpRect, 0-pthis->psLBox.ptOrg.x, 0-pthis->psLBox.ptOrg.y );
    
    return TRUE;

}

static int
LView_OnGetItemState
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iItem,
    UINT                statemask
)
{
    LV_ITEM             lvi;

    lvi.iItem = iItem;
    lvi.iSubItem = 0;
    lvi.mask = LVIF_STATE;
    lvi.stateMask = statemask;
    if ( LView_OnGetItem ( hWnd, pthis, &lvi ) )
        return lvi.state;
    return 0;
}

static int
LView_OnGetItemText
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iItem,
    LV_ITEM*            lplvi
)
{
    LVITEMPATH          ItmPath;
    LPSTR               lpText  = ( LPSTR )NULL;
    int                 iLen;
    ItmPath.lCol = lplvi->iSubItem;
    ItmPath.lRow = iItem;
    if ( !LView_IOpenSubItm ( pthis, &ItmPath, FALSE ) )
        return 0;
    if ( ( lpText = ItmPath.lpSubItm->pszText ) == LPSTR_TEXTCALLBACK )
    {
        LV_ITEM         lvi = *lplvi;
        LView_IGetItmText ( hWnd, &lvi, &lpText );
    }

    if ( ( iLen = lstrlen ( lpText ) ) > lplvi->cchTextMax )
        iLen = lplvi->cchTextMax - 1;
    lstrcpyn ( lplvi->pszText, lpText, iLen );
    lplvi->pszText[iLen] = 0;
    LView_ICloseSubItm ( pthis, &ItmPath );
    return TRUE;
}

/* revised */
static int
LView_OnInsertCol 
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iCol,
    LV_COLUMN*          lpLCol 
)
{
    int                 iResult = 0;
    
    if ( ( pthis->uStyles & LVS_TYPEMASK ) == LVS_REPORT )
    {
        HD_ITEM hdi;
        hdi.mask = 0;
        hdi.fmt = 0;        
        if ( lpLCol->mask & LVCF_WIDTH )
        {
            hdi.mask |= HDI_WIDTH;    
            hdi.cxy = lpLCol->cx; 
        }
        if ( lpLCol->mask & LVCF_TEXT )
        {
            hdi.mask |= HDI_TEXT;    
            hdi.pszText = lpLCol->pszText; 
            hdi.cchTextMax = lstrlen(lpLCol->pszText); 
            hdi.fmt |= HDF_STRING;
        }
        hdi.fmt = HDF_LEFT; 
        if ( lpLCol->mask & LVCF_FMT )
        {
            hdi.fmt = lpLCol->fmt; 
        }
 
        iResult = SendMessage( pthis->hwHeader, HDM_INSERTITEM, 
                               (WPARAM) iCol, (LPARAM) &hdi); 

    }
    
    if ( iResult != -1 )
    {
        if ( lpLCol->mask & LVCF_WIDTH )
        {
            /*pthis->psLBox.Extent.cx += lpLCol->cx; */
            if ( iCol == 0 )
            {
                pthis->Rows.ColWidth = lpLCol->cx;
                return 0;
            }
        }
        iResult = LVCols_InsertCol ( &pthis->Cols, iCol, lpLCol ); 
    }

    return iResult;
}

/* new */
static int
LView_OnInsertItem 
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    LV_ITEM*            lplvi 
)
{
    int                 iItm;
    NM_LISTVIEW         nml;
    int			iItmIndx;

    if ( ( pthis->uStyles & LVS_SORTASCENDING ||
           pthis->uStyles & LVS_SORTASCENDING    ) &&
         ( lplvi->mask & LVIF_TEXT && lplvi->pszText == LPSTR_TEXTCALLBACK )
       )
       return -1;
    if ( ( iItm = LView_IAllocItmHandle ( pthis ) ) < 0 )
        return -1;
    nml.hdr.code = LVN_INSERTITEM;
    nml.hdr.hwndFrom = hWnd;
    nml.hdr.idFrom = GetDlgCtrlID ( hWnd );
    nml.iItem = lplvi->iItem;
    SendMessage ( GetParent ( hWnd ), WM_NOTIFY, nml.hdr.idFrom, ( LPARAM )&nml );
    if ( ( iItmIndx = LVRows_SetRowItm ( &pthis->Rows, iItm, lplvi, 
					 pthis->uStyles ) ) != -1 )
	InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
    return iItmIndx;
}

static BOOL
LView_OnRedrawItms
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iFirst,
    int                 iLast
)
{
    int                 i;
    int                 tmp;
    RECT                rcItem;
    
    if ( iFirst > iLast )
    {
        tmp = iFirst;
        iFirst = iLast;
        iLast = tmp;
    }
    
    if ( iFirst >= pthis->iCount )
        return FALSE;

    if ( iLast >= pthis->iCount )
        iLast = pthis->iCount - 1;

    for ( i = iFirst; i <= iLast && i < pthis->iCount; i++ )
    {
        rcItem.left = LVIR_BOUNDS;
        if ( !LView_OnGetItemRect ( hWnd, pthis, i, &rcItem ) )
            return FALSE;
        InvalidateRect ( hWnd, &rcItem, TRUE );
    }

    return TRUE;
}

BOOL
LView_OnScroll
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 dx,
    int                 dy
)
{
    UINT                styleMask = pthis->uStyles & LVS_TYPEMASK;

    if ( styleMask == LVS_REPORT )
        dy *= pthis->psLBox.ItmHeight;
    if ( styleMask == LVS_LIST )
        dx *= pthis->psLBox.ItmWidth;

    pthis->psLBox.ptOrg.x += dx;
    pthis->psLBox.ptOrg.y += dy;

    if ( pthis->psLBox.ptOrg.x < 0 )
        pthis->psLBox.ptOrg.x = 0;
    if ( pthis->psLBox.ptOrg.y < 0 )
        pthis->psLBox.ptOrg.y = 0;

    SetScrollPos ( hWnd, SB_HORZ, pthis->psLBox.ptOrg.x, TRUE );
    SetScrollPos ( hWnd, SB_VERT, pthis->psLBox.ptOrg.y, TRUE );
    InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );

    if ( styleMask == LVS_REPORT && pthis->hwHeader )
    {
        RECT    rc;
        GetClientRect ( pthis->hwHeader, &rc );
        ValidateRect ( hWnd, &rc );
    }
    return TRUE;
}

static BOOL
LView_OnSetCol
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iCol,
    LV_COLUMN*          lpLCol
)
{
    HD_ITEM             hdi;

    hdi.mask = 0;
    hdi.fmt = 0;        
    if ( lpLCol->mask & LVCF_WIDTH )
    {
        hdi.mask |= HDI_WIDTH;    
        hdi.cxy = lpLCol->cx; 
    }
    if ( lpLCol->mask & LVCF_TEXT )
    {
        hdi.mask |= HDI_TEXT;    
        hdi.pszText = lpLCol->pszText; 
        hdi.cchTextMax = lstrlen(lpLCol->pszText); 
        hdi.fmt |= HDF_STRING;
    }
    hdi.fmt = HDF_LEFT; 
    if ( lpLCol->mask & LVCF_FMT )
    {
        hdi.fmt = lpLCol->fmt; 
    }
 
    if ( !SendMessage( pthis->hwHeader, HDM_SETITEM, (WPARAM) iCol, (LPARAM) &hdi) )
        return FALSE;

    if ( iCol == 0 && lpLCol->mask & LVCF_WIDTH )
        pthis->Rows.ColWidth = lpLCol->cx;

    return TRUE;

}

static BOOL
LView_OnSetColWidth
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iCol,
    int                 iWidth
)
{
    LV_COLUMN           LCol;

    LCol.mask = LVCF_WIDTH;
    LCol.cx = iWidth; 
    LView_OnSetCol ( hWnd, pthis, iCol, &LCol );

    return TRUE;
}

/* new */
static BOOL
LView_OnSetItemText
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iItem,
    LV_ITEM*            lplvi 
)
{
    LVITEMPATH          ItmPath;

    ItmPath.lCol = lplvi->iSubItem;
    ItmPath.lRow = iItem;
    if ( !LView_IOpenSubItm ( pthis, &ItmPath, TRUE ) ||
         !LView_IUpdateSubItm ( &ItmPath, lplvi ) )
        return FALSE;
    LView_ICloseSubItm ( pthis, &ItmPath );
    return TRUE;
}

/* need revisit to process item state*/
static int
LView_OnGetNextItem
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iStart,
    UINT                flags
)
{
    PLVIEWROW           pItms       = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndxs      = ( PLVSORTITEM )NULL;
    PLVIEWROW           pItm        = ( PLVIEWROW )NULL;
    PLVSORTITEM         pIndx       = ( PLVSORTITEM )NULL;
    UINT                uStyleMask  = 0;
    int                 iNext       = -1;

    if ( WLLock ( &pthis->Rows.RowItms, ( LPVOID )&pItms ) ||
         WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs )
       )
       return -1;
    if ( iStart >= pthis->iCount )
       return -1;

    pIndx = pIndxs + iStart;
    pItm = pItms + ( int )pIndx->hItm;

    uStyleMask = pthis->uStyles & LVS_TYPEMASK;
    if ( flags & LVNI_ABOVE )
    {
        if ( uStyleMask == LVS_ICON || uStyleMask == LVS_SMALLICON )
            iNext = pItm->Above;
        else
            iNext = iStart - 1;
    }
    else
    if ( flags & LVNI_BELOW )
    {
        if ( uStyleMask == LVS_ICON || uStyleMask == LVS_SMALLICON )
            iNext = pItm->Below;
        else
        if ( iStart < pthis->iCount - 1 )
            iNext = iStart + 1;
    }
    else
    if ( flags & LVNI_TOLEFT )
    {
        if ( uStyleMask == LVS_ICON || uStyleMask == LVS_SMALLICON )
            iNext = pItm->Left;
    }
    else
    if ( flags & LVNI_TORIGHT )
    {
        if ( uStyleMask == LVS_ICON || uStyleMask == LVS_SMALLICON )
            iNext = pItm->Right;
    }
    else
    if ( flags & LVNI_SELECTED )
        iNext = pthis->psLBox.lCurSel;

    WLUnlock ( &pthis->Rows.RowItms, ( LPVOID )&pItms );
    WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )&pIndxs );
    return iNext;
}

static int
LView_OnGetStrWidth
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    LPSTR               lpText
)
{
    SIZE                Size;
    HDC                 hDC     = GetDC ( hWnd );

    if ( hDC )
    {
        HFONT hOldFont = SelectObject ( hDC, pthis->hFont );
        GetTextExtentPoint32 ( hDC, lpText, lstrlen ( lpText ), &Size );
        SelectObject ( hDC, hOldFont );
        ReleaseDC ( hWnd, hDC );
        return Size.cx;
    }

    return 0;
}

static int
LView_OnHitTest
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    LV_HITTESTINFO*     lpht
)
{
    int                 nIndx;
    int                 nHitItem;
    UINT                uStyleMask;
    RECT                rc;
    RECT                rcItm;
    
    GetClientRect ( hWnd, &rc );
    lpht->flags = 0;
    lpht->iItem = -1;
    if ( lpht->pt.y < rc.top )
        lpht->flags |= LVHT_ABOVE;
    else
    if ( lpht->pt.y > rc.bottom )
        lpht->flags |= LVHT_BELOW;
    if ( lpht->pt.x < rc.left )
        lpht->flags |= LVHT_TOLEFT;
    else
    if ( lpht->pt.x > rc.right )
        lpht->flags |= LVHT_TORIGHT;
    if ( lpht->flags != 0 )
        return -1;

    uStyleMask = pthis->uStyles & LVS_TYPEMASK;
    nIndx = pthis->psLBox.lTopIndx;

    if ( uStyleMask == LVS_REPORT )
        rc.top += DEF_HEADERHEIGHT;
    nHitItem = ( lpht->pt.y - rc.top ) / pthis->psLBox.ItmHeight;

    if ( nHitItem >= pthis->iCount )
        return -1;
    if ( uStyleMask == LVS_REPORT )
    {
        nHitItem = nHitItem + nIndx;  /* account for the header height */
    }
    else
    if ( uStyleMask == LVS_LIST )
    {
        int iItmsPerCol = rc.bottom / pthis->psLBox.ItmHeight;
        nHitItem += nIndx + iItmsPerCol * ( ( lpht->pt.x - rc.left ) / pthis->psLBox.ItmWidth );
    }
    else
    if ( pthis->uStyles & LVS_AUTOARRANGE )
    {
        int iItmsPerRow = rc.right / pthis->psLBox.ItmWidth;
        nHitItem = nHitItem * iItmsPerRow + nIndx +
                   ( lpht->pt.x - rc.left ) / pthis->psLBox.ItmWidth;
    }
    else
    if ( ( nHitItem = LView_IGetItmFromPt ( hWnd, pthis, &lpht->pt ) ) == -1 )
    {
        lpht->flags = LVHT_NOWHERE;
        return -1;
    }

    rcItm.left = LVIR_SELECTBOUNDS;
    LView_OnGetItemRect ( hWnd, pthis, nHitItem, &rcItm );
    if ( !PtInRect ( &rcItm, lpht->pt ) )
    {
        lpht->flags = LVHT_NOWHERE;
        return -1;
    }

    if ( lpht->pt.x > rcItm.left + pthis->BmpSize.cx )
        lpht->flags = LVHT_ONITEMICON;
    else
        lpht->flags = LVHT_ONITEMLABEL;

    if ( nHitItem < pthis->iCount )
    {
        lpht->iItem = nHitItem;
        return lpht->iItem;
    }

    lpht->flags = LVHT_NOWHERE;
    return -1;

}

static HIMAGELIST
LView_OnSetImageList
(
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iImageType,
    HIMAGELIST          hIml
)
{
    HIMAGELIST          hImgList    = ( HIMAGELIST )NULL;

    if ( iImageType == LVSIL_NORMAL )
    {
        hImgList = pthis->hImgList;
        pthis->hImgList = hIml;
    }
    else
    {
        hImgList = pthis->hSImgList;
        pthis->hSImgList = hIml;
    }

    ImageList_GetIconSize ( hIml, &pthis->BmpSize.cx, &pthis->BmpSize.cy );

    InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
    return hImgList;
}

/* in progress */
static BOOL
LView_OnSetItem
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    LV_ITEM*            lplvi
)
{
    LVITEMPATH          ItmPath;

    ItmPath.lCol = lplvi->iSubItem;
    ItmPath.lRow = lplvi->iItem;
    if ( !LView_IOpenSubItm ( pthis, &ItmPath, TRUE ) )
        return FALSE;

    if ( lplvi->mask & LVIF_TEXT && lplvi->pszText != LPSTR_TEXTCALLBACK )
    {
        LPSTR*   ppText = lplvi->iSubItem == 0 ? &ItmPath.lpRow->lvItm.pszText :
                                                 &ItmPath.lpSubItm->pszText;
        if ( *ppText && *ppText != LPSTR_TEXTCALLBACK )
            *ppText = WinRealloc ( ( void * )( *ppText ), lstrlen ( lplvi->pszText ) + 1 );
	else
            *ppText = WinMalloc ( lstrlen ( lplvi->pszText ) + 1 );
        if ( *ppText == 0 )
            return FALSE;
        lstrcpy ( *ppText, lplvi->pszText );
    }

    if ( lplvi->mask & LVIF_IMAGE )
        ItmPath.lpRow->lvItm.iImage = lplvi->iImage;
    if ( lplvi->mask & LVIF_PARAM )
        ItmPath.lpRow->lvItm.lParam = lplvi->lParam;
    if ( lplvi->mask & LVIF_STATE )
    {
        ItmPath.lpRow->lvItm.state |= lplvi->stateMask;
        ItmPath.lpRow->lvItm.state &= ( ~lplvi->stateMask | lplvi->state ); /* ? */
    }
    
    LView_ICloseSubItm ( pthis, &ItmPath );
    return TRUE;
}

static void
LView_OnSetItemCount
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iCount
)
{
    WLSetSize ( &pthis->Rows.RowItms, iCount );
}


static BOOL
LView_OnSetItemState
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iItem,
    LV_ITEM*            lplvi
)
{
    lplvi->iItem = iItem;
    lplvi->iSubItem = 0;
    return LView_OnSetItem ( hWnd, pthis, lplvi );
}

static BOOL
LView_OnSetItemPos
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    int                 iItem,
    int                 x,
    int                 y
)
{
    LVITEMPATH          ItmPath;
    UINT                uStyleMask  = 0;
    
    if ( ( uStyleMask = pthis->uStyles & LVS_TYPEMASK ) != LVS_ICON &&
         uStyleMask != LVS_SMALLICON )
         return FALSE;
    ItmPath.lRow = iItem;
    ItmPath.lCol = 0;
    if ( !LView_IOpenSubItm ( pthis, &ItmPath, FALSE ) )
        return FALSE;
    LView_IReposition ( hWnd, pthis, iItem, x, y );
    SetRect ( &ItmPath.lpRow->rcItem, x, y, 
              x + ItmPath.lpRow->rcItem.right - ItmPath.lpRow->rcItem.left,
              y + ItmPath.lpRow->rcItem.bottom - ItmPath.lpRow->rcItem.top );

    LView_ICloseSubItm ( pthis, &ItmPath );

    InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
    return TRUE;
}

/* revised */
static BOOL
LView_OnSortItems
( 
    HWND                hWnd, 
    LISTVIEWDATA*       pthis,
    LPARAM              lParamSort,
    PFNLVCOMPARE        pfnCompare
)
{
    PLVSORTITEM         pSortAr     = ( PLVSORTITEM )NULL;
/*	LVITEMDATA*         pItem       = ( LVITEMDATA* )NULL;*/

    if ( WLLock ( &pthis->Rows.SortedRows, ( LPVOID )&pSortAr ) )
        return FALSE;
    LView_IDoHeapSort ( pSortAr, pthis->iCount-1, pfnCompare, lParamSort );
    if ( pSortAr )
        WLUnlock ( &pthis->Rows.SortedRows, ( LPVOID )&pSortAr );
    InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
    return TRUE;
}


static LRESULT  
LView_LVMWndProc
(
	HWND			hWnd,
    UINT			uMessage,
    WPARAM			wParam,
    LPARAM			lParam
)
{
	HGLOBAL			hListView	= ( HGLOBAL )NULL;
	LISTVIEWDATA*	pthis		= ( LISTVIEWDATA* )NULL;
	
	int			ErrorCode	= 0;

	if ( ! ( hListView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WLV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( LISTVIEWDATA *) GlobalLock ( hListView ) ) )
		ErrorCode = WLV_ERR_GLOBALLOCK;
	else
	{
		switch (uMessage)
		{
            case LVM_ARRANGE:
                {
                    if ( ( pthis->uStyles & LVS_TYPEMASK ) != LVS_ICON )
                        return FALSE;
                    pthis->bReArrange = TRUE;
                    pthis->psLBox.Extent.cx = pthis->psLBox.Extent.cy = 0;
                    InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
                    return TRUE;
                }
			case LVM_CREATEDRAGIMAGE:
                return ( LRESULT )LView_OnCreateDragImage ( hWnd, pthis, ( int )wParam, ( LPPOINT )lParam );
            case LVM_DELETEALLITEMS:
                return ( LRESULT )LView_OnDeleteAllItems ( hWnd, pthis );
            case LVM_DELETECOLUMN:
                return ( LRESULT )LView_OnDeleteCol ( hWnd, pthis, ( int )wParam );
            case LVM_DELETEITEM:
                return ( LRESULT )LView_OnDeleteItem ( hWnd, pthis, ( int )wParam );
            case LVM_EDITLABEL:
                return ( LRESULT )LView_OnEditLabel ( hWnd, pthis, ( int )wParam ); 
            case LVM_ENSUREVISIBLE:
                return ( LRESULT )LView_OnEnsureVisible ( hWnd, pthis, ( int )wParam, ( BOOL )lParam );
            case LVM_FINDITEM:
                return ( LRESULT )LView_OnFindItem ( hWnd, pthis, ( int )wParam, ( LV_FINDINFO* )lParam );
            case LVM_GETBKCOLOR:
                return ( LRESULT )pthis->BkColor;
            case LVM_GETCALLBACKMASK:
                return ( LRESULT )pthis->CallBMask;
            case LVM_GETCOLUMN:
                return ( LRESULT )LView_OnGetCol ( hWnd, pthis, ( UINT )wParam, ( LV_COLUMN* )lParam );
            case LVM_GETCOLUMNWIDTH:
                return ( LRESULT )LView_OnGetColWidth ( hWnd, pthis, ( UINT )wParam );
            case LVM_GETCOUNTPERPAGE:
                return ( LRESULT )LView_OnGetCountPerPage ( hWnd, pthis );
            case LVM_GETEDITCONTROL:
                return ( LRESULT )pthis->hwEdit;
            case LVM_GETIMAGELIST:
                {
                    if ( wParam == LVSIL_NORMAL )
                        return ( LRESULT )pthis->hImgList;
                    else
                        return ( LRESULT )pthis->hSImgList;
                }

            case LVM_GETISEARCHSTRING:
                break;
            case LVM_GETITEM:
                return ( LRESULT )LView_OnGetItem ( hWnd, pthis, ( LV_ITEM* )lParam );
            case LVM_GETITEMCOUNT:
                return ( LRESULT )pthis->iCount;
            case LVM_GETITEMPOSITION:
                return ( LRESULT )LView_OnGetItemPos ( hWnd, pthis, ( int )wParam, ( LPPOINT )lParam );
            case LVM_GETITEMRECT:
                return ( LRESULT )LView_OnGetItemRect ( hWnd, pthis, ( int )wParam, ( LPRECT )lParam );
            case LVM_GETITEMSTATE:
                return ( LRESULT )LView_OnGetItemState ( hWnd, pthis, ( int )wParam, ( UINT )lParam );
            case LVM_GETITEMTEXT:
                return ( LRESULT )LView_OnGetItemText ( hWnd, pthis, ( int )wParam, ( LV_ITEM* )lParam );
            case LVM_GETNEXTITEM:
                return ( LRESULT )LView_OnGetNextItem ( hWnd, pthis, ( int )wParam, ( UINT )LOWORD ( lParam ) );
            case LVM_GETORIGIN:
                {
                    LPPOINT lppt = ( LPPOINT )lParam;
                    UINT    styleMask = pthis->uStyles & LVS_TYPEMASK;
                    if ( styleMask == LVS_REPORT || styleMask == LVS_LIST )
                        return FALSE;
                    lppt->x = pthis->psLBox.ptOrg.x;
                    lppt->y = pthis->psLBox.ptOrg.y;
                    return TRUE;
                }
            case LVM_GETSELECTEDCOUNT:
                return ( LRESULT )pthis->psLBox.lSelCount;
            case LVM_GETSTRINGWIDTH:
                return ( LRESULT )LView_OnGetStrWidth ( hWnd, pthis, ( LPSTR )lParam );
            case LVM_GETTEXTBKCOLOR:
                return ( LRESULT )pthis->TxtBkColor;
            case LVM_GETTEXTCOLOR:
                return ( LRESULT )pthis->TxtBkColor;
            case LVM_GETTOPINDEX:
                return ( LRESULT )pthis->psLBox.lTopIndx;
            case LVM_GETVIEWRECT:
                {
                    LPRECT lpRect;
                    UINT uStyle;
                    if ( ( uStyle = pthis->uStyles & LVS_TYPEMASK ) == LVS_REPORT ||
                         uStyle == LVS_LIST )
                         return ( LRESULT )FALSE;
                    lpRect = ( LPRECT )lParam;
                    SetRect ( lpRect, 0, 0, pthis->psLBox.Extent.cx, 
                                            pthis->psLBox.Extent.cy );
                    return ( LRESULT )TRUE;
                }
            case LVM_HITTEST:
                return ( LRESULT )LView_OnHitTest ( hWnd, pthis, ( LV_HITTESTINFO* )lParam );
            case LVM_INSERTCOLUMN:
                return ( LRESULT )LView_OnInsertCol ( hWnd, pthis, ( int )wParam, ( LV_COLUMN* )lParam );  
            case LVM_INSERTITEM:
                return ( LRESULT )LView_OnInsertItem ( hWnd, pthis, ( LV_ITEM* )lParam );  
            case LVM_REDRAWITEMS:
                return ( LRESULT )LView_OnRedrawItms ( hWnd, pthis, ( int )wParam, ( int )lParam );
            case LVM_SCROLL:
                return ( LRESULT )LView_OnScroll ( hWnd, pthis, ( int )wParam, ( int )lParam );
            case LVM_SETBKCOLOR:
                {
                    pthis->BkColor = ( COLORREF )lParam;
                    return ( LRESULT )TRUE;
                }
            case LVM_SETCALLBACKMASK:
                {
                    pthis->CallBMask = ( UINT )wParam;
                    return ( LRESULT )TRUE;
                }
            case LVM_SETCOLUMN:
                return ( LRESULT )LView_OnSetCol ( hWnd, pthis, ( int )wParam, ( LV_COLUMN* )lParam );
            case LVM_SETCOLUMNWIDTH:
                return ( LRESULT )LView_OnSetColWidth ( hWnd, pthis, ( int )wParam, ( int )LOWORD ( lParam ) );
            case LVM_SETIMAGELIST:
                return ( LRESULT )LView_OnSetImageList ( hWnd, pthis, ( int )wParam, ( HIMAGELIST )lParam );  
            
            case LVM_SETITEM:
                return ( LRESULT )LView_OnSetItem ( hWnd, pthis, ( LV_ITEM* )lParam );
            case LVM_SETITEMCOUNT:
                LView_OnSetItemCount ( hWnd, pthis, ( int )wParam );
                break;
            case LVM_SETITEMSTATE:
                return ( LRESULT )LView_OnSetItemState ( hWnd, pthis, ( int )wParam, ( LV_ITEM* )lParam );
            case LVM_SETITEMTEXT:
                return ( LRESULT )LView_OnSetItemText ( hWnd, pthis, ( int )wParam, ( LV_ITEM* )lParam );  
            case LVM_SETITEMPOSITION:
                return ( LRESULT )LView_OnSetItemPos ( hWnd, pthis, ( int )wParam, 
                                    ( int )LOWORD( lParam ), ( int )HIWORD ( lParam ) );

            case LVM_SETTEXTBKCOLOR:
                {
                    pthis->TxtBkColor = ( COLORREF )lParam;
                    return ( LRESULT )TRUE;
                }
            case LVM_SETTEXTCOLOR:
                {
                    pthis->TxtColor = ( COLORREF )lParam;
                    return ( LRESULT )TRUE;
                }
            case LVM_SORTITEMS:
                return ( LRESULT )LView_OnSortItems ( hWnd, pthis, 
                                       wParam, ( PFNLVCOMPARE )lParam );
            case LVM_UPDATE:
                {
                    pthis->bReArrange = TRUE;
                    pthis->psLBox.Extent.cx = pthis->psLBox.Extent.cy = 0;
                    InvalidateRect ( hWnd, ( LPRECT )NULL, TRUE );
                    return TRUE;
                }
            default:
			    return DefWindowProc(hWnd, uMessage, wParam, lParam);
		}
	}

	if ( pthis )
		GlobalUnlock ( hListView );

	return 0;
}


int WINAPI 
LView_Initialize 
(
	HINSTANCE		hinst
) 
{

	WNDCLASS		wc;
	int			ErrorCode	= 0;

	memset ( &wc, 0, sizeof ( WNDCLASS ) );
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
	wc.lpfnWndProc   = LView_WndProc;
	wc.cbWndExtra    = 0; /*sizeof( TView_WNDEXTRABYTES );*/
	wc.hInstance     = hinst;
	wc.hCursor       = LoadCursor( ( HINSTANCE )NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject ( WHITE_BRUSH );
	wc.lpszClassName = WWC_LISTVIEW;

	if ( ! RegisterClass ( &wc ) )
		ErrorCode = WLV_ERR_REGISTERCLASS;

	return ( ErrorCode );
}

void WINAPI LView_Terminate 
(
	HINSTANCE			hInstance
) 
{
	WNDCLASS			WndClass;


	if ( GetClassInfo ( hInstance, WWC_LISTVIEW, &WndClass ) )
		UnregisterClass ( WWC_LISTVIEW, hInstance );

}





