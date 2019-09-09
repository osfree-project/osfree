/******************************************************************************
Module name: proppage.c - Application-level Implementation for Property
	Page CallBack function and Dialog Procedure function
******************************************************************************/
/*	@(#)proppage.c	1.2 10/23/96 13:02:00 /users/sccs/src/samples/ctrldemo/s.proppage.c*/
#include <windows.h>
#include <stdio.h> 
#include <commctrl.h>
#include "resource.h"
/*///////////////////////////////////////////////////////////////////////////*/


/*prototype */
void PropPg_OnHScroll (HWND hwnd, HWND hwndCtl, UINT codeNotify, int Pos );
void PropPg_OnVScroll (HWND hwnd, HWND hwndCtl, UINT codeNotify, int Pos );
/*void PropPg_OnOK (HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);*/


/*///////////////////////////////////////////////////////////////////////////*/

BOOL PropPg_OnInitDialog (HWND hwnd, HWND hwndFocus, LPARAM lParam) 
{
	RECT		rcDlg;
/*	SetWindowPos ( hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE ); */
	GetClientRect ( hwnd, &rcDlg );
	return(TRUE);         /* Accept default focus window.*/
}


/*///////////////////////////////////////////////////////////////////////////*/



/*///////////////////////////////////////////////////////////////////////////*/


 
 
/* OnWMNotify - provides the tooltip control with the appropriate text 
//     to display for a control window. This function is called by 
//     the dialog box procedure in response to a WM_NOTIFY message. 
// lParam - second message parameter of the WM_NOTIFY message 
*/
VOID PropPg_OnWMNotify ( HWND hDlg, UINT uCtlID, LPARAM lParam ) 
{ 
	LPTOOLTIPTEXT			lpttt; 
	int				idCtrl; 

	if ( ( ( ( LPNMHDR ) lParam )->code ) == TTN_NEEDTEXT ) 
	{ 
		lpttt = (LPTOOLTIPTEXT) lParam; 
		idCtrl = ( ( LPNMHDR ) lParam )->idFrom; 
		if ( lpttt->uFlags & TTF_IDISHWND )
			idCtrl = GetDlgCtrlID ( ( HWND ) ( ( LPNMHDR ) lParam )->idFrom ); 
		switch ( uCtlID ) 
		{ 
		    case 0: 
				lpttt->lpszText = "Tab Item 1"; 
				break;
		    case 1: 
				lpttt->lpszText = "Tab Item 2"; 
				break;
		    case 2: 
				lpttt->lpszText = "Tab Item 3"; 
				break;
		} 
	}
	else
	if ( ( ( ( LPNMHDR ) lParam )->code ) == PSN_APPLY ) 
		PropSheet_UnChanged ( GetParent ( hDlg ), hDlg );	
	return; 
} 
 


/*///////////////////////////////////////////////////////////////////////////*/


void PropPg_OnDestroy (HWND hwnd) 
{
     	LPARAM		hImageList = 0L;
}


/*///////////////////////////////////////////////////////////////////////////*/




void PropPg_OnCommand (HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) 
{
                       
	switch ( id )
	{
		case IDOK    : 
		case IDCANCEL: 
			EndDialog ( hwnd, FALSE );
		case IDC_CHECK1:
		case IDC_CHECK2:
		case IDC_CHECK3:
		case IDC_CHECK4:
		case IDC_BUTTON1:
		case IDC_BUTTON2:
		case IDC_BUTTON3:
		case IDC_BUTTON4:
		case IDC_BUTTON5:
		case IDC_BUTTON6:
		case IDC_RADIO1:
		case IDC_RADIO2:
		case IDC_RADIO3:
		case IDC_RADIO4:
			PropSheet_Changed ( GetParent( hwnd ), hwnd );
	}                       
}

#if 0
void PropPg_OnOK (HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) 
{
	static UINT		TrackbarID;
	char			buf[256];
	HWND			hwFocus		= ( HWND )NULL;
	HWND			hwTrackbar	= ( HWND )NULL;
	static POINT		ptPair;

}
#endif


void PropPg_OnHScroll (HWND hwnd, HWND hwndCtl, UINT codeNotify, int Pos ) 
{
}

void PropPg_OnVScroll (HWND hwnd, HWND hwndCtl, UINT codeNotify, int Pos ) 
{
}


/*/////////////////////////////////////////////////////////////////////////////
// Callback function to be called by the property sheet whenever
// the page is created or destroyed
 */
UINT CALLBACK 
PropPg_Proc 
(	
	HWND			hwnd, 
	UINT			uMsg,
	LPPROPSHEETPAGE		ppsp
) 
{
	return ( 1 );
}

/*/////////////////////////////////////////////////////////////////////////////
// This function processes messages sent to Property Page dialog box.
 */
BOOL CALLBACK 
PropPg_DlgProc 
(
	HWND			hwnd, 
	UINT			uMsg,
	WPARAM			wParam, 
	LPARAM			lParam
) 
{

   switch (uMsg) 
   {
	   case WM_INITDIALOG :
		   return PropPg_OnInitDialog ( ( hwnd ), ( HWND )( wParam ), lParam );
	   case WM_DESTROY :  
		   PropPg_OnDestroy ( hwnd );
		   break;
	   case WM_COMMAND :
		   PropPg_OnCommand ( hwnd, (int)( LOWORD( wParam ) ), ( HWND )( lParam ), ( UINT )HIWORD ( wParam ) );
		   break;
	   case WM_HSCROLL :
		   PropPg_OnHScroll ( hwnd, ( HWND )(lParam), ( UINT )( LOWORD ( wParam ) ), (int)(short )HIWORD ( wParam ) );
		   break;
	   case WM_VSCROLL :
		   PropPg_OnHScroll ( hwnd, ( HWND )(lParam), ( UINT )( LOWORD ( wParam ) ), (int)(short )HIWORD ( wParam ) );
		   break;
	   case WM_NOTIFY  :
		   PropPg_OnWMNotify ( hwnd, wParam, lParam );
		   break;
		   
   }
   return(FALSE);         /* We didn't process the message.*/
}

/*////////////////////////////// End of File ////////////////////////////////*/


