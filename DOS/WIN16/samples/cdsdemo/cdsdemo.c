/*
 *      CDSDemo.c
 *
 *	@(#)cdsdemo.c	1.3 7/15/96 14:57:42 /users/sccs/src/samples/cdsdemo/s.cdsdemo.c
 *
 *      Commmon Dialogs Demonstration Application
 *
 *      Demonstrates the following:
 *
 *      - use of Common dialogs functions and associated structs:
 *        GetOpenFileName, GetSaveFileName, OPENFILENAME
 *        ChooseColor, CHOOSECOLOR
 *        ChooseFont, CHOOSEFONT
 *        PrintDlg, PRINTDLG
 *      - use of resource and module definition compilers.
 *      - registering classes, and creating windows.
 *      - standard message loop and message dispatch logic.
 *      - calling the ShellAbout library call.
 */

#include "windows.h"
#include "string.h"
#include <stdlib.h>
#include "commdlg.h"
#include "cdsdemo.h"

HINSTANCE hThisInst = 0;
HBRUSH hColorBrush = 0;
HFONT hFont = 0;
COLORREF crFont = RGB(0,0,0);
LOGFONT LogFont;
char fnbuffer[_MAX_PATH];
char Title[]="Common Dialogs Sample Application";
char Program[]="CDSDemo";
char szFilter[]="Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";

