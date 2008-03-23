/*    
	FindReplDlg.c	1.5 common dialogs Find Replace Dialogs
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


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */

#include "windows.h"
#include "windowsx.h"
#include "commdlg.h"
#include "dlgs.h"
#include "cderr.h"
#include "Log.h"
#include "Dialog.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>


/* Dialog procedures for CommDlg, these are NOT exported */
static LRESULT FAR PASCAL ReplaceDlgProc    (HWND, UINT, WPARAM, LPARAM);
static LRESULT FAR PASCAL FindDlgProc       (HWND, UINT, WPARAM, LPARAM);

static HWND MakeModelessDialogBox(HINSTANCE, HWND, LPCSTR, BOOL, BOOL, LPCSTR, 
			void *, DLGPROC);

extern DWORD	LastCommonDialogError;

/*----------------------------------------------------------------------------
				FindText()
  ----------------------------------------------------------------------------*/
HWND
FindText(FINDREPLACE *lpfr)
{
	if ( lpfr->Flags & FR_ENABLEHOOK  &&  !lpfr->lpfnHook ) {
		LastCommonDialogError = CDERR_NOHOOK;
		return 0;
	}
	return MakeModelessDialogBox(lpfr->hInstance, 
			lpfr->hwndOwner, lpfr->lpTemplateName, 
			lpfr->Flags & FR_ENABLETEMPLATE,
			lpfr->Flags & FR_ENABLETEMPLATEHANDLE,
			MAKEINTRESOURCE(FINDDLGORD), (void *)lpfr, 
			(DLGPROC)FindDlgProc);
}


