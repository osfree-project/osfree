/*************************************************************************
*
*	TViewApp.c
*
*	@(#)TViewApp.c	1.2
*
*	Copyright (c) 1995-1997, Willows Software Inc.  All rights reserved.
*
**************************************************************************/

#include "TViewApp.h"
#include "commctrl.h"
#include "resource.h"

HWND        g_hwndMain = ( HWND )NULL;
HINSTANCE   _hInstance = ( HINSTANCE )NULL;
extern HWND       g_hMDlg; 
extern HTREEITEM  g_hItem;

int PASCAL WinMain(HINSTANCE hinst, HINSTANCE hinstPrev, LPSTR lpszCmdLine, int cmdShow)
{
    MSG       msg;

    _hInstance = hinst;
    if (!hinstPrev)
    {
        WNDCLASS cls;

        cls.hCursor         = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
        cls.hIcon           = LoadIcon(_hInstance, MAKEINTRESOURCE(IDI_WILLOWS));
        cls.lpszMenuName    = MAKEINTRESOURCE(IDR_MAINMENU);
        cls.hInstance       = hinst;
        cls.lpszClassName   = "TView_MainWnd";
        cls.hbrBackground   = (HBRUSH)(COLOR_WINDOW+1);
        cls.lpfnWndProc     = Main_WndProc;
        cls.style           = CS_DBLCLKS;
        cls.cbWndExtra      = 0;
        cls.cbClsExtra      = 0;

        if (!RegisterClass(&cls))
            return FALSE;
    }

    g_hwndMain = CreateWindowEx(
            0L,
            "TView_MainWnd",
            "TreeView Sample Application",
            WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
            CW_USEDEFAULT, CW_USEDEFAULT,
            CW_USEDEFAULT, CW_USEDEFAULT,
            (HWND)NULL,
            (HMENU)NULL,
            hinst,
            NULL);

    if (g_hwndMain == (HWND)NULL)
    return FALSE;

    InitCommonControls();
    ShowWindow(g_hwndMain, cmdShow);

    /*
    // If a message exists in the queue, translate and dispatch it.
    */
    while(GetMessage(&msg, (HWND)NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    DestroyWindow(g_hwndMain);
    g_hwndMain = (HWND)	NULL;

    return TRUE;
}

LRESULT CALLBACK Main_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        HANDLE_MSG(hwnd, WM_CLOSE, Main_OnClose);
        HANDLE_MSG(hwnd, WM_COMMAND, Main_OnCommand);
        
        case WM_SYSCOLORCHANGE:
            /*HLSystemColorChange();*/
            return TRUE;
        break;
        
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

VOID Main_OnClose(HWND hwnd)
{
    PostQuitMessage(0);
    if ( g_hMDlg )
	DestroyWindow ( g_hMDlg );
}

int CALLBACK
SortCompare ( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
    if ( lParam1 < lParam2 )
        return -1;
    else
    if ( lParam1 > lParam2 )
        return  1;
    return 0;
}

VOID Main_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT code)
{
    switch (id)
    {
       case IDM_FILEEXIT:
           PostQuitMessage(0);
           break;

       case IDM_HELPABOUT:
           AboutDlg_Do(hwnd);
           break;

       case IDM_TREEVIEW:
           TVDialog_Do(hwnd, ID_HEIRDLG );
           break;       

       case IDM_EDITITEM:
           {
                HTREEITEM   hItem;
           if ( g_hMDlg )
           {
               HWND hTreeV = GetDlgItem ( g_hMDlg,IDC_TREEVIEW );
               hItem = TreeView_GetSelection ( hTreeV );
               TreeView_EditLabel ( hTreeV, hItem );
           }
           break;
           }

       case IDM_ENDEDITSAVE:
       case IDM_ENDEDITNOSAVE:
           if ( g_hMDlg )
           {
               HWND hTreeV = GetDlgItem ( g_hMDlg,IDC_TREEVIEW );
               TreeView_EndEditLabelNow ( hTreeV, 
                   id == IDM_ENDEDITSAVE ? FALSE : TRUE );
           }
           break;
       
       case IDM_SELECTITEM:
           if ( g_hMDlg && g_hItem )
               TreeView_SelectItem ( GetDlgItem ( g_hMDlg,IDC_TREEVIEW ) , 
                                     g_hItem );
           break;
       
       case IDM_SETINDENT:
           if ( g_hMDlg )
           {
               HWND hwTree = GetDlgItem ( g_hMDlg,IDC_TREEVIEW );
               int iIndent = TreeView_GetIndent ( hwTree );
               TreeView_SetIndent ( hwTree, iIndent + 2 );
           }
           break;
       
       case IDM_SORTCHILDREN:
           if ( g_hMDlg )
           {
               HTREEITEM   hItem;
               HWND hTreeV = GetDlgItem ( g_hMDlg,IDC_TREEVIEW );
               hItem = TreeView_GetSelection ( hTreeV );
               TreeView_SortChildren ( hTreeV, hItem, 0 );
           }
           break;

       case IDM_SORTCHILDRENCB:
           if ( g_hMDlg )
           {
               TV_SORTCB   tvSort;
               /*HTREEITEM   hItem;*/
               HWND hTreeV = GetDlgItem ( g_hMDlg,IDC_TREEVIEW );
               tvSort.hParent = TreeView_GetSelection ( hTreeV );
               tvSort.lParam = 0;
               tvSort.lpfnCompare = SortCompare;
               TreeView_SortChildrenCB ( hTreeV, &tvSort, 0 );
           }
           break;
    }
}

/* About box
 */
VOID AboutDlg_Do(HWND hwndOwner)
{
    DLGPROC lpfndp;

    lpfndp = (DLGPROC)MakeProcInstance((FARPROC)AboutDlg_DlgProc, _hInstance);

    if (!lpfndp)
        return;

    DialogBoxParam(_hInstance, MAKEINTRESOURCE(IDR_ABOUTDLG),
            hwndOwner, lpfndp, 0L);


    FreeProcInstance((FARPROC)lpfndp);
}

BOOL CALLBACK AboutDlg_DlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {

    case WM_COMMAND:
        if (wParam == IDOK || wParam == IDCANCEL)
            EndDialog(hwndDlg, TRUE);
        return TRUE;
        break;

    case WM_INITDIALOG:
        return TRUE;
    }
    return FALSE;
}
