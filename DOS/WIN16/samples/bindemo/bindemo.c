/*
 *      BinDemo.c
 *
 *      @(#)bindemo.c	1.4 10/10/96 15:08:24 /users/sccs/src/samples/bindemo/s.bindemo.c 
 * 
 *      Dynamic Linking Sample Application
 *
 *      Demonstrates the following:
 *
 *      - registering classes, and creating windows.
 *      - standard message loop and message dispatch logic.
 *      - use of resources, menues, bitmaps, stringtables and accelerators.
 *
 *      - linking windows DLLs with LoadLibrary().
 *      - mapping a binary interface to the DLL-functions
 *      - accessing library functions with GetProcAdress()
 *      - using resources of a loaded library.
 *      - unloading dynamically loaded libraries.
 *
 *      - calling the ShellAbout library call.
 */

#include "windows.h"
#include "bindemo.h"
#include "math.h"

/* Global Resources */
HINSTANCE hInst;        /* the global instance pointer */
HINSTANCE hLibInst;     /* handle to the loaded library */

char    szWinName[] = "BinDemo";
int     csx, csy;       /* size of cards */
int     carddx,carddy;  /* size of cards on display */
double  csize;      /* chosen size */
double  sizes[ID_LASTSIZEID - ID_FIRSTSIZEID +1]={0.5, 1, 2};
int     cardoffset, firstcard, cardback;

/* Pointer to the library-functions we will use */
BOOL (FAR PASCAL *cdtInit)(int FAR *pdxCard, int FAR *pdyCard) = NULL;
BOOL (FAR PASCAL *cdtDrawExt)(HDC hdc, int x, int y, int carddx, int carddy, int cd, int mode, DWORD rgbBgnd) =NULL;
BOOL (FAR PASCAL *cdtDraw)(HDC hdc, int x, int y, int cd, int mode, DWORD rgbBgnd) =NULL;
void (FAR PASCAL *cdtTerm)() = NULL;

#ifndef _WINDOWS
void LibMain();
#endif

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
#ifndef _WINDOWS
    LibMain();
#endif

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
            hLibInst = LoadLibrary("cards.dll");
            if((UINT) hLibInst > 32) { 
                /* don't care about the type casts. they just prevent some warning messages to be issued */
                cdtInit = 
                    (BOOL (FAR PASCAL *)(int FAR *, int FAR *)) GetProcAddress(hLibInst,"cdtInit");
                cdtDrawExt = 
                    (BOOL (FAR PASCAL *)(HDC, int, int, int, int, int, int, DWORD)) GetProcAddress(hLibInst,"cdtDrawExt");
                cdtDraw = 
                    (BOOL (FAR PASCAL *)(HDC, int, int, int, int, DWORD)) GetProcAddress(hLibInst,"cdtDraw");
                cdtTerm = 
                    (void (FAR PASCAL *)()) GetProcAddress(hLibInst,"cdtTerm");
            } else {
                MessageBox(hWnd,"Error loading library","Error",MB_OK);
                DestroyWindow(hWnd);
                return 0;
            }
            if(cdtInit == NULL || cdtDrawExt == NULL || 
                cdtDraw == NULL || cdtTerm == NULL) {
                MessageBox(0,"Error loading functions","Error",MB_OK);
                DestroyWindow(hWnd);
                return 0;
            }
            (*cdtInit)(&csx,&csy);        /* Get the card size */

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
                    ShellAbout(hWnd, "BinDemo Sample Application", "BinDemo", hIcon);
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
                  (*cdtDrawExt)(hDC,x0+dx,y0+dy,carddx, carddy, i*cardoffset+firstcard,cardback,RGBBKND);
                if(i!=0)    /* the first card is the same for both directions */
                {
                  SetRect(&r1, x0-dx, y0-dy, x0-dx+carddx, y0-dy+carddy);
                  if(IntersectRect(&r2, &r1, &ps.rcPaint))                
                    (*cdtDrawExt)(hDC,x0-dx,y0-dy,carddx, carddy, i*cardoffset+firstcard,cardback,RGBBKND);
                }
                phi += dphi;
                d += dd;
            }

            EndPaint(hWnd,&ps);
          }  
          break;
    
        case WM_DESTROY:    
            if(cdtTerm)
            {
                (*cdtTerm)();
                FreeLibrary(hLibInst);
            }
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
    
#ifndef _WINDOWS
     
#include "windows.h"
#include "Endian.h"
#include "OEM.h"

#include "kerndef.h"
#include "BinTypes.h"
#include "Log.h"
#include "DPMI.h"
#include "Resources.h"
#include "Module.h"
 

