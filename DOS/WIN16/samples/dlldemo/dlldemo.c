/*
 *      DllDemo.c
 *      Dynamic Linking Sample Application
 *
 *      Demonstrates the following:
 *
 *      - use of resource and module definition compilers.
 *      - procedure for building shared libraries.
 *      - linking libraries on a command line.
 *
 *      - registering classes, and creating windows.
 *      - standard message loop and message dispatch logic.
 *      - use of resources, menues, bitmaps, stringtables and accelerators.
 *
 *      - linking libraries with LoadLibrary().
 *      - accessing library functions with GetProcAdress()
 *      - loading resources from a loaded library.
 *      - unloading dynamically loaded libraries.
 *
 *      - calling the ShellAbout library call.
 */

#include "dlldemo.h"
#include "bitmaps.h"
#include "dllres.h"

HINSTANCE  hInstance;
HACCEL     hAccelTable;
char       libraryname[256];

int PASCAL 
WinMain(HINSTANCE hInst, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{

    HWND        hWnd;
    MSG         msg;
    char        title[256];
    int         len;

    len = LoadString(hInst,DLL_APP_TITLE,title,256);
    hInstance = hInst;

    hAccelTable = LoadAccelerators( hInstance, (LPSTR)"RESOURCE" );
    if (!hPrevInstance)
       if (!DllDemoInit(hInst))
          return (0);

    hWnd = CreateWindow("DllDemo",
                        title,
                        WS_OVERLAPPEDWINDOW,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        0,
                        0,
                        hInst,
                        NULL);

    if (!hWnd)
       return (0);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&msg, 0, 0, 0)) {
      if (TranslateAccelerator( hWnd, hAccelTable, (LPMSG)&msg ) == 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      } 
    }
    return (msg.wParam);
}

BOOL 
DllDemoInit(HINSTANCE hInst)
{
    WNDCLASS wc;
    BOOL bSuccess;

    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_WILLOWS));
    wc.lpszMenuName  = (LPSTR) "DllMenu";
    wc.lpszClassName = (LPSTR) "DllDemo";
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.hInstance     = hInst;
    wc.style       = 0;
    wc.lpfnWndProc = DllWndProc;
    wc.cbWndExtra  = 0;
    wc.cbClsExtra  = 0;

    bSuccess = RegisterClass(&wc);

    return (bSuccess);
}

long FAR PASCAL __export 
DllWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{     
    HICON  hIcon;
    static HINSTANCE hLibrary;
                           
    FARPROC lpfnc;
    char buffer[140];
    char tempstring[140];
    PAINTSTRUCT ps;
    int height, line;
    HBITMAP hBitmap; 
    HDC hdc, hdcMemory;
    HBITMAP hbmpOld;
    BITMAP bm;
                           
    switch (message) {

    case WM_COMMAND:
        switch (wParam) {
        case IDM_ABOUT:
            hIcon = LoadIcon(hInstance,MAKEINTRESOURCE(IDI_WILLOWS));
            ShellAbout(hWnd, "DLLDemo", "Dynamic Linking Sample Application", hIcon);
            break;

        case IDM_LOAD:
            if ( hLibrary == 0 ) 
            {
                if(LoadString(hInstance, DLL_IDS_RESOURCE, libraryname, 256)) 
                {
                  hLibrary = LoadLibrary(libraryname);
                  if ( hLibrary < HINSTANCE_ERROR ) 
                    hLibrary = 0;        
                }
                InvalidateRect(hWnd, NULL, TRUE);
            }    
            break;
        case IDM_FREE:
            if ( hLibrary ) 
            {
              FreeLibrary(hLibrary);
              hLibrary = 0;
              InvalidateRect(hWnd, NULL, TRUE);
            }
            break;

        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        }
        break;
        
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        
        line = 1;
        height = HIWORD(GetTextExtent(hdc, "XXX", 3));

        if ( hLibrary != 0 ) 
        {
          wsprintf(buffer, "Library \"%s\" loaded",libraryname);
          TextOut(hdc, 0, line*height, buffer, lstrlen(buffer));
          line++;
        
          lpfnc = GetProcAddress(hLibrary, "GetResourceString");
          if (lpfnc) 
          {
            wsprintf(buffer,"Address of function GetResourceString in Library \"%s\" : %0lx",libraryname,lpfnc);
            TextOut(hdc, 0, line*height, buffer, lstrlen(buffer)); 
            line++;
  
            (*lpfnc)(tempstring);   
            wsprintf(buffer, "Resource: GetResourceString = %s", tempstring);    
            TextOut(hdc, 0, line*height, buffer, lstrlen(buffer)); 
            line++;
          }
          else 
          {
            wsprintf(buffer, "Could not get address of GetResourceString in %s",libraryname);
            TextOut(hdc, 0, line*height, buffer, lstrlen(buffer)); 
            line++;
          }
        }
        else
        {
          wsprintf(buffer,"Dynamic link library not loaded");
          TextOut(hdc, 0, line*height, buffer, lstrlen(buffer));
          line++;
        }               
        
        hBitmap = GetLibraryBitmap();
                    
        line++;
        wsprintf(buffer,"Bitmap handle received from GetLibraryBitmap: %x",hBitmap);
        TextOut(hdc, 0, line*height, buffer, lstrlen(buffer));
        line++;

        GetObject(hBitmap, sizeof(BITMAP), &bm);

        hdcMemory = CreateCompatibleDC(hdc);
        hbmpOld = SelectObject(hdcMemory, hBitmap);

        BitBlt(hdc, 0, line*height, bm.bmWidth, bm.bmHeight, hdcMemory, 0, 0, SRCCOPY);
        SelectObject(hdcMemory, hbmpOld);
        DeleteDC(hdcMemory);
        ReleaseDC(hWnd, hdc);
        
        EndPaint(hWnd,&ps);
        break;

    case WM_DESTROY:
    case WM_CLOSE:
        if ( hLibrary ) 
        {
          MessageBox(0, "Calling FreeLibrary for Resource Module",
                        "Application Cleanup", MB_OK);
          FreeLibrary(hLibrary);
          hLibrary = 0;
        }
        PostQuitMessage(0);
        break;

    default:
        return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);
}


