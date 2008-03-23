/*  
	WTView.c	
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

/*
 * - Is this sequence the one implemented by treeview control:
 *   1)the user clicks on a selected item or the app sending TVM_EDITLABEL
 *   2)treeview control sends its parent TVN_BEGINLABELEDIT notify message
 *     and if got 0 as return value, displays the edit control for editing.
 *   3(?))the editing is ended when user presses Escape or Enter key( or 
 *     equally, selects another item ). The treeview control sends
 *     TVN_ENDLABELEDIT to its parent, setting pszText of TV_ITEM 
 *     to zero when Escape key is pressed.
 *   4(?)) treeview parent responds by sending TVM_ENDEDITLABELNOW
 *     setting wParam to FALSE if it wants the the control to store
 *     the text internally.
 *   5(?)) TVM_ENDEDITLABELNOW would eventually evokes TVN_ENDLABELEDIT
 *     again, now with pszText of TV_ITEM set to zero ????
 *   
 *   
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
#include "WTView.h"
#include "WTVLBox.h"

#define ROWS        4
#define COLS        3  
#define IDTVLIST    101 
#define IDTVEDIT    102 

#define STARTEDIT_TIMER 16
#define DEF_ITEMHEIGHT  20

#ifndef _WINDOWS
#define  GetTextExtentPoint32   GetTextExtentPoint
#endif

enum { eNoDrag = 0, eEndDrag = eNoDrag, eCancelDrag,
       eLBtnDown, eRBtnDown, eBeginDrag, eDragging };

static char TView_PROP[] = "TView_Data";	

#define GETTHISDATA(hwnd) \
   GetProp(hwnd, TView_PROP )
#define SETTHISDATA(hwnd, pthisNew) \
   SetProp(hwnd, TView_PROP, pthisNew)
#define REMOVETHISDATA(hwnd) \
   RemoveProp(hwnd, TView_PROP )

extern LRESULT WINAPI 
TViewTip_WndProc ( HWND hWnd, UINT uMessage, WPARAM	wParam, LPARAM lParam );

extern	BOOL WINAPI ImageList_Draw
(
	HIMAGELIST	hImageList,
	int		ImageIndex,
	HDC		hDC,
	int		X,
	int		Y,
	UINT		DrawFlags
);

extern  BOOL WINAPI ImageList_GetIconSize
(
	HIMAGELIST	hImageList,
	int		*pIconWidth,
	int		*pIconHeight
);


/*prototypes*/
static void 
TView_IShowSubItems
(
    HWND            hWnd,
    PTVLBOXDATA     pLBox, 
    int             wSelItm,
    PTVITEMDATA     pItmList, 
    HTREEITEM       hItem
);
static BOOL
TView_OnGetItemRect
( 
    HWND                hWnd, 
    TREEVIEWDATA*       pthis,
    BOOL                fItemRect,
    LPRECT              lpRect,
    LPSTR               lpszText
);
static int TView_OnEraseBkground ( HWND hWnd, HDC hDC );
static BOOL TView_OnEndEditLabelNow ( HWND hWnd, BOOL fCancel ) ;
static LRESULT  
TView_TVMWndProc ( HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam );
void TView_IShowSubItems
(
    HWND            hWnd, 
    PTVLBOXDATA     pLBox,
    int             wSelItm,
    PTVITEMDATA     pItmList, 
    HTREEITEM       hItem
);

static HTREEITEM
TView_IAllocItmHandle 
(
    TREEVIEWDATA*       pthis
)
{
    HTREEITEM           hItem       = ( HTREEITEM )NULL;
	PTVITEMDATA         pItmList    = ( PTVITEMDATA )NULL;
	PTVITEMDATA         pDeleted    = ( PTVITEMDATA )NULL;

    if ( pthis->hDeleted )
    {
        hItem = pthis->hDeleted;
        if ( !WLLock ( &pthis->ItmList, ( LPVOID )&pItmList ) )
        {
            pDeleted = pItmList + ( int )hItem;
            if ( pDeleted->tvi.cChildren == 0 )
                pthis->hDeleted = pDeleted->NSibling;
            else
            {
                PTVITEMDATA pLChild = pItmList + ( int )pDeleted->LChild;
                pLChild->NSibling = pDeleted->NSibling;
                pthis->hDeleted = pDeleted->FChild;
            }

            WLUnlock ( &pthis->ItmList, ( LPVOID )&pItmList );
        }
    }
    else
    {
     	TVITEMDATA          TVItm;
        memset ( &TVItm, 0, sizeof ( TVITEMDATA ) );
       	if ( !WLInsert ( &pthis->ItmList, pthis->lCount+1, &TVItm ) )
            hItem = ( HTREEITEM )( pthis->lCount + 1 );
    }
   
    return hItem;
}


/* HeapSort implementation */
static void
TView_IDownHeapCB
(
    PTVSORTITEM         pSortAr,
    int                 N,
    int                 k,
    LPTV_SORTCB         lpSortCB
)
{
    int                 j;
    TVSORTITEM          v;
    PFNTVCOMPARE        lpfnCmp;

    j = pSortAr[k].Ord;
    memcpy ( &v, pSortAr + j, sizeof ( TVSORTITEM ) ); 
    v.Ord = j;
    lpfnCmp = lpSortCB->lpfnCompare;
    while ( k <= N / 2 )
    {
        j = k+k;
        if ( j < N && 
             ( *lpfnCmp )( pSortAr[pSortAr[j].Ord].lParam, 
                           pSortAr[pSortAr[j+1].Ord].lParam, lpSortCB->lParam ) < 0 )
            j++;
        if ( ( *lpfnCmp ) ( v.lParam, pSortAr[pSortAr[j].Ord].lParam, lpSortCB->lParam ) >= 0 )
            break;
        pSortAr[k].Ord = pSortAr[j].Ord;
        k = j;
    }
    pSortAr[k].Ord = v.Ord; 
}

static void
TView_IDownHeap
(
    PTVSORTITEM         pSortAr,
    int                 N,
    int                 k    
)
{
    int                 j;
    TVSORTITEM          v;

    j = pSortAr[k].Ord;
    memcpy ( &v, pSortAr + j, sizeof ( TVSORTITEM ) ); 
    v.Ord = j;
    while ( k <= N / 2 )
    {
        j = k+k;
        if ( j < N && 
             lstrcmp ( pSortAr[pSortAr[j].Ord].szText, 
                       pSortAr[pSortAr[j+1].Ord].szText ) < 0 )
            j++;
        if ( lstrcmp ( v.szText, pSortAr[pSortAr[j].Ord].szText ) >= 0 )
            break;
        pSortAr[k].Ord = pSortAr[j].Ord;
        k = j;
    }
    pSortAr[k].Ord = v.Ord; 
}

/* Heap Sort */
static void
TView_IDoHeapSort
(
    PTVSORTITEM         pSortAr,
    int                 Count,
    LPTV_SORTCB         lpSortCB
)
{
    int k;
    if ( lpSortCB )
        for ( k = Count/2; k >= 1; k-- )
            TView_IDownHeapCB ( pSortAr, Count, k, lpSortCB );
    else
        for ( k = Count/2; k >= 1; k-- )
            TView_IDownHeap ( pSortAr, Count, k );

    while ( Count > 1 )
    {
        k = pSortAr[1].Ord;
        pSortAr[1].Ord = pSortAr[Count].Ord;
        pSortAr[Count].Ord = k;
        if ( lpSortCB )
            TView_IDownHeapCB ( pSortAr, --Count, 1, lpSortCB );
        else
            TView_IDownHeap ( pSortAr, --Count, 1 );
    }
}

#ifdef 0
static void
TView_IDoShellSort
(
    PTVSORTITEM         pSortAr,
    int                 Count
)
{
    int                 i, j, h;
    TVSORTITEM          v;

    for ( h = 1; h<= Count/9;h = 3*h+1 );
    for ( ; h > 0; h /= 3 )
        for ( i=h+1; i <= Count; i += 1 )
        {
            memcpy ( &v, pSortAr + pSortAr[i].Ord, sizeof ( TVSORTITEM ) );
            j = i;
            while ( j>h && 
                    lstrcmp ( pSortAr[pSortAr[j-h].Ord].szText, v.szText ) > 0 )
            {
                pSortAr[j].Ord = pSortAr[j-h].Ord;
                j -= h;
            }
            pSortAr[j].Ord = v.Ord;
        }
}
#endif

static void
TView_IDrawBorder 
(
		HWND			hWnd, 
		HDC				hDC 
)
{
		RECT			rect;
		HPEN			hPen    = GetStockObject ( BLACK_PEN );
        HBRUSH          hBr     = GetStockObject ( NULL_BRUSH );
	if ( hDC )
	{
		hPen = SelectObject ( hDC, hPen );
        hBr = SelectObject ( hDC, hBr );
		GetClientRect ( hWnd, &rect );
		Rectangle ( hDC, rect.left, rect.top,
					   rect.right, rect.bottom );
		SelectObject ( hDC, hPen );
        SelectObject ( hDC, hBr );
	}

}


HGLOBAL
TView_IGetResource
(
    HWND                hwTree,
    PTREEDATA*          ppTree,
    PTVITEMDATA*        ppItmList,
    PTVSTR*             ppStrList  
)
{
    HGLOBAL             hRsrc = GETTHISDATA ( hwTree );

    if ( hRsrc &&
         ( *ppTree = ( PTREEDATA )GlobalLock ( hRsrc ) )
       )
    {
        PTREEDATA pTree = *ppTree;
        if ( ppItmList &&
             ( WLLock ( &pTree->ItmList, ( LPVOID )ppItmList ) )
           )
        {
            GlobalUnlock ( hRsrc );
            return ( HGLOBAL )NULL;
        }

        if ( ppStrList &&
             ( WLLock ( &pTree->StrList, ( LPVOID )ppStrList ) )
           )
        {
            if ( ppItmList )
                WLUnlock ( &pTree->ItmList, ( LPVOID )ppItmList );
            GlobalUnlock ( hRsrc );
            return ( HGLOBAL )NULL;
        }
        return hRsrc;
    }

    return ( HGLOBAL )NULL;
}

static BOOL
TView_IGetItem 
( 
    PTVITEMDATA         pItmList,
    HTREEITEM           hItem,
    PTVITEMDATA         ptvid
)
{
    PTVITEMDATA         pItem       = ( PTVITEMDATA )NULL;

    pItem = pItmList + ( int )hItem;
    memcpy ( ptvid, pItem, sizeof ( TVITEMDATA ) );
    return TRUE;

}

static void
TView_IGetItemInfo 
(
    HWND                    hwnd,
    TREEVIEWDATA*           pthis,    
    TVITEMDATA*             pItem,
    TVIEWDRAWITEMSTRUCT*    lptvdis
)
{
    PTVSTR              pStrList    = ( PTVSTR )NULL;
    int                 ErrorCode   = 0;

    if ( WLLock ( &pthis->StrList, ( LPVOID )&pStrList ) )
		ErrorCode = WTV_ERR_LISTLOCK;
    else
    {                              
        TV_DISPINFO tvdi;
        int idCtrl =  GetDlgCtrlID ( hwnd );
        tvdi.hdr.code = TVN_GETDISPINFO;
        tvdi.hdr.hwndFrom = hwnd;
        tvdi.hdr.idFrom = idCtrl;
        tvdi.item = pItem->tvi;
        tvdi.item.mask = 0;
        tvdi.item.pszText = lptvdis->pszText;

        if ( pthis->hEditItem == tvdi.item.hItem )
            lptvdis->pszText[0] = '\0';
        else
        if ( pItem->tvi.pszText != LPSTR_TEXTCALLBACK ) 
            lstrcpy ( lptvdis->pszText, ( LPSTR )( pStrList + ( int )pItem->tvi.pszText ) );
        else
        {
            tvdi.item.mask = TVIF_TEXT;
            tvdi.item.cchTextMax = MAXSTRING;
/*            tvdi.item.pszText = lptvdis->pszText;*/
        }

        lptvdis->nStImage = -1;
        if ( pItem->tvi.stateMask & TVIS_STATEIMAGEMASK )
        {
            lptvdis->nStImage = pItem->tvi.state >> 12;
        }

        if ( pItem->tvi.state & TVIS_SELECTED ) 
        {
            lptvdis->nImage = pItem->tvi.iSelectedImage;
            if ( lptvdis->nImage == I_IMAGECALLBACK )
                tvdi.item.mask |= TVIF_SELECTEDIMAGE;
        }
        else
        {
            lptvdis->nImage = pItem->tvi.iImage;
            if ( lptvdis->nImage == I_IMAGECALLBACK )
                tvdi.item.mask |= TVIF_IMAGE;
        }

        if ( tvdi.item.mask != 0 ) 
        {
            SendMessage ( GetParent ( hwnd ), WM_NOTIFY, idCtrl, ( LPARAM )&tvdi );
            if ( lptvdis->nImage == I_IMAGECALLBACK )
            {
                if ( pItem->tvi.state & TVIS_SELECTED )
                    lptvdis->nImage = tvdi.item.iSelectedImage;
                else
                    lptvdis->nImage = tvdi.item.iImage;
            }
            
        }

        /*if ( lpSize )*/
        {
            HDC     hDC = GetDC ( hwnd );
            SelectObject ( hDC, pthis->hFont );
            GetTextExtentPoint32 ( hDC, lptvdis->pszText, strlen ( lptvdis->pszText ), &lptvdis->TextSize ); 
            lptvdis->TextSize.cx += 5;
            lptvdis->TextSize.cy += 3;
            ReleaseDC ( hwnd, hDC );
        }

    }

    WLUnlock ( &pthis->StrList, ( LPVOID )&pStrList );

}


