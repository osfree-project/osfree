/*
 *  dlgdemo.h -- Header file for dlgdemo
 *
 *	@(#)dlgdemo.h	1.2 2/13/96 13:01:25 /users/sccs/src/samples/dlgdemo/s.dlgdemo.h
 *
 *	Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
 *
 */

long FAR PASCAL __export DlgWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL FAR PASCAL __export About(HWND, unsigned, WORD, LONG);
BOOL FAR PASCAL __export DlgBox(HWND, unsigned, WORD, LONG);

#define IDM_ABOUT	100
#define IDM_EXIT 	101
#define IDM_DLGBOX  102

#define IDD_LIST    110
#define IDD_NUMBER  111
#define IDD_LETTER  112
#define IDD_UPPER   113

#define IDI_ICON1   200
