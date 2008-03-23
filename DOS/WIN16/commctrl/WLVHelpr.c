/*  
	WLVHelpr.c	1.5 
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

#include "windows.h"
#include "commctrl.h"
#include "WLists.h"
#include "WLView.h"

#define     DEF_COLWIDTH    20

void 
LVCols_Init 
(
    PLVIEWCOLS      lpLVCols
)
{
    LVIEWCOL        dummy;
    WLCreate ( &lpLVCols->Columns, sizeof ( LVIEWCOL ) );
    WLInsert ( &lpLVCols->Columns, 0, &dummy );
    lpLVCols->lCount = 1;
}

void 
LVCols_Destroy
(
    PLVIEWCOLS      lpLVCols
)
{
    int             i;
    PLVIEWCOL       pCol;
    PLVIEWCOL       pCols;

    if ( WLLock ( &lpLVCols->Columns, ( LPVOID )&pCols ) )
        return;
    for ( i = 0; i < lpLVCols->lCount; i++ )
    {
        pCol = pCols + i;
        WLDestroy ( &pCol->ColItms );
    }

    WLUnlock ( &lpLVCols->Columns, ( LPVOID )NULL );
    WLDestroy ( &lpLVCols->Columns );
}


int
LVCols_InsertCol 
( 
    PLVIEWCOLS          lpCols,
    int                 iCol,
    LV_COLUMN*          lpLCol 
)
{
    UINT                 Count = 0;
    LVIEWCOL            newCol;
    
    WLCount ( &lpCols->Columns, &Count );
    if ( iCol > Count )
        iCol = Count;
    memset ( &newCol, 0, sizeof ( LVIEWCOL ) );
    /*newCol.ColInfo = *lpLCol;*/
    newCol.ColWidth = DEF_COLWIDTH;
    if ( lpLCol->mask & LVCF_WIDTH )
        newCol.ColWidth = lpLCol->cx;
    if ( WLCreate ( &newCol.ColItms, sizeof ( LVCOLITEM ) ) ||
         WLInsert ( &lpCols->Columns, iCol, &newCol ) )
        return -1;
    lpCols->lCount++;
    return iCol;
}

BOOL
LVCols_LocateSubItem
(
    PLVIEWCOL           lpCols,
    PLVITEMPATH         pSubItmPath,
    BOOL                bCreate
)
{
    LVCOLITEM           colItm;
    UINT                Count;
    int                 iCurCol, iCurRow, iNxtCol;
    int                 iCol        = pSubItmPath->lCol;
    PLVIEWCOL           lpCol       = ( PLVIEWCOL )NULL;
    PLVIEWCOL           lpNxtCol    = ( PLVIEWCOL )NULL;
    PLVCOLITEM          lpColItms   = ( PLVCOLITEM )NULL;
    PLVCOLITEM          lpNxtColItm = ( PLVCOLITEM )NULL;
    PLVCOLITEM          lpColItm    = &pSubItmPath->lpRow->firstCol;

    memset ( &colItm, 0, sizeof ( LVCOLITEM ) );
    colItm = *lpColItm;
    iCurCol = iNxtCol = 0;
    iCurRow = pSubItmPath->lRow; 
    while ( TRUE )
    {
        if ( iNxtCol == iCol )
            goto Found;
        if ( iNxtCol > iCol ) 
            break;
        {
            if ( ( iNxtCol = colItm.NSCol ) == 0 )
                break;
            lpNxtCol = lpCols + iNxtCol;
            WLLock ( &lpNxtCol->ColItms, ( LPVOID )&lpColItms );
            lpNxtColItm = lpColItms + colItm.NSItm;
            iCurRow = colItm.NSItm; /* dangerous */
            colItm = *lpNxtColItm;
            WLUnlock ( &lpNxtCol->ColItms, ( LPVOID )&lpColItms );
            iCurCol = iNxtCol;
        }
    }
    
    if ( !bCreate )
        return FALSE;

    lpNxtCol = lpCols + iCol;
    WLCount ( &lpNxtCol->ColItms, &Count );
    memset ( &colItm, 0, sizeof ( LVCOLITEM ) );
    WLInsert ( &lpNxtCol->ColItms, Count, &colItm );
    iNxtCol = iCol;
    
    if ( iCurCol == 0 )
    {
        lpColItm = &pSubItmPath->lpRow->firstCol;
        lpColItm->NSItm = Count;
        lpColItm->NSCol = iCol;
    }
    else
    {
        lpCol = lpCols + iCurCol;
        WLLock ( &lpCol->ColItms, ( LPVOID )&lpColItms );
        lpColItm = lpColItms + iCurRow;
#if 1
        if ( lpColItm->NSCol )
        {
            PLVCOLITEM  lpTempColItms = ( PLVCOLITEM )NULL;
            lpNxtCol = lpCols + iCol;
            WLLock ( &lpCol->ColItms, ( LPVOID )&lpTempColItms );
            lpNxtColItm = lpTempColItms + Count;
            lpNxtColItm->NSItm = lpColItm->NSItm;
            lpNxtColItm->NSCol = lpColItm->NSCol;
            WLUnlock ( &lpCol->ColItms, ( LPVOID )NULL );
        }
#endif
        lpColItm->NSItm = Count;
        lpColItm->NSCol = iCol;
        WLUnlock ( &lpNxtCol->ColItms, ( LPVOID )&lpColItms );
    }
    iCurCol = iCol;
    iCurRow = Count;

Found:
    if ( iCurCol == 0 )
    {
        pSubItmPath->lpSubItm = &pSubItmPath->lpRow->firstCol;
    }
    else
    {
        pSubItmPath->lpCol = lpCols + iCurCol;
        /*pSubItmPath->lpCol = lpCols + pSubItmPath->lCol;*/
        WLLock ( &pSubItmPath->lpCol->ColItms, ( LPVOID )&lpColItms );
        pSubItmPath->lpSubItm = lpColItms + iCurRow;
    }
    return TRUE;
}

