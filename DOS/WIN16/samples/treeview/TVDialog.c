/*************************************************************************
*
*	TVDialog.c
*
*	@(#)TVDialog.c	1.3
*
*	Copyright (c) 1995-1997, Willows Software Inc.  All rights reserved.
*
**************************************************************************/

#include "windows.h"
#include "commctrl.h"
#include "TViewApp.h"

#include <math.h>
#include "resource.h"

#define STRING_LENGTH   40
#define LEVEL0_IMAGE    0
#define LEVEL1_IMAGE    1
#define LEVEL2_IMAGE    2
#define LEVEL_IMAGES    3
#define BITMAP_WIDTH    16
#define BITMAP_HEIGHT   16
#define NUM_BITMAPS     13
#define OVERLAY_INDEX   12

BOOL g_fDragging = FALSE;

long CALLBACK TVDialog_DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL TVDialog_OnInitDialog(HWND hwndDlg, HWND hwndFocus, LPARAM lParam);
VOID TVDialog_OnCommand(HWND hwndDlg, UINT id, HWND hwndCtl, UINT code);
LRESULT TVDialog_OnNotify ( HWND hwndDlg, UINT idCtrl, LPARAM lParam );


#define ROWS 1
#define COLS 12

#define MAXBUF  96
#define MAXKIDS 6

typedef struct
{
    int         iImage;
    LPARAM      lParam;
    UINT        idStr;
} TVITEMINFO, *PTVITEMINFO;


TVITEMINFO g_ItmInfo[MAXKIDS*2+1];
HWND       g_hMDlg = ( HWND )NULL; 
HTREEITEM  g_hItem = ( HTREEITEM )NULL;


int TVDialog_Do(HWND hwndOwner, UINT DlgTemplateID )
{
    if ( !g_hMDlg )
    {
        g_hMDlg = CreateDialog(_hInstance,
            MAKEINTRESOURCE(DlgTemplateID),
            hwndOwner, TVDialog_DlgProc);
        ShowWindow ( g_hMDlg, TRUE );
    }

    return 0;
}


void
TVDialog_DropItem
(
    HWND            hwnd,
    HTREEITEM       hDragItem 
)
{
	HTREEITEM hParent, hNewItem, hTarget;
	TV_ITEM tvTarget;
	TV_INSERTSTRUCT tvis;
    	char buffer[STRING_LENGTH];

	hTarget = TreeView_GetDropHilight(hwnd);
					
	hParent = TreeView_GetParent(hwnd, hTarget);

	tvTarget.hItem = hDragItem;
    	tvTarget.pszText = buffer;
    	tvTarget.cchTextMax = STRING_LENGTH;
	tvTarget.mask = TVIF_CHILDREN | TVIF_STATE | TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	TreeView_GetItem(hwnd, &tvTarget);
		
    	tvis.item = tvTarget;
	tvis.hInsertAfter = hTarget;
	tvis.hParent = hParent;
	
	hNewItem = (HTREEITEM)SendMessage(hwnd, TVM_INSERTITEM, 0, 
		(LPARAM)(LPTV_INSERTSTRUCT)&tvis);

	TreeView_DeleteItem(hwnd, hDragItem);

	TreeView_SelectDropTarget(hwnd, (HTREEITEM)NULL);
}