/* need revisit: passing both pItem & hItem is a redundancy
 * lpRect: the bounding rectangle containing the text
 */
static void
TView_IGetItemText 
(
    HWND                hwnd,
    TREEVIEWDATA*       pthis,    
    TVITEMDATA*         pItem,
    LPSTR               szText,
    int                 nTextMax,
    LPSIZE              lpSize
)
{
    PTVSTR              pStrList    = ( PTVSTR )NULL;
    int                 ErrorCode   = 0;

    if ( WLLock ( &pthis->StrList, ( LPVOID )&pStrList ) )
		ErrorCode = WTV_ERR_LISTLOCK;
    else
    {
        if ( pItem->tvi.pszText != LPSTR_TEXTCALLBACK ) 
            lstrcpy ( szText, ( pStrList + ( int )pItem->tvi.pszText )->Label );
        else
        {
            TV_DISPINFO tvdi;
            int idCtrl =  GetDlgCtrlID ( hwnd );
            tvdi.hdr.code = TVN_GETDISPINFO;
            tvdi.hdr.hwndFrom = hwnd;
            tvdi.hdr.idFrom = idCtrl;
            tvdi.item = pItem->tvi;
            tvdi.item.mask = TVIF_TEXT;
            tvdi.item.cchTextMax = nTextMax;
            tvdi.item.pszText = szText;
            SendMessage ( GetParent ( hwnd ), WM_NOTIFY, idCtrl, ( LPARAM )&tvdi );
            if ( tvdi.item.pszText != szText )
                lstrcpyn ( szText, tvdi.item.pszText, min ( nTextMax, tvdi.item.cchTextMax ) ); 
            
        }

        if ( lpSize )
        {
            HDC     hDC = GetDC ( hwnd );
            SelectObject ( hDC, pthis->hFont );
            GetTextExtentPoint32 ( hDC, szText, strlen ( szText ), lpSize ); 
            ReleaseDC ( hwnd, hDC );
        }

    }

    WLUnlock ( &pthis->StrList, ( LPVOID )&pStrList );

}

static void
TView_IInsertLBItem
(
    HWND            hWnd,
    TREEVIEWDATA*   pthis,
    TVITEMDATA*     pItmList,
    HTREEITEM       hAfterMe,
    HTREEITEM       hItem
)
{
    int             nIndx;
    int             nCount;
    HTREEITEM       hChild      = ( HTREEITEM )NULL;
    TVITEMDATA*     pAfterMe    = ( TVITEMDATA* )NULL;
    TVITEMDATA*     pItem    	= ( TVITEMDATA* )NULL;
    TVITEMDATA*     pChild      = ( TVITEMDATA* )NULL;

    nIndx = TVLBox_FindStringLP ( &pthis->psLBox, 0, ( LPARAM )hAfterMe );
    if ( nIndx == LB_ERR )
        return; /* should throw an exception here? */

    nIndx++;
    pItem = pItmList + ( int )hItem;
    if ( pItem->Parent == hAfterMe )
	goto InsertString;
    pAfterMe = pItmList + ( int )hAfterMe;
    if ( pAfterMe->tvi.cChildren == 1 && pAfterMe->bOpened )
    {
        nCount = TVLBox_GetCount ( &pthis->psLBox );
        for ( ; nIndx < nCount; nIndx++ )
        {
            hChild = ( HTREEITEM )TVLBox_GetItemData ( &pthis->psLBox, nIndx );
            pChild = pItmList + ( int )hChild;
            if ( pChild->nLevel <= pAfterMe->nLevel )
                break;
        }
    }
InsertString:
    TVLBox_InsertStringLP ( hWnd, &pthis->psLBox, nIndx, ( LPARAM )hItem ); 

}

static BOOL
TView_IIsBtnClicked 
(
    HWND            hwTView,
    TREEVIEWDATA*   pthis,
    int             nLBItem,
    int             x,
    int             y
)
{
    HTREEITEM       hItem;
    TVITEMDATA      tvid;
    POINT           pt;
    RECT            rcItm;
    BOOL            bResult     = FALSE;

    PTVITEMDATA     pItmList    = ( PTVITEMDATA )NULL;

    if ( !pthis || nLBItem < 0 )
        return FALSE;
    hItem   = ( HTREEITEM )TVLBox_GetItemData ( &pthis->psLBox, nLBItem );
    TVLBox_GetItemRect ( hwTView, &pthis->psLBox, nLBItem, &rcItm );
    if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItmList ) )
        return FALSE;

    memset ( &tvid, 0, sizeof ( TVITEMDATA ) );
    TView_IGetItem ( pItmList, hItem, &tvid );
    if ( tvid.tvi.cChildren == 1 )
    {
        TVLBox_IGetIndentPoint ( &rcItm, tvid.nLevel, pthis->nIndent, &pt );
        if ( TVLBox_IPtInBtn ( &pt, x, y ) )
            bResult = TRUE;
    }

    WLUnlock ( &pthis->ItmList, ( LPVOID )&pItmList );

    return bResult;

}

/*
 * do a recursive expansion on an item. Before that, this item's parent should be
 * expanded already, otherwise the function would call itself to expand the
 * item's parent.
 * pIndex: pointer to buffer containing the index of the item's parent just expanded
 */
static BOOL
TView_IRecursiveExpand
(
    HWND                hWnd,
    PTVLBOXDATA         pLBox,
    PTVITEMDATA         pItmList,
    PTVITEMDATA         pItem,
    int*                pIndex
)
{
    HTREEITEM           hItem            = ( HTREEITEM )NULL;
    PTVITEMDATA         pParent          = ( PTVITEMDATA )NULL;
    int                 iParentIndx      = -1;
    BOOL                bParentExpanded  = FALSE;
    BOOL                bRootItm         = FALSE;

    if ( pItem->Parent )
    {
        pParent = pItmList + ( int )pItem->Parent;
        hItem = pParent->tvi.hItem;
        if ( !pParent->bOpened )
            bParentExpanded = TView_IRecursiveExpand ( hWnd, pLBox, pItmList, pParent, &iParentIndx );
        else
            bParentExpanded = TRUE;
    }
    else
    {
        hItem = pItem->tvi.hItem;
        bRootItm = TRUE;
    }

    if ( bRootItm || bParentExpanded ) /* this guy must be a root item, hence must be visible somewhere in listbox 
                                       * or a parent that is successfully expanded
                                       */
    {
        *pIndex = TVLBox_FindStringLP ( pLBox, iParentIndx, ( LPARAM )hItem );
        if ( *pIndex == LB_ERR )
            return FALSE;
        TView_IShowSubItems ( hWnd, pLBox, *pIndex, pItmList, hItem );
        return TRUE;
    }
    
    return FALSE;
}


void
TView_IReleaseResource
(
    HGLOBAL             hRsrc,
    PTREEDATA           pTree,
    PTVITEMDATA*        ppItmList,
    PTVSTR*             ppStrList  
)
{
    if ( ppItmList )
        WLUnlock ( &pTree->ItmList, ( LPVOID )ppItmList );
    if ( ppStrList )
        WLUnlock ( &pTree->StrList, ( LPVOID )ppStrList );
    if ( pTree )
        GlobalUnlock ( hRsrc );
}


static void 
TView_IShowSubItems
(
    HWND            hWnd,
    PTVLBOXDATA     pLBox, 
    int             wSelItm,
    PTVITEMDATA     pItmList, 
    HTREEITEM       hItem
)
{
    PTVITEMDATA     pItem;
    PTVITEMDATA     pChildItm;


    HTREEITEM       hDelete;

    pItem = pItmList + ( int )hItem;

    if ( pItem->bOpened )
    {
        pItem->bOpened = FALSE;
        wSelItm++;
        hDelete = ( HTREEITEM )TVLBox_GetItemData( pLBox, wSelItm );
        while ( hDelete != ( HTREEITEM )LB_ERR )
        {
            pChildItm = pItmList + ( int )hDelete;
            if ( pChildItm->tvi.cChildren == 1 && pChildItm->bOpened )
                TView_IShowSubItems ( hWnd, pLBox, wSelItm, pItmList, hDelete );
            TVLBox_DeleteStringLP ( hWnd, pLBox, wSelItm );
            if ( hDelete == pItem->LChild )
                break;
            hDelete = ( HTREEITEM )TVLBox_GetItemData ( pLBox, wSelItm );
        }
    }
    else
    {
        pItem->bOpened = TRUE;
        wSelItm++;
        SendMessage ( hWnd, WM_SETREDRAW, FALSE, 0L );   /* Disable redrawing.*/
        pChildItm = pItmList + ( int )pItem->FChild;
        while ( TRUE )
        {
            TVLBox_InsertStringLP ( hWnd, pLBox, wSelItm++, ( LPARAM )pChildItm->tvi.hItem );
            if ( ( pChildItm->tvi.hItem == pItem->LChild ) ||
                 ( pChildItm->NSibling == ( HTREEITEM )NULL )
               )
                break;
            pChildItm = pItmList + ( int )pChildItm->NSibling;
        }
         /*
         // Make sure as many child items as possible are showing
         */
        SendMessage(hWnd, WM_SETREDRAW, TRUE, 0L);  
    }

    InvalidateRect(hWnd, NULL, TRUE);            /* Force redraw */
}


static void
TView_ISortChildren
(
    HWND                hWnd,
    HTREEITEM           hParent,
    BOOL                bRecursive
)
{

}

static void 
TView_OnCommand
(
    HWND                hwTrView, 
    UINT                id, 
    HWND                hwndCtl, 
    UINT                code
)
{

    if ( id == IDOK ) 
    {
        MessageBox ( ( HWND )NULL, "Enter key detected", "IDOK", MB_OK );
    }
}


static BOOL 
TView_OnCreate 
(
	HWND		    hWnd, 
	CREATESTRUCT*	    pcs
) 
{
	RECT                rc;
    	TVITEMDATA          dummy;
    	TVSTR               StrDummy;
    	HGLOBAL		    hTreeView	= ( HGLOBAL )NULL;
	TREEVIEWDATA*	    pthis	= ( TREEVIEWDATA* )NULL;

	int		    ErrorCode	= 0;

	if ( ! ( hTreeView = GlobalAlloc ( GHND, sizeof ( TREEVIEWDATA ) ) ) )
		ErrorCode = WTV_ERR_GLOBALALLOC;
	else
	if ( ! ( pthis = ( TREEVIEWDATA *) GlobalLock ( hTreeView ) ) )
		ErrorCode = WTV_ERR_GLOBALLOCK;
	else
	{
		SETTHISDATA(hWnd, hTreeView);
		pthis->uStyles = pcs->style;
		pthis->hFont = ( HFONT )GetStockObject ( ANSI_VAR_FONT );
        GetClientRect ( hWnd, &rc );
        pthis->psLBox.ItmWidth = rc.right - rc.left;
        pthis->psLBox.ItmHeight = DEF_ITEMHEIGHT;
        pthis->psLBox.lCurSel = pthis->psLBox.lDropSel = -1;
        pthis->psLBox.lCaret = -1;
        pthis->psLBox.iHExtent = rc.right - rc.left;

        /* is ES_WANTRETURN supported in Twin32 ? */
        pthis->hwEdit = CreateWindowEx((DWORD)0L,
                 (LPSTR)"EDIT",(LPSTR)NULL,
                 WS_CHILD | WS_BORDER |
                 ES_AUTOHSCROLL | ES_WANTRETURN,
                 0, 0, 0, 0,
                 hWnd,
                 (HMENU)IDTVEDIT,
                 pcs->hInstance,
                 NULL);
        SendMessage ( pthis->hwEdit, WM_SETFONT, ( WPARAM )pthis->hFont, ( LPARAM )FALSE );
        TVEdit_Subclass ( pthis->hwEdit );

#if 1
        pthis->hwTipTxt = CreateWindowEx((DWORD)0L,
                 WC_TVTIPTEXT,(LPSTR)NULL,
                 WS_POPUP | WS_BORDER,
                 0, 0, 0, 0,
                 ( HWND )NULL,
                 (HMENU)NULL, 
                 pcs->hInstance,
                 NULL);
        SetWindowLong ( pthis->hwTipTxt, GWL_USERDATA, ( LONG )hWnd ); 

        SendMessage ( pthis->hwTipTxt, WM_SETFONT, ( WPARAM )pthis->hFont, ( LPARAM )FALSE );
#endif

        WLCreate ( &pthis->ItmList, sizeof ( TVITEMDATA ) );
   		WLCreate ( &pthis->StrList, sizeof ( TVSTR ) );
   		WLCreate ( &pthis->psLBox.ItmList, sizeof ( TVLBOXITEM ) );

/* the first item is a dummy. This is transparent to the application
   because all HTREEITEMs are supposed to be NON_NULL and are indices
   into this list */        
        memset ( &dummy, 0, sizeof ( TVITEMDATA ) );
        dummy.nLevel = -1;
        dummy.bOpened = TRUE;
        if ( pthis->uStyles & TVS_HASLINES && pthis->uStyles & TVS_LINESATROOT )
            dummy.dwConnect = 1;
        WLInsert ( &pthis->ItmList, 0, &dummy );
        memset ( &StrDummy, 0, sizeof ( TVSTR ) );
        WLInsert ( &pthis->StrList, 0, &StrDummy );
        
        pthis->nIndent = 16;
	}

	if ( pthis )
		GlobalUnlock ( hTreeView );
	return ( ErrorCode >= 0 );
}

