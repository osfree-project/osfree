/*
 *      prtdemo.c
 *
 *      @(#)prtdemo.h	1.1 5/3/96 16:43:48 /users/sccs/src/samples/prtdemo/s.prtdemo.h
 *
 *      Printing Demonstration Application
 *
 *      Copyright Willows Software, Inc. 1995
 * 
 */

long FAR PASCAL __export PrtWndProc(HWND, UINT, WPARAM, LPARAM);
void FAR PASCAL ShellAbout(HWND, LPCSTR, LPCSTR, HICON);


#define IDM_ABOUT       100
#define IDM_EXIT        101
#define IDM_PRINT       102
#define IDM_SETUP       103 
#define IDM_HELP        104
#define IDM_TWIPS       105
#define IDM_TEXT        106

#define IDI_ICON1           200

