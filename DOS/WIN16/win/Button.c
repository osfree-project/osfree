/*    
	Button.c	2.20
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

#include <string.h>

#include "windows.h"
#include "windowsx.h"

#include "kerndef.h"
#include "Log.h"
#include "Button.h"
#include "Classes.h"

/* external linkage */
extern BOOL TestWF(HWND, DWORD);
extern void SetControlStyle(HWND, WORD);
extern LONG InternalSetText(HWND, LPSTR);
extern HPEN GetSysColorPen(int);
extern LONG GetClassHandleLong(HCLASS32,int);
extern void ButtonDrawItem(HWND, LPDRAWITEMSTRUCT, LONG);
extern void ButtonEraseBackground(HWND, HDC);
extern void SendButtonItemDraw(HWND, WORD, WORD, BOOL);
extern void CheckDefPushButton(HWND,HWND,HWND);

/* exported routines */
LRESULT DefBUTTONProc(HWND, UINT, WPARAM, LPARAM);

static WNDPROC lpButtonBinToNat = 0;

/* *API* functions */

LRESULT
DefBUTTONProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    RECT rcClient;
    WORD wBtnStyle;
    POINT pt;
    WORD wState;
    BOOL bOwnerDraw;
    DRAWITEMSTRUCT dis;
    PAINTSTRUCT ps;
    DWORD dwStyle;
    HCLASS32 hButtonClass32;
    LRESULT lret;

    APISTR((LF_APICALL,"DefBUTTONProc(HWND=%x,UINT=%x,WPARAM=%x,LPARAM=%x)\n",
                wMsg, hWnd,wParam,lParam));

    dwStyle = GetWindowLong(hWnd, GWL_STYLE);
    wBtnStyle = LOWORD(dwStyle);
    wBtnStyle &= 0xf;

    /* LATER treat userbutton as ownerdraw */
    bOwnerDraw = (wBtnStyle == BS_OWNERDRAW) |
                 (wBtnStyle == BS_USERBUTTON);

    switch(wMsg) {
	case WM_NCCREATE:
	    if (wBtnStyle == BS_GROUPBOX) {
		LPCREATESTRUCT lpcs = (LPCREATESTRUCT)lParam;
		lpcs->dwExStyle |= WS_EX_TRANSPARENT;
	    }

 	    lret = DefWindowProc(hWnd, wMsg, wParam, lParam);
	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",lret));
	    return lret;

        case WM_ENABLE:
	    if (wParam)
		ClearButtonStatus(hWnd, BSF_DISABLED);
	    else
		OrButtonStatus(hWnd, BSF_DISABLED);
	    wState = GetButtonStatus(hWnd);
	    if (!bOwnerDraw)
		SendButtonItemDraw(hWnd, ODA_DRAWENTIRE | ODA_FOCUS, wState,
			FALSE);
	    else {
		SendButtonItemDraw(hWnd,ODA_DRAWENTIRE,wState,TRUE);
#ifdef	LATER
		SendButtonItemDraw(hWnd,ODA_FOCUS,wState,TRUE);
#endif
	    }

	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
	    return 1L;

        case WM_ERASEBKGND:
	    if (wBtnStyle != BS_GROUPBOX)
		ButtonEraseBackground(hWnd,(HDC)wParam);
		
	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
	    return 1L;

        case WM_GETDLGCODE:
	    switch(wBtnStyle) {
		case BS_PUSHBUTTON:
#if (WINVER < 0x030a)
		case BS_PUSHBOX:
#endif
		    lret = (LONG)(DLGC_UNDEFPUSHBUTTON|DLGC_BUTTON);
		    break;
		case BS_DEFPUSHBUTTON:
		    lret =  (LONG)(DLGC_DEFPUSHBUTTON|DLGC_BUTTON);
		    break;
		case BS_RADIOBUTTON:
		case BS_AUTORADIOBUTTON:
		    lret =  (LONG)(DLGC_RADIOBUTTON|DLGC_BUTTON);
		    break;
		case BS_GROUPBOX:
		    lret = (LRESULT)DLGC_STATIC;
		    break;
		default:
		    lret = (LONG)DLGC_BUTTON;
		    break;
	    }

    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",lret));
	    return lret;
	

        case WM_GETFONT:
	    lret = (LRESULT)GetWindowWord(hWnd, BWD_HFONT); 
    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",lret));
	    return lret;

        case WM_SETFONT:
	    SetWindowWord(hWnd, BWD_HFONT, wParam);
	    if (lParam) {
		wState = GetButtonStatus(hWnd);
		if (!bOwnerDraw)
		    SendButtonItemDraw(hWnd, ODA_TEXT | ODA_FOCUS,
			wState, FALSE);
		else {
		    SendButtonItemDraw(hWnd,ODA_DRAWENTIRE,wState,TRUE);
		}
	    }

    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
	    return(1L);
	    
        case WM_SETFOCUS:
	    CheckDefPushButton(GetParent(hWnd),(HWND)wParam,hWnd);
	    OrButtonStatus(hWnd, BSF_FOCUS);
	    SendButtonItemDraw(hWnd, ODA_FOCUS, 
			GetButtonStatus(hWnd),
			bOwnerDraw);

    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
	    return 1L;

        case WM_KILLFOCUS:
	    ClearButtonStatus(hWnd, BSF_FOCUS);
	    SendButtonItemDraw(hWnd, ODA_FOCUS, GetButtonStatus(hWnd),
			bOwnerDraw);

    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
            return 1L;

        case WM_SETTEXT:
	    InternalSetText(hWnd, (LPSTR)lParam);
	    if (lParam) {
		wState = GetButtonStatus(hWnd);
		if (!bOwnerDraw)
		    SendButtonItemDraw(hWnd, ODA_TEXT | ODA_FOCUS,
			wState, FALSE);
		else {
		    SendButtonItemDraw(hWnd,ODA_DRAWENTIRE,wState,TRUE);
#ifdef	LATER
		    SendButtonItemDraw(hWnd,ODA_FOCUS,wState,TRUE);
#endif
		}
	    }

    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
	    return 1L;

	  case WM_CHAR:
	    if (TestButtonStatus(hWnd,BSF_DISABLED)) {
    	        APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
		return 1L;
	    }

	    switch ( wParam )
	    {
	      case VK_SPACE:
		switch(wBtnStyle) 
		{
#if (WINVER < 0x030a)
		  case BS_PUSHBOX:
#endif
		  case BS_PUSHBUTTON:
		  case BS_DEFPUSHBUTTON:
		    OrButtonStatus(hWnd, BSF_BUTTONDOWN);
		    SendMessage(hWnd, BM_SETSTATE, TRUE, 0L);
		    SendMessage(hWnd, BM_SETSTATE, FALSE, 0L);
		    ClearButtonStatus(hWnd, BSF_BUTTONDOWN);
		    break;
		  case BS_AUTORADIOBUTTON:
		  case BS_RADIOBUTTON:
		    break;
		  case BS_AUTOCHECKBOX:
		    SendMessage(hWnd, BM_SETCHECK, 
				!IsDlgButtonChecked(GetParent(hWnd), 
						    GetDlgCtrlID(hWnd)), 0);
		  case BS_CHECKBOX:
		    break;
		  case BS_AUTO3STATE:
		    TWIN_Cycle3StateButton(hWnd);
		  case BS_3STATE:
		    break;
		  case BS_GROUPBOX:
		  default:
		    break;
		}
		SendMessage(GetParent(hWnd), WM_COMMAND, 
			    GET_WM_COMMAND_MPS(GetDlgCtrlID(hWnd), 
					       hWnd, BN_CLICKED));
		break;
	    } 

    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
	    return 1L;

        case WM_KEYDOWN:
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            break;

        case WM_LBUTTONDBLCLK:
#ifdef	LATER
	Figure out what this message does extra
#endif
        case WM_LBUTTONDOWN:
	    if (TestButtonStatus(hWnd,BSF_DISABLED)) {
    	        APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
		return 1L;
	    }

	    OrButtonStatus(hWnd, BSF_BUTTONDOWN);
	    if (!TestButtonStatus(hWnd, BSF_CAPTUREACTIVE)) {
	 	SetCapture(hWnd);
		OrButtonStatus(hWnd, BSF_CAPTUREACTIVE);
	    }
	    if (!TestButtonStatus(hWnd, BSF_FOCUS))
		SetFocus(hWnd);
	    GetClientRect(hWnd, &rcClient);
	    pt.x = LOWORD(lParam);
	    pt.y = HIWORD(lParam);
	    if (PtInRect(&rcClient, pt))
		SendMessage(hWnd,BM_SETSTATE,1,0L);

    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
            return 1L;

        case WM_LBUTTONUP:
	    if ((!TestButtonStatus(hWnd, BSF_BUTTONDOWN)) ||
		TestButtonStatus(hWnd,BSF_DISABLED)) {
    	        APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
		return 1L;
	    }
	    ClearButtonStatus(hWnd, BSF_BUTTONDOWN);
	    if (TestButtonStatus(hWnd, BSF_CAPTUREACTIVE)) {
		ReleaseCapture();
		ClearButtonStatus(hWnd, BSF_CAPTUREACTIVE);
	    }
	    pt.x = LOWORD(lParam);
	    pt.y = HIWORD(lParam);
	    GetClientRect(hWnd, &rcClient);
	    if (PtInRect(&rcClient,pt)) {
	        if (wBtnStyle == BS_AUTOCHECKBOX) {
		    wState = GetButtonStatus(hWnd);
		    SendMessage(hWnd,BM_SETCHECK,(wState & 0x1)?0:1,0L);
		}
		if (wBtnStyle == BS_AUTO3STATE) {
		    TWIN_Cycle3StateButton(hWnd);
		}
	        if (wBtnStyle == BS_AUTORADIOBUTTON) {
		    CheckRadioButton(GetParent(hWnd),0,0xffff,
			GetWindowID(hWnd));
		}
		if (TestButtonStatus(hWnd, BSF_SELECTED))
		    SendMessage(hWnd,BM_SETSTATE,0,0L);
	        SendMessage(GetParent(hWnd), WM_COMMAND,
			GET_WM_COMMAND_MPS(GetWindowID(hWnd),
				hWnd, BN_CLICKED));
	    }

    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
            return 1L;

        case WM_MOUSEMOVE:
	    if ((!TestButtonStatus(hWnd, BSF_BUTTONDOWN)) ||
		TestButtonStatus(hWnd,BSF_DISABLED)) {
    	        APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
		return 1L;
	    }
	    pt.x = LOWORD(lParam);
	    pt.y = HIWORD(lParam);
	    GetClientRect(hWnd, &rcClient);
	    if (TestButtonStatus(hWnd, BSF_SELECTED) &&
		!PtInRect(&rcClient,pt)) {
		SendMessage(hWnd,BM_SETSTATE,0,0L);
		break;
	    }
	    if (!TestButtonStatus(hWnd, BSF_SELECTED) &&
		PtInRect(&rcClient,pt)) {
		SendMessage(hWnd,BM_SETSTATE,1,0L);
	    }
            break;

        case WM_NCHITTEST:
	    if (wBtnStyle == BS_GROUPBOX) {
		lret = HTTRANSPARENT;
    	        APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",lret));
		return lret;
	    }
	
            lret = DefWindowProc(hWnd,wMsg,wParam,lParam);
    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",lret));
	    return lret;

        case WM_PAINT:
	    BeginPaint(hWnd,&ps);
	    if (dwStyle & WS_VISIBLE) {
		GetClientRect(hWnd, &rcClient);
		wState = GetButtonStatus(hWnd);
		dis.CtlType = ODT_BUTTON;
		dis.CtlID = GetWindowID(hWnd);
		dis.itemID = 0;
		dis.itemAction = (bOwnerDraw)?ODA_DRAWENTIRE:
			ODA_DRAWENTIRE|ODA_FOCUS;
		dis.hwndItem = hWnd;
		dis.hDC = ps.hdc;
		CopyRect(&dis.rcItem, &rcClient);
		dis.itemData = 0L;
		if (bOwnerDraw) {
		    WORD wUserState = 0;
		    if (wState & BSF_SELECTED)
			wUserState |= ODS_SELECTED;
		    if (wState & BSF_GRAYED)
			wUserState |= ODS_GRAYED;
		    if (wState & BSF_DISABLED)
			wUserState |= ODS_DISABLED;
		    if (wState & BSF_CHECKED)
			wUserState |= ODS_CHECKED;
		    if (wState & BSF_FOCUS)
			wUserState |= ODS_FOCUS;
		    wState = wUserState;
		}
		dis.itemState  = wState;
		if (bOwnerDraw)
		    SendMessage(GetParent(hWnd), WM_DRAWITEM,
				(WPARAM)dis.CtlID, (LONG)&dis);
		else
	    	    ButtonDrawItem(hWnd, &dis, dwStyle);
	    }

	    EndPaint(hWnd,&ps);

    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
            return 1L;

        case BM_GETCHECK:
	    switch(wBtnStyle) {
		case BS_CHECKBOX:
		case BS_AUTOCHECKBOX:
		case BS_RADIOBUTTON:
		case BS_AUTORADIOBUTTON:
		    lret = (LONG)(TestButtonStatus(hWnd, BSF_CHECKED));
    	    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",lret));
		    return lret;

		case BS_3STATE:
		case BS_AUTO3STATE:
		    lret = (LONG)(TestButtonStatus(hWnd,BSF_CHECKMASK));
    	    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",lret));
		    return lret;

		default:
    	    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",0L));
		    return 0L;
	    }
            break;

        case BM_SETCHECK:
	    switch(wBtnStyle) {
		case BS_PUSHBUTTON:
		case BS_DEFPUSHBUTTON:
		case BS_OWNERDRAW:
    	    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",0L));
		    return 0L;

		case BS_CHECKBOX:
		case BS_AUTOCHECKBOX:
		case BS_RADIOBUTTON:
		case BS_AUTORADIOBUTTON:
		    wState = TestButtonStatus(hWnd,BSF_CHECKED);
		    if ( ( wState && (!wParam) ) ||
		         ( (!wState) && wParam ) )
 			ToggleButtonStatus(hWnd, BSF_CHECKED);
		    wState = GetButtonStatus(hWnd);
		    SendButtonItemDraw(hWnd, ODA_SELECT,
			wState, bOwnerDraw);
		    if ( wBtnStyle == BS_AUTORADIOBUTTON ||
		         wBtnStyle == BS_RADIOBUTTON ) {
			dwStyle = GetWindowLong(hWnd, GWL_STYLE);
			if ( wState & BSF_CHECKED )
				dwStyle |= WS_TABSTOP;
			else
				dwStyle &= ~WS_TABSTOP;
			SetWindowLong(hWnd, GWL_STYLE, dwStyle);
		    }

    	    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1));
		    return 1L;

		case BS_3STATE:
		case BS_AUTO3STATE:
		    wState = wParam & BSF_CHECKMASK;
		    if (wState == 3)
			wState = 2;
		    SetButtonStatus(hWnd, wState);
		    SendButtonItemDraw(hWnd, ODA_SELECT, wState, bOwnerDraw);

    	    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1));
		    return 1L;

		case BS_USERBUTTON:
#ifdef	LATER
#endif
		    break;
	    }
            break;

        case BM_GETSTATE:
	    lret = ((LONG)(TestButtonStatus(hWnd, BSF_STATEMASK)));
    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",lret));
	    return lret;

        case BM_SETSTATE:
	    if (TestButtonStatus(hWnd,BSF_SELECTED)) {
		if (wParam) {
    	    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",0L));
		    return 0L;
		}
	    }
	    else {
		if (!wParam) {
    	    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",0L));
		    return 0L;
	 	}
	    }

 	    ToggleButtonStatus(hWnd, BSF_SELECTED);
	    wState = GetButtonStatus(hWnd);
	    SendButtonItemDraw(hWnd,
		(bOwnerDraw)?ODA_SELECT:ODA_DRAWENTIRE|ODA_FOCUS,
		wState, bOwnerDraw);

    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
	    return 1L;

        case BM_SETSTYLE:
	    SetControlStyle(hWnd, wParam);
	    if (lParam) {
		wState = GetButtonStatus(hWnd);
		if (!bOwnerDraw)
		    SendButtonItemDraw(hWnd, ODA_DRAWENTIRE | ODA_FOCUS, 
			wState, FALSE);
		else {
		    SendButtonItemDraw(hWnd,ODA_DRAWENTIRE,wState,TRUE);
#ifdef	LATER
		    SendButtonItemDraw(hWnd,ODA_FOCUS,wState,TRUE);
#endif
		}
	    }

    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",1L));
            return 1L;

	case WM_CONVERT:
	    if (!lpButtonBinToNat) {
		hButtonClass32 = FindClass("BUTTON",0);
		lpButtonBinToNat = (WNDPROC)GetClassHandleLong(
				hButtonClass32,GCL_BINTONAT);
	    }
	    if (lpButtonBinToNat) {
		lret = lpButtonBinToNat(hWnd, wMsg, wParam, lParam);
	    } else {
		lret = 0L;
	    }

    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",lret));
	    return lret;

	default:
	    lret = DefWindowProc(hWnd, wMsg, wParam, lParam);
    	    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",lret));
	    return lret;
    }

    APISTR((LF_APIRET,"DefBUTTONProc: return LRESULT %x\n",0));
    return 0L;
}

void TWIN_Cycle3StateButton(HWND hWnd)
{
    unsigned int check_state = SendMessage(hWnd, BM_GETCHECK, 0, 0);
    
    check_state++;
    if (check_state > 2)
	check_state = 0;
    
    SendMessage(hWnd, BM_SETCHECK, check_state, 0);
}