void
TView_OnDrawItem
(
    	HWND                    hwnd, 
	TREEVIEWDATA*	        pthis,
    	int                     iIndx
)
{
    	TVIEWDRAWITEMSTRUCT     tvdis;
	HTREEITEM               hItem       = ( HTREEITEM )NULL;
	TVITEMDATA*             pTVItmList  = ( TVITEMDATA* )NULL;
	TVITEMDATA*             pItem       = ( TVITEMDATA* )NULL;
   
	int			ErrorCode   = 0;
    	char                    szText[MAXSTRING];

    	int                     nLevel;
    	int                     ItmWidth;
  
	TVITEMDATA*             pParent         = ( TVITEMDATA* )NULL;

    if ( WLLock ( &pthis->ItmList, ( LPVOID )&pTVItmList ) )
		ErrorCode = WTV_ERR_LISTLOCK;
    else
    {
        memset ( &tvdis, 0, sizeof ( TVIEWDRAWITEMSTRUCT ) ); 
        tvdis.pTree = ( LPVOID )pthis;
        TVLBox_GetItemRect ( hwnd, &pthis->psLBox, iIndx, &tvdis.rcItem ); 
        tvdis.itemData = ( DWORD )TVLBox_GetItemData ( &pthis->psLBox, iIndx );    
        tvdis.itemID = iIndx;
        tvdis.itemAction = 0;
        tvdis.itemState = 0;
        hItem = ( HTREEITEM )tvdis.itemData;
        pItem = pTVItmList + ( int )hItem;
        pParent = pTVItmList + ( int )pItem->Parent;
        if ( tvdis.itemID == ( UINT )pthis->psLBox.lCurSel  ||
             tvdis.itemID == ( UINT )pthis->psLBox.lDropSel )
	{
             tvdis.itemState |= ODS_SELECTED;
	     pItem->tvi.state |= TVIS_SELECTED;
	}
	else
	     pItem->tvi.state &= ~TVIS_SELECTED;
    if ( pthis->psLBox.bFocus &&
	     tvdis.itemID == ( UINT )pthis->psLBox.lCaret )
	{
             tvdis.itemState |= ODS_FOCUS;
	     pItem->tvi.state |= TVIS_FOCUSED;
	}
	else
	     pItem->tvi.state &= ~TVIS_FOCUSED;
        
        nLevel = pItem->nLevel;
        pItem->dwConnect = pParent->dwConnect;
        pItem->dwConnect *= 2;
        if ( pParent->NSibling == ( HTREEITEM )NULL )
            pItem->dwConnect &= ~(DWORD)0x2;
            pItem->dwConnect |= 1;
        tvdis.dwConnect = pItem->dwConnect;
        tvdis.bLastKid = FALSE;
        if ( pItem->NSibling == ( HTREEITEM )NULL )
            tvdis.bLastKid = TRUE;
        if ( pItem->FChild )
            tvdis.bHasChild = TRUE;
        tvdis.nLevel  = pItem->nLevel;
        tvdis.bOpened = pItem->bOpened;
        tvdis.pszText = szText;

        TView_IGetItemInfo ( hwnd, pthis, pItem, &tvdis );
        /* to be changed */

        nLevel = ( pthis->uStyles & TVS_HASBUTTONS )? pItem->nLevel + 1 :
                                                      pItem->nLevel;

        ItmWidth = ( ( nLevel + 1 ) * pthis->nIndent + XBMPOFFSET ) +
                   tvdis.TextSize.cx;
        if ( ItmWidth > TVLBox_GetHorzExtent ( &pthis->psLBox ) )
            TVLBox_SetHorzExtent ( hwnd, &pthis->psLBox, ItmWidth ); 
        tvdis.nLineHeight = TVLBox_GetItemHeight ( &pthis->psLBox );
        tvdis.nTextHeight = tvdis.nLineHeight - 4;

        TVLBox_OnDrawItem ( hwnd, &tvdis );
        WLUnlock ( &pthis->ItmList, ( LPVOID )&pTVItmList );
    }
    return;
}

static HTREEITEM
TView_OnHitTest
( 
    HWND                hWnd, 
    TREEVIEWDATA*       pthis,
    LPTV_HITTESTINFO    lpht
)
{
    int                 iHitItem;
    int                 iHit;      
    int                 iCount;
    HTREEITEM           hItem       = ( HTREEITEM )NULL;
    UINT                uResult     = 0;
    PTVITEMDATA         pItmList    = ( PTVITEMDATA )NULL;
    PTVITEMDATA         pItem       = ( PTVITEMDATA )NULL;

    DWORD   dwPoint = TVLBox_ItemFromPoint ( hWnd, &pthis->psLBox, lpht->pt.x, lpht->pt.y );
    iCount = TVLBox_GetCount ( &pthis->psLBox );
    iHit = HIWORD ( dwPoint );
    iHitItem = LOWORD ( dwPoint );
    if ( iHit == 0 ) /* hit inside */
    {
        if ( iHitItem < 0 || iHitItem >= iCount )
            uResult = TVHT_NOWHERE;
        else
        if ( ( pthis->uStyles & TVS_HASBUTTONS ) &&
             TView_IIsBtnClicked ( hWnd, pthis, iHitItem, lpht->pt.x, lpht->pt.y ) ) 
                uResult = TVHT_ONITEMBUTTON;
        else
        {
            RECT rcText;
            LPRECT    lpRect = &rcText;
            hItem = ( HTREEITEM )TVLBox_GetItemData ( &pthis->psLBox, iHitItem );
            *( HTREEITEM *)lpRect = hItem;
            TView_OnGetItemRect ( hWnd, pthis, TRUE, lpRect, NULL );
            if ( lpht->pt.x > rcText.right )
                uResult = TVHT_ONITEMRIGHT;
            else
            if ( lpht->pt.x <= rcText.right && lpht->pt.x >= rcText.left )
                uResult |= TVHT_ONITEMLABEL;
            else
            if ( !WLLock ( &pthis->ItmList, ( LPVOID )&pItmList ) )
            {
                pItem = pItmList + ( int )hItem;
                if ( lpht->pt.x <= rcText.left && lpht->pt.x >= rcText.left - pthis->ImgSize.cx )
                    uResult = TVHT_ONITEMICON;
                else
                {
                    rcText.left -= pthis->ImgSize.cx;
                    if ( pItem->tvi.stateMask & TVIS_STATEIMAGEMASK )
                    {
                        if ( lpht->pt.x <= rcText.left && lpht->pt.x >= rcText.left - pthis->StImgSize.cx )
                            uResult = TVHT_ONITEMSTATEICON;
                        else
                            uResult |= TVHT_ONITEMINDENT;
                    }
                    else
                        uResult |= TVHT_ONITEMINDENT;
                }

                WLUnlock ( &pthis->ItmList, ( LPVOID )&pItmList );
            }


        }
    }
    else /* hit outside */
    {
        RECT    rcLBox;
        GetClientRect ( hWnd, &rcLBox );
        if ( lpht->pt.y < rcLBox.top )
            uResult = TVHT_ABOVE;
        else
        if ( lpht->pt.y > rcLBox.bottom )
            uResult = TVHT_BELOW;
        if ( lpht->pt.x < rcLBox.left )
            uResult |= TVHT_TOLEFT;
        else
        if ( lpht->pt.x > rcLBox.right )
            uResult |= TVHT_TORIGHT;

    }

    lpht->flags = uResult;
    lpht->hItem = hItem;
    return hItem;
}

void 
TView_OnKeyDown (HWND hWnd, int VKey, int cRepeat, UINT keyFlags) 
{
    	RECT            rcScroll;
    	TV_KEYDOWN      tk;
    	HWND            hwFocus     	= GetFocus ();
	HGLOBAL		hRsrc   	= ( HGLOBAL )NULL;
	TREEVIEWDATA*	pthis		= ( TREEVIEWDATA* )NULL;
    
    	assert ( hRsrc = TView_IGetResource ( hWnd, &pthis, ( PTVITEMDATA* )NULL, ( PTVSTR* )NULL ) );
    	if ( ( VKey == VK_ESCAPE || VKey == VK_RETURN )
        	 && hwFocus == pthis->hwEdit )
    	{
        	BOOL bCancel = VKey == VK_ESCAPE ? TRUE : FALSE;
        	TView_OnEndEditLabelNow ( hWnd, bCancel );
    	}
    	else
    	if ( VKey == VK_UP && pthis->psLBox.lCurSel > 0 ) 
    	{
        	if ( pthis->psLBox.lTopIndx >= pthis->psLBox.lCurSel )
        	{
            		SendMessage ( hWnd, WM_VSCROLL, MAKELONG ( SB_LINEUP, 0 ), 0 );
        	}
        	TVLBox_SetCurSel ( hWnd, pthis, pthis->psLBox.lCurSel - 1, TVC_BYKEYBOARD, NULL );
    	}
    	else
    	if ( VKey == VK_DOWN && pthis->psLBox.lCurSel < pthis->psLBox.lCount - 1 ) 
    	{
        	GetClientRect ( hWnd, &rcScroll );
        	if ( ( pthis->psLBox.lCurSel - pthis->psLBox.lTopIndx + 2 ) * pthis->psLBox.ItmHeight > 
             	     ( rcScroll.bottom - rcScroll.top ) )
        	{
            		SendMessage ( hWnd, WM_VSCROLL, MAKELONG ( SB_LINEDOWN, 0 ), 0 );
        	}
        	TVLBox_SetCurSel ( hWnd, pthis, pthis->psLBox.lCurSel + 1, 
                           TVC_BYKEYBOARD, NULL );
    	}
    	else
    	if ( VKey == VK_TAB )
    	{
        	SetFocus ( GetNextDlgTabItem ( GetParent ( hWnd ), hWnd, FALSE ) );
    	}

    	TView_IReleaseResource ( hRsrc, pthis, ( PTVITEMDATA* )NULL, ( PTVSTR* )NULL );
    	tk.hdr.hwndFrom = hWnd;
    	tk.hdr.idFrom = GetDlgCtrlID ( hWnd );
    	tk.hdr.code = TVN_KEYDOWN;
    	tk.wVKey = VKey;
    	SendMessage ( hWnd, WM_NOTIFY, tk.hdr.idFrom, ( LPARAM )&tk );
}


static void 
TView_OnKeyUp (HWND hWnd, int VKey, int cRepeat, UINT keyFlags) 
{
	HGLOBAL			hTreeView	= ( HGLOBAL )NULL;
	TREEVIEWDATA*	pthis		= ( TREEVIEWDATA* )NULL;

	int			    ErrorCode	= 0;

	if ( ! ( hTreeView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TREEVIEWDATA *) GlobalLock ( hTreeView ) ) )
		ErrorCode = WTV_ERR_GLOBALLOCK;
	else
	{
	}	
	
	if ( pthis )
		GlobalUnlock ( hTreeView );
}

static void 
TView_OnLButtonDblClk (HWND hWnd, int x, int y, UINT keyFlags) 
{
    WORD                wItemNum;
    DWORD               dwData;
    RECT                rect;
    TVITEMDATA          tvid;
    PTREEDATA           pthis       = ( PTREEDATA )NULL;
    PTVITEMDATA         pItmList    = ( PTVITEMDATA )NULL;
    HGLOBAL             hRsrc       = ( HGLOBAL )NULL;

    assert ( hRsrc = TView_IGetResource ( hWnd, &pthis, &pItmList, NULL ) );
    if ( pthis->hwEdit )
    {
        SetWindowPos ( pthis->hwEdit, HWND_BOTTOM, 0, 0, 0, 0, 
                       SWP_HIDEWINDOW | SWP_NOSIZE | SWP_NOMOVE );
        pthis->hEditItem = ( HTREEITEM )NULL;
        KillTimer ( hWnd, STARTEDIT_TIMER ); 
    }
    GetClientRect ( hWnd, &rect );
    wItemNum = (WORD)TVLBox_GetCurSel( &pthis->psLBox );
    dwData   = (DWORD)TVLBox_GetItemData ( &pthis->psLBox, wItemNum );
    TView_IGetItem ( pItmList, ( HTREEITEM )dwData, &tvid );
    if ( tvid.tvi.mask & TVIF_CHILDREN ) 
    {
        if ( tvid.tvi.cChildren == 1 && 
             !( tvid.tvi.state & TVIS_EXPANDEDONCE ) )
        {
            NM_TREEVIEW ntv;
            PTVITEMDATA pItem   = ( PTVITEMDATA )NULL;
            UINT        idCtrl  =  GetDlgCtrlID ( hWnd );
            pItem = pItmList + ( int )dwData;
            ntv.hdr.code = TVN_ITEMEXPANDING;
            ntv.hdr.idFrom = idCtrl;
            ntv.hdr.hwndFrom = hWnd;
            ntv.action = pItem->bOpened ? TVE_COLLAPSE : TVE_EXPAND;
            ntv.itemNew = pItem->tvi;
            ntv.itemNew.hItem = ( HTREEITEM )dwData;
            ntv.ptDrag.x = x;
            ntv.ptDrag.y = y;
            pthis->psLBox.iHExtent = rect.right;
            /*TVLBox_SetHorzExtent ( hWnd, &pthis->psLBox, rect.right ); */
            if ( SendMessage ( GetParent ( hWnd ), WM_NOTIFY, idCtrl, ( LPARAM )&ntv ) == 0 )
            {
                TView_IShowSubItems ( hWnd, &pthis->psLBox, wItemNum, pItmList, ( HTREEITEM )dwData );
                ntv.hdr.code = TVN_ITEMEXPANDED;
                ntv.action = pItem->bOpened ? TVE_EXPAND : TVE_COLLAPSE;
                SendMessage ( GetParent ( hWnd ), WM_NOTIFY, idCtrl, ( LPARAM )&ntv );
            }
        }
    }

    TView_IReleaseResource ( hRsrc, pthis, &pItmList, NULL );
}