/* need enhancement */
LPSTR
LVRows_Alloc
(
    PLVIEWROWS      lpRows,
    int             size
)
{
    return ( LPSTR )WinMalloc ( size );
}


int
LVReport_BSearch
(
    PLVIEWROW       pRows,
    PLVSORTITEM     pSortRows,
    long            Count,
    HLVITEM         hItem,
    UINT            style
)
{
    PLVIEWROW       pRowX;
    PLVIEWROW       pInsert;
    PLVSORTITEM     pSortRow;
    int             l   = 0;
    int             r   = Count - 1;
    int             x   = 0;
    int             iCompare;

    pInsert = pRows + ( int )hItem;
    while ( r >= l )
    {
        x = ( r + l ) / 2;
        pSortRow = pSortRows + x;
        pRowX = pRows + ( int )pSortRow->hItm;

        iCompare = lstrcmp ( pRowX->lvItm.pszText,
                             pInsert->lvItm.pszText );
        if (((iCompare >= 0) && (style & LVS_SORTASCENDING)) ||
            ((iCompare <= 0) && (style & LVS_SORTDESCENDING )))
            return x;
        if ( iCompare < 0 && style & LVS_SORTASCENDING )
            l = x + 1;
        else
        if ( iCompare > 0 && style & LVS_SORTDESCENDING )
            r = x - 1;

    }

    return -1;
}

#if 0
int
LVRows_Connect
(
    PLVIEWROWS      lpRows,
    HLVITEM         hItem,
    UINT            style
)
{
    int             indx;
    PLVIEWROW       pItmList    = ( PLVIEWROW )NULL;
    PLVIEWROW       pItm        = ( PLVIEWROW )NULL;
    PLVSORTITEM     pSortList   = ( PLVSORTITEM )NULL;
    LVSORTITEM      Sort;

    if ( WLLock ( &lpRows->RowItms, ( LPVOID )&pItmList ) ||
         WLLock ( &lpRows->SortedRows, ( LPVOID )&pSortList )
       )
       return -1;
    memset ( &Sort, 0, sizeof ( LVSORTITEM ) ); 
    Sort.hItm = hItem;
    indx = LVRows_BSearch ( pItmList, pSortList, lpRows->lCount, hItem, style );
    
    WLUnlock ( &lpRows->RowItms, ( LPVOID )&pItmList );
    WLUnlock ( &lpRows->SortedRows, ( LPVOID )&pSortList );
    
    WLInsert ( &lpRows->SortedRows, indx, &Sort );
    return indx;
}
#endif

void 
LVRows_Init 
(
    PLVIEWROWS      lpLVRows
)
{
    WLCreate ( &lpLVRows->RowItms, sizeof ( LVIEWROW ) );
    WLCreate ( &lpLVRows->SortedRows, sizeof ( LVSORTITEM ) );
    lpLVRows->lCount = 0;
}

int
LVRows_SetRowItm
(
    PLVIEWROWS      lpRows,
    int             iItem,
    LV_ITEM*        lplvi,
    UINT            style
)
{
    int             indx;
    LVSORTITEM      Sort;
    PLVIEWROW       pItmList    = ( PLVIEWROW )NULL;
    PLVIEWROW       pItm        = ( PLVIEWROW )NULL;
    PLVSORTITEM     pIndxs      = ( PLVSORTITEM )NULL;
    BOOL            bCont       = TRUE;
    
    if ( WLLock ( &lpRows->RowItms, ( LPVOID )&pItmList ) ||
         WLLock ( &lpRows->SortedRows, ( LPVOID )&pIndxs )
       )
       return -1;

    pItm = pItmList + iItem;
    pItm->lvItm = *lplvi;
    pItm->Above = pItm->Below = pItm->Left = pItm->Right = -1;
    pItm->rcItem.left = pItm->rcItem.top = -1;
    if ( lplvi->mask & LVIF_TEXT && 
         lplvi->pszText != LPSTR_TEXTCALLBACK )
    {
        pItm->lvItm.pszText = LVRows_Alloc ( lpRows, lstrlen ( lplvi->pszText )+1 );
        if ( pItm->lvItm.pszText )
            lstrcpy ( pItm->lvItm.pszText, lplvi->pszText ); 
        else
            bCont = FALSE;
    }
    pItm->firstCol.pszText = pItm->lvItm.pszText;

    if ( bCont )
    {
        memset ( &Sort, 0, sizeof ( LVSORTITEM ) ); 
        Sort.hItm = iItem;
        Sort.lParam = lplvi->lParam;
        if ( style & LVS_SORTASCENDING ||
             style & LVS_SORTDESCENDING  )
            indx = LVReport_BSearch ( pItmList, pIndxs, 
                                lpRows->lCount, iItem, style );
        else
            indx = lplvi->iItem;   /*iItem;*/
    }

    WLUnlock ( &lpRows->RowItms, ( LPVOID )&pItmList );
    WLUnlock ( &lpRows->SortedRows, ( LPVOID )&pIndxs );
    if ( bCont && indx != -1 )
        indx = WLInsertEx ( &lpRows->SortedRows, indx, &Sort );
    return indx;
}

