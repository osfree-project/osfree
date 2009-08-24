/*
 *	windemo.c -- Simple Windows Demonstration Application
 *
 *      @(#)windemo.c	1.2 10/10/96 15:24:29 /users/sccs/src/samples/windemo/s.windemo.c	
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
 */

#include "windows.h"
#include "windemo.h"

void FAR PASCAL ShellAbout(HWND, LPCSTR, LPCSTR, HICON);

char _filler_[64*1024] = { "333333333333333..." };

HANDLE	hInstance = (HANDLE)NULL;
char    Program[] = "WinDemo";
char    Title[256] = "";

long FAR PASCAL __export 
WinWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
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
           }
	   break;

    case WM_SIZE:
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

        if ( !RegisterClass(&wc) )
            return FALSE;
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