static LRESULT FAR PASCAL 
FindDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND		hWnd;
	HDC 		hDC;
	HBRUSH 		hBrush;
	RECT		rcClient;
	FINDREPLACE	*cf;
	UINT		uiRet = 0;
	static UINT	uiHelpMsg, uiFindMsg;

	cf = (FINDREPLACE *)GetWindowLong(hDlg,DWL_LPARAM); 
	if ( message != WM_INITDIALOG  &&  cf->Flags & FR_ENABLEHOOK ) {
		uiRet = LOWORD(cf->lpfnHook(hDlg,message,wParam,lParam));
		if ( uiRet ) 
			return (LRESULT)uiRet;
	}
	SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);

	switch(message) {
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg, edt1, WM_SETTEXT,
				strlen(cf->lpstrFindWhat),
				(LPARAM)cf->lpstrFindWhat);

		/* Show/hide the whole word stuff */
		if(cf->Flags & FR_HIDEWHOLEWORD)
			ShowWindow(GetDlgItem(hDlg, chx1), SW_HIDE);
		else {
			if(cf->Flags & FR_WHOLEWORD)
				CheckDlgButton(hDlg,chx1,TRUE);
			if(cf->Flags & FR_NOWHOLEWORD)
				EnableWindow(GetDlgItem(hDlg, chx1), FALSE);
		}

		/* Show/hide the matchcase stuff */
		if(cf->Flags & FR_HIDEMATCHCASE)
			ShowWindow(GetDlgItem(hDlg, chx2), SW_HIDE);
		else {
			if(cf->Flags & FR_MATCHCASE)
				CheckDlgButton(hDlg,chx2,TRUE);
			if(cf->Flags & FR_NOMATCHCASE)
				EnableWindow(GetDlgItem(hDlg, chx2), FALSE);
		}

		/* show/tell the up/down group and controls */
		if(cf->Flags & FR_HIDEUPDOWN) {
			ShowWindow(GetDlgItem(hDlg, rad1), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, rad2), SW_HIDE);
			ShowWindow(GetDlgItem(hDlg, grp1), SW_HIDE);
		}
		else {
			if (cf->Flags & FR_DOWN)
				CheckRadioButton(hDlg, rad1, rad2, rad2);
			else
				CheckRadioButton(hDlg, rad1, rad2, rad1);
			if(cf->Flags & FR_NOUPDOWN) {
				EnableWindow(GetDlgItem(hDlg, rad1), FALSE);
				EnableWindow(GetDlgItem(hDlg, rad2), FALSE);
				EnableWindow(GetDlgItem(hDlg, grp1), FALSE);
			}
		}

		if ( !(cf->Flags & FR_SHOWHELP) )
			ShowWindow(GetDlgItem(hDlg, pshHelp), SW_HIDE);
		else  
			uiHelpMsg = RegisterWindowMessage(HELPMSGSTRING);

		uiFindMsg = RegisterWindowMessage(FINDMSGSTRING);

		if ( !strlen(cf->lpstrFindWhat) )
			EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);

		if ( cf->Flags & FR_ENABLEHOOK )
			cf->lpfnHook(hDlg, message, wParam, lParam);

		return TRUE;

	case WM_COMMAND:

		cf->Flags &= ~(FR_FINDNEXT|FR_REPLACE|FR_REPLACEALL);
		cf->Flags &= ~(FR_DOWN|FR_WHOLEWORD|FR_MATCHCASE);
		
		if ( GET_WM_COMMAND_ID(wParam,lParam) == edt1 &&
		     GET_WM_COMMAND_CMD(wParam,lParam) == EN_CHANGE ) {
			SendMessage(GET_WM_COMMAND_HWND(wParam,lParam),
				WM_GETTEXT,
				cf->wFindWhatLen, (LPARAM)cf->lpstrFindWhat);
			hWnd = GetDlgItem(hDlg, IDOK);
			if ( strlen(cf->lpstrFindWhat) ) {
				if ( !IsWindowEnabled(hWnd) )
					EnableWindow(hWnd, TRUE);
			} else {
				if ( IsWindowEnabled(hWnd) )
					EnableWindow(hWnd, FALSE);
			}
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			return TRUE;
		}

		if (GET_WM_COMMAND_ID(wParam,lParam) == IDOK) {
			if ( IsDlgButtonChecked(hDlg, rad2) )
				cf->Flags |= FR_DOWN;
			if ( IsDlgButtonChecked(hDlg, chx1) )
				cf->Flags |= FR_WHOLEWORD;
			if ( IsDlgButtonChecked(hDlg, chx2) )
				cf->Flags |= FR_MATCHCASE;
			cf->Flags |= FR_FINDNEXT;
			SendMessage(cf->hwndOwner,uiFindMsg,0,(LPARAM)cf);
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			return TRUE;
		}
		if (GET_WM_COMMAND_ID(wParam,lParam) == IDCANCEL) {
			cf->Flags = FR_DIALOGTERM;
			SendMessage(cf->hwndOwner, uiFindMsg, 0, (LPARAM)cf);
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			DestroyWindow(hDlg);
			return FALSE;
		}
		if (GET_WM_COMMAND_ID(wParam,lParam) == pshHelp) {
			SendMessage(cf->hwndOwner,uiHelpMsg,0,0);
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			return TRUE;
		}

		break;

	case WM_ERASEBKGND: {
	    hDC = GetDC(hDlg);
	    hBrush = (HBRUSH)SendMessage(hDlg,
			GET_WM_CTLCOLOR_MSG(CTLCOLOR_DLG),
			GET_WM_CTLCOLOR_MPS(hDC,hDlg,CTLCOLOR_DLG));
	    if (hBrush) {
		GetClientRect(hDlg, &rcClient);
		FillRect((HDC)wParam, &rcClient, hBrush);
		ReleaseDC(hDlg, hDC);
		SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)TRUE);
		return TRUE;
	    }
	    else {
		ReleaseDC(hDlg,hDC);
		SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)FALSE);
		return FALSE;
	    }
	}

	default:
	    break;
	}
	return FALSE;
}


/*----------------------------------------------------------------------------
				ReplaceText()
  ----------------------------------------------------------------------------*/
HWND
ReplaceText(FINDREPLACE *lpfr)
{
	if ( lpfr->Flags & FR_ENABLEHOOK  &&  !lpfr->lpfnHook ) {
		LastCommonDialogError = CDERR_NOHOOK;
		return 0;
	}
	return MakeModelessDialogBox(lpfr->hInstance, 
			lpfr->hwndOwner, lpfr->lpTemplateName, 
			lpfr->Flags & FR_ENABLETEMPLATE,
			lpfr->Flags & FR_ENABLETEMPLATEHANDLE,
			MAKEINTRESOURCE(REPLACEDLGORD), (void *)lpfr, 
			(DLGPROC)ReplaceDlgProc);
}