static void 
TView_OnLButtonDown 
(
    	HWND            	hWnd, 
    	UINT            	uMessage, 
    	int             	x, 
    	int             	y, 
    	UINT            	keyFlags, 
    	BOOL            	bLBtnDown
) 
{
    	HGLOBAL			hTreeView	        = ( HGLOBAL )NULL;
	TREEVIEWDATA*		pthis		        = ( TREEVIEWDATA* )NULL;

    	BOOL            	bStartEditTimer     	= FALSE;
	
	int			ErrorCode	        = 0;

	if ( ! ( hTreeView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TREEVIEWDATA *) GlobalLock ( hTreeView ) ) )
		ErrorCode = WTV_ERR_GLOBALLOCK;
	else
    	if ( IsWindowVisible ( pthis->hwEdit ) )
    	{
        	SetWindowPos ( pthis->hwEdit, HWND_BOTTOM, 0, 0, 0, 0, 
                   SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE );
        	pthis->hEditItem = ( HTREEITEM )NULL;
        	SetFocus ( hWnd );
    	}
    	else
	{
        int     iSel;
        long    dwHit;
        int     iNewSel, iHiW;
        NMHDR   nm;
        TV_HITTESTINFO  ht;

        ht.pt.x = x;
        ht.pt.y = y;

        SetFocus ( hWnd );
        iSel = TVLBox_GetCurSel ( &pthis->psLBox );
        dwHit = TVLBox_ItemFromPoint ( hWnd, &pthis->psLBox, x, y );
        iNewSel = LOWORD ( dwHit );
        iHiW = HIWORD ( dwHit );
        
        TView_OnHitTest ( hWnd, pthis, &ht );
        if ( ht.flags & TVHT_ONITEMBUTTON )  
        {
            pthis->psLBox.lCurSel = iNewSel;
            PostMessage ( hWnd, WM_LBUTTONDBLCLK, 0, 0L ); 
        }
        else
        if ( iHiW == 0 && iNewSel != -1 )
        {
            if ( iSel == iNewSel && ht.flags & TVHT_ONITEMLABEL )
                bStartEditTimer = TRUE;
            else
            if ( ht.flags & TVHT_ONITEM || pthis->psLBox.lCurSel == -1 )
                TVLBox_SetCurSel ( hWnd, pthis, iNewSel, TVC_BYMOUSE, &ht.pt  );
                /*iSel = iNewSel;*/

            if ( bStartEditTimer && pthis->uStyles & TVS_EDITLABELS )
                SetTimer ( hWnd, STARTEDIT_TIMER, 500, NULL );

        }

        pthis->ptDrag.x = x;
        pthis->ptDrag.y = y;
        pthis->enDrag = bLBtnDown ? eLBtnDown : eRBtnDown;

        nm.code = NM_CLICK;
        nm.hwndFrom = hWnd;
        nm.idFrom = GetDlgCtrlID ( hWnd );
        SendMessage ( GetParent ( hWnd ), WM_NOTIFY, ( WPARAM )hWnd, ( LPARAM )&nm );

	}	
	
	if ( pthis )
		GlobalUnlock ( hTreeView );
}

static void 
TView_OnLButtonUp (HWND hWnd, UINT uMessage, int x, int y, UINT keyFlags) 
{
    	POINT           	pt;
    	HGLOBAL			hTreeView	= ( HGLOBAL )NULL;
	TREEVIEWDATA*		pthis		= ( TREEVIEWDATA* )NULL;

	
	int				ErrorCode	= 0;

	if ( ! ( hTreeView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TREEVIEWDATA *) GlobalLock ( hTreeView ) ) )
		ErrorCode = WTV_ERR_GLOBALLOCK;
    else
    {
        if ( pthis->enDrag == eDragging && hWnd == GetCapture () )
            ReleaseCapture ( );
        if ( pthis->enDrag == eDragging )
        {
            pt.x = x;
            pt.y = y;
            MapWindowPoints ( hWnd, GetParent ( hWnd ), &pt, 1 );
            SendMessage ( GetParent ( hWnd ), uMessage , ( WPARAM )keyFlags, ( LPARAM )MAKELONG ( pt.x, pt.y ) );
        }
        pthis->enDrag = eNoDrag;
    }

	if ( pthis )
		GlobalUnlock ( hTreeView );

}

/*Need revisit this*/
static void 
TView_OnDestroy ( HWND hWnd )
{
	HGLOBAL			hTreeView	= ( HGLOBAL )NULL;
	TREEVIEWDATA*		pthis		= ( TREEVIEWDATA* )NULL;

	int			ErrorCode	= 0;

	if ( ! ( hTreeView = GlobalAlloc ( GHND, sizeof ( TREEVIEWDATA ) ) ) )
		ErrorCode = WTV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TREEVIEWDATA *) GlobalLock ( hTreeView ) ) )
		ErrorCode = WTV_ERR_GLOBALLOCK;
	else
	{
/*		DestroyWindow ( pthis->hwLBox );*/
        if ( pthis->hwEdit )
            DestroyWindow ( pthis->hwEdit );
        if ( pthis->hwTipTxt )
            DestroyWindow ( pthis->hwTipTxt );
        WLDestroy ( &pthis->ItmList );
        WLDestroy ( &pthis->StrList );
        WLDestroy ( &pthis->psLBox.ItmList );
        if ( pthis->hImgList )
            ImageList_Destroy ( pthis->hImgList );
        if ( pthis->hStImgList )
            ImageList_Destroy ( pthis->hStImgList );

        if ( pthis )
            GlobalUnlock ( hTreeView );
        REMOVETHISDATA ( hWnd );
		GlobalFree ( hTreeView );
	}
}

/* TVM_EDITLABEL handler */
static HWND 
TView_OnEditLabel ( HWND hWnd, HTREEITEM hItem ) 
{
	int             	nIndx;
    	HWND            	hwEdit      = ( HWND )NULL;
    	HGLOBAL			hRsrc	    = ( HGLOBAL )NULL;
	TREEVIEWDATA*		pthis	    = ( TREEVIEWDATA* )NULL;
	
    assert ( hRsrc = TView_IGetResource ( hWnd, &pthis, 
                            ( PTVITEMDATA *)NULL, NULL ) );
    nIndx = TVLBox_GetTopIndex ( &pthis->psLBox );
    nIndx = TVLBox_FindStringLP ( &pthis->psLBox, nIndx, ( LPARAM )hItem ); 

    if ( nIndx != LB_ERR )
    {
        TVLBox_SetCurSel ( hWnd, pthis, nIndx, TVC_UNKNOWN, NULL ); 
        SetTimer ( hWnd, STARTEDIT_TIMER, 100, NULL );
        hwEdit = pthis->hwEdit;
    }
    TView_IReleaseResource ( hRsrc, pthis, ( PTVITEMDATA *)NULL, NULL );
    return hwEdit;
}


/* TVM_ENDEDITLABELNOW handler : if fCancel is TRUE, then pass a null string
 * in item.pszText of a TV_DISPINFO in TVN_ENDLABELEDIT, otherwise pass the text
 * int the edit box. It's up to the application to call TVM_SETITEM to update
 * text of item. This handler doesn't update item text automatically on a FALSE
 * value of fCancel.
 */
static BOOL 
TView_OnEndEditLabelNow ( HWND hWnd, BOOL fCancel ) 
{
  
    	HGLOBAL		    hRsrc    	= ( HGLOBAL )NULL;
	TREEVIEWDATA*	    pthis	= ( TREEVIEWDATA* )NULL;
	PTVITEMDATA         pItmList    = ( PTVITEMDATA )NULL;
	PTVITEMDATA         pItem       = ( PTVITEMDATA )NULL;
    

    assert ( hRsrc = TView_IGetResource ( hWnd, &pthis, &pItmList, NULL ) );
    {
        int       iSel;

        char      szText[MAXSTRING];
        HTREEITEM hItem;
   


        iSel = TVLBox_GetCurSel ( &pthis->psLBox );
        hItem = ( HTREEITEM )TVLBox_GetItemData ( &pthis->psLBox, iSel );
        pItem = pItmList + ( int )hItem;
        if ( !fCancel )
            GetWindowText ( pthis->hwEdit, szText, MAXSTRING );

        {
            RECT    rc;
            LPRECT  lpRect = &rc;
            *( HTREEITEM* )lpRect = pthis->hEditItem;
            SetWindowPos ( pthis->hwEdit, HWND_BOTTOM, 0, 0, 0, 0,
                           SWP_HIDEWINDOW | SWP_NOMOVE | SWP_NOSIZE );
            TView_OnGetItemRect ( hWnd, pthis, TRUE, lpRect, NULL ); 
            pthis->hEditItem = ( HTREEITEM )NULL;
            InvalidateRect ( hWnd, lpRect, TRUE );
            SetFocus ( hWnd );
        }
        
        {
            TV_DISPINFO tvdi;
            tvdi.hdr.code = TVN_ENDLABELEDIT;
            tvdi.hdr.hwndFrom = hWnd;
            tvdi.hdr.idFrom = GetDlgCtrlID ( hWnd );
            tvdi.item = pItem->tvi;
            if ( fCancel )
                tvdi.item.pszText = ( LPSTR )NULL;
            else
                tvdi.item.pszText = szText;
            SendMessage ( GetParent ( hWnd ), WM_NOTIFY, tvdi.hdr.idFrom,
                          ( LPARAM )&tvdi );

            if ( pItem->tvi.pszText == LPSTR_TEXTCALLBACK && !fCancel )
            {
                tvdi.hdr.code = TVN_SETDISPINFO;
                SendMessage ( GetParent ( hWnd ), WM_NOTIFY, tvdi.hdr.idFrom,
                          ( LPARAM )&tvdi );
            }
        }

    }
    TView_IReleaseResource ( hRsrc, pthis, &pItmList, NULL );
    return TRUE;
}

static int 
TView_OnEraseBkground 
(
		HWND			hWnd, 
		HDC				hDC 
)
{
		RECT			rect;
		HPEN			hPen    = GetStockObject ( BLACK_PEN );
		HBRUSH			hBrush	= CreateSolidBrush ( GetSysColor ( COLOR_WINDOW ) );
	if ( hDC )
	{
		hPen = SelectObject ( hDC, hPen );
		hBrush = SelectObject ( hDC, hBrush );
		GetClientRect ( hWnd, &rect );
		Rectangle ( hDC, rect.left, rect.top,
					   rect.right, rect.bottom );
		hPen = SelectObject ( hDC, hPen );
		hBrush = SelectObject ( hDC, hBrush );
        DeleteObject ( hBrush );
		return 1;
	}

	return 0;

}

static void 
TView_OnMeasureItem
(   
    HWND                hWnd, 
    MEASUREITEMSTRUCT*  lpMeasureItem 
)
{
    TEXTMETRIC          tm;
    HDC                 hDC;
    PTREEDATA           pthis       = ( PTREEDATA )NULL;
    HGLOBAL             hRsrc       = ( HGLOBAL )NULL;

    assert ( hRsrc = TView_IGetResource ( hWnd, &pthis, NULL, NULL ) );
    hDC = GetDC ( hWnd );
    GetTextMetrics ( hDC, &tm );
    
    lpMeasureItem->itemHeight = max ( tm.tmHeight, DEF_ITEMHEIGHT );
    ReleaseDC ( hWnd, hDC );
    TView_IReleaseResource ( hRsrc, pthis, NULL, NULL );
}