BOOL FAR PASCAL cnb_cdtInit(int FAR *, int FAR *);
BOOL FAR PASCAL cnb_cdtDrawExt(HDC , int , int , int , int , int , int , DWORD );
BOOL FAR PASCAL cnb_cdtDraw(HDC , int , int , int , int , DWORD );
void FAR PASCAL cnb_cdtTerm();

HSMT_OEMENTRYTAB OEM_tab_cards[] =
{       /* 000 */       { "", 0 },
        /* 001 */       { "CDTINIT",    (long int (*)())cnb_cdtInit },
        /* 002 */       { "CDTDRAW",    (long int (*)())cnb_cdtDraw },
        /* 003 */       { "CDTDRAWEXT", (long int (*)())cnb_cdtDrawExt },
        /* 004 */       { "CDTTERM",    (long int (*)())cnb_cdtTerm }
};


/* 
 * This routine would be called on loading of cards.so
 * This will map the above binary interface to cards.dll, when
 * that library is loaded.
 */
void
LibMain()
{
	extern void AddOEMTable(char *,HSMT_OEMENTRYTAB *);
        AddOEMTable("cards", OEM_tab_cards);
}

extern ENV *envp_global;

/* 
 * Interface routine for the cards.dll init routine
 * passed pointers to two integers
 *
 * the interface does the following...
 *    maps 2 words to the dos address space
 *    pushes them on the stack, as two far pointers
 *    calls the underlying routine
 *    sets the 'real' values from the intermediate values
 *    (note: x,y are intel format, must use a macro to convert to native)
 *    frees the mapping selectors
 *    returns the result
 */

extern invoke_binary();

BOOL FAR PASCAL 
cnb_cdtInit(int FAR *pdxCard, int FAR *pdyCard)
{
    WORD wSel0=0,wSel1=0;
    WORD x,y;

    envp_global->reg.sp -= DWORD_86 + DWORD_86;

    wSel0 = ASSIGNSEL(&y, 2);
    PUTDWORD(envp_global->reg.sp,MAKELP(wSel0,0));

    wSel1 = ASSIGNSEL(&x, 2);
    PUTDWORD(envp_global->reg.sp+4,MAKELP(wSel1,0));

    invoke_binary();

    /* don't forget to take care of the endian issue: use GETWORD !!!*/
    *pdxCard = GETWORD(&x);
    *pdyCard = GETWORD(&y);

    FreeSelector(wSel0);
    FreeSelector(wSel1);

    return envp_global->reg.ax;
}

/*
 *  Interface to cdtDrawExt
 *  push enough space for arguments
 *  push the arguments, no special handling required
 *  invoke the binary code
 *  return the result code
 */

BOOL FAR PASCAL 
cnb_cdtDrawExt(HDC hdc, int x, int y, int dx, int dy, int cd, int mode, DWORD rgbBgnd)
{
    envp_global->reg.sp -= 7 * WORD_86 + DWORD_86;
    PUTDWORD(envp_global->reg.sp,rgbBgnd);
    PUTWORD(envp_global->reg.sp+4,mode);
    PUTWORD(envp_global->reg.sp+6,cd);
    PUTWORD(envp_global->reg.sp+8,dy);
    PUTWORD(envp_global->reg.sp+10,dx);
    PUTWORD(envp_global->reg.sp+12,y);
    PUTWORD(envp_global->reg.sp+14,x);
    PUTWORD(envp_global->reg.sp+16,hdc);

    invoke_binary();

    return envp_global->reg.ax;
}

/*
 *  Interface to cdtDraw
 *  push enough space for arguments
 *  push the arguments, no special handling required
 *  invoke the binary code
 *  return the result code
 */

BOOL FAR PASCAL 
cnb_cdtDraw(HDC hdc, int x, int y, int cd, int mode, DWORD rgbBgnd)
{
    envp_global->reg.sp -= 5 * WORD_86 + DWORD_86;
    PUTDWORD(envp_global->reg.sp,rgbBgnd);
    PUTWORD(envp_global->reg.sp+4,mode);
    PUTWORD(envp_global->reg.sp+6,cd);
    PUTWORD(envp_global->reg.sp+8,y);
    PUTWORD(envp_global->reg.sp+10,x);
    PUTWORD(envp_global->reg.sp+12,hdc);

    invoke_binary();

    return envp_global->reg.ax;
}

/*
 *  Interface to cdtTerm
 *  no special handling required
 *  invoke the binary code
 */
void FAR PASCAL 
cnb_cdtTerm()
{
        invoke_binary();
}

#endif      /* _WINDOWS */

    
