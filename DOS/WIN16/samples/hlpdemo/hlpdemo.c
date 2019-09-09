/*
 *  hlpdemo.c -- Simple Windows Demonstration Application
 *
 *  @(#)hlpdemo.c	1.3 10/10/96 15:21:21 /users/sccs/src/samples/hlpdemo/s.hlpdemo.c 
 *
 *  Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
 *
 *  Demonstrates the following:
 *
 *  - use of the WinHelp() API function.
 *  - use of resource and module definition compilers.
 *  - registering classes, and creating windows.
 *  - standard message loop and message dispatch logic.
 *  - use of resources, menues, bitmaps, stringtables and accelerators.
 *  - calling the ShellAbout API.
 */

#include "windows.h"
#include "hlpdemo.h"
#include "hlpalias.h"

void FAR PASCAL ShellAbout(HWND, LPCSTR, LPCSTR, HICON);

char _filler_[60000] = { "333333333333333..." };

HANDLE  hInstance = (HANDLE)NULL;
char    Program[] = "HlpDemo";
char    HelpFile[] = "hlpdemo.hlp";
char    Title[256] = "";

long FAR PASCAL __export 
WinWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

    HICON hIcon;

    switch (message) {

    case WM_COMMAND:
				switch (wParam) {
				           
				case IDM_ABOUT:
				   hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
				   ShellAbout(hWnd, Program, Title, hIcon);
				   DestroyIcon(hIcon);
				   break;
				           
				case IDM_EXIT:
				   DestroyWindow(hWnd);
				   break;
				           
				/* Show the Contents topic of the help file. */
				case IDM_HELPCONTENTS:
				   WinHelp( hWnd, HelpFile, HELP_CONTENTS, (DWORD)NULL );
				   break;
				           
				/* Show a topic in the help file. */
				case IDM_HELPTOPIC:
				   WinHelp( hWnd, HelpFile, HELP_CONTEXT, IDH_FILEMENU );
				   break;
				           
				/* Show the search dialog box with the help file. */
				case IDM_HELPSEARCH:
				   WinHelp( hWnd, HelpFile, HELP_PARTIALKEY, (DWORD)(LPSTR) "" );
				   break;
				               
				/* Show the help viewer's help file. */
				case IDM_HELPONHELP:
				   WinHelp( hWnd, NULL, HELP_HELPONHELP, (DWORD)NULL );
				   break;
				}
				break;

    case WM_SIZE:

				switch (wParam) {
				  case SIZEICONIC:
				      MessageBeep(0);
				}
				break;

    case WM_DESTROY:
           
				/* Close the help viewer's window. */
				WinHelp( hWnd, Program, HELP_QUIT, (DWORD)NULL );
				           
				PostQuitMessage(0);
				break;

    default:
        return (DefWindowProc(hWnd, message, wParam, lParam));
    }

    return (0);
}


BOOL PASCAL 
WinMain(HANDLE hInst, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

    HACCEL hAccelTable;
    HWND   hWnd;
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
				
				if ( !RegisterClass(&wc) ) return FALSE;
    }

    LoadString(hInst,IDS_TITLE,Title,256);
    hInstance = hInst;

    hAccelTable = LoadAccelerators( hInstance, (LPSTR)"RESOURCE" );

    hWnd = CreateWindow(Program,
				Title,
				WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
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
				if (TranslateAccelerator( hWnd, hAccelTable, (LPMSG)&Msg ) == 0) {
						TranslateMessage(&Msg);
						DispatchMessage(&Msg);
				} 
    }
    return Msg.wParam;
}

