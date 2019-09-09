/* dlldemo.h */

#ifndef _dlldemo_h_
#define _dlldemo_h_

#ifndef RC_INVOKED
#include <stdlib.h>
#ifndef macintosh
#include <malloc.h>
#endif /* macintosh */
#endif /* RC_INVOKED */

#include "windows.h"

int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);

BOOL DllDemoInit(HANDLE);
long FAR PASCAL __export DllWndProc(HWND, UINT, WPARAM, LPARAM);

#define IDM_LOAD        100
#define IDM_FREE        101
#define IDM_ABOUT       102
#define IDM_EXIT        103
#define IDM_BITMAP      104

#define DLL_APP_TITLE           200
#define DLL_IDS_RESOURCE        201

#define IDI_WILLOWS     100
                
/* Following is a prototype for an external DLL function. */

void FAR PASCAL ShellAbout(HWND, LPCSTR, LPCSTR, HICON);

#endif  /* _dlldemo_h_ */
                
