/*  
	WTlbCust.c	1.6 		Toolbar customize dlg 
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
#include "commctrl.h"
#else
#include "WCommCtrl.h"
#endif

#include "string.h"
#include "WResourc.h"
#include "WToolbar.h"
#include "WTlbCust.h"
#include "WDragLbx.h"

/*
 * Note: this piece of code was tested on Win95 platform before being ported onto
 *   UNIX. Some problems on both platforms still lingering and are worth being
 *   mentioned here:
 *   On Win95, the toolbar customize dialogbox is created using DialogBoxParam()
 *   with Toolbar handle passed as parent. But somehow it doesn't work ( GetParent()
 *   against the dialogbox always returns the toolbar's parent instead of the
 *   handle of toolbar itself. As a workaround, Toolbar handle is passed as 
 *   lParam and on initialization, the toolbar customize dialogbox stores it 
 *   away as a window property.
 *   On LINUX, GetProp ( I also tried GetWindowLong vs. SetWindowLong and would
 *   have the same trouble ) would frequently fail inside WM_MEASUREITEM handler
 *   if the two listboxes of the custom dialog have style set to LBS_OWNERDRAWFIXED.
 *   To the contrary, if they are set to LBS_OWNERDRAWVARIABLE, GetProp() would
 *   succeed but the two listboxes are not scrollable. Luckily, GetParent() now
 *   works. The following macros would hence reflect those facts. 
 *   Hung.
 */

#if _WINDOWS
#define GETTLBHANDLE(hwnd) \
   GetProp(hwnd, TOOLBAR_HANDLE )
#define SETTLBHANDLE(hwnd, pthisNew) \
   SetProp(hwnd, TOOLBAR_HANDLE, pthisNew)
#define REMOVETLBHANDLE(hwnd) \
   RemoveProp(hwnd, TOOLBAR_HANDLE )
#else
#define GETTLBHANDLE(hwDlg) \
   GetParent(hwDlg)
#define SETTLBHANDLE(hwDlg, pthisNew) 
#define REMOVETLBHANDLE(hwnd) 
#endif

#define GETBTNINFO(hwnd) \
   GetProp(hwnd, BTNTEXT_HANDLE )
#define SETBTNINFO(hwnd, pthisNew) \
   SetProp(hwnd, BTNTEXT_HANDLE, pthisNew)
#define REMOVEBTNINFO(hwnd) \
   RemoveProp(hwnd, BTNTEXT_HANDLE )

#define MAX_BTNTXTLEN   40

enum { eNull,
       eDragRight, eDropRight, eDDropRight,
       eDragLeft,  eDropLeft,  eDDropLeft,
       eRemove, eAdd, eAddSeparator, eRemoveSeparator };

static char BTNTEXT_HANDLE[]    = "BtnText_Handle";	
static char SEPARATOR_TEXT[]    = "Separator";
static UINT g_DLMessage         = 0 ;

extern void
Toolbr_MoveBtn
(
    HWND                hWnd,
    int                 iBtnMove,
    int                 iBtnAfter
);

extern void 
Toolbr_OnMeasureItem 
(
	HWND			    hWnd, 
	LPMEASUREITEMSTRUCT	lpmi
);

static void
Toolbr_ResetDlgControls
(
    HWND            hwDlg
);

extern void
Toolbr_DrawDragLBxItem 
( 
    HWND                hwDlg,
    HWND                hwToolbar, 
    LPDRAWITEMSTRUCT    lpdi,
    LPSTR               pBtnText
);

