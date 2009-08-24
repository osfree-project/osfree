/*
 *      CDSDemo.h
 *
 *	@(#)cdsdemo.h	1.2 5/3/96 16:10:08 /users/sccs/src/samples/cdsdemo/s.cdsdemo.h
 *
 *      Commmon Dialogs Demonstration Application
 *
 */

long FAR PASCAL __export CDSWndProc(HWND, UINT, WPARAM, LPARAM);
void FAR PASCAL ShellAbout(HWND, LPCSTR, LPCSTR, HICON);

#define IDM_ABOUT       100
#define IDM_EXIT        101
#define IDM_COLOR       102
#define IDM_FONT        103
#define IDM_OPEN        104
#define IDM_SAVE        105
#define IDM_PRINT       106

#define IDI_ICON1       200
