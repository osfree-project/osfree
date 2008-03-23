/*
 *      BinDemo.c
 *
 *      @(#)bindemo2.c	1.5 10/10/96 15:11:23 /users/sccs/src/samples/bindemo2/s.bindemo2.c
 *
 *      Dynamic Linking Sample Application
 *
 *      Demonstrates the following:
 *
 *      - registering classes, and creating windows.
 *      - standard message loop and message dispatch logic.
 *      - use of resources, menues, bitmaps, stringtables and accelerators.
 *
 *      - using resources of a windows DLL by using a shared native object
 *        that maps the calls to the binary DLL
 *
 *      - calling the ShellAbout library call.
 */

#include "windows.h"
#include "bindemo2.h"
#include "math.h"

#include "binlib2.h"
/* Global Resources */
HINSTANCE hInst;        /* the global instance pointer */

char    szWinName[] = "BinDemo2";
int     csx, csy;       /* size of cards */
int     carddx,carddy;  /* size of cards on display */
double  csize;      /* chosen size */
double  sizes[ID_LASTSIZEID - ID_FIRSTSIZEID +1]={0.5, 1, 2};
int     cardoffset, firstcard, cardback;

int PASCAL 
WinMain(HANDLE hInstance, HANDLE hPrevInstance, LPSTR lpCmdLine, WORD nCmdShow)
{
    HWND    hWnd;
    MSG     msg;

    if (!hPrevInstance)
    {
        WNDCLASS    WndClass;

        WndClass.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
        WndClass.hIcon = LoadIcon(hInstance, "BinDemoIcon");
        WndClass.lpszMenuName = (LPSTR) "BinDemoMenu";
        WndClass.lpszClassName = (LPSTR) szWinName;
        WndClass.hbrBackground = GetStockObject(WHITE_BRUSH);
        WndClass.hInstance = hInstance;
        WndClass.style = 0;
        WndClass.lpfnWndProc = (WNDPROC) BinDemoWndProc;
        WndClass.cbWndExtra = 0;
        WndClass.cbClsExtra = 0;
    
        if(!RegisterClass(&WndClass))
                return (FALSE);
    }   

    hInst = hInstance;
    
    hWnd = CreateWindow(szWinName,
                szWinName,
                WS_OVERLAPPEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                (HWND)NULL,
                (HMENU)NULL,
                hInstance,
                NULL);

    /* register or die */
    if (!hWnd)
        return (FALSE);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    /* start handling messages */
    while (GetMessage(&msg, 0,0,0)) 
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (msg.wParam);
}


long FAR PASCAL 
BinDemoWndProc(HWND hWnd, unsigned message, WORD wParam, LONG lParam)
{
    PAINTSTRUCT ps;
    HDC  hDC;
    RECT rect;

    switch (message) 
    {
        case WM_CREATE:
            cdtInit(&csx,&csy);        /* Get the card size */

            csize = 1;          /* display normal size */
            cardoffset = 4;
            firstcard = 0;      /* clubs */
            cardback = 0;       /* show front */
            SetCardMarks(hWnd, IDM_CLUBS);           
            SetCardMarks(hWnd, IDM_NORMAL);
            carddx = (int) (csize * csx);
            carddy = (int) (csize * csy);
            break;
    
        case WM_SIZE:
            InvalidateRect(hWnd, NULL, TRUE);
            break;

        case WM_COMMAND:
            switch(wParam)
            {
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                case IDM_ABOUT:
                  {
                    HICON hIcon;
                    
                    hIcon = LoadIcon(hInst, "BinDemoIcon");
                    ShellAbout(hWnd, "BinDemo2 Sample Application", "BinDemo2", hIcon);
                    DestroyIcon(hIcon);
                    break;
                  }
                case IDM_CLUBS:
                case IDM_DIAMONDS:
                case IDM_HEARTS:
                case IDM_SPADES:
                    ClearCardMarks(hWnd, ID_FIRSTCARDID, ID_LASTCARDID);
                    SetCardMarks(hWnd, wParam);
                    firstcard = wParam - ID_FIRSTCARDID;    
                    cardoffset =4;
                    cardback = 0;
                    InvalidateRect(hWnd, NULL, TRUE);
                    break;                  
                case IDM_BACKSIDES:
                    ClearCardMarks(hWnd, ID_FIRSTCARDID, ID_LASTCARDID);
                    SetCardMarks(hWnd, IDM_BACKSIDES);
                    cardback = 1; 
                    cardoffset =1;
                    firstcard = 53;
                    InvalidateRect(hWnd, NULL, TRUE);
                    break;                  
                    
                case IDM_SMALL:
                case IDM_NORMAL:
                case IDM_LARGE:
                    csize = sizes[wParam - ID_FIRSTSIZEID];
                    carddx = (int) (csize*csx);
                    carddy = (int) (csize*csy);    
                    ClearCardMarks(hWnd, ID_FIRSTSIZEID, ID_LASTSIZEID);
                    SetCardMarks(hWnd, wParam);
                    InvalidateRect(hWnd, NULL, TRUE);                   
                    break;
            }
            break;
    
            
        case WM_PAINT:
          { 
            int i, d, x0, y0, dx, dy, dd;
            double phi, dphi;
            RECT r1, r2;
                                       
            hDC = BeginPaint(hWnd,&ps);
            
            GetClientRect(hWnd, &rect);
                                            
            /* calculate the starting position */                                            
            phi = 0;        
            d = 0;
            x0 = (rect.right-carddx) /2;
            y0 = (rect.bottom-carddy)/2;                 
            dd = min(x0, y0) / 12;
            dphi = 3.1415926535 / 12;
            
            for(i=0;i<13;i++) 
            {                         
                dy = (int) (d * cos(phi));
                dx = (int) (d * sin(phi));
                SetRect(&r1, x0+dx, y0+dy, x0+dx+carddx, y0+dy+carddy);
                if(IntersectRect(&r2, &r1, &ps.rcPaint))                
                  cdtDrawExt(hDC,x0+dx,y0+dy,carddx, carddy, i*cardoffset+firstcard,cardback,RGBBKND);
                if(i!=0)    /* the first card is the same for both directions */
                {
                  SetRect(&r1, x0-dx, y0-dy, x0-dx+carddx, y0-dy+carddy);
                  if(IntersectRect(&r2, &r1, &ps.rcPaint))                
                    cdtDrawExt(hDC,x0-dx,y0-dy,carddx, carddy, i*cardoffset+firstcard,cardback,RGBBKND);
                }
                phi += dphi;
                d += dd;
            }

            EndPaint(hWnd,&ps);
          }  
          break;
    
        case WM_DESTROY:    
            PostQuitMessage(0);
            break;
    
        default:
            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (0);
}

void ClearCardMarks(HWND hWnd, UINT id_first, UINT id_last)
{
    HMENU hMenu;
    UINT i;
    
    hMenu = GetMenu(hWnd);
    for (i=id_first;  i<=id_last; i++)
      CheckMenuItem(hMenu, i, MF_BYCOMMAND|MF_UNCHECKED);
}

void SetCardMarks(HWND hWnd, UINT menuid)
{
    HMENU hMenu;
    
    hMenu = GetMenu(hWnd);
    CheckMenuItem(hMenu, menuid, MF_BYCOMMAND|MF_CHECKED);
}