void
Toolbr_GetLBxBtnInfo  
(
    HWND            hwDlg,
    HWND            hwLB,
    int             iLBIndx,
    TBCDLGBTN*      pBtnInfo
)
{
    int             iItmData;
    HGLOBAL         hBtns       = ( HGLOBAL )NULL;
    TBCDLGBTN*      pBtns       = ( TBCDLGBTN* )NULL;
    TBCDLGBTN*      pBtn        = ( TBCDLGBTN* )NULL;
    int             ErrorCode   = 0;

    if ( !( hBtns = GETBTNINFO ( hwDlg ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pBtns = ( TBCDLGBTN *) GlobalLock ( hBtns ) ) )
		ErrorCode = WTLB_ERR_GLOBALLOCK;
	else
    {
        iItmData = SendMessage ( hwLB, LB_GETITEMDATA, iLBIndx, 0L );
        if ( iItmData != -1 )
	{
	    pBtn = pBtns + iItmData;
            memcpy ( pBtnInfo, pBtn, sizeof ( TBCDLGBTN ) );
	}
        else
        {
            pBtnInfo->iCmdID = -1;
            lstrcpy ( pBtnInfo->Text, SEPARATOR_TEXT );
        }
    }

    if ( pBtns ) 
        GlobalUnlock ( hBtns );
}

static int
Toolbr_LbxIndxToTlb
(
    HWND            hwDlg,
    HWND            hwToolbar,
    HWND            hwLB,
    int             iLBIndx,
    int*            piCmd
)
{
    int             count;       
    int             iCmd;
    int             iNext;
    TBCDLGBTN       BtnInfo;
    int             iTBIndx     = -1;

    count = SendMessage ( hwLB, LB_GETCOUNT, 0, 0L );
    Toolbr_GetLBxBtnInfo ( hwDlg, hwLB, iLBIndx, &BtnInfo );
    *piCmd = BtnInfo.iCmdID;
    if ( *piCmd != -1 )
        iTBIndx = SendMessage ( hwToolbar, TB_COMMANDTOINDEX, *piCmd, 0L );
    else              /* if the list box item is a toolbar separator */
    if ( count > 1 && iLBIndx  < count - 1 )
    {
                
        if ( iLBIndx > 0 )
            iNext = iLBIndx - 1;
        else
        if ( iLBIndx < count - 1 )
            iNext = iLBIndx + 1;
        
        Toolbr_GetLBxBtnInfo ( hwDlg, hwLB, iNext, &BtnInfo );
        /*lItmData = SendMessage ( hwLB, LB_GETITEMDATA, iNext, 0L );*/
        if ( ( iCmd = BtnInfo.iCmdID ) != -1 )
        {
            iTBIndx = SendMessage ( hwToolbar, TB_COMMANDTOINDEX, iCmd, 0L );
            iTBIndx = iNext < iLBIndx ? iTBIndx + 1 : iTBIndx - 1;
        }
    }


    return iTBIndx;

}

static void
Toolbr_MoveLBItems
(
    HWND            hwDlg,
    HWND            hwLBDest,
    HWND            hwLBSrc,
    int             iLBInsert,
    int             iLBMove,
    int             iAction
)
{
    int             iCmdMove;
    int             iCmdInsert;
    int             iTBMove;
    int             iTBInsert;
    HWND            hwToolbar;
    TBBUTTON        tb;
    int             iItemData   = -1;
    UINT            uMsg        = LB_INSERTSTRING;

	if ( !( hwToolbar = GETTLBHANDLE ( hwDlg ) ) || iLBInsert == -1 )
        return;
    if ( iLBInsert == -1 || iLBMove == -1 )
        return;
    iItemData = SendMessage ( hwLBSrc, LB_GETITEMDATA, iLBMove, 0L );
    iTBMove = Toolbr_LbxIndxToTlb ( hwDlg, hwToolbar, hwLBSrc, iLBMove, &iCmdMove );
    iTBInsert = Toolbr_LbxIndxToTlb ( hwDlg, hwToolbar, hwLBDest , iLBInsert, &iCmdInsert );
    
    if ( iCmdMove == -1 && iAction == eAdd ) /* separator list box item */
        iAction = eAddSeparator;        
    if ( ( iCmdMove == -1 && iAction == eRemove ) ||
         ( iCmdMove == -1 && iCmdInsert == -1 ) )
        iAction = eRemoveSeparator;        
    if ( ( iAction == eAddSeparator && iTBInsert == -1 ) || 
         ( iAction == eRemoveSeparator && iTBMove == -1 )
       )
        iAction = eNull;        


    if ( iTBInsert == -1 && iAction == eRemove ) 
        uMsg = LB_ADDSTRING;

    tb.fsStyle = TBSTYLE_SEP;
    tb.fsState = TBSTATE_ENABLED;
    tb.idCommand = -1;
    tb.iBitmap = -1;
    tb.iString = -1;
    if ( iAction == eAddSeparator )
    {
        SendMessage ( hwToolbar, TB_INSERTBUTTON, iTBInsert,( LPARAM )&tb );
        InvalidateRect ( hwToolbar, NULL, TRUE );
        UpdateWindow ( hwToolbar );
        SendMessage ( hwLBDest, LB_INSERTSTRING, iLBInsert, -1L );
    }
    else
    if ( iAction == eRemoveSeparator )
    {
        SendMessage ( hwToolbar, TB_DELETEBUTTON, iTBMove, 0L );
        if ( iLBMove >= 0 )
        SendMessage ( hwLBSrc, LB_DELETESTRING, iLBMove, 0L );
    }
    else
    if ( iAction == eAdd || iAction == eRemove )
    {
        Toolbr_MoveBtn ( hwToolbar, iTBMove, iTBInsert );
        SendMessage ( hwToolbar, TB_HIDEBUTTON, iCmdMove, 
                        MAKELONG( iAction == eRemove, 0 ) );
        SendMessage ( hwLBSrc, LB_DELETESTRING, iLBMove, 0L );
        if ( iLBInsert == 0 && iAction == eRemove )
            iLBInsert++;
        SendMessage ( hwLBDest, uMsg, iLBInsert, ( LPARAM )iItemData );
    }
    else
    if ( iAction == eDDropRight )
    {
        Toolbr_MoveBtn ( hwToolbar, iTBMove, iTBInsert );
        SendMessage ( hwLBSrc, LB_DELETESTRING, iLBMove, 0L );
        if ( iLBMove < iLBInsert )
            iLBInsert--;
        SendMessage ( hwLBDest, uMsg, iLBInsert, ( LPARAM )iItemData );
    }


    if ( iAction != eNull )
    {
        InvalidateRect ( hwLBSrc, NULL, TRUE );
        InvalidateRect ( hwLBDest, NULL, TRUE );
        UpdateWindow ( hwLBSrc );
        UpdateWindow ( hwLBDest );
        SendMessage ( hwLBDest, LB_SETCURSEL, iLBInsert, 0L );

        Toolbr_ResetDlgControls ( hwDlg );
    }
}

static void
Toolbr_ResetDlgControls
(
    	HWND            hwDlg
)
{
    	int             count;
	
    	count = SendMessage ( GetDlgItem ( hwDlg, IDC_LBOX_CUSTOMIZE_RIGHT ), 
                          LB_GETCOUNT, 0, 0L );
    	EnableWindow ( GetDlgItem ( hwDlg, IDC_BTN_CUSTOMIZE_REMOVE ), count > 0 );
    	count = SendMessage ( GetDlgItem ( hwDlg, IDC_LBOX_CUSTOMIZE_LEFT ), 
                          LB_GETCOUNT, 0, 0L );
    	EnableWindow ( GetDlgItem ( hwDlg, IDC_BTN_CUSTOMIZE_ADD ), count > 0 );
}
 
static void 
TlbCustom_OnCommand 
(   
    	HWND            hwnd, 
    	int             id, 
    	HWND            hwndCtl, 
    	UINT            codeNotify
) 
{
    
    	int             iLBRight;
    	int             iLBLeft;
    	int             count;
    	HWND            hwRight     = GetDlgItem ( hwnd, IDC_LBOX_CUSTOMIZE_RIGHT );
    	HWND            hwLeft      = GetDlgItem ( hwnd, IDC_LBOX_CUSTOMIZE_LEFT );

    	iLBRight = SendMessage ( hwRight, LB_GETCURSEL, 0, 0L );
    	iLBLeft = SendMessage ( hwLeft, LB_GETCURSEL, 0, 0L );
    	count = SendMessage ( hwRight, LB_GETCOUNT, 0, 0L );
    	switch ( id )
	{
    	    	case IDC_BTN_CUSTOMIZE_ADD :
       	    	 	Toolbr_MoveLBItems ( hwnd, hwRight, hwLeft,
                                 iLBRight, iLBLeft, eAdd );
            		break;
       		case IDC_BTN_CUSTOMIZE_REMOVE :
            		Toolbr_MoveLBItems ( hwnd, hwLeft, hwRight,
                                 iLBLeft, iLBRight, eRemove );
            		break;
        	case IDC_BTN_CUSTOMIZE_MOVEUP :
            		if ( iLBRight > 0 && iLBRight < count -1 )
                		Toolbr_MoveLBItems ( hwnd, hwRight, hwRight,
                                     iLBRight-1, iLBRight, eDDropRight );
            		break;
        	case IDC_BTN_CUSTOMIZE_MOVEDOWN :
            		if ( iLBRight+2 < count )
                		Toolbr_MoveLBItems ( hwnd, hwRight, hwRight,
                                    iLBRight+2, iLBRight, eDDropRight );
            		break;
        	case IDC_BTN_CUSTOMIZE_HELP :
        	        break;
        	case IDOK    : 
		case IDCANCEL: 
			EndDialog ( hwnd, FALSE );
	}                       
}

static void 
TlbCustom_OnDestroy 
(   
    	HWND            hwDlg
) 
{
    	HGLOBAL         hBtns       = ( HGLOBAL )NULL;
    	int             ErrorCode   = 0;

    	if ( !( hBtns = GETBTNINFO ( hwDlg ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
    	{
    	    REMOVETLBHANDLE ( hwDlg );
    	    GlobalFree ( hBtns );
    	}

}

static int
TlbCustom_OnCharToItem
(   
    	HWND                hwDlg, 
    	UINT                VKey, 
  	HWND    	    hwLBox 
) 
{
	int		    nItem;

	nItem = SendMessage ( hwLBox, LB_GETCURSEL, 0, 0L );
	SetFocus ( hwLBox );
	switch ( VKey )
	{
	   case VK_DOWN:
#if 0
		SendMessage ( hwLBox, LB_SETCURSEL, nItem + 1, 0L );
		return -2;
#else
		return ( nItem + 1 );
#endif
	   case VK_UP:
#if 0
		SendMessage ( hwLBox, LB_SETCURSEL, nItem + 1, 0L );
		return -2;
#else
		return ( nItem - 1 );
#endif
	}
	return -1;	
}

static int
TlbCustom_OnVKeyToItem
(   
    	HWND                hwDlg, 
    	UINT                VKey, 
  	HWND    	    hwLBox 
) 
{
	int		    nItem;

	nItem = SendMessage ( hwLBox, LB_GETCURSEL, 0, 0L );
	SetFocus ( hwLBox );
	switch ( VKey )
	{
	   case VK_DOWN:
#if 0
		SendMessage ( hwLBox, LB_SETCURSEL, nItem + 1, 0L );
		return -2;
#else
		return ( nItem  );
#endif
	   case VK_UP:
#if 0
		SendMessage ( hwLBox, LB_SETCURSEL, nItem + 1, 0L );
		return -2;
#else
		return ( nItem  );
#endif
	}
	return -1;	
}

static BOOL 
TlbCustom_OnDrawItem 
(   
    	HWND                hwDlg, 
    	UINT                idCtl, 
    	LPDRAWITEMSTRUCT    lpdi 
) 
{
    	HWND                hwToolbar   = ( HWND )NULL;
    	HGLOBAL             hBtnText    = ( HGLOBAL )NULL;
    	LPSTR               pBtnText    = ( HWND )NULL;

    	int                 ErrorCode   = 0;

	if ( ! ( hwToolbar = GETTLBHANDLE ( hwDlg ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( hBtnText = GETBTNINFO ( hwDlg ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
	if ( ! ( pBtnText = GlobalLock ( hBtnText ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
    	if ( SendMessage ( GetDlgItem ( hwDlg, idCtl ), LB_GETCOUNT, 0, 0L ) > 0 )
    	{
    	    Toolbr_DrawDragLBxItem ( hwDlg, hwToolbar, lpdi, pBtnText );
#if 0
	    InvalidateRect ( lpdi->hwndItem, &lpdi->rcItem, TRUE );
	    ValidateRect ( lpdi->hwndItem, &lpdi->rcItem );
#endif
    	    GlobalUnlock ( hBtnText );
    	    return TRUE;
    	}

    	return FALSE;
}


static BOOL 
TlbCustom_OnInitDialog 
(   
    HWND            hwDlg, 
    HWND            hwndFocus, 
    LPARAM          lParam 
) 
{
    int             count;
    int             i, itemData;
    TBNOTIFY        tbNtf;
    HWND            hwLBRight;
    HWND            hwLBLeft;
    HWND            hwTBParent      = ( HWND )NULL;
    HWND            hwToolbar       = ( HWND )NULL;
    HGLOBAL         hBtnText        = ( HGLOBAL )NULL;
    TBCDLGBTN*      pBtnTxt         = ( TBCDLGBTN* )NULL;
    TBCDLGBTN*      pText           = ( TBCDLGBTN* )NULL;

    hwLBRight = GetDlgItem ( hwDlg, IDC_LBOX_CUSTOMIZE_RIGHT );
    hwLBLeft = GetDlgItem ( hwDlg, IDC_LBOX_CUSTOMIZE_LEFT );
    if ( WMakeDragList ( hwLBRight ) && WMakeDragList ( hwLBLeft ) )
        g_DLMessage = RegisterWindowMessage ( DRAGLISTMSGSTRING );
    hwToolbar = ( HWND )lParam ;
    SETTLBHANDLE ( hwDlg, hwToolbar ); 
    tbNtf.hdr.code = TBN_GETBUTTONINFO;
    hwTBParent = GetParent ( hwToolbar  );
    count = SendMessage ( hwToolbar, TB_BUTTONCOUNT, 0, 0L );
    if ( !( hBtnText = GlobalAlloc ( GHND, count * sizeof ( TBCDLGBTN ) ) ) )
        return FALSE;
    SETBTNINFO ( hwDlg, hBtnText );
    if ( !( pText = ( TBCDLGBTN *)GlobalLock ( hBtnText ) ) )
        return FALSE;
    for ( i = 0; i < count; i++ )
    {
        itemData = -1;
	pBtnTxt = pText + i;
        memset ( &tbNtf, 0, sizeof ( TBNOTIFY ) );
        tbNtf.pszText = pBtnTxt->Text;
        tbNtf.iItem = i;
	tbNtf.hdr.code = TB_GETBUTTONTEXT;
        /*SendMessage ( hwTBParent, WM_NOTIFY, 0, ( LPARAM )&tbNtf );*/ 
        SendMessage ( hwToolbar, TB_GETBUTTON, i, 
                    ( LPARAM )&tbNtf.tbButton );
        if ( ( pBtnTxt->iCmdID = tbNtf.tbButton.idCommand ) >  0 )
	{
        	SendMessage ( hwToolbar, TB_GETBUTTONTEXT, pBtnTxt->iCmdID, 
			      ( LPARAM )pBtnTxt->Text ); 
		itemData = i;
	}
        if ( !( tbNtf.tbButton.fsState & TBSTATE_HIDDEN ) )
            SendMessage ( hwLBRight, LB_ADDSTRING, 0, ( LPARAM )itemData );
        else
            SendMessage ( hwLBLeft, LB_ADDSTRING, 0, ( LPARAM )itemData );
    }


    SendMessage ( hwLBRight, LB_ADDSTRING, 0, -1 ); 
    SendMessage ( hwLBLeft, LB_INSERTSTRING, 0, -1 );
    
    GlobalUnlock ( hBtnText );
    Toolbr_ResetDlgControls ( hwDlg );
    return TRUE;
}

static BOOL 
TlbCustom_OnMeasureItem 
(   
    HWND                hwDlg, 
    UINT                idCtl, 
    LPMEASUREITEMSTRUCT lpmi 
) 
{
    HWND                hwToolbar   = ( HWND )NULL;
    int                 ErrorCode   = 0;

	if ( ! ( hwToolbar = GETTLBHANDLE ( hwDlg ) ) )
		ErrorCode = WTLB_ERR_GETWINPROP;
	else
    if ( lpmi->CtlType == ODT_LISTBOX )
    {
        Toolbr_OnMeasureItem ( hwToolbar, lpmi );
        return TRUE;
    }

    return FALSE;
}

static LRESULT
Toolbr_DoDragDrop
(
	HWND			hWnd,
    	LPDRAGLISTINFO  lpdli
)
{
    	int             iItem;
    	int             iDrop;
    	int             iBtnCmd;
    	TBCDLGBTN       BtnInfo;
    	static int      iItemToMove;
    	static int      iDragStat   = eNull;
    	static HWND     hwRight     = ( HWND )NULL;
    	static HWND     hwLeft      = ( HWND )NULL;
   

    	switch (lpdli->uNotification) 
    	{
    	    case DL_BEGINDRAG:
    	        hwRight = GetDlgItem ( hWnd, IDC_LBOX_CUSTOMIZE_RIGHT );
    	        hwLeft = GetDlgItem ( hWnd, IDC_LBOX_CUSTOMIZE_LEFT );
    	        iDragStat = lpdli->hWnd == hwRight ? eDragRight : eDragLeft;
    	        iItemToMove = WLBItemFromPt(lpdli->hWnd, lpdli->ptCursor, TRUE);
		/*printf ( " Item to Move %d\n", iItemToMove );*/
	    	if ( iDragStat == eDragRight && 
                     ( iItemToMove + 1  == SendMessage ( lpdli->hWnd, LB_GETCOUNT, 0, 0L ) ) )
	    	{
			/*printf ( " Drag Cancel \n" );*/
			return FALSE;
            	}
            	SendMessage ( lpdli->hWnd, LB_SETCURSEL, iItemToMove, 0L ); 
            	return DL_MOVECURSOR;
           case DL_DRAGGING:
            	iItem = WLBItemFromPt(lpdli->hWnd, lpdli->ptCursor, TRUE);
            	WDrawInsert(hWnd, lpdli->hWnd, iItem);
            	if (iItem!=-1) 
                 	return DL_MOVECURSOR;
            	return DL_STOPCURSOR;
           case DL_DROPPED:
            	WDrawInsert(hWnd, lpdli->hWnd, -1);
            	Toolbr_GetLBxBtnInfo ( hWnd, lpdli->hWnd, iItemToMove, &BtnInfo );
            	iBtnCmd = BtnInfo.iCmdID;
            	iItem = WLBItemFromPt( GetDlgItem ( hWnd, IDC_LBOX_CUSTOMIZE_RIGHT ), 
                                    lpdli->ptCursor, TRUE );
            	iDrop = iItem == -1 ? eNull : eDropRight;
            	if ( iDrop == eNull )
            	{
                	iItem = WLBItemFromPt( GetDlgItem ( hWnd, IDC_LBOX_CUSTOMIZE_LEFT ), 
                                    lpdli->ptCursor, TRUE );
                	iDrop = iItem != -1 ? eDropLeft : eNull;
            	}

            	if ( iDragStat == eDragRight )
	    	{
                	iDragStat = iDrop == eDropRight ? eDDropRight : 
                            	iDrop == eDropLeft ? eRemove : eNull;
			if ( iItemToMove + 1 == SendMessage ( hwRight, LB_GETCOUNT, 0, 0L ) )
				iDragStat = eNull;
	    	}
	    	else
            	if ( iDragStat == eDragLeft )
                	iDragStat = iDrop == eDropLeft? eDDropLeft : 
                            iDrop == eDropRight ? eAdd : eNull;
            	if ( iDragStat == eAdd )
                	Toolbr_MoveLBItems ( hWnd, hwRight, hwLeft, iItem, iItemToMove, iDragStat );
            	else
            	if ( iDragStat == eRemove )
                	Toolbr_MoveLBItems ( hWnd, hwLeft, hwRight, iItem, iItemToMove, iDragStat );
            	else
            	if ( iDragStat == eDDropRight )
                	Toolbr_MoveLBItems ( hWnd, hwRight, hwRight, iItem, iItemToMove, iDragStat );
            	return DL_CURSORSET;
    	}

	return 0;
}
LRESULT CALLBACK 
Toolbr_CustomizeWndProc
(
	HWND			hWnd,
    	UINT			uMessage,
    	WPARAM			wParam,
    	LPARAM			lParam
)
{
    	LRESULT         	lResult         = 0L;
    
    	switch ( uMessage )
    	{
        	case WM_INITDIALOG :
		       	lResult = ( LRESULT )TlbCustom_OnInitDialog ( hWnd, ( HWND )( wParam ), lParam );
               		break;

        	case WM_DRAWITEM :
		       	lResult = ( LRESULT )TlbCustom_OnDrawItem ( hWnd, ( UINT )( wParam ), 
                                           ( LPDRAWITEMSTRUCT )lParam );
               		break;
#if 1
		case WM_CHARTOITEM :
		       	lResult = ( LRESULT )TlbCustom_OnCharToItem ( hWnd, ( UINT )HIWORD( wParam ),
					   ( HWND )lParam );
		case WM_VKEYTOITEM :
		       	lResult = ( LRESULT )TlbCustom_OnVKeyToItem ( hWnd, ( UINT )HIWORD( wParam ),
					   ( HWND )lParam );
			break;
#endif
        	case WM_MEASUREITEM :
		       	lResult = ( LRESULT )TlbCustom_OnMeasureItem ( hWnd, ( UINT )( wParam ), 
                                           ( LPMEASUREITEMSTRUCT )lParam );
               		break;
        	case WM_COMMAND :
		       	TlbCustom_OnCommand ( hWnd, (int)( LOWORD( wParam ) ), ( HWND )( lParam ), ( UINT )HIWORD ( wParam ) );
		       	break;
        	case WM_DESTROY :
		       	TlbCustom_OnDestroy ( hWnd );
		       	break;
    	}

    	if ( uMessage == g_DLMessage )
    	{
    	    LPDRAGLISTINFO lpdli = (LPDRAGLISTINFO)lParam;
    	    lResult = Toolbr_DoDragDrop ( hWnd, lpdli ); 
    	}
    	return lResult;
}
