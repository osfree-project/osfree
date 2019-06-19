/*
 *	ctrldemo.c -- Simple Windows Demonstration Application
 *	@(#)ctrldemo.c	1.2 10/23/96 12:52:03 /users/sccs/src/samples/ctrldemo/s.ctrldemo.c
 *
 *	Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
 *
 *	Demonstrates the following:
 *
 *	- use of resource and module definition compilers.
 *	- registering classes, and creating windows.
 *	- standard message loop and message dispatch logic.
 *	- use of resources, menues, bitmaps, stringtables and accelerators.
 *	- calling the ShellAbout API.
 *	- use of Toolbars, Status Windows, Spin Buttons, Tab Controls, 
 *	  Property Sheets, and Tooltips from the Common Controls API.
 */

#define NONAMELESSUNION

#include "windows.h"
#include "commctrl.h"
#include "resource.h"

BOOL CALLBACK PropPg_DlgProc(HWND, UINT, WPARAM, LPARAM);
char _filler_[60000] = { "333333333333333..." };

HWND hWnd, hWndStatus, hWndToolbar;
HANDLE	hInstance = (HANDLE)NULL;
char    Program[] = "CtrlDemo";
char    Title[256] = "";

void CreatePropSheet(HWND hwndOwner)
{
    PROPSHEETPAGE psp[3];
    PROPSHEETHEADER psh;

    psp[0].dwSize = sizeof(PROPSHEETPAGE);
    psp[0].dwFlags = PSP_USEICONID;
    psp[0].hInstance = hInstance;
    psp[0].u.pszTemplate = MAKEINTRESOURCE(IDD_TAB1);
    psp[0].u2.pszIcon = MAKEINTRESOURCE(IDI_ICON1);
    psp[0].pfnDlgProc = PropPg_DlgProc;
    psp[0].pszTitle = "Test1";
    psp[0].lParam = 0;

    psp[1].dwSize = sizeof(PROPSHEETPAGE);
    psp[1].dwFlags = PSP_USEICONID;
    psp[1].hInstance = hInstance;
    psp[1].u.pszTemplate = MAKEINTRESOURCE(IDD_TAB2);
    psp[1].u2.pszIcon = MAKEINTRESOURCE(IDI_ICON1);
    psp[1].pfnDlgProc = PropPg_DlgProc;
    psp[1].pszTitle = "Test2";
    psp[1].lParam = 0;

    psp[2].dwSize = sizeof(PROPSHEETPAGE);
    psp[2].dwFlags = PSP_USEICONID;
    psp[2].hInstance = hInstance;
    psp[2].u.pszTemplate = MAKEINTRESOURCE(IDD_TAB3);
    psp[2].u2.pszIcon = MAKEINTRESOURCE(IDI_ICON1);
    psp[2].pfnDlgProc = PropPg_DlgProc;
    psp[2].pszTitle = "Test3";
    psp[2].lParam = 0;	
	
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.hwndParent = hwndOwner;
    psh.hInstance = hInstance;
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGE);
    psh.u3.ppsp = (LPCPROPSHEETPAGE) &psp;
	psh.u2.nStartPage = 0;
    psh.dwFlags = PSH_PROPSHEETPAGE; 
       
	PropertySheet(&psh);
}

void
CreateToolbar(HWND hwOwner)
{

	TBBUTTON Buttons[3];

	Buttons[0].iBitmap = 1;
	Buttons[0].idCommand = IDM_PROPSHEET;
	Buttons[0].fsState = TBSTATE_ENABLED;
	Buttons[0].fsStyle = TBSTYLE_BUTTON;
	Buttons[0].dwData = 0L;
	Buttons[0].iString = 0;

    Buttons[1].iBitmap = 2; 
    Buttons[1].idCommand = IDM_UPDOWN;
    Buttons[1].fsState = TBSTATE_ENABLED;
    Buttons[1].fsStyle = TBSTYLE_BUTTON;
    Buttons[1].dwData = 0L;
    Buttons[1].iString = 0;

    Buttons[2].iBitmap = 0; 
    Buttons[2].idCommand = IDM_ABOUT;
    Buttons[2].fsState = TBSTATE_ENABLED;
    Buttons[2].fsStyle = TBSTYLE_BUTTON;
    Buttons[2].dwData = 0L;
    Buttons[2].iString = 0;

	hWndToolbar=CreateToolbarEx (hwOwner,
			WS_VISIBLE | WS_CHILD | WS_BORDER | CCS_TOP |
			TBSTYLE_TOOLTIPS,
			ID_TOOLBAR,
			3,
			hInstance, 
			IDB_TOOLBAR,
			Buttons,
			3,
			16, 16, 
			16, 16,	
			sizeof(TBBUTTON));
}