static LRESULT FAR PASCAL 
ReplaceDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	HWND		hWnd;
	HDC 		hDC;
	HBRUSH 		hBrush;
	RECT		rcClient;
	FINDREPLACE	*cf;
	UINT		uiRet = 0;
	static UINT	uiHelpMsg, uiFindMsg;

	cf = (FINDREPLACE *)GetWindowLong(hDlg,DWL_LPARAM); 
	if ( message != WM_INITDIALOG  &&  cf->Flags & FR_ENABLEHOOK ) {
		uiRet = LOWORD(cf->lpfnHook(hDlg,message,wParam,lParam));
		if ( uiRet ) 
			return (LRESULT)uiRet;
	}
	SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);

	switch(message) {
	case WM_INITDIALOG:
		SendDlgItemMessage(hDlg,edt1,WM_SETTEXT,
				strlen(cf->lpstrFindWhat),
				(LONG)cf->lpstrFindWhat);

		SendDlgItemMessage(hDlg,edt2,WM_SETTEXT,
				strlen(cf->lpstrReplaceWith),
				(LPARAM)cf->lpstrReplaceWith);

		/* Show/hide the whole word stuff */
		if(cf->Flags & FR_HIDEWHOLEWORD)
			ShowWindow(GetDlgItem(hDlg, chx1), SW_HIDE);
		else {
			if(cf->Flags & FR_WHOLEWORD)
				CheckDlgButton(hDlg,chx1,TRUE);
			if(cf->Flags & FR_NOWHOLEWORD)
				EnableWindow(GetDlgItem(hDlg, chx1), FALSE);
		}

		/* Show/hide the matchcase stuff */
		if(cf->Flags & FR_HIDEMATCHCASE)
			ShowWindow(GetDlgItem(hDlg, chx2), SW_HIDE);
		else {
			if(cf->Flags & FR_MATCHCASE)
				CheckDlgButton(hDlg,chx2,TRUE);
			if(cf->Flags & FR_NOMATCHCASE)
				EnableWindow(GetDlgItem(hDlg, chx2), FALSE);
		}

		if ( !(cf->Flags & FR_SHOWHELP) )
			ShowWindow(GetDlgItem(hDlg, pshHelp), SW_HIDE);
		else
			uiHelpMsg = RegisterWindowMessage(HELPMSGSTRING);

		uiFindMsg = RegisterWindowMessage(FINDMSGSTRING);

		if ( cf->Flags & FR_ENABLEHOOK )
			cf->lpfnHook(hDlg,message,wParam,lParam);

		if ( !strlen(cf->lpstrFindWhat) ) {
			EnableWindow(GetDlgItem(hDlg, IDOK), FALSE);
			EnableWindow(GetDlgItem(hDlg, psh1), FALSE);
			EnableWindow(GetDlgItem(hDlg, psh2), FALSE);
		}

		return TRUE;

	case WM_COMMAND:
		SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)1);
		cf->Flags &= ~(FR_FINDNEXT|FR_REPLACE|FR_REPLACEALL);
		cf->Flags &= ~(FR_DOWN|FR_WHOLEWORD|FR_MATCHCASE);
		if ( IsDlgButtonChecked(hDlg, chx1) )
			cf->Flags |= FR_WHOLEWORD;
		if ( IsDlgButtonChecked(hDlg, chx2) )
			cf->Flags |= FR_MATCHCASE;

		if ( GET_WM_COMMAND_ID(wParam,lParam) == edt1 &&
		     GET_WM_COMMAND_CMD(wParam,lParam) == EN_CHANGE ) {
			SendMessage(GET_WM_COMMAND_HWND(wParam,lParam),
				WM_GETTEXT, cf->wFindWhatLen,
				(LPARAM)cf->lpstrFindWhat);
			hWnd = GetDlgItem(hDlg, IDOK);
			if ( strlen(cf->lpstrFindWhat) ) {
				if ( !IsWindowEnabled(hWnd) ) {
					EnableWindow(hWnd,TRUE);
					EnableWindow(GetDlgItem(hDlg,psh1),TRUE);
					EnableWindow(GetDlgItem(hDlg,psh2),TRUE);
				}
			} else
				if ( IsWindowEnabled(hWnd) ) {
					EnableWindow(hWnd, FALSE);
					EnableWindow(GetDlgItem(hDlg,psh1),FALSE);
					EnableWindow(GetDlgItem(hDlg,psh2),FALSE);
				}
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			return TRUE;
		}
		if ( GET_WM_COMMAND_ID(wParam,lParam) == edt2 &&
		     GET_WM_COMMAND_CMD(wParam,lParam) == EN_CHANGE ) {
			SendMessage(GET_WM_COMMAND_HWND(wParam,lParam),
				WM_GETTEXT, 
				cf->wReplaceWithLen, 
				(LPARAM)cf->lpstrReplaceWith);
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			return TRUE;
		}
		if (GET_WM_COMMAND_ID(wParam,lParam) == IDOK) {
			cf->Flags |= FR_FINDNEXT;
			SendMessage(cf->hwndOwner,uiFindMsg,0,(LPARAM)cf);
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			return TRUE;
		}
		if (GET_WM_COMMAND_ID(wParam,lParam) == psh1) {
			cf->Flags |= FR_REPLACE;
			SendMessage(cf->hwndOwner,uiFindMsg,0,(LPARAM)cf);
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			return TRUE;
		}
		if (GET_WM_COMMAND_ID(wParam,lParam) == psh2) {
			cf->Flags |= FR_REPLACEALL;
			SendMessage(cf->hwndOwner,uiFindMsg,0,(LPARAM)cf);
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			return TRUE;
		}
		if (GET_WM_COMMAND_ID(wParam,lParam) == IDCANCEL) {
			cf->Flags = FR_DIALOGTERM;
			SendMessage(cf->hwndOwner,uiFindMsg,0,(LPARAM)cf);
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			DestroyWindow(hDlg);
			return FALSE;
		}
		if (GET_WM_COMMAND_ID(wParam,lParam) == pshHelp) {
			SendMessage(cf->hwndOwner,uiHelpMsg,0,0);
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			return TRUE;
		} 

		break;

	case WM_ERASEBKGND: {
	    hDC = GetDC(hDlg);
	    hBrush = (HBRUSH)SendMessage(hDlg,
			GET_WM_CTLCOLOR_MSG(CTLCOLOR_DLG),
			GET_WM_CTLCOLOR_MPS(hDC,hDlg,CTLCOLOR_DLG));
	    if (hBrush) {
		GetClientRect(hDlg, &rcClient);
		FillRect((HDC)wParam, &rcClient, hBrush);
		ReleaseDC(hDlg, hDC);
		SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)TRUE);
		return TRUE;
	    }
	    else {
		ReleaseDC(hDlg,hDC);
		SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)FALSE);
		return FALSE;
	    }
	}

	default:
	    break;
    }
    return FALSE;
}


static HWND
MakeModelessDialogBox(HINSTANCE hInstance, HWND hwndOwner, 
		LPCSTR lpTemplateName, BOOL bEnableTemplate, 
		BOOL bEnableTemplateHandle, LPCSTR lpRsc, 
		void *lpCommDlgStruct, DLGPROC lpfnDlgProc)
{
	HINSTANCE	hInst;
	LPSTR		lpDlg;
	void FAR	*lpvDlgTmp;

	LastCommonDialogError = 0;

	if ( !(hInst = GetModuleHandle("COMMDLG")) )
		hInst = LoadLibrary("COMMDLG");
	
	if ( bEnableTemplateHandle ) {
		lpvDlgTmp = GlobalLock((HGLOBAL)hInstance);
		return CreateDialogIndirectParam(hInst, lpvDlgTmp, hwndOwner,
					lpfnDlgProc, (LPARAM)lpCommDlgStruct);
	}

	if ( bEnableTemplate ) {
		lpDlg = (LPSTR)lpTemplateName;
		hInst = hInstance;
	}
	else
		lpDlg = (LPSTR)lpRsc;

	return CreateDialogParam(hInst, lpDlg, hwndOwner,
			    lpfnDlgProc, (LPARAM)lpCommDlgStruct);
}