long CALLBACK TVDialog_DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hWndTreeView;   
	TV_HITTESTINFO tvHit;       
	static HTREEITEM hDragItem;
	static HTREEITEM hTarget   = ( HTREEITEM )NULL;

    switch (msg)
    {
        case WM_INITDIALOG:
             TVDialog_OnInitDialog(hwndDlg, (HWND)(wParam), lParam);
             hWndTreeView = GetDlgItem ( hwndDlg, IDC_TREEVIEW );
             return 1;
        break;

        case WM_COMMAND:
             TVDialog_OnCommand(hwndDlg, (int)LOWORD(wParam), (HWND)lParam, (UINT)HIWORD(wParam));
             return 1;
             break;

        case WM_NOTIFY:
            {
                long    lResult = 1;
                if ( ( ( NMHDR *)lParam )->code == TVN_BEGINDRAG)
                    hDragItem = ((NM_TREEVIEW *)lParam)->itemNew.hItem;    
                lResult = TVDialog_OnNotify ( hwndDlg, ( int )wParam, lParam ); 
                return lResult;
            }

        case WM_MOUSEMOVE:
			/* if dragging, move the image */
			if (g_fDragging)
			{
				HTREEITEM hOldTarget = hTarget;

                /* if the cursor is on an item, hilite it as*/
				/* the drop target */
				tvHit.pt.x = LOWORD(lParam);
				tvHit.pt.y = HIWORD(lParam);
				if ((hTarget = TreeView_HitTest(hWndTreeView, &tvHit)) != NULL)
                {
					TreeView_SelectDropTarget(hWndTreeView, hTarget);
				    if (hOldTarget && hOldTarget != hTarget)
                    {
                        RECT    rc;
                        TreeView_GetItemRect(hWndTreeView, hOldTarget, &rc, FALSE );
                        InvalidateRect(hWndTreeView, &rc, TRUE );
                        UpdateWindow(hWndTreeView);
                    }
                }

                if ( !ImageList_DragMove(LOWORD(lParam),HIWORD(lParam)) )
                    OutputDebugString ( "ImageList_DragMove failed\n" );
			}
			break;
		case WM_LBUTTONDOWN:
            OutputDebugString ( "wm_lbuttondown\n" );
            break;

		case WM_LBUTTONUP:
			/* If dragging, stop it. */
			if (g_fDragging)
			{
				/* Process the item drop.*/
				TVDialog_DropItem( hWndTreeView, hDragItem ); 
				
				/* Inform the image list that dragging has stopped. */
				ImageList_EndDrag();

				/* Release the mouse. */
				ReleaseCapture();

				/* Show the cursor. */
				ShowCursor(TRUE);

				/* Reset the global Boolean flag to a nondragging state. */
				g_fDragging = FALSE;

				   
			}
			break;

        case WM_CHARTOITEM:
             if ( wParam == VK_RETURN ) 
             {
                WORD   wItemNum;
                DWORD  dwData;
                HWND   hwndCtl;

                hwndCtl = GetDlgItem(hwndDlg, IDLIST);
                wItemNum = (WORD)  SendMessage(hwndCtl, LB_GETCURSEL, 0, 0L);
                dwData   = (DWORD) SendMessage(hwndCtl, LB_GETITEMDATA, wItemNum, 0L);
                return 1;
             }
        break;

    }
    return 0;
}

BOOL TVDialog_OnInitDialog(HWND hwndDlg, HWND hwndFocus, LPARAM lParam)
{
    int  i;
    TV_INSERTSTRUCT tvi;
    char szText[MAXBUF];
    HIMAGELIST  hIml, hStateImgL;
    HBITMAP hBmp;
    HTREEITEM   hParent;
    HWND hwTree = GetDlgItem(hwndDlg, IDC_TREEVIEW);
    HINSTANCE hInst = ( HINSTANCE )GetWindowLong ( hwndDlg, GWL_HINSTANCE );

    /*srand( (unsigned)time( NULL ) );*/

    memset ( &tvi, 0, sizeof ( TV_INSERTSTRUCT ) );
    tvi.hParent = TVI_ROOT;
    tvi.hInsertAfter = TVI_LAST;
    tvi.item.mask = TVIF_CHILDREN | TVIF_TEXT | TVIF_IMAGE | 
                    TVIF_SELECTEDIMAGE | TVIF_PARAM | TVIF_STATE;
    tvi.item.stateMask = TVIS_STATEIMAGEMASK ;
    tvi.item.state = INDEXTOSTATEIMAGEMASK( 1 );
    tvi.item.iImage = I_IMAGECALLBACK;
    tvi.item.iSelectedImage = I_IMAGECALLBACK;
    tvi.item.cChildren = 0; /*I_CHILDRENCALLBACK;*/
    g_ItmInfo[0].lParam = tvi.item.lParam = -1;  /* ROOT level */
    g_ItmInfo[0].iImage = LEVEL0_IMAGE;
    for ( i = 0; i < 9; i++ ) 
    {
       LoadString ( hInst, IDS_ROOTITEM1 + i, szText, MAXBUF );
       tvi.item.pszText = szText;
       tvi.item.cchTextMax = lstrlen ( szText ) + 1;
       if ( i == 8 )
       {
            tvi.item.cChildren = 1;
            hParent = ( HTREEITEM ) SendMessage(hwTree, TVM_INSERTITEM, 0, (LONG)&tvi);
       }
       else
            SendMessage(hwTree, TVM_INSERTITEM, 0, (LONG)&tvi);
       tvi.item.cChildren = 0;
    }

    tvi.hParent = hParent; 
    tvi.item.iImage = I_IMAGECALLBACK;
    tvi.item.iSelectedImage = I_IMAGECALLBACK;
    tvi.item.pszText = LPSTR_TEXTCALLBACK;
    for ( i = 1; i <= MAXKIDS; i++ ) 
    {
       tvi.item.cChildren = 0;
       if ( i == 6 )
           tvi.item.cChildren = 1;
       tvi.item.lParam = g_ItmInfo[i].lParam = rand();
       if ( i == 6 )
           hParent = ( HTREEITEM )SendMessage(hwTree, TVM_INSERTITEM, 0, (LONG)&tvi);
       else
           SendMessage(hwTree, TVM_INSERTITEM, 0, (LONG)&tvi);
       g_ItmInfo[i].idStr = IDS_KID10 + i - 1;
       g_ItmInfo[i].iImage = LEVEL1_IMAGE;
    }

    tvi.hParent = hParent; 
    tvi.item.iImage = I_IMAGECALLBACK;
    tvi.item.iSelectedImage = I_IMAGECALLBACK;
    tvi.item.pszText = LPSTR_TEXTCALLBACK;
    tvi.item.cChildren = 0;
    for ( i = MAXKIDS+1; i <= MAXKIDS*2; i++ ) 
    {
       
       tvi.item.lParam = g_ItmInfo[i].lParam = rand();
       SendMessage(hwTree, TVM_INSERTITEM, 0, (LONG)&tvi);
       g_ItmInfo[i].idStr = IDS_KID10 + i - 1;
       g_ItmInfo[i].iImage = LEVEL2_IMAGE;
    }


    hIml = ImageList_Create( BITMAP_WIDTH, BITMAP_HEIGHT,                      
		                     0, NUM_BITMAPS, 0 );                                

    hBmp = LoadBitmap ( hInst, MAKEINTRESOURCE ( IDB_IMAGELIST ) ); 
    ImageList_Add ( hIml, hBmp, ( HBITMAP )NULL );
    if ( !ImageList_SetOverlayImage ( hIml, 12, 1 ) )
        OutputDebugString ( "ImageList_SetOverlayImage() failed\n" );
    TreeView_SetImageList ( hwTree, hIml, TVSIL_NORMAL );
    DeleteObject ( hBmp );

    hStateImgL = ImageList_Create( BITMAP_WIDTH, BITMAP_HEIGHT,                      
		                     0, NUM_BITMAPS, 0 );                                
    hBmp = LoadBitmap ( hInst, MAKEINTRESOURCE ( IDB_ITEMSTATE ) ); 
    ImageList_Add ( hStateImgL, hBmp, ( HBITMAP )NULL );
    TreeView_SetImageList ( hwTree, hStateImgL, TVSIL_STATE );
    DeleteObject ( hBmp );
    
    return TRUE;
}