long __export CALLBACK
CDSWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
        HFONT hOldFont;
        HBRUSH hOldBrush;
        RECT rect;
        HDC hdc;
        HICON hIcon;
        PAINTSTRUCT ps;

        switch(msg)
        {
          case WM_COMMAND:
            switch (wParam)
            {
                case IDM_ABOUT:
                   hIcon=LoadIcon(hThisInst, MAKEINTRESOURCE(IDI_ICON1));
                   ShellAbout(hWnd,Program, Title, hIcon);
                   DestroyIcon(hIcon);
                   break;
                    
                case IDM_FONT:
                   {
                        CHOOSEFONT ChFont;
                              
                        memset(&ChFont, 0, sizeof(CHOOSEFONT));
                              
                        ChFont.lStructSize = sizeof(CHOOSEFONT);
                        
                        ChFont.hwndOwner = hWnd;
                        ChFont.lpLogFont = &LogFont;
                        ChFont.Flags = CF_INITTOLOGFONTSTRUCT | CF_EFFECTS | CF_SCREENFONTS;
                        ChFont.rgbColors = crFont;
                        
                        
                        if (ChooseFont(&ChFont))
                        {
                          if(hFont)
                            DeleteObject(hFont);
                          hFont=CreateFontIndirect(&LogFont);
                          crFont = ChFont.rgbColors;
                          InvalidateRect(hWnd, NULL, TRUE); 
                        }
                   }
                   break;
                            
                case IDM_COLOR:
                   {
                        CHOOSECOLOR ChColor;
                        COLORREF CustColors[16];
                        int Index;
                        
                        for (Index = 0; Index < 16; Index++)
                        CustColors[Index] = RGB(255, 255, 255);
                        
                        memset(&ChColor, 0, sizeof(CHOOSECOLOR));
                        
                        ChColor.lStructSize = sizeof(CHOOSECOLOR);
                        
                        ChColor.hwndOwner = hWnd;
                        ChColor.rgbResult = RGB(0,0,0);
                        ChColor.lpCustColors = CustColors;
                        ChColor.Flags = 0;
                        
                        if (ChooseColor(&ChColor))
                        {
                          if(hColorBrush) 
                            DeleteObject(hColorBrush);
                          hColorBrush=CreateSolidBrush(ChColor.rgbResult);
                          InvalidateRect(hWnd, NULL, TRUE);  
                        }
                   }     
                   break;     
                   
                case IDM_OPEN:
                   {
                        OPENFILENAME ofn;
                        char buffer[100];
                        static char openTitle[]="CDSDemo Open File";
                        
                        memset(&ofn, 0, sizeof(OPENFILENAME));
                        memset(fnbuffer, 0, sizeof(fnbuffer));
                        
                        ofn.lStructSize = sizeof(OPENFILENAME);
                        ofn.hwndOwner = hWnd;        
                        ofn.lpstrFilter = szFilter;
                        ofn.nFilterIndex = 1;         
                        ofn.lpstrFile = fnbuffer;
                        ofn.nMaxFile = sizeof(fnbuffer);
                        ofn.nMaxFileTitle = sizeof(buffer);
                        ofn.lpstrFileTitle = buffer;
                        ofn.lpstrTitle=openTitle;
                        ofn.Flags = OFN_HIDEREADONLY;
                        if (GetOpenFileName(&ofn))
                        {                  
                          wsprintf(buffer, "Filename: %s", (LPSTR) fnbuffer);
                          MessageBox(hWnd, buffer, "CDSDemo Open File", MB_OK);
                        }
                   }    
                   break;
                         
                case IDM_SAVE:
                   {
                        OPENFILENAME ofn;
                        char buffer[100];
                        static char saveTitle[]="CDSDemo Save File";
                        
                        memset(&ofn, 0, sizeof(OPENFILENAME));
                        memset(fnbuffer, 0, sizeof(fnbuffer));
                        
                        ofn.lStructSize = sizeof(OPENFILENAME);
                        ofn.hwndOwner = hWnd;        
                        ofn.lpstrFilter = szFilter;
                        ofn.nFilterIndex = 0;         
                        ofn.lpstrFile = fnbuffer;
                        ofn.nMaxFile = sizeof(fnbuffer);
                        ofn.lpstrFileTitle = NULL;
                        ofn.lpstrTitle=saveTitle;
                        ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
                        if (GetSaveFileName(&ofn))
                        {                  
                          wsprintf(buffer, "Filename: %s", (LPSTR) fnbuffer);
                          MessageBox(hWnd, buffer, "CDSDemo Open File", MB_OK);
                        }
                   }    
                   break;     
                   
                case IDM_PRINT:
                   {
                        PRINTDLG pd;
                        
                        memset(&pd, 0, sizeof(PRINTDLG));
                        
                        pd.lStructSize = sizeof(PRINTDLG);
                        pd.hwndOwner = hWnd;
                        pd.nCopies = 1;
                        pd.Flags = PD_ALLPAGES;
                        if(PrintDlg(&pd))
                        {
                          MessageBox(hWnd, "OK selected in Printer Dialog", "CDSDemo Print", MB_OK);
                        }
                   }
                   break;
                         
                case IDM_EXIT:
                   DestroyWindow(hWnd);
                   break;
            }    
            break;
        case WM_DESTROY:
        case WM_CLOSE:
                if(hColorBrush) DeleteObject(hColorBrush);
                if(hFont) DeleteObject(hFont);
                PostQuitMessage(0);
                break;
        case WM_PAINT:
            {
                char buffer[100];
                
                hdc = BeginPaint(hWnd, &ps);
                wsprintf(buffer, "Selected color:");
                TextOut(hdc, 50,20, buffer, lstrlen(buffer));
                if(hColorBrush)
                {              
                  hOldBrush = SelectObject(hdc, hColorBrush);
                  SetRect(&rect, 50, 50, 150, 150);
                  Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
                  SelectObject(hdc, hOldBrush);
                }       
                wsprintf(buffer, "Font:");
                TextOut(hdc, 250, 20, buffer, lstrlen(buffer));
                if(hFont)
                {
                  hOldFont=SelectObject(hdc,hFont);
                  wsprintf(buffer, "CDSDemo");
                  SetBkMode(hdc, TRANSPARENT);
                  SetTextColor(hdc, crFont);
                  TextOut(hdc, 250,50, buffer, lstrlen(buffer));
                  SelectObject(hdc, hOldFont);
                }
                EndPaint(hWnd, &ps);
                break;
            }    
        default:
                return (DefWindowProc(hWnd, msg, wParam, lParam));
        }
        return 0;
} /* CDSWndProc() */

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

        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = CDSWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
        wc.hCursor = LoadCursor(0, IDC_ARROW);
        wc.hbrBackground = GetStockObject(WHITE_BRUSH);
        wc.lpszMenuName = "CDSMenu";
        wc.lpszClassName = "CDSClass";

        if ( !RegisterClass(&wc) )
            return FALSE;
    }

    hThisInst = hInstance;
    hColorBrush = CreateSolidBrush(RGB(255,255,255));
    
    LogFont.lfHeight = 20;
    LogFont.lfWidth  = 0;
    LogFont.lfEscapement  = 0;
    LogFont.lfOrientation = 0;
    LogFont.lfWeight = 400;
    LogFont.lfItalic = 0;
    LogFont.lfUnderline = 0;
    LogFont.lfStrikeOut = 0;
    LogFont.lfCharSet    = ANSI_CHARSET;
    LogFont.lfOutPrecision = OUT_DEFAULT_PRECIS;
    LogFont.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    LogFont.lfQuality = DEFAULT_QUALITY;
    LogFont.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    lstrcpy(LogFont.lfFaceName, "Modern");

    hWnd = CreateWindow(
        "CDSClass",
        Title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        500,
        300,
        0,
        0,
        hInstance,
        NULL);

    if ( !hWnd )
        return FALSE;

    ShowWindow(hWnd, nShow);
    UpdateWindow(hWnd);
                
    while ( GetMessage(&Message, 0, 0, 0) ) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    
    return (BOOL)Message.wParam;

} /* WinMain() */