static void 
TView_OnMouseMove 
(
	HWND		hWnd,
    	UINT            uMessage,
	int	        x, 
	int		y, 
	UINT		keyFlags
) 
{
    	POINT           pt;
	HGLOBAL		hTreeView	= ( HGLOBAL )NULL;
	TREEVIEWDATA*	pthis		= ( TREEVIEWDATA* )NULL;
	int		ErrorCode	= 0;
    	static BOOL     bLBtnDwn        = TRUE;

	if ( ! ( hTreeView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TREEVIEWDATA *) GlobalLock ( hTreeView ) ) )
		ErrorCode = WTV_ERR_GLOBALLOCK;
	else
    if ( ( pthis->enDrag == eLBtnDown ) ||
         ( pthis->enDrag == eRBtnDown ) )
	{
        bLBtnDwn = pthis->enDrag == eLBtnDown ? TRUE : FALSE;
        if ( !( pthis->uStyles & TVS_DISABLEDRAGDROP ) &&
              ( abs( pthis->ptDrag.x - x ) > 5 ||
                abs( pthis->ptDrag.y - y ) > 5  )
           )
             pthis->enDrag = eBeginDrag;
	}
    else
    if ( pthis->enDrag == eBeginDrag )
    {
        NM_TREEVIEW nmtv;
        PTVITEMDATA pItmList    = ( PTVITEMDATA )NULL;
        PTVITEMDATA pItem       = ( PTVITEMDATA )NULL;
        int         iSel        = TVLBox_GetCurSel ( &pthis->psLBox );
        HTREEITEM   hItem       = ( HTREEITEM )TVLBox_GetItemData ( &pthis->psLBox, iSel );

        if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItmList ) == 0 )
        {
            pItem = pItmList + ( int )hItem;
            nmtv.itemNew = pItem->tvi;
            nmtv.hdr.hwndFrom = hWnd;
            nmtv.hdr.idFrom = GetDlgCtrlID ( hWnd );
            nmtv.hdr.code = bLBtnDwn ? TVN_BEGINDRAG : TVN_BEGINRDRAG;
            nmtv.ptDrag.x = x;
            nmtv.ptDrag.y = y;
            SendMessage ( GetParent ( hWnd ), WM_NOTIFY, nmtv.hdr.idFrom, ( LPARAM )&nmtv ); 
            WLUnlock ( &pthis->ItmList, ( LPVOID )&pItmList );
        }

        pthis->enDrag = eDragging;

    }
    else
    if ( pthis->enDrag == eDragging )
    {
        pt.x = x;
        pt.y = y;
        SendMessage ( GetParent ( hWnd ), uMessage , ( WPARAM )keyFlags, 
		      ( LPARAM )MAKELONG ( pt.x, pt.y ) );
    }
    else
    {
        RECT    rc;
        LPRECT  lpRect = &rc;
        TV_HITTESTINFO ht;
        char      szText[MAXSTRING];
        ht.pt.x = x; ht.pt.y = y;
        TView_OnHitTest ( hWnd, pthis, &ht );
        if ( ht.flags & TVHT_ONITEMLABEL )
        {
            RECT    rcDad;
            *( HTREEITEM *)lpRect = ht.hItem;
            TView_OnGetItemRect ( hWnd, pthis, TRUE, lpRect, szText );
            GetClientRect ( hWnd, &rcDad );
            if ( lpRect->right >= rcDad.right && pthis->hwTipTxt )
            {
                MapWindowPoints ( hWnd, ( HWND )NULL, ( LPPOINT )lpRect, 2 ); 
                SetWindowPos ( pthis->hwTipTxt, HWND_TOP, lpRect->left, lpRect->top, 
                               lpRect->right - lpRect->left + 5, 
                               lpRect->bottom - lpRect->top + 2, 
                               SWP_SHOWWINDOW | SWP_NOACTIVATE );
                SetWindowText ( pthis->hwTipTxt, szText );
                SetCapture ( pthis->hwTipTxt );
            }
        }
    }
	
	if ( pthis )
		GlobalUnlock ( hTreeView );
}

static void 
TView_OnPaint 
(
	HWND	    	hWnd, 
	HDC		hPaintDC 
)
{
	PAINTSTRUCT     ps;
	HGLOBAL		hTreeView	= ( HGLOBAL )NULL;
	TREEVIEWDATA*	pthis		= ( TREEVIEWDATA* )NULL;
	
	int		ErrorCode	= 0;

	BeginPaint(hWnd, &ps);
	if ( !IsWindowVisible ( hWnd ) )
		goto EndTViewPaint;
	if ( ! ( hTreeView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TREEVIEWDATA *) GlobalLock ( hTreeView ) ) )
		ErrorCode = WTV_ERR_GLOBALLOCK;
	else
	{
        int                     i, Count;
        RECT                    rc;
        GetClientRect ( hWnd, &rc );
        Count = TVLBox_GetCount ( &pthis->psLBox );

        /*SetWindowOrgEx ( ps.hdc, pthis->psLBox.lOffset, 0, NULL );*/
        for ( i = pthis->psLBox.lTopIndx; i < Count && ( i - pthis->psLBox.lTopIndx ) * pthis->psLBox.ItmHeight < rc.bottom ; i++ )
        {
            TView_OnDrawItem ( hWnd, pthis, i );
        }
        TView_IDrawBorder ( hWnd, ps.hdc );
        pthis->bShow = TRUE;
        if ( pthis->psLBox.iHExtent > rc.right - rc.left )
            ShowScrollBar ( hWnd, SB_HORZ, TRUE );        
        else
            ShowScrollBar ( hWnd, SB_HORZ, FALSE );        
	}
	if ( pthis )
		GlobalUnlock ( hTreeView );
EndTViewPaint:
	EndPaint(hWnd, &ps);
}


static void 
TView_OnSetFocus ( HWND hWnd, BOOL bSetFocus ) 
{
	HGLOBAL		hRsrc	    	= ( HGLOBAL )NULL;
	TREEVIEWDATA*	pthis		= ( TREEVIEWDATA* )NULL;
	
    	assert ( hRsrc = TView_IGetResource ( hWnd, &pthis, ( PTVITEMDATA *)NULL, NULL ) );
    	pthis->psLBox.bFocus = bSetFocus;
    	if ( pthis->psLBox.lCurSel != -1 )
        	TView_OnDrawItem ( hWnd, pthis, pthis->psLBox.lCurSel );
    	TView_IReleaseResource ( hRsrc, pthis, ( PTVITEMDATA *)NULL, NULL  );
}

static void 
TView_OnTimer 
(
	HWND			hWnd, 
	int			idTimer 
)
{
	HGLOBAL			hRsrc	    = ( HGLOBAL )NULL;
	TREEVIEWDATA*		pthis	    = ( TREEVIEWDATA* )NULL;
    	PTVITEMDATA         	pItmList    = ( PTVITEMDATA )NULL;
    	PTVITEMDATA         	pItem       = ( PTVITEMDATA )NULL;
	

    assert ( hRsrc = TView_IGetResource ( hWnd, &pthis, &pItmList, NULL ) );
    {
        int       iSel;
        RECT      rcText;
        LRESULT   lStop;
        char      szText[MAXSTRING];
        HTREEITEM hItem;
        TV_DISPINFO tvdi;
        LPRECT    lpRect = &rcText;
        KillTimer ( hWnd, idTimer );

        if ( pthis->enDrag != eBeginDrag && pthis->enDrag != eDragging )
        {
        iSel = TVLBox_GetCurSel ( &pthis->psLBox );
        hItem = ( HTREEITEM )TVLBox_GetItemData ( &pthis->psLBox, iSel );

        pItem = pItmList + ( int )hItem;
        tvdi.hdr.code = TVN_BEGINLABELEDIT;
        tvdi.hdr.hwndFrom = hWnd;
        tvdi.hdr.idFrom = GetDlgCtrlID ( hWnd );
        tvdi.item = pItem->tvi;
        
        /* NOTE: why this SendMessage always returns 0 - need revisit  */
        lStop = SendMessage ( GetParent ( hWnd ), WM_NOTIFY, tvdi.hdr.idFrom,
                           ( LPARAM )&tvdi );
        if ( lStop == 0 )
        {
            *( HTREEITEM *)lpRect = ( HTREEITEM )TVLBox_GetItemData ( &pthis->psLBox, iSel );
            TView_OnGetItemRect ( hWnd, pthis, TRUE, lpRect, szText );
            SetWindowPos ( pthis->hwEdit, HWND_TOP, lpRect->left, lpRect->top, 
                           lpRect->right - lpRect->left + 5, 
                           lpRect->bottom - lpRect->top + 2, 
                           SWP_SHOWWINDOW );
            pthis->hEditItem = hItem;
            SetFocus ( pthis->hwEdit );
            SetWindowText ( pthis->hwEdit, szText );
            SendMessage ( pthis->hwEdit, EM_SETSEL, 0, -1 );

            if ( pthis->hwTipTxt && IsWindowVisible ( pthis->hwTipTxt ) )
                SetWindowPos ( pthis->hwTipTxt, HWND_BOTTOM, 0, 0, 0, 0, 
                               SWP_HIDEWINDOW | SWP_NOACTIVATE );
        }

        }
    }
    
    TView_IReleaseResource ( hRsrc, pthis, &pItmList, NULL );
}


static LRESULT WINAPI 
TView_WndProc
(
	HWND			hWnd,
        UINT			uMessage,
        WPARAM			wParam,
        LPARAM			lParam
)
{
	switch (uMessage)
	{
		case WM_COMMAND:
            		TView_OnCommand ( hWnd, (int)LOWORD(wParam), (HWND)lParam, (UINT)HIWORD(wParam));
            		return TRUE;
		case WM_CREATE:
			if ( ! TView_OnCreate ( hWnd, ( LPCREATESTRUCT )lParam ) )
				return ( LRESULT ) -1L;
			break;
		case WM_DESTROY:
			TView_OnDestroy ( hWnd );
			break;
		case WM_ERASEBKGND:
			return ( LRESULT )TView_OnEraseBkground ( hWnd, ( HDC )wParam );
			break;
		case WM_GETDLGCODE:
			return ( LRESULT )DLGC_WANTALLKEYS;

        	case WM_HSCROLL:
            		TVLBox_OnHScroll ( hWnd, ( int )LOWORD ( wParam ), ( int )HIWORD ( wParam ) );
            		return FALSE;
        
        	case WM_VSCROLL:
            		TVLBox_OnVScroll ( hWnd, ( int )LOWORD ( wParam ), ( int )HIWORD ( wParam ) );
            		return FALSE;

        	case WM_MEASUREITEM:
            		TView_OnMeasureItem(hWnd, (MEASUREITEMSTRUCT*)lParam );
            		return TRUE;
		
		case WM_KEYDOWN:
			TView_OnKeyDown ( hWnd, (UINT)( wParam ), ( int )( short )LOWORD ( lParam ), 
				(UINT)HIWORD ( lParam ) );
			break;
		
		case WM_KEYUP:
			TView_OnKeyUp ( hWnd, (UINT)( wParam ), ( int )( short )LOWORD ( lParam ), 
				(UINT)HIWORD ( lParam ) );
			break;

		case WM_KILLFOCUS:
            		TView_OnSetFocus ( hWnd, FALSE );
			break;

		case WM_LBUTTONDOWN:
			TView_OnLButtonDown ( hWnd, uMessage, (int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )wParam, TRUE );
			break;

        	case WM_LBUTTONDBLCLK:
			TView_OnLButtonDblClk ( hWnd,(int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )wParam );
			break;

        	case WM_LBUTTONUP:
			TView_OnLButtonUp ( hWnd, uMessage, (int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )wParam );
			break;

		case WM_MOUSEMOVE:
			TView_OnMouseMove ( hWnd, uMessage, (int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )wParam );
			break;
		case WM_PAINT:
			TView_OnPaint( hWnd, ( HDC ) wParam  );
			break;
		case WM_RBUTTONDOWN:
			TView_OnLButtonDown ( hWnd, uMessage, (int)(short)LOWORD(lParam), 
				( int )( short )HIWORD ( lParam ), ( UINT )wParam, FALSE );
			break;

        	case WM_SETFOCUS:
            		TView_OnSetFocus ( hWnd, TRUE );
			break;

        	case WM_TIMER:
		    	TView_OnTimer ( hWnd, ( int )wParam );
            		break;

		default:
		    if ( uMessage >= TV_FIRST && uMessage <= TV_FIRST+64 )
			    return TView_TVMWndProc(hWnd, uMessage, wParam, lParam);
		    return DefWindowProc(hWnd, uMessage, wParam, lParam);
	}
    
	    return 0;
}