VOID TVDialog_OnCommand(HWND hwndDlg, UINT id, HWND hwndCtl, UINT code)
{
    HWND hwndOK     = GetDlgItem(hwndDlg, IDOK);

    if (id == IDOK && code == BN_CLICKED)
    {
        DestroyWindow(hwndDlg);
        g_hMDlg = ( HWND )NULL;
    }
}


VOID 
TVDialog_BeginDrag
(
    HWND            hwTree, 
    NM_TREEVIEW*    lItem
)
{
	HIMAGELIST      hIml;
	RECT            rcl;
	
	hIml = TreeView_CreateDragImage( hwTree, lItem->itemNew.hItem);

	TreeView_GetItemRect( hwTree, lItem->itemNew.hItem, &rcl, TRUE);

	if ( !ImageList_BeginDrag(hIml, 0, 0, 0) )
        OutputDebugString ( "ImageList_BeginDrag failed\n" );
	
    ImageList_DragEnter(hwTree, lItem->ptDrag.x, lItem->ptDrag.y);


	/*ShowCursor(FALSE);*/

	SetCapture( hwTree );

	g_fDragging = TRUE;
}

BOOL
TVDialog_GetItemInfo ( PTVITEMINFO lpInfo )
{
    int     i;
    BOOL    bFound     = FALSE;

    for ( i = 0; i <= MAXKIDS*2; i++ )
    {
        if ( g_ItmInfo[i].lParam == lpInfo->lParam )
        {
            bFound = TRUE;
            *lpInfo = g_ItmInfo[i];
            break;
        }
    }
    
    return bFound;
}


