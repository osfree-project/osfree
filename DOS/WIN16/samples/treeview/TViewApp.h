/*************************************************************************
*
*	TViewApp.h
*
*	"@(#)TViewApp.h	1.2
*
*	Copyright (c) 1995-1997, Willows Software Inc.  All rights reserved.
*
**************************************************************************/

#ifndef TViewApp__h
#define TViewApp__h

#include "windows.h"
#include "windowsx.h"


int PASCAL WinMain(HINSTANCE hinst, HINSTANCE hinstPrev, LPSTR lpszCmdLine, int cmdShow);
LRESULT CALLBACK Main_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AboutDlg_DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

extern HINSTANCE _hInstance;
extern HINSTANCE _hPrevInstance;
extern int _cmdShow;
extern HWND g_hwndMain;


VOID AboutDlg_Do(HWND hwndOwner);
VOID Main_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT code);
VOID Main_OnClose(HWND hwnd);
int  HLDialog_Do(HWND hwndOwner,UINT DlgTemplateID);
int  TVDialog_Do(HWND hwndOwner,UINT DlgTemplateID);
VOID HLSystemColorChange(VOID);




#define IDR_MAINMENU            1


#define IDR_LISTICONS           999

#define IDR_MAINICON            100
#define IDI_WILLOWS             105

#define IDR_ABOUTDLG            200

#define IDHEIRDLG                   100
#define IDLIST                      101
#define ID_HEIRDLG                  106
#define IDHEIRDLG_OLD               100
#define IDC_TREEVIEW                1000
#endif /* TViewApp__h */