static HIMAGELIST
TView_OnCreateDragImage
( 
    HWND                hWnd, 
    TREEVIEWDATA*       pthis,
    HTREEITEM           hItem
)
{
	char                szText[MAXSTRING];
    SIZE                size; 
    RECT                rc;
    HFONT               hOldFont;
    HBITMAP             hBmp        = ( HBITMAP )NULL;
    HBITMAP             hOldBmp     = ( HBITMAP )NULL;
    HDC                 hDC         = ( HDC )NULL;
    HDC                 hTopDC      = ( HDC )NULL;
    HWND                hwTop       = ( HWND )NULL;
    HIMAGELIST          hImgL       = ( HIMAGELIST )NULL;
    TVITEMDATA*         pItem       = ( TVITEMDATA* )NULL;
	TVITEMDATA*         pItmList    = ( TVITEMDATA* )NULL;

    assert ( WLLock ( &pthis->ItmList, ( LPVOID )&pItmList ) == 0 );
    pItem = pItmList + ( int )hItem;
    TView_IGetItemText ( hWnd, pthis, pItem, szText, MAXSTRING, &size );
    size.cx += pthis->ImgSize.cx;
    if ( size.cy < pthis->ImgSize.cy )
        size.cy = pthis->ImgSize.cy;
    hTopDC = GetDC ( ( hwTop = GetDesktopWindow ( ) ) );
    if ( ( hDC = CreateCompatibleDC ( hTopDC ) ) &&
         ( hBmp = CreateCompatibleBitmap ( hTopDC, size.cx, size.cy ) ) &&
         ( hImgL = ( HIMAGELIST )ImageList_Create ( size.cx, size.cy, ILC_COLOR, 10, 10 ) )
       )
    {
        int iImage = pItem->tvi.iImage;
        hOldBmp = SelectObject ( hDC, hBmp );
        hOldFont = SelectObject ( hDC, pthis->hFont );
        if ( iImage == I_IMAGECALLBACK )
        {
            TV_DISPINFO tvdi;
            int idCtrl =  GetDlgCtrlID ( hWnd );
            tvdi.hdr.code = TVN_GETDISPINFO;
            tvdi.hdr.hwndFrom = hWnd;
            tvdi.hdr.idFrom = idCtrl;
            tvdi.item = pItem->tvi;
            tvdi.item.mask = TVIF_IMAGE;
            SendMessage ( GetParent ( hWnd ), WM_NOTIFY, idCtrl, ( LPARAM )&tvdi );
            iImage = tvdi.item.iImage;
        }
        ImageList_Draw ( pthis->hImgList, iImage, hDC, 0, 0, ILD_NORMAL );
        SetRect ( &rc, pthis->ImgSize.cx, 0, size.cx, size.cy );
#if 0
        ExtTextOut ( hDC, rc.left + 1, 0, ETO_CLIPPED,
                     &rc, szText, lstrlen( szText ), NULL );
#else
        DrawText ( hDC, szText, lstrlen( szText ),
                     &rc, DT_LEFT );
#endif
        SelectObject ( hDC, hOldBmp );
        SelectObject ( hDC, hOldFont );
#if 1
        ImageList_Add ( hImgL, hBmp, ( HBITMAP )NULL );
#else
        ImageList_AddMasked ( hImgL, hBmp, RGB ( 255, 255, 255 ) );
#endif
    }

    if ( hDC )
        DeleteDC ( hDC );
    if ( hBmp )
        DeleteObject ( hBmp );
    ReleaseDC ( hwTop, hTopDC );
    WLUnlock ( &pthis->ItmList, ( LPVOID )&pItmList );
    return hImgL;
}

/*
 * delete the corresponding item on the embedded listbox, put it into a recycle 
 * pool for later reuse. Note that even after being put in the recycle pool,
 * the deleted item, if a parent node, still maintains valid FChild/LChild - only
 * that they are now also be deleted items - and valid tvi.pszText.
 */
static BOOL
TView_OnDeleteItem
( 
    HWND                hWnd, 
    TREEVIEWDATA*       pthis,
    HTREEITEM           hItem
)
{
    int                 nIndx;
    NM_TREEVIEW         ntv;
    BOOL                bRefresh    = FALSE;
    PTVITEMDATA         pItmList    = ( PTVITEMDATA )NULL;
    PTVITEMDATA         pNext       = ( PTVITEMDATA )NULL;
    PTVITEMDATA         pItem       = ( PTVITEMDATA )NULL;
    TVITEMDATA          dummy;
    TVSTR               StrDummy;

    if ( hItem == TVI_ROOT )
    {
        int i;
        int iCount = TVLBox_GetCount ( &pthis->psLBox );
        for ( i = 0; i < iCount; i++ )
            TVLBox_DeleteStringLP ( hWnd, &pthis->psLBox, i );
        WLDestroy ( &pthis->ItmList );
        WLDestroy ( &pthis->StrList );
        WLCreate ( &pthis->ItmList, sizeof ( TVITEMDATA ) );
   		WLCreate ( &pthis->StrList, sizeof ( TVSTR ) );
        memset ( &dummy, 0, sizeof ( TVITEMDATA ) );
        dummy.nLevel = -1;
        dummy.bOpened = TRUE;
        if ( pthis->uStyles & TVS_HASLINES && 
	     pthis->uStyles & TVS_LINESATROOT )
            dummy.dwConnect = 1;
        WLInsert ( &pthis->ItmList, 0, &dummy );
        memset ( &StrDummy, 0, sizeof ( TVSTR ) );
        WLInsert ( &pthis->StrList, 0, &StrDummy );
        
        pthis->lCount = 0;
        bRefresh = TRUE;
        goto refresh;
    }

    if ( hItem == 0L )
        return FALSE;
    if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItmList ) )
        return FALSE;

    nIndx = TVLBox_GetTopIndex ( &pthis->psLBox );
    if ( ( nIndx = TVLBox_FindStringLP ( &pthis->psLBox, nIndx, ( LPARAM )hItem ) ) != LB_ERR )
    {
#if 0
        TView_IDeleteItm ( hWnd, &pthis->psLBox, pItmList, hItem, nIndx );
#else
        TVLBox_DeleteStringLP ( hWnd, &pthis->psLBox, nIndx );
        bRefresh = TRUE;
#endif
    }
    
    pItem = pItmList + ( int )hItem;
    ntv.hdr.code = TVN_DELETEITEM;
    ntv.hdr.idFrom = GetDlgCtrlID ( hWnd );
    ntv.hdr.hwndFrom = hWnd;
    ntv.itemOld = pItem->tvi;
    ntv.itemOld.hItem = hItem;
    SendMessage ( GetParent ( hWnd ), WM_NOTIFY, ntv.hdr.idFrom, ( LPARAM )&ntv );

    pNext = pItmList + ( int )pItem->Parent;
    if ( pNext && pNext->FChild == hItem )
         pNext->FChild = pItem->NSibling;
    else
    if ( pNext && pNext->LChild == hItem )
    {
         pNext->LChild = pItem->PSibling;
         pNext = pItmList + ( int )pItem->PSibling;
         pNext->NSibling = ( HTREEITEM )NULL;
    }
    else
    {
        pNext = pItmList + ( int )pItem->NSibling;
        if ( pNext )
             pNext->PSibling = pItem->PSibling;
        pNext = pItmList + ( int )pItem->PSibling;
        if ( pNext )
             pNext->NSibling = pItem->NSibling;
    }
    
    pItem->PSibling = pItem->NSibling = ( HTREEITEM )NULL;
    if ( pthis->hDeleted )
    {
        PTVITEMDATA pDelete = pItmList + ( int )pthis->hDeleted;
        pDelete->PSibling = hItem;
        pItem->NSibling = pDelete->tvi.hItem;
    }
    pthis->hDeleted = hItem;

    pthis->lCount--;
    WLUnlock ( &pthis->ItmList, ( LPVOID )&pItmList );

refresh:
    if ( pthis->bShow && bRefresh )
        InvalidateRect ( hWnd, ( HWND )NULL, TRUE );
    return TRUE;
}

static BOOL
TView_OnExpand
( 
    HWND                hWnd, 
    TREEVIEWDATA*       pthis,
    UINT                uAct,
    HTREEITEM           hItem
)
{
    TVITEMDATA          tvid;
    PTVITEMDATA         pItmList    = ( PTVITEMDATA )NULL;
    BOOL                bResult     = FALSE;
    if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItmList ) )
        return FALSE;
    TView_IGetItem ( pItmList, hItem, &tvid );
#if 0	/* BUG: If you use TVM_INSERTITEM to add a child, parent's TVIF_CHILDREN is not updated */
    if ( tvid.tvi.mask & TVIF_CHILDREN ) 
    {
        if ( tvid.tvi.cChildren == 1 )
#else	/* BUG FIX */
    if (tvid.FChild != NULL)
#endif	/* BUG FIX */
        {
            PTVITEMDATA pItem   = ( PTVITEMDATA )NULL;
            int nIndx;
            pItem = pItmList + ( int )hItem;
            nIndx = TVLBox_GetCurSel ( &pthis->psLBox );
            nIndx = TVLBox_FindStringLP ( &pthis->psLBox, nIndx, ( LPARAM )hItem );  
            if ( nIndx != LB_ERR )
            {
                if ( uAct & TVE_COLLAPSE || uAct & TVE_COLLAPSERESET )
                    pItem->bOpened = TRUE;
                else
                if ( uAct & TVE_EXPAND )
                    pItem->bOpened = FALSE;
                else
                if ( uAct & TVE_TOGGLE )
                    pItem->bOpened ^= 1;
                TView_IShowSubItems ( hWnd, &pthis->psLBox, nIndx, pItmList, ( HTREEITEM )hItem );
                bResult = TRUE;
            }
        }
#if 0	/* BUG FIX */
    }
#endif	/* BUG FIX */

    WLUnlock ( &pthis->ItmList, ( LPVOID )&pItmList );
    return bResult;
}

static BOOL
TView_OnGetItem
( 
    HWND                hWnd, 
    TREEVIEWDATA*       pthis,
    TV_ITEM*            lptvi
)
{
    HTREEITEM           hItem;
    BOOL                bResult     = TRUE;
	TVITEMDATA*         pItem       = ( TVITEMDATA* )NULL;
	TVITEMDATA*         pTVItmList  = ( TVITEMDATA* )NULL;

    if ( !lptvi || !lptvi->hItem )
        return FALSE;
    if ( lptvi->mask & TVIF_TEXT &&
         ( lptvi->pszText == 0  || lptvi->pszText == LPSTR_TEXTCALLBACK )
       )
       return FALSE;
    if ( WLLock ( &pthis->ItmList, ( LPVOID )&pTVItmList ) )
        return FALSE;

    hItem = lptvi->hItem;
    pItem = pTVItmList + ( int )hItem;
    if ( lptvi->mask & TVIF_CHILDREN )
        lptvi->cChildren = pItem->tvi.cChildren;
    if ( lptvi->mask & TVIF_HANDLE )
        lptvi->hItem = pItem->tvi.hItem;
    if ( lptvi->mask & TVIF_IMAGE )
        lptvi->iImage = pItem->tvi.iImage;
    if ( lptvi->mask & TVIF_SELECTEDIMAGE )
        lptvi->iSelectedImage = pItem->tvi.iSelectedImage;
    if ( lptvi->mask & TVIF_STATE )
    {
        lptvi->stateMask = pItem->tvi.stateMask;
        lptvi->state = pItem->tvi.state;
    }
    if ( lptvi->mask & TVIF_PARAM )
        lptvi->lParam = pItem->tvi.lParam;
    if ( lptvi->mask & TVIF_TEXT )
    {
        PTVSTR  pStrList    = ( PTVSTR )NULL;
        PTVSTR  pStr        = ( PTVSTR )NULL;
        if ( pItem->tvi.pszText == LPSTR_TEXTCALLBACK )
            lptvi->pszText = LPSTR_TEXTCALLBACK;
        else
        if ( WLLock ( &pthis->StrList, ( LPVOID )&pStrList ) )
            bResult = FALSE;
        else
        {
            pStr = pStrList + ( int )pItem->tvi.pszText;
            lstrcpyn ( lptvi->pszText, ( LPSTR )pStr, 
                       min ( lptvi->cchTextMax, pItem->tvi.cchTextMax ) );
            WLUnlock ( &pthis->StrList, ( LPVOID )&pStrList );
        }
    }


    WLUnlock ( &pthis->ItmList, ( LPVOID )&pTVItmList );

    return bResult;

}


static BOOL
TView_OnGetItemRect
( 
    HWND                hWnd, 
    TREEVIEWDATA*       pthis,
    BOOL                fItemRect,
    LPRECT              lpRect,
    LPSTR               lpszText
)
{
    HTREEITEM           hItem;
    RECT                rcLBox;
    int                 nItmHeight;
    int                 nIndx;
    int                 nCount;
    BOOL                bFound      = FALSE;
	TVITEMDATA*         pItem       = ( TVITEMDATA* )NULL;
	TVITEMDATA*         pTVItmList  = ( TVITEMDATA* )NULL;

    if ( !lpRect )
        return FALSE;
    hItem = *( HTREEITEM *)lpRect;

    GetClientRect ( hWnd, &rcLBox );
    nItmHeight = TVLBox_GetItemHeight ( &pthis->psLBox );
    nIndx = TVLBox_GetTopIndex ( &pthis->psLBox );
    nCount = TVLBox_GetCount ( &pthis->psLBox );
    while ( rcLBox.top < rcLBox.bottom  && nIndx < nCount )
    {
        if ( hItem == ( HTREEITEM )TVLBox_GetItemData ( &pthis->psLBox, nIndx ) )
        {
            bFound = TRUE;
            break;
        }

        nIndx++;
        rcLBox.top += nItmHeight;
    }

    memset ( lpRect, 0, sizeof ( RECT ) );
    if ( bFound )
    {                               
        TVLBox_GetItemRect ( hWnd, &pthis->psLBox, nIndx, lpRect );
        if ( fItemRect )
        {
            char szText[MAXSTRING];
            SIZE size;
            int  nLevel;
            assert ( WLLock ( &pthis->ItmList, ( LPVOID )&pTVItmList ) == 0 );
            pItem = pTVItmList + ( int )hItem;
            if ( lpszText )
                TView_IGetItemText ( hWnd, pthis, pItem, lpszText, MAXSTRING, &size );
            else
                TView_IGetItemText ( hWnd, pthis, pItem, szText, MAXSTRING, &size );
            nLevel = ( pthis->uStyles & TVS_HASBUTTONS )? pItem->nLevel + 1 :
                                                          pItem->nLevel;

            lpRect->left += ( ( nLevel + 1 ) * pthis->nIndent + XBMPOFFSET );
            if ( pItem->tvi.stateMask & TVIS_STATEIMAGEMASK )
            {
                /*int cxBtn = 0, cyBtn = 0;*/
                /*ImageList_GetIconSize ( pthis->hStImgList, &cxBtn, &cyBtn );*/
                lpRect->left += pthis->StImgSize.cx;
            }
            lpRect->right = lpRect->left + size.cx;

            WLUnlock ( &pthis->ItmList, ( LPVOID )&pTVItmList );
        }
    }

    return bFound;

}