LRESULT 
TVDialog_OnNotify ( HWND hwndDlg, UINT idCtrl, LPARAM lParam )
{
    TVITEMINFO  ItmInfo;
    BOOL    bFound = FALSE;
    LPNM_TREEVIEW lpNMTView;
    TV_ITEM tvi;

    if ( ( ( NMHDR *)lParam )->code == NM_CLICK )
    {
        DWORD dwpos;
        TV_HITTESTINFO  ht;
        NMHDR *lpHdr = ( NMHDR * )lParam;
        lpNMTView = (LPNM_TREEVIEW)lParam;
        dwpos = GetMessagePos();
        ht.pt.x = LOWORD(dwpos);
        ht.pt.y = HIWORD(dwpos);
        ScreenToClient ( lpHdr->hwndFrom, &ht.pt ); 
        TreeView_HitTest ( lpHdr->hwndFrom, &ht );
        if ( ht.flags == TVHT_ONITEMSTATEICON )
        {
            UINT    state;
            tvi.hItem = ht.hItem;
            tvi.mask = TVIF_STATE;
            tvi.stateMask = TVIS_STATEIMAGEMASK;
            TreeView_GetItem ( lpHdr->hwndFrom, &tvi );
            state = tvi.state & TVIS_STATEIMAGEMASK;
            if ( state == INDEXTOSTATEIMAGEMASK ( 1 ) )
                tvi.state = INDEXTOSTATEIMAGEMASK( 2 );
            else
                tvi.state = INDEXTOSTATEIMAGEMASK( 1 );
            TreeView_SetItem(lpHdr->hwndFrom, &tvi );
        }
    }
    else
    if ( ( ( NMHDR *)lParam )->code == TVN_GETDISPINFO )
    {
        HINSTANCE hInst;
        TV_DISPINFO* lpDispInfo = ( TV_DISPINFO * )lParam;
        hInst = ( HINSTANCE )GetWindowLong ( hwndDlg, GWL_HINSTANCE );

        ItmInfo.lParam = lpDispInfo->item.lParam;
        bFound = TVDialog_GetItemInfo ( &ItmInfo );

        if ( lpDispInfo->item.mask & TVIF_TEXT && bFound )
        {
            LoadString ( hInst, ItmInfo.idStr,
                         lpDispInfo->item.pszText, lpDispInfo->item.cchTextMax );
        }

        if (  ( lpDispInfo->item.mask & TVIF_IMAGE || 
		lpDispInfo->item.mask & TVIF_SELECTEDIMAGE ) && bFound )
        {
            int iImgIndx = 0;
	    tvi.mask = TVIF_CHILDREN | TVIF_STATE;
	    tvi.hItem = lpDispInfo->item.hItem;
	    TreeView_GetItem ( lpDispInfo->hdr.hwndFrom, &tvi );
            if ( tvi.cChildren == 0 )
                iImgIndx = 2;
            else
            if ( tvi.state & TVIS_EXPANDED )
                iImgIndx = 1;
           
	    if ( lpDispInfo->item.mask & TVIF_IMAGE )
            lpDispInfo->item.iImage = ItmInfo.iImage *
                                      LEVEL_IMAGES + iImgIndx;
	    if ( lpDispInfo->item.mask & TVIF_SELECTEDIMAGE )
            lpDispInfo->item.iSelectedImage = ItmInfo.iImage *
                                        LEVEL_IMAGES + iImgIndx;
        }
#if 0
        if ( lpDispInfo->item.mask & TVIF_SELECTEDIMAGE && bFound )
        {
            int iImgIndx = 0;
            if ( lpDispInfo->item.state & TVIS_EXPANDED )
                iImgIndx = 1;
            else
            if ( lpDispInfo->item.cChildren == 0 )
                iImgIndx = 2;
            lpDispInfo->item.iSelectedImage = ItmInfo.iImage *
                                        LEVEL_IMAGES + iImgIndx;
        }
#endif
    }
    else
    if ( ( ( NMHDR *)lParam )->code == TVN_ITEMEXPANDING )
    {
        TV_ITEM       tvi;
        HWND          hTView;
        lpNMTView = (LPNM_TREEVIEW)lParam;
        hTView = lpNMTView->hdr.hwndFrom;
        tvi = lpNMTView->itemNew;
        tvi.mask = TVIF_IMAGE ;
      /*  TreeView_GetItem (hTView, &tvi); */
        ItmInfo.lParam = tvi.lParam;
        bFound = TVDialog_GetItemInfo ( &ItmInfo );
        if ( lpNMTView->action == TVE_EXPAND && bFound )
            tvi.iImage = ItmInfo.iImage * LEVEL_IMAGES + 1;
        else
        if (lpNMTView->action == TVE_COLLAPSE && bFound )
            tvi.iImage = ItmInfo.iImage * LEVEL_IMAGES;
        tvi.iSelectedImage = tvi.iImage;
        TreeView_SetItem (hTView, &tvi);
    }
    else
    if ( ( ( NMHDR *)lParam )->code == TVN_BEGINLABELEDIT )
    {
        return 1;
    }
    else 
    if ( ( ( NMHDR *)lParam )->code == TVN_ENDLABELEDIT)
    {
        TV_DISPINFO FAR *lptvdi;
        TV_ITEM     tvi;

       lptvdi = (TV_DISPINFO FAR *)lParam;
       tvi    = lptvdi->item;

       if (tvi.pszText)
       {
           tvi.mask = TVIF_TEXT;
           TreeView_SetItem (lptvdi->hdr.hwndFrom,&tvi);
       }

    }
    else 
    if ( ( ( NMHDR *)lParam )->code == TVN_BEGINDRAG)
    {
       HWND     hwTView = ( ( NMHDR *)lParam )->hwndFrom;
       TVDialog_BeginDrag ( hwTView,  ( NM_TREEVIEW* )lParam );
    }
    return 0;
}

