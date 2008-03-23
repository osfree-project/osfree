/*
 *      prtdemo.c
 *
 *      @(#)prtdemo.c	1.2 10/10/96 15:22:59 /users/sccs/src/samples/prtdemo/s.prtdemo.c 
 *
 *      Printing Demonstration Application
 *
 *      Copyright Willows Software, Inc. 1995
 *
 *      Demonstrates the following:
 *
 *      - use of resource compilers.
 *      - registering classes, and creating windows.
 *      - standard message loop and message dispatch logic.
 *      - use of resources, menues.
 *      - use of printer common dialogs 
 *      - calling the ShellAbout library call.
 */

#include "windows.h"
#include "commdlg.h"
#include "prtdemo.h"

static HINSTANCE hThisInst = (HINSTANCE)NULL;
int ONE_INCH = 1440;
int PRINT_MARGIN = 720;     /* / 2 */


HFONT CreatePrintFont(int Points)
{
    LOGFONT lf;

    lf.lfHeight = Points;
    lf.lfWidth = 0;
    lf.lfEscapement = 0;
    lf.lfOrientation = 0;
    lf.lfWeight = 0;
    lf.lfItalic = 0;
    lf.lfUnderline = 0;
    lf.lfStrikeOut = 0;
    lf.lfCharSet = 0;
    lf.lfOutPrecision = 0;
    lf.lfClipPrecision = 0;
    lf.lfQuality = 0;
    lf.lfPitchAndFamily = 0;
    wsprintf((LPSTR)lf.lfFaceName,(LPSTR)"Helv");

    return CreateFontIndirect((LPLOGFONT)&lf);
}

void SetPrinterMapMode(HDC hPrtDC)
{
    SetMapMode( hPrtDC, MM_ANISOTROPIC );
    SetWindowExt( hPrtDC, ONE_INCH, ONE_INCH );
    SetViewportExt( hPrtDC, GetDeviceCaps( hPrtDC, LOGPIXELSX),
                            GetDeviceCaps( hPrtDC, LOGPIXELSY) );
    SetWindowOrg( hPrtDC, -PRINT_MARGIN, -PRINT_MARGIN );
    SetViewportOrg( hPrtDC, 0, 0 );
}

void PrintPage(void)
{
    PRINTDLG PrtDlg;
    HFONT hFont;

    memset(&PrtDlg, 0, sizeof(PRINTDLG));

    PrtDlg.lStructSize = sizeof(PRINTDLG);
    PrtDlg.Flags = PD_RETURNDC;

    if ( PrintDlg(&PrtDlg) ) {
        DOCINFO dinfo;
        char String[100];
        char Title[100];
        int FontSize;
        int PageLine;

        SetPrinterMapMode(PrtDlg.hDC);
        
        dinfo.cbSize = sizeof(DOCINFO);
        wsprintf((LPSTR)Title,(LPSTR)"PrtDemoDoc");
        dinfo.lpszDocName = (LPSTR)Title;
        dinfo.lpszOutput = NULL;

        StartDoc(PrtDlg.hDC, (LPDOCINFO)&dinfo);
        StartPage(PrtDlg.hDC);

        wsprintf(String, (LPSTR)"One Inch = %d:  PrtDemo Printing Test", ONE_INCH);
        TextOut(PrtDlg.hDC, PRINT_MARGIN, PRINT_MARGIN, (LPSTR)String, lstrlen(String));

        for (FontSize = 100, PageLine = ONE_INCH; FontSize > 0; FontSize -= 10 ) {
            wsprintf(String, (LPSTR)"Line %d Points %d:  PrtDemo Printing Test",
                 PageLine, FontSize);

            hFont = CreatePrintFont(FontSize);
            if ( hFont ) {
                hFont = SelectObject(PrtDlg.hDC, hFont);
                TextOut(PrtDlg.hDC, PRINT_MARGIN, PageLine, (LPSTR)String, lstrlen(String));

                hFont = SelectObject(PrtDlg.hDC, hFont);
                DeleteObject(hFont);
            }

            PageLine += ONE_INCH;
        }

        EndPage(PrtDlg.hDC);
        EndDoc(PrtDlg.hDC);

        DeleteDC(PrtDlg.hDC);
        
        if ( PrtDlg.hDevMode )
                GlobalFree(PrtDlg.hDevMode);
        if ( PrtDlg.hDevNames )
                GlobalFree(PrtDlg.hDevNames);
    }

} /* PrintPage() */

long __export CALLBACK
PrtWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HMENU hMenu;        
    HICON hIcon;
    
        if ( msg == WM_COMMAND ) {
            switch (wParam) {

                case IDM_PRINT:
                    PrintPage();
                    break;

                case IDM_SETUP:
                    {
                        PRINTDLG PrintDialog;

                        memset(&PrintDialog, 0, sizeof(PRINTDLG));

                        PrintDialog.lStructSize = sizeof(PRINTDLG);
                        PrintDialog.hwndOwner = hWnd;
                        PrintDialog.Flags = PD_PRINTSETUP;

                        PrintDlg(&PrintDialog);
                    }
                    break;                          
                    
                case IDM_ABOUT:
                        hIcon=LoadIcon(hThisInst, MAKEINTRESOURCE(IDI_ICON1));
                        ShellAbout(hWnd, "PrtDemo" , "PrtDemo Windows Application", hIcon);
                        DeleteObject(hIcon);
                        break;    

                case IDM_TWIPS:
                        hMenu = GetSubMenu(GetMenu(hWnd), 1);
                        CheckMenuItem(hMenu, IDM_TWIPS, MF_BYCOMMAND | MF_CHECKED);
                        CheckMenuItem(hMenu, IDM_TEXT, MF_BYCOMMAND | MF_UNCHECKED);
                        ONE_INCH = 1440;
                        PRINT_MARGIN = ONE_INCH / 2;
                    break;

                case IDM_TEXT:
                        hMenu = GetSubMenu(GetMenu(hWnd), 1);
                        CheckMenuItem(hMenu, IDM_TEXT, MF_BYCOMMAND | MF_CHECKED);
                        CheckMenuItem(hMenu, IDM_TWIPS, MF_BYCOMMAND | MF_UNCHECKED);
                        ONE_INCH = 300;
                        PRINT_MARGIN = ONE_INCH / 2;
                    break;

                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                }    
        }
        else if ( msg == WM_DESTROY || msg == WM_CLOSE )
            PostQuitMessage(0);
        else
            return (DefWindowProc(hWnd, msg, wParam, lParam));

    return 0;

} /* PrtWndProc() */

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
        wc.lpfnWndProc = PrtWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
        wc.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
        wc.hbrBackground = GetStockObject(WHITE_BRUSH);
        wc.lpszMenuName = "PrtMenu";
        wc.lpszClassName = "PrtClass";

        if ( !RegisterClass(&wc) )
            return FALSE;
    }

    hThisInst = hInstance;

    hWnd = CreateWindow(
        "PrtClass",
        "PrtDemo Windows Application",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
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