static HTREEITEM
TView_OnInsertItem 
( 
    HWND                hWnd, 
    TREEVIEWDATA*       pthis,
    LPTV_INSERTSTRUCT   lpTvi 
)
{


    HTREEITEM           hAfterMe    = ( HTREEITEM )-1;
    HTREEITEM           hItm        = ( HTREEITEM )NULL;
    TVITEMDATA*         pTVItmList  = ( TVITEMDATA* )NULL;
    TVITEMDATA*         pParent     = ( TVITEMDATA* )NULL;
    TVITEMDATA*         pItem       = ( TVITEMDATA* )NULL;
    TVITEMDATA*         pNext       = ( TVITEMDATA* )NULL;
    TVITEMDATA*         pInsert     = ( TVITEMDATA* )NULL;
    BOOL                bSort       = FALSE;
    int                 ErrorCode   = 0;

    hItm = TView_IAllocItmHandle ( pthis );
    if ( WLLock ( &pthis->ItmList, ( LPVOID )&pTVItmList ) )
		ErrorCode = WTV_ERR_LISTLOCK;
    else
    {
        LPSTR   pszText = ( LPSTR )NULL;
        pthis->lCount++;
        pInsert = pTVItmList + ( int )hItm; /*TVItm.tvi.hItem;*/ 

        /* if pInsert from recycled pool, then pszText is valid or LPSTR_TEXTCALLBACK 
         * and hence needs being preserved, otherwise memory leak would happen.
         */
        pszText = pInsert->tvi.pszText;
        pInsert->tvi = lpTvi->item;
        pInsert->tvi.hItem = hItm;
        pInsert->tvi.pszText = pszText;

        /* wire it up */
        if ( lpTvi->hParent == ( HTREEITEM )NULL ||
             lpTvi->hParent == TVI_ROOT )
            pParent = pTVItmList; /* the dummy first item */
        else
        {
            pParent = pTVItmList + ( int )lpTvi->hParent;
            pParent->tvi.mask |= TVIF_CHILDREN;
            pParent->tvi.cChildren = 1;
        }

        pInsert->Parent = pParent->tvi.hItem;
        pInsert->nLevel = pParent->nLevel + 1;
        pInsert->dwConnect = pParent->dwConnect;
        if ( pInsert->nLevel == 0 && 
             pthis->uStyles & TVS_HASLINES && 
             pthis->uStyles & TVS_LINESATROOT )
            pInsert->dwConnect = 1;
        if ( lpTvi->hInsertAfter == TVI_FIRST ||
             lpTvi->hInsertAfter == TVI_SORT )
        {
            if ( pParent->FChild )
            {
                pItem = pTVItmList + ( int )pParent->FChild;
                pInsert->NSibling = pItem->tvi.hItem;
                pInsert->PSibling = ( HTREEITEM )NULL;
                pItem->PSibling = pInsert->tvi.hItem;
                pParent->FChild = pInsert->tvi.hItem;
     
                if ( lpTvi->hInsertAfter == TVI_SORT )
                    bSort = TRUE;
            }
            else
            {
                pParent->FChild = pParent->LChild = pInsert->tvi.hItem;
            }

            if ( pParent->bOpened )
                hAfterMe = pParent->tvi.hItem;
        }
        else
        if ( lpTvi->hInsertAfter == TVI_LAST || 
             lpTvi->hInsertAfter == TVI_ROOT )
        {
            if ( pParent->LChild )
            {
                pItem = pTVItmList + ( int )pParent->LChild;
                pItem->NSibling = pInsert->tvi.hItem;
                pInsert->PSibling = pItem->tvi.hItem;
                pParent->LChild = pInsert->tvi.hItem;
                if ( pParent->bOpened )
                    hAfterMe = pItem->tvi.hItem;
            }
            else
            {
                pParent->FChild = pParent->LChild = pInsert->tvi.hItem;
                if ( pParent->bOpened )
                    hAfterMe = pParent->tvi.hItem;
            }
        }
        else
        {
            if ( pParent->bOpened )
                hAfterMe = lpTvi->hInsertAfter;
            pItem = pTVItmList + ( int )hAfterMe;
            if ( pItem->NSibling == ( unsigned long )NULL )
            {
                pParent = pTVItmList + ( int )pItem->Parent;
                pParent->LChild = pInsert->tvi.hItem;
                pItem->NSibling = pInsert->tvi.hItem;
            }
            else
            {
                pInsert->NSibling = pItem->NSibling;
                pNext = pTVItmList + ( int )pItem->NSibling;
                pNext->PSibling = pItem->NSibling = pInsert->tvi.hItem;
            }
            pInsert->PSibling = pItem->tvi.hItem;
        }

        /* only add to pseudolistbox if hAfterMe is there already. If hAfterMe is 0
         * , then the item is the first to be inserted into the pseudolistbox
         */
        if ( hAfterMe == 0 )
            TVLBox_InsertStringLP ( hWnd, &pthis->psLBox, 0, ( LPARAM )hItm );                 
        else
        if ( hAfterMe != ( HTREEITEM )-1 ) /*&&
            ( nIndx = TVLBox_FindStringLP ( &pthis->psLBox, 0, ( LPARAM )hAfterMe ) ) != LB_ERR )
            TVLBox_InsertStringLP ( hWnd, &pthis->psLBox, nIndx+1, hItm ); */
            TView_IInsertLBItem ( hWnd, pthis, pTVItmList, hAfterMe, hItm );

        /* get item string */
        if ( lpTvi->item.mask & TVIF_TEXT &&
             lpTvi->item.pszText != ( LPSTR )NULL &&
             lpTvi->item.pszText != LPSTR_TEXTCALLBACK )
        {
            UINT Count;
            if ( pInsert->tvi.pszText != ( LPSTR )NULL &&
                 pInsert->tvi.pszText != LPSTR_TEXTCALLBACK )
            {
             
                PTVSTR              pStrList    = ( PTVSTR )NULL;
                PTVSTR              pStr        = ( PTVSTR )NULL;
                assert ( WLLock ( &pthis->StrList, ( LPVOID )&pStrList ) == 0 );
                pStr = pStrList + ( int )pInsert->tvi.pszText;
                lstrcpyn ( ( LPSTR )pStr, lpTvi->item.pszText, 
                           min ( MAXSTRING, lpTvi->item.cchTextMax ) );
                WLUnlock ( &pthis->StrList, ( LPVOID )&pStrList );

            }
            else
            {
                char szTemp[MAXSTRING];
                lstrcpyn ( szTemp, lpTvi->item.pszText, min ( MAXSTRING, lpTvi->item.cchTextMax + 1 ) );
                WLCount ( &pthis->StrList, &Count );
                assert ( WLInsert ( &pthis->StrList, Count, &szTemp ) == 0 );
                pInsert->tvi.pszText = ( LPSTR )Count; /* offset into pthis->StrList */
            }
        }
        else
            pInsert->tvi.pszText = lpTvi->item.pszText;
    

        if ( pTVItmList )
            WLUnlock ( &pthis->ItmList, ( LPVOID )&pTVItmList );
        if ( bSort )
            TView_ISortChildren ( hWnd, lpTvi->hParent, FALSE ); 
        if ( pthis->bShow )
            InvalidateRect ( hWnd, ( HWND )NULL, TRUE );
    }
    
    return ( ErrorCode == 0? hItm : ( HTREEITEM )NULL ); 
}

static HTREEITEM
TView_OnGetNextItem
( 
    HWND                hWnd, 
    TREEVIEWDATA*       pthis,
    UINT                uNext,
    HTREEITEM           hItem
)
{
    TVITEMDATA          tvid;
    PTVITEMDATA         pItmList    = ( PTVITEMDATA )NULL;
  
    HTREEITEM           hNext       = ( HTREEITEM )NULL;
    if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItmList ) )
        return FALSE;
    TView_IGetItem ( pItmList, hItem, &tvid );
    if ( uNext == TVGN_CHILD )
    {
        if ( ( tvid.tvi.mask & TVIF_CHILDREN ) && tvid.tvi.cChildren == 1 )
            hNext = tvid.FChild;
    }
    else
    if ( uNext == TVGN_NEXT )
        hNext = tvid.NSibling;
    else
    if ( uNext == TVGN_PARENT )
        hNext = tvid.Parent;
    else
    if ( uNext == TVGN_PREVIOUS )
        hNext = tvid.PSibling;
    else
    if ( uNext == TVGN_ROOT )
    {
        if ( pthis->lCount >= 1 )
            hNext = ( HTREEITEM )1;
    }
    else
    if ( uNext == TVGN_DROPHILITE )
    {
        if ( pthis->psLBox.lDropSel != -1 )
            hNext = ( HTREEITEM )TVLBox_GetItemData ( &pthis->psLBox, pthis->psLBox.lDropSel );
    }
    else
    if ( uNext == TVGN_CARET )
    {
        long lSel = TVLBox_GetCaretIndex ( &pthis->psLBox );
        hNext = ( HTREEITEM )TVLBox_GetItemData ( &pthis->psLBox, lSel );
        if ( hNext == ( HTREEITEM )-1 )
            hNext = ( HTREEITEM )NULL;
    }
    else
    if ( uNext == TVGN_FIRSTVISIBLE )
    {
        WORD wSel = TVLBox_GetTopIndex ( &pthis->psLBox );
        hNext = ( HTREEITEM )TVLBox_GetItemData ( &pthis->psLBox, wSel );
        if ( hNext == ( HTREEITEM )-1 )
            hNext = ( HTREEITEM )NULL;
    }

    WLUnlock ( &pthis->ItmList, ( LPVOID )&pItmList );
    return hNext;
}


static int
TView_OnGetVisibleCount
( 
    HWND                hWnd, 
    TREEVIEWDATA*       pthis
)
{
    RECT                rcLBox;
    int                 nItmHeight;
    int                 nIndx;
    int                 nCount;
    int                 nVisibleCount   = 0;

    GetClientRect ( hWnd, &rcLBox );
    nItmHeight = TVLBox_GetItemHeight ( &pthis->psLBox );
    nIndx = TVLBox_GetTopIndex ( &pthis->psLBox );
    nCount = TVLBox_GetCount ( &pthis->psLBox );

    while ( rcLBox.top < rcLBox.bottom  && nIndx < nCount )
    {
        nIndx++;
        nVisibleCount++;
        rcLBox.top += nItmHeight;
    }
    
    return nVisibleCount;
}


static BOOL
TView_OnSelectItem 
(
    HWND                hWnd, 
    TREEVIEWDATA*       pthis,
    UINT                uAction,
    HTREEITEM           hItem,
    BOOL                bEnsureVisible  /* whether called in response to TVM_ENSUREVISIBLE
                                         * or TVM_SELECTITEM */
)
{
    int                 nTopIndx;
    int                 nIndx;
    BOOL                bContinue   = FALSE;
    PTVITEMDATA         pItmList    = ( PTVITEMDATA )NULL;
    PTVITEMDATA         pItem       = ( PTVITEMDATA )NULL;
    PTVITEMDATA         pParent     = ( PTVITEMDATA )NULL;

    nTopIndx = TVLBox_GetTopIndex ( &pthis->psLBox );
    nIndx = TVLBox_FindStringLP ( &pthis->psLBox, nTopIndx, ( LPARAM )hItem ); 
    if ( nIndx == LB_ERR )
    {

        if ( uAction == TVGN_DROPHILITE )
            bContinue = TRUE;
        else
        if ( uAction == TVGN_FIRSTVISIBLE )
            return FALSE;
        if ( WLLock ( &pthis->ItmList, ( LPVOID )&pItmList ) )
            return FALSE;
        pItem = pItmList + ( int )hItem;
        pParent = pItmList + ( int )pItem->Parent;
        if ( ( !pParent->bOpened ) &&
             TView_IRecursiveExpand ( hWnd, &pthis->psLBox, pItmList, pParent, &nIndx ) 
           )
        {
            nIndx = TVLBox_FindStringLP ( &pthis->psLBox, nIndx, ( LPARAM )hItem );
            if ( nIndx != LB_ERR )
                bContinue = TRUE;
        }

        WLUnlock ( &pthis->ItmList, ( LPVOID )&pItmList );
        if ( !bContinue ) return FALSE;
    }

    if ( uAction == TVGN_FIRSTVISIBLE )
    {
         TVLBox_SetTopIndex ( &pthis->psLBox, nIndx );
         return FALSE;
    }
    else
    if ( !bEnsureVisible && uAction == TVGN_CARET )
    {
         TVLBox_SetCurSel ( hWnd, pthis, nIndx, TVC_UNKNOWN, NULL );
         return FALSE;
    }
    else                                         
    if ( !bEnsureVisible && uAction == TVGN_DROPHILITE )
    {
         TVLBox_SetDropHilite ( hWnd, pthis, nIndx );
         return FALSE;
    }
    return TRUE;
}