void
CreateStatusBar(HWND hwOwner) 
{
	hWndStatus = CreateStatusWindow(
			WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 
			"Ready",
			hwOwner,	
			4242 );	
}

BOOL APIENTRY UpDownProc(
        HWND hDlg,
        UINT message,
        UINT wParam,
        LONG lParam )
{
        static HWND hWndUpDown, hWndBuddy;
        
        switch (message)
        {
                case WM_INITDIALOG:
                        hWndBuddy = GetDlgItem(hDlg, IDE_BUDDY);

                        hWndUpDown = CreateWindowEx(
        	                        0L,
               	 	                UPDOWN_CLASS,
                              		"",
                                	WS_CHILD | WS_BORDER | WS_VISIBLE | 
					UDS_WRAP | UDS_ARROWKEYS | 
                                        UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
                                	135,80,30,22,
                                	hDlg,
                                	(HMENU)ID_UPDOWN,
                                	hInstance,
                                	NULL);

                        SendMessage( hWndUpDown, UDM_SETBUDDY,
                                     (LONG)hWndBuddy, 0L );

			SendMessage( hWndUpDown, UDM_SETRANGE, 0L, 
				     MAKELONG (100, 1));

                        SetDlgItemInt( hDlg, IDE_BUDDY, 1, FALSE );
                        return TRUE;

                case WM_COMMAND:
                        switch (LOWORD(wParam))
                        {
                                case IDOK:
									InvalidateRect(hWnd, NULL, TRUE);
                                case IDCANCEL:
                               		EndDialog(hDlg, TRUE);
	                        		break;
                        }
                  	break;
        }
        return (FALSE);
}

void GetToolTipText ( LPTOOLTIPTEXT lpttt)
{
	switch(lpttt->hdr.idFrom)
	{
		case IDM_ABOUT:
			lpttt->lpszText = "About CtrlDemo";
			break;
		case IDM_UPDOWN:
			lpttt->lpszText = "Up Down Spin Button Demo";
			break;
		case IDM_PROPSHEET:	
			lpttt->lpszText = "Property Sheet Demo";
			break;
	}
	lpttt->hinst = (HINSTANCE)NULL;
}

void 
DisplayStatusMenuHelp ( HWND hWndStatus, UINT idMsg )
{
	static char sz[80];

	if ( idMsg != -1 ) 
	{
		SendMessage( hWndStatus, SB_SIMPLE, TRUE, 0);
		if ( idMsg != 0 ) 
		{
			switch(idMsg)
			{
				case IDM_ABOUT:
				        wsprintf(sz, "About CtrlDemo");
					break;
				case IDM_EXIT:
					wsprintf(sz, "Exit CtrlDemo");
					break;
				case IDM_UPDOWN:
					wsprintf(sz,"Up Down Spin Button Demo");
					break;
				case IDM_PROPSHEET:
					wsprintf(sz,"Property Sheet Demo");
					break;
			}
			SendMessage ( hWndStatus, SB_SETTEXT, 255,
					(LPARAM)sz );
		}
		else
		{
			SendMessage ( hWndStatus, SB_SIMPLE, FALSE, 0);
		}
	}
}

#define GET_WM_MENUSELECT_FLAGS(wParam, lParam) ((UINT)HIWORD(wParam))
#define GET_WM_MENUSELECT_CMD(wParam, lParam) ((UINT)LOWORD(wParam))
#define GET_WM_MENUSELECT_HMENU(wParam, lParam) ((HMENU)lParam)

