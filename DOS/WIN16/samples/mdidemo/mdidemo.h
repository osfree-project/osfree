/*
 *      MDIDemo.h
 *
 *      @(#)mdidemo.h	1.2 7/25/96 18:29:15 /users/sccs/src/samples/mdidemo/s.mdidemo.h
 * 
 *      MDI Windows Sample Application
 *
 */

#ifndef _mdidemo_h_
#define _mdidemo_h_

#ifndef RC_INVOKED
#include <stdlib.h>
#endif
#include "windows.h"

int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);
long FAR PASCAL _export FrameWndProc(HWND, UINT, WPARAM, LPARAM);
long FAR PASCAL _export ChildWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL FAR PASCAL _export CloseChildProc(HWND, LONG);

/* # of submenu where windowlist is to be appended */
/* this position can be different depending on the menu used */
#define WLPOS_INITMENU  0
#define WLPOS_MDIMENU   1

/* Menuitems */
#define IDM_NEW         100
#define IDM_ABOUT       101
#define IDM_EXIT        102
#define IDM_CLOSE       103
#define IDM_CASCADE     110
#define IDM_TILE        111
#define IDM_CLOSEALL    112
#define IDM_ARRANGE     113

#define IDM_FIRSTCHILD  200

/* String resource */
#define MDI_APP_TITLE   200
                     
/* icons */                     
#define IDI_WILLOWS     100
                
/* Following is a prototype for an external DLL function. */
void FAR PASCAL ShellAbout(HWND, LPCSTR, LPCSTR, HICON);

#endif  /* _mdidemo_h_ */
                
