/*
 *  dlgdemo.c -- A simple windows program
 *
 *      @(#)dlgdemo.c	1.4 10/10/96 15:13:42 /users/sccs/src/samples/dlgdemo/s.dlgdemo.c
 *
 *	Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
 *
 */

#include <ctype.h>
#include "windows.h"
#include "dlgdemo.h"

char _filler_[60000] = { "333333333333333..." };

static HINSTANCE hThisInst = (HINSTANCE)NULL;

static LPSTR Letters[] = { "club",
			   "heart",
			   "diamond",
			   "spade",
			   "ace",
			   "king",
			   "queen",
			   "jack",
			   "trump",
			   "double" };

static LPSTR Numbers[] = { "1 -- one",
			   "2 -- two",
			   "3 -- three",
			   "4 -- four",
			   "5 -- five",
			   "6 -- six",
			   "7 -- seven",
			   "8 -- eight",
			   "9 -- nine",
			   "10 -- ten" };

static int Index = 0;

BOOL __export CALLBACK
About(HWND hDlg, UINT msg, WORD wParam, LONG lParam)
{
    switch (msg) {
       case WM_INITDIALOG:
	   return TRUE;
	   
       case WM_COMMAND:
	   EndDialog(hDlg, TRUE);
	   return TRUE;
    
    } /* endswitch() */
    
	return FALSE;

} /* About() */

BOOL __export CALLBACK
Dialog(HWND hDlg, UINT msg, WORD wParam, LONG lParam)
{
    char String[40];
    int Place;
    
    switch (msg) {
	case WM_INITDIALOG:
	CheckRadioButton(hDlg, IDD_NUMBER, IDD_LETTER, IDD_NUMBER);
	    return TRUE;

	case WM_COMMAND:
	    switch ( wParam ) {
		case IDCANCEL:
			EndDialog(hDlg, TRUE);
		break;
			    
		case IDOK:
		if ( IsDlgButtonChecked(hDlg, IDD_LETTER) )
		    lstrcpy(String, Letters[Index]);
		else
		    lstrcpy(String, Numbers[Index]);
		Index = (Index+1)%10;
		if ( IsDlgButtonChecked(hDlg, IDD_UPPER) ) {
		    for ( Place = 0; String[Place]; Place++ )
			String[Place] = (char)toupper(String[Place]);
		}
		SendDlgItemMessage(hDlg, IDD_LIST, LB_ADDSTRING, 0, (LPARAM)(LPSTR)String);
		break;

	    case IDD_NUMBER:
	    case IDD_LETTER:
		CheckRadioButton(hDlg, IDD_NUMBER, IDD_LETTER, wParam);
		break;
		
	    case IDD_UPPER:
		CheckDlgButton(hDlg, wParam, 1 - IsDlgButtonChecked(hDlg, wParam));
		break;
	    }
	return TRUE;
    }
    return FALSE;
}

long __export CALLBACK
DlgWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    FARPROC lpProcDialog;
    
	if ( msg == WM_COMMAND ) {
	    switch (wParam) {

		case IDM_DLGBOX:
		    lpProcDialog = MakeProcInstance((FARPROC)Dialog, hThisInst);
		    DialogBox(hThisInst, "DlgBox", hWnd, lpProcDialog);
		    FreeProcInstance(lpProcDialog);
		    break;

		case IDM_ABOUT:
		    lpProcDialog = MakeProcInstance((FARPROC)About, hThisInst);
		    DialogBox(hThisInst, "AboutBox", hWnd, lpProcDialog);
		    FreeProcInstance(lpProcDialog);
		    break;
		    
		case IDM_EXIT:
		    DestroyWindow(hWnd);
		    break;
			}    
	}
	else if ( msg == WM_DESTROY || msg == WM_CLOSE )
	    PostQuitMessage(0);
	
	return (DefWindowProc(hWnd, msg, wParam, lParam));

} /* DlgWndProc() */

BOOL PASCAL
WinMain( HINSTANCE hInstance,
	 HINSTANCE hPrevInst, 
	 LPSTR lpCommand,
	 int nShow)
{
    HWND hWnd;
    MSG Message;

    if ( !hPrevInst ) {
	WNDCLASS wc;

	wc.style = 0;
	wc.lpfnWndProc = DlgWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon((HINSTANCE)NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = "DlgMenu";
	wc.lpszClassName = "DlgClass";

	if ( !RegisterClass(&wc) )
	    return FALSE;
    }

    hThisInst = hInstance;

    hWnd = CreateWindow(
	"DlgClass",
	"DlgDemo",
	WS_OVERLAPPEDWINDOW,
	CW_USEDEFAULT,
	CW_USEDEFAULT,
	200,
	200,
	(HWND)NULL,
	(HMENU)NULL,
	hInstance,
	NULL);

    if ( !hWnd )
	return FALSE;

    ShowWindow(hWnd, nShow);
    UpdateWindow(hWnd);
		

    while ( GetMessage(&Message,(HWND)NULL, 0, 0) ) {
	TranslateMessage(&Message);
	DispatchMessage(&Message);
    }

    return (BOOL)Message.wParam;

} /* WinMain() */