void 
GetMenuSelect ( HWND hWndStatus, UINT message, 
			WPARAM wParam, LPARAM lParam) 
{
	UINT nStringID = 0;

	UINT fuFlags = GET_WM_MENUSELECT_FLAGS(wParam, lParam) & 0xffff;
	UINT uCmd = GET_WM_MENUSELECT_CMD(wParam, lParam);
	HMENU hMenu = GET_WM_MENUSELECT_HMENU(wParam, lParam);

	if (fuFlags == 0xffff && hMenu == (HMENU)NULL)
		nStringID = (UINT) -1;

	else if (fuFlags & MF_SEPARATOR)
		nStringID = 0;

	else if (fuFlags & MF_POPUP)
	{
		if (fuFlags & MF_SYSMENU)
			nStringID = 0;
	}
	else
	{
		nStringID = uCmd;
	}
	DisplayStatusMenuHelp(hWndStatus, nStringID);
}


long FAR PASCAL 
WinWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HICON hIcon;
	FARPROC lpDlg = (FARPROC)NULL;
	LPNMHDR pnmh;

	pnmh = (LPNMHDR) lParam; 

	switch (message) {
	case WM_CREATE:
		hInstance = ( ( LPCREATESTRUCT )lParam )->hInstance;
		InitCommonControls();
        CreateToolbar(hWnd);
		CreateStatusBar(hWnd);
		break;
	case WM_MENUSELECT:
		GetMenuSelect (hWndStatus, message, wParam, lParam);
		break;
	case WM_NOTIFY:
		switch (pnmh->code)
		{
			case TTN_NEEDTEXT:
				GetToolTipText((LPTOOLTIPTEXT)lParam);
				break;
			default:
				return (DefWindowProc(hWnd, message, 
					wParam, lParam));
		}	
		break;
	case WM_COMMAND:
        	switch (wParam) {
        	case IDM_ABOUT:
               		hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	       		    ShellAbout(hWnd, Program, Title, hIcon);
               		DestroyIcon(hIcon);
               		break;
	    	case IDM_UPDOWN:
               		if ( DialogBox ( hInstance, MAKEINTRESOURCE 
					( IDD_UPDOWNTEST ), hWnd,
                     			(DLGPROC)UpDownProc ) == -1 )
                       		MessageBox ( hWnd, "Dialog Failed", "Error", 
						MB_OK);
	       	    	break;
			case IDM_PROPSHEET:
					CreatePropSheet(hWnd);
					break;
			case IDM_EXIT:
	       		DestroyWindow(hWnd);
	       		break;
          	 }
	break;
	case WM_SIZE:
       		SendMessage(hWndToolbar, message, wParam, lParam);
     		SendMessage(hWndStatus, message, wParam, lParam);
       		switch (wParam) {
       		case SIZEICONIC:
	    		MessageBeep(0);
           	}
           	break;
    	    case WM_DESTROY:
       		    PostQuitMessage(0);
        	    break;
	default:
        	return (DefWindowProc(hWnd, message, wParam, lParam));
	}
	return (0);
}

BOOL PASCAL 
WinMain(HANDLE hInst, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HACCEL hAccelTable;
	MSG    Msg;

	if (!hPrevInstance) {
       		WNDCLASS wc;

        	wc.style         = 0;
        	wc.lpfnWndProc   = WinWndProc;
        	wc.cbClsExtra    = 0;
        	wc.cbWndExtra    = 0;
        	wc.hInstance     = hInst;
        	wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
        	wc.hCursor       = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
        	wc.hbrBackground = GetStockObject(WHITE_BRUSH);
        	wc.lpszMenuName  = (LPSTR) "WinMenu";
        	wc.lpszClassName = (LPSTR) Program;

        	if ( !RegisterClass(&wc) )
            		return FALSE;
    	}

	LoadString(hInst,IDS_TITLE,Title,256);
	hInstance = hInst;

	hAccelTable = LoadAccelerators( hInstance, (LPSTR)"RESOURCE" );
	hWnd = CreateWindow(Program,
			Title,
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			400,
			300,
			(HWND)NULL,
			(HMENU)NULL,
			hInst,
			NULL);

	if (!hWnd)
       		return FALSE;
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Msg, (HWND)NULL, 0, 0)) {
      		if (TranslateAccelerator( hWnd, hAccelTable, 
			(LPMSG)&Msg ) == 0) 
		{
		      	TranslateMessage(&Msg);
      			DispatchMessage(&Msg);
      		}	
	}
    	return Msg.wParam;
}