static HIMAGELIST
TView_OnSetImageList
(
    HWND                hWnd, 
    TREEVIEWDATA*       pthis,
    int                 iImageType,
    HIMAGELIST          hIml
)
{
    HIMAGELIST          hImgList    = ( HIMAGELIST )NULL;

    if ( iImageType == TVSIL_NORMAL )
    {
        hImgList = pthis->hImgList;
        pthis->hImgList = hIml;
        ImageList_GetIconSize ( hIml, &pthis->ImgSize.cx, &pthis->ImgSize.cy );
    }
    else
    {
        hImgList = pthis->hStImgList;
        pthis->hStImgList = hIml;
        ImageList_GetIconSize ( hIml, &pthis->StImgSize.cx, &pthis->StImgSize.cy );
    }


    InvalidateRect ( hWnd, ( RECT* )NULL, TRUE );
    return hImgList;
}

static int
TView_OnSetItem
( 
    HWND                hWnd, 
    TREEVIEWDATA*       pthis,
    TV_ITEM*            lptvi
)
{
    HTREEITEM           hItem;
    int                 nIndx;
    int                 iResult     = 0;
	TVITEMDATA*         pItem       = ( TVITEMDATA* )NULL;
	TVITEMDATA*         pTVItmList  = ( TVITEMDATA* )NULL;

    if ( !lptvi || !lptvi->hItem )
        return -1;
    hItem = lptvi->hItem;
    if ( WLLock ( &pthis->ItmList, ( LPVOID )&pTVItmList ) )
        return -1;
    pItem = pTVItmList + ( int )hItem;
    if ( lptvi->mask & TVIF_IMAGE )
        pItem->tvi.iSelectedImage = pItem->tvi.iImage = lptvi->iImage;
    if ( lptvi->mask & TVIF_SELECTEDIMAGE )
        pItem->tvi.iSelectedImage = lptvi->iSelectedImage;
    if ( pItem->tvi.iSelectedImage < 0 )
        pItem->tvi.iSelectedImage = -1;
    if ( pItem->tvi.iImage < 0 )
        pItem->tvi.iImage = -1;

    if ( lptvi->mask & TVIF_STATE )
    {
        pItem->tvi.state |= lptvi->stateMask;
        pItem->tvi.state &= ( ~lptvi->stateMask | lptvi->state ); 
    }
    if ( lptvi->mask & TVIF_PARAM )
        pItem->tvi.lParam = lptvi->iImage;
    if ( lptvi->mask & TVIF_TEXT )
    {
        if ( !lptvi->pszText )
            iResult = -1;
        else
        {
            char szText[MAXSTRING];
            PTVSTR  pStrList    = ( PTVSTR )NULL;
            PTVSTR  pStr        = ( PTVSTR )NULL;
            if ( WLLock ( &pthis->StrList, ( LPVOID )&pStrList ) )
                iResult = -1;
            else
            {
                if ( pItem->tvi.pszText == LPSTR_TEXTCALLBACK )
                {
                    UINT Index;
                    WLCount ( &pthis->StrList, &Index );
                    WLInsert ( &pthis->StrList, Index, szText);
                    pItem->tvi.pszText = ( LPSTR )Index;
                    pStr = pStrList + Index;
                }
                else
                    pStr = pStrList + ( int )pItem->tvi.pszText;

                lstrcpyn ( ( LPSTR )pStr, lptvi->pszText, 
                           min ( lstrlen ( lptvi->pszText ) + 1, MAXSTRING ) );
                WLUnlock ( &pthis->StrList, ( LPVOID )&pStrList );
            }
        }
    }


    WLUnlock ( &pthis->ItmList, ( LPVOID )&pTVItmList );
    nIndx = TVLBox_GetTopIndex ( &pthis->psLBox );
    if ( ( nIndx = TVLBox_FindStringLP ( &pthis->psLBox, 
                                         nIndx, ( LPARAM )hItem ) ) >= 0 ) 
        TView_OnDrawItem ( hWnd, pthis, nIndx );
    return iResult;

}


static BOOL
TView_OnSortChildren
( 
    HWND                hWnd, 
    TREEVIEWDATA*       pthis,
    HTREEITEM           hItem,
    LPTV_SORTCB         lpSortCB
)
{
    HGLOBAL             hGlobal;
    int                 i;
    int                 Count       = 0;
    BOOL                bExpand     = FALSE;
    PTVSORTITEM         pSortAr     = ( PTVSORTITEM )NULL;
	TVITEMDATA*         pItem       = ( TVITEMDATA* )NULL;
	TVITEMDATA*         pChild      = ( TVITEMDATA* )NULL;
	TVITEMDATA*         pTVItmList  = ( TVITEMDATA* )NULL;

    if ( WLLock ( &pthis->ItmList, ( LPVOID )&pTVItmList ) )
        return FALSE;
    pItem = pTVItmList + ( int )hItem;
    if ( pItem->tvi.cChildren == 1 )
    {
        pChild = pTVItmList + ( int )pItem->FChild; 
        Count = 1;
        while ( pChild->NSibling )
        {
            Count++;
            pChild = pTVItmList + ( int )pChild->NSibling;
        }

        if ( ( hGlobal = GlobalAlloc ( GHND, ( Count + 1 ) * sizeof ( TVSORTITEM ) ) ) &&
             ( pSortAr = ( PTVSORTITEM )GlobalLock ( hGlobal ) ) )
        {
            SIZE size;
            pChild = pTVItmList + ( int )pItem->FChild; 
            bExpand = pItem->bOpened;
            if ( bExpand )
                TView_OnExpand ( hWnd, pthis, TVE_COLLAPSE, hItem );
            for ( i = 1; i <= Count; i++ )
            {
                pSortAr[i].Ord = i;
                pSortAr[i].hItem = pChild->tvi.hItem;
                if ( lpSortCB )
                    pSortAr[i].lParam = pChild->tvi.lParam;
                else
                    TView_IGetItemText ( hWnd, pthis, pChild, pSortAr[i].szText, MAXSTRING, &size );
                pChild = pTVItmList + ( int )pChild->NSibling;
            }
#if 1
            TView_IDoHeapSort ( pSortAr, Count, lpSortCB );
#else
            TView_IDoShellSort ( pSortAr, Count );
#endif

            pItem->FChild = pSortAr[pSortAr[1].Ord].hItem;
            pItem->LChild = pSortAr[pSortAr[Count].Ord].hItem;
            for ( i = 1; i <= Count; i++ )
            {
                pChild = pTVItmList + ( int )pSortAr[pSortAr[i].Ord].hItem;
                pChild->NSibling = ( i == Count ? ( HTREEITEM )NULL :
                                                pSortAr[pSortAr[i+1].Ord].hItem );
                pChild->PSibling = pSortAr[pSortAr[i-1].Ord].hItem;
            }

            GlobalUnlock ( hGlobal );
            GlobalFree ( hGlobal );
        }
    }

    WLUnlock ( &pthis->ItmList, ( LPVOID )&pTVItmList );
    if ( bExpand )
        TView_OnExpand ( hWnd, pthis, TVE_EXPAND, hItem );
    return TRUE;
}


static LRESULT  
TView_TVMWndProc
(
	HWND			hWnd,
    UINT			uMessage,
    WPARAM			wParam,
    LPARAM			lParam
)
{
	HGLOBAL			hTreeView	= ( HGLOBAL )NULL;
	TREEVIEWDATA*	pthis		= ( TREEVIEWDATA* )NULL;


	
	int			ErrorCode	= 0;

	if ( ! ( hTreeView = GETTHISDATA ( hWnd ) ) )
		ErrorCode = WTV_ERR_GETWINPROP;
	else
	if ( ! ( pthis = ( TREEVIEWDATA *) GlobalLock ( hTreeView ) ) )
		ErrorCode = WTV_ERR_GLOBALLOCK;
	else
	{
		switch (uMessage)
		{
			case TVM_CREATEDRAGIMAGE:
                return ( LRESULT )TView_OnCreateDragImage ( hWnd, pthis, ( HTREEITEM )lParam );
                break;
            case TVM_DELETEITEM:
                return ( LRESULT )TView_OnDeleteItem ( hWnd, pthis, ( HTREEITEM )lParam );
            case TVM_EDITLABEL:
                return ( LRESULT )TView_OnEditLabel ( hWnd, ( HTREEITEM )lParam ); 
            case TVM_ENDEDITLABELNOW:
                return ( LRESULT )TView_OnEndEditLabelNow ( hWnd, ( BOOL )wParam );
            case TVM_ENSUREVISIBLE:
                return ( LRESULT )TView_OnSelectItem ( hWnd, pthis, TVGN_CARET, ( HTREEITEM )lParam, TRUE );
            case TVM_EXPAND:
                return ( LRESULT )TView_OnExpand ( hWnd, pthis, ( UINT )wParam, ( HTREEITEM )lParam );
            case TVM_GETCOUNT:
                return ( LRESULT )pthis->lCount;
            case TVM_GETEDITCONTROL:
                return ( LRESULT )pthis->hwEdit;
            case TVM_GETIMAGELIST:
                {
                    if ( wParam == TVSIL_NORMAL )
                        return ( LRESULT )pthis->hImgList;
                    else
                        return ( LRESULT )pthis->hStImgList;
                }
            case TVM_GETINDENT:
                return  pthis->nIndent;
            case TVM_GETISEARCHSTRING:
                break;
            case TVM_GETITEM:
                return ( LRESULT )TView_OnGetItem ( hWnd, pthis, ( TV_ITEM* )lParam );
            case TVM_GETITEMRECT:
                return ( LRESULT )TView_OnGetItemRect ( hWnd, pthis, ( BOOL )wParam, ( LPRECT )lParam, NULL );
            case TVM_GETNEXTITEM:
                return ( LRESULT )TView_OnGetNextItem ( hWnd, pthis, ( UINT )wParam, ( HTREEITEM )lParam );
            case TVM_GETVISIBLECOUNT:
                return ( LRESULT )TView_OnGetVisibleCount ( hWnd, pthis );
            case TVM_HITTEST:
                return ( LRESULT )TView_OnHitTest ( hWnd, pthis, ( LPTV_HITTESTINFO )lParam );
            case TVM_INSERTITEM:
                return ( LRESULT )TView_OnInsertItem ( hWnd, pthis, ( LPTV_INSERTSTRUCT )lParam );  
                break;
            case TVM_SELECTITEM:
                return ( LRESULT )TView_OnSelectItem ( hWnd, pthis, ( UINT )wParam, ( HTREEITEM )lParam, FALSE );
            case TVM_SETIMAGELIST:
                return ( LRESULT )TView_OnSetImageList ( hWnd, pthis, ( int )wParam, ( HIMAGELIST )lParam );  
            case TVM_SETINDENT:
                pthis->nIndent = ( int )wParam;
                InvalidateRect ( hWnd, ( HWND )NULL, TRUE );
                break;
            case TVM_SETITEM:
                return ( LRESULT )TView_OnSetItem ( hWnd, pthis, ( TV_ITEM* )lParam );
            case TVM_SORTCHILDREN:
                return ( LRESULT )TView_OnSortChildren ( hWnd, pthis, 
                       ( HTREEITEM )lParam, ( LPTV_SORTCB )NULL );
            case TVM_SORTCHILDRENCB:
                return ( LRESULT )TView_OnSortChildren ( hWnd, pthis, 
                       ( ( LPTV_SORTCB )lParam )->hParent, ( LPTV_SORTCB )lParam );
				break;
			default:
			    return DefWindowProc(hWnd, uMessage, wParam, lParam);
		}
	}

	if ( pthis )
		GlobalUnlock ( hTreeView );

	return 0;
}


int WINAPI 
TView_Initialize 
(
	HINSTANCE		hinst
) 
{

	WNDCLASS		wc;
	int			ErrorCode	= 0;

	memset ( &wc, 0, sizeof ( WNDCLASS ) );
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS | CS_GLOBALCLASS;
	wc.lpfnWndProc   = TView_WndProc;
	wc.cbWndExtra    = 0; /*sizeof( TView_WNDEXTRABYTES );*/
	wc.hInstance     = hinst;
	wc.hCursor       = LoadCursor( ( HINSTANCE )NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject ( WHITE_BRUSH );
	wc.lpszClassName = WWC_TREEVIEW;

	if ( ! RegisterClass ( &wc ) )
		ErrorCode = WTV_ERR_REGISTERCLASS;

#if 1
    memset ( &wc, 0, sizeof ( WNDCLASS ) );
	wc.style         = CS_SAVEBITS | CS_HREDRAW | CS_CLASSDC | CS_GLOBALCLASS;
	wc.lpfnWndProc   = TViewTip_WndProc;
	wc.cbWndExtra    = sizeof( HFONT );
	wc.hInstance     = hinst;
	wc.hCursor       = LoadCursor( ( HINSTANCE )NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject ( WHITE_BRUSH );
	wc.lpszClassName = WC_TVTIPTEXT;

	if ( ! RegisterClass ( &wc ) )
		ErrorCode = WTV_ERR_REGISTERCLASS;
#endif

    return ( ErrorCode );
}

void WINAPI TView_Terminate 
(
	HINSTANCE			hInstance
) 
{
	WNDCLASS			WndClass;


	if ( GetClassInfo ( hInstance, WWC_TREEVIEW, &WndClass ) )
		UnregisterClass ( WWC_TREEVIEW, hInstance );

}





