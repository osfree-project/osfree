/*
 *      windemo.c -- Simple Windows Demonstration Application
 *
 *      @(#)windemo.c   1.1 2/13/96 13:08:13 /users/sccs/src/samples/windemo/s.windemo.c
 *
 *      Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
 *
 *      Demonstrates the following:
 *
 *      - use of resource and module definition compilers.
 *      - registering classes, and creating windows.
 *      - standard Msg loop and Msg dispatch logic.
 *      - use of resources, menues, bitmaps, stringtables and accelerators.
 *      - calling the ShellAbout API.
 */

/*
** System Includes
*/
#define OEMRESOURCE

#include <windows.h>

#include <stdlib.h>
#include <string.h>
/*
** Private Includes
*/
#include "windemo.h"

/*
** Local Function Prototypes
*/
static void ShowKey( HWND , char *,long );
static void ShowMouse( HWND , char *,UINT ,LPARAM );
static void CheckClient( HWND );
static void UpdateMessage(HWND , char *);

LPVOID
WinMalloc(unsigned int size)
{
    DWORD dwSize = (size+3) & ~3;
    LPVOID lpCore = malloc(dwSize+32);
    return lpCore;
}

void
WinFree(LPVOID ptr)
{
    LPVOID lpCore = ptr;
    free(lpCore);
}

BOOL WINAPI
Beep
(
  DWORD  dwFreq,          /* Sound frequency, in hertz  */
  DWORD  dwDuration     /* Sound duration, in milliseconds  */
)
{
  MessageBeep(MB_OK);
  return (TRUE);
}


/*
** Global Variables
*/

static HANDLE    hInstance;
static char    Program[] = "WinDemo";
static char    Child[] = "WinDemo:child";
static char    Title[256];

static
void doregion(HWND hWnd)
{
        HDC     hDC;
        HRGN    hRgn1;
        HBRUSH  hRed,hBlue;
        LOGBRUSH lbr;
        RECT    client;
        RECT    r;

        // get the client area
        GetClientRect(hWnd,&client);

        //logstr(-1,"dorgn: getdc for region\n");
        hDC = GetDC(hWnd);

        //logstr(-1,"dorgn: getdc for region done, fill test rect\n");
        lbr.lbStyle = BS_SOLID;
        lbr.lbColor = RGB(0xff,0,0);
        lbr.lbHatch = 0;

        // create red and blue brushes
        hRed = CreateBrushIndirect(&lbr);

        lbr.lbColor = RGB(0,0,0xff);
        hBlue = CreateBrushIndirect(&lbr);

        // fill test area with blue
        SetRect(&r,40,40,80,80);
        FillRect(hDC,&r,hBlue);
        //logstr(-1,"dorgn: fill test done, now create region\n");

        // create a region covering blue
        hRgn1 = CreateRectRgn(40,40,80,80);

        //logstr(-1,"dorgn: create region done, now select it\n");
        // select this region
        SelectObject(hDC,hRgn1);

        //logstr(-1,"dorgn: select done, now fill whole rectangle\n");
        // should only overwrite the blue...
        FillRect(hDC,&client,hRed);

        TextOut(hDC, 20,50, "ABCDEFGHIJKLMNOPQRSTUVWXYZ",26);

        //logstr(-1,"dorgn: rectangle done, now release everthing\n");
        ReleaseDC(hWnd,hDC);

        DeleteObject(hRgn1);

        DeleteObject(hBlue);
        DeleteObject(hRed);
}


static
void PaintFont(HDC hDC,int size,int style)
{
        HFONT   hFont;
        HFONT   hOld;
        static  int loc;
        char    szsize[80];
        long    len;
        RECT    r;

        hFont = CreateFont(size,
                0,                      //width
                0,                      // escapement
                0,                      // orientation
                style & 1? 700:0,       // weight
                style & 2? 1:0,         // italics
                style & 4? 1:0,         // underline
                style & 8? 1:0,         // strikethrough
                0,                      // charset
                0,                      // outputprecision
                0,                      // clipprecision
                0,                      // quality
                0,                      // pitchandfamily
                (char *)0);                     // face

                hOld = SelectObject(hDC,hFont);

                        loc+= 20;

                        wsprintf(szsize,"size=%d style=%x",size,style);
                        TextOut(hDC, 20,loc, szsize,strlen(szsize));

                        len = GetTextExtent(hDC,szsize,strlen(szsize));
                        SetRect(&r,20,loc,20+LOWORD(len),loc+HIWORD(len));
                        FrameRect(hDC,&r,GetStockObject(BLACK_BRUSH));


                SelectObject(hDC,hOld);

        DeleteObject(hFont);
}


static
void domemdc(HWND hWnd)
{
        HDC     hDC,hDCmem;
        HBITMAP hBitmap,hOldbitmap;
        BITMAP  bm;
        int     flag = -1;
        char    bits[256];
        int     n;
        LPBITMAPINFOHEADER lpbmih;
        LPBITMAPINFO       lpbmi;
        char    bmheader[1024+40];

        hDC = GetDC(hWnd);

        if (flag & 2) {
           hDCmem = CreateCompatibleDC(hDC);

           if (flag & 4) {
                //hBitmap = LoadBitmap(0,(LPCSTR) OBM_CHECKBOXES);
                //hBitmap = LoadBitmap(0,(LPCSTR) "SPLASH");
                for(n=0;n<256;n++)
                        bits[n] = n;

                //memset(bmheader,0,1024+40);
                lpbmih = (LPBITMAPINFOHEADER) bmheader;
                lpbmi  = (LPBITMAPINFO) bmheader;

                lpbmih->biSize = 40;
                lpbmih->biWidth = 16;
                lpbmih->biHeight = 16;
                lpbmih->biPlanes = 1;
                lpbmih->biBitCount = 8;
                lpbmih->biCompression = BI_RGB;
                lpbmih->biSizeImage   = 256;
                lpbmih->biXPelsPerMeter = 80;
                lpbmih->biYPelsPerMeter = 80;
                lpbmih->biClrUsed       = 256;
                lpbmih->biClrImportant  = 0;

                hBitmap = CreateDIBitmap(hDCmem,lpbmih,CBM_INIT,
                        bits,lpbmi,0);


                if(hBitmap)
                    GetObject(hBitmap,sizeof(BITMAP),&bm);

                if (flag & 8) {
                        RECT r;
                        hOldbitmap = SelectObject(hDCmem,hBitmap);

                        if(flag & 16) {
                                BitBlt(hDC,10 ,10,
                                        bm.bmWidth,bm.bmHeight,
                                        hDCmem,0,0,SRCCOPY);
                        }
                        SetRect(&r,80,80,80+100,80+100);
                        FillRect(hDC,&r,GetStockObject(GRAY_BRUSH));

                        if(flag & 32) {
                                StretchBlt(hDC,80,80, 100,100,

                                        hDCmem,0,0, bm.bmWidth,bm.bmHeight,
                                        SRCCOPY);
                        }

                        FrameRect(hDC,&r,GetStockObject(BLACK_BRUSH));

                        SelectObject(hDCmem,hOldbitmap);

                }
                DeleteObject(hBitmap);
           }

           DeleteDC(hDCmem);
        }

        ReleaseDC(hWnd,hDC);
}


static
void dotimer(HWND hWnd)
{
        static int count;
        switch (count) {
        case 0:
                SetTimer(hWnd,1,1000,0);
                count++;
                break;
        case 1:
                SetTimer(hWnd,2,5000,0);
                count++;
                break;
        case 2:
                KillTimer(hWnd,1);
                count++;
                break;
        case 3:
                KillTimer(hWnd,2);
                count = 0;
                break;

        }
}

static
void doclr(HWND hWnd)
{
        RECT r;
        HDC  hdc;
        LPLOGPALETTE lp;
        int  i,j;
        COLORREF cr;
        int     red,green,blue;
        HBRUSH hbr;
        int     w,h;

        hdc = GetDC(hWnd);

        GetClientRect(hWnd,&r);
        w = r.right/16;
        h = r.bottom/16;

        lp = WinMalloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*256);
        GetSystemPaletteEntries(hdc,0,256,lp->palPalEntry);

        for(i=0;i<16;i++)
           for(j=0;j<16;j++) {
                red = lp->palPalEntry[i*16+j].peRed;
                green = lp->palPalEntry[i*16+j].peGreen;
                blue = lp->palPalEntry[i*16+j].peBlue;
                cr = RGB(red,green,blue);

                hbr = CreateSolidBrush(cr);

                SetRect(&r,w*j,h*i,w*j + w, h*i + h);

                FillRect(hdc,&r,hbr);

                DeleteObject(hbr);
           }

        WinFree(lp);
        ReleaseDC(hWnd,hdc);
}

static
void doblt(HWND hWnd)
{
        LPLOGPALETTE lp;
        HDC        hdc;
        int        i;
        COLORREF   cr;
        HPALETTE   hpal;
        HPALETTE   hpalold;
        HDC        hdcmem;
        unsigned char bits[16*16];
        RECT       r;
        LPBITMAPINFO  lpbmi;
        LPBITMAPINFOHEADER lpbmih;
        WORD            *pw;
        HBITMAP    hbitmap,hbitmapold;
        int        b,u,l;
        int        dopal = 2;
        int        paltype;

        lp    = WinMalloc(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY)*256);
        lpbmi = WinMalloc(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));

        hdc = GetDC(hWnd);
        hdcmem = CreateCompatibleDC(hdc);


            GetSystemPaletteEntries(hdc,0,256,lp->palPalEntry);

            if(dopal & 2) {

                for(i=0;i<256;i++) {
                        lp->palPalEntry[i].peFlags = PC_EXPLICIT;
                        lp->palPalEntry[i].peBlue  = 0;
                        lp->palPalEntry[i].peRed   = (i & 0xff);
                        lp->palPalEntry[i].peGreen = (i & 0xff00) >> 8;
                }
            }

            cr = RGB(0xff,0xff,0xff);

            lp->palVersion = 0x300;
            lp->palNumEntries = 256;

            hpal = CreatePalette(lp);

            b = 0;
            u = 0xff;
            l = 0xfe;

            memset(bits,u,16*16);
            memset(bits,l,8*16);

            for(i=0;i<16;i++)
                    bits[i] = b;

            for(i=0;i<16;i++) {
                    bits[i*16 + 0]  = b;
                    bits[i*16 + 15] = b;
            }

            for(i=0;i<16;i++)
                    bits[15*16 + i] = b;


        lpbmih = (LPBITMAPINFOHEADER) lpbmi;

        lpbmi->bmiHeader.biSize   = sizeof(BITMAPINFOHEADER);
        lpbmi->bmiHeader.biWidth  = 16;
        lpbmi->bmiHeader.biHeight = 16;
        lpbmi->bmiHeader.biPlanes = 1;
        lpbmi->bmiHeader.biBitCount = 8;
        lpbmi->bmiHeader.biCompression = BI_RGB;
        lpbmi->bmiHeader.biSizeImage   = 0;
        lpbmi->bmiHeader.biXPelsPerMeter   = 0;
        lpbmi->bmiHeader.biYPelsPerMeter   = 0;
        lpbmi->bmiHeader.biClrUsed        = 256;
        lpbmi->bmiHeader.biClrImportant   = 0;


                hbitmap = CreateDIBitmap(hdcmem,lpbmih,CBM_INIT,
                        bits,lpbmi,0);

                hbitmapold = SelectObject(hdcmem,hbitmap);

                BitBlt(hdc,0 ,0, 16,16, hdcmem,0,0,SRCCOPY);

                SelectObject(hdcmem,hbitmapold);

                DeleteObject(hbitmap);

        /*===================================================*/


        SetRect(&r,0,0,32,32);
        FillRect(hdc,&r,GetStockObject(BLACK_BRUSH));

        SetRect(&r,0,0,16,16);
        FillRect(hdc,&r,GetStockObject(WHITE_BRUSH));


        hbitmap = CreateCompatibleBitmap(hdc,16,16);

        hbitmapold = SelectObject(hdcmem,hbitmap);

        if(dopal & 3) {
                hpalold = SelectPalette(hdcmem,hpal,0);
                RealizePalette(hdcmem);
        }

        BitBlt(hdcmem,0,0,16,16,hdc,0,0,SRCCOPY);

        if(dopal & 2) {
                pw = (WORD *) lpbmi->bmiColors;
                for(i=0;i<256;i++)
                        pw[i] = i;
        }

        paltype = DIB_RGB_COLORS;

        if(dopal & 2)
                paltype = DIB_PAL_COLORS;

        StretchDIBits(hdcmem,0,0,16,16,0,0,16,16,
                bits,lpbmi,paltype,SRCCOPY);

        BitBlt(hdc,0,0,16,16,hdcmem,0,0,SRCCOPY);

        SelectObject(hdcmem,hbitmapold);
        DeleteObject(hbitmap);

        if(dopal & 3)
                SelectPalette(hdcmem,hpalold,0);

        DeleteObject(hpal);

        DeleteObject(hdcmem);

        ReleaseDC(hWnd,hdc);

        WinFree(lpbmi);
        WinFree(lp);
}

void
dofont(HWND hWnd)
{
        HDC     hDC;
        hDC = GetDC(hWnd);

        PaintFont(hDC,8,0);
        PaintFont(hDC,16,1);
        PaintFont(hDC,24,2);
        PaintFont(hDC,32,4);
        PaintFont(hDC,48,7);

        ReleaseDC(hWnd,hDC);
}

BOOL
MyDialogProc(HWND hDlg,UINT wMsg,WPARAM wParam,LPARAM lParam)
{
    switch(wMsg) {
    case WM_INITDIALOG:
        return 1;
    case WM_COMMAND:

        switch(LOWORD(wParam)) {
            case IDOK:
            case IDCANCEL:
                EndDialog(hDlg,LOWORD(wParam));
                break;
        }
        break;

    default:
        break;
    }
    return 0;
}
COLORREF        colors[8] = {
        RGB(0,0,0),
        RGB(255,0,0),
        RGB(0,255,0),
        RGB(0,0,255),
        RGB(255,255,0),
        RGB(0,255,255),
        RGB(192,192,192),
        RGB(80,80,80),
};

void
dotext(HWND hWnd,HDC hdc,char *str)
{
        RECT    r;
        HDC     hDC;
        char msgstr[4*26+4];
        SIZE    dwSize;
        int     line,delta;
        static  int testing;
        static  int c;

        strcpy(msgstr,str);
        strcat(msgstr,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        strcat(msgstr,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        strcat(msgstr,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");
        strcat(msgstr,"ABCDEFGHIJKLMNOPQRSTUVWXYZ");

        if(hdc == 0)
                hDC = GetDC(hWnd);
        else
                hDC = hdc;

        c &= 0x7;
        SetTextColor(hDC,colors[c]);
        c++;

        GetWindowRect(hWnd,&r);
        GetClientRect(hWnd,&r);

        GetTextExtentPoint( hDC, msgstr, strlen(msgstr), &dwSize );

        line = r.bottom-16;
        delta = dwSize.cy;

        TextOut( hDC, r.right-64,line, msgstr, strlen(msgstr) );
        line += delta;

        TextOut( hDC, r.right-64,line, msgstr, strlen(msgstr) );
        line += delta;

        TextOut( hDC, r.right-64,line, msgstr, strlen(msgstr) );
        line += delta;

        TextOut( hDC, r.right-64,line, msgstr, strlen(msgstr) );
        line += delta;


        if(testing) {
            GetClientRect(hWnd,&r);
            //ScrollWindow(hWnd,testing,testing,&r,0);
            ScrollDC(hDC,testing,testing,0,0,0,0);
        }

        if(hdc == 0)
                ReleaseDC(hWnd,hDC);
}

void
dowin(HWND hWnd)
{
    int rc;
    rc = DialogBox(hInstance,"edittest",hWnd,(DLGPROC)MyDialogProc);
}


void
dobeep(HWND hWnd)
{
       Beep(3330,250);
}

void
domsgbeep(HWND hWnd)
{
       MessageBeep(0);
}



void
dovcr(HWND hWnd)
{
        RECT    r,w,u;
        HDC     hDC;
        HRGN    hrgn,stock;

        //logstr(-1,"VCR START========================================\n");
        //logstr(-1,"VCR before erase client\n");
        hDC = GetDC(hWnd);
        GetUpdateRect(hWnd,&u,0);

        //logstr(-1,"VCR initial update rect = %d,%d,%d,%d\n",
        //    u.left,u.top,u.right,u.bottom);

        GetClientRect(hWnd,&r);
        GetWindowRect(hWnd,&w);

        //logstr(-1,"VCR initial client rect = %d,%d,%d,%d\n",
          //  r.left,r.top,r.right,r.bottom);
        //logstr(-1,"VCR initial window rect = %d,%d,%d,%d\n",
          //  w.left,w.top,w.right,w.bottom);

        FillRect(hDC,&r,GetStockObject(GRAY_BRUSH));

        ValidateRect(hWnd,&r);

        GetUpdateRect(hWnd,&u,0);

        //logstr(-1,"VCR update rect after validate = %d,%d,%d,%d\n",
          //  u.left,u.top,u.right,u.bottom);

        FillRect(hDC,&r,GetStockObject(WHITE_BRUSH));
        SetRect(&w,0,0,500,400);
        FillRect(hDC,&w,GetStockObject(WHITE_BRUSH));

        MoveTo(hDC,r.left,r.top);
        LineTo(hDC,r.right,r.bottom);
        MoveTo(hDC,r.right,r.top);
        LineTo(hDC,r.left,r.bottom);

        hrgn = CreateRectRgn(r.left,r.top,r.right,r.bottom);
        stock = SelectObject(hDC,hrgn);

        GetUpdateRect(hWnd,&u,0);

        //logstr(-1,"VCR update rect after selectregion = %d,%d,%d,%d\n",
        //    u.left,u.top,u.right,u.bottom);

        FillRect(hDC,&r,GetStockObject(GRAY_BRUSH));

        ScrollDC(hDC,0,40,&r,&r,0,&u);
        //logstr(-1,"VCR update rect after scrolldc = %d,%d,%d,%d\n",
        //    u.left,u.top,u.right,u.bottom);

        SelectObject(hDC,stock);
        DeleteObject(hrgn);

        ReleaseDC(hWnd,hDC);

        //logstr(-1,"VCR END========================================\n");
}


/*********************************************************************
**
** WinWndProc()
**
** Window Procedure
**
**********************************************************************/

char *GetTwinMsgCode(HWND,UINT);

long FAR PASCAL __export
WinWndProc( HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam )
{
  HDC   hDC;
  static BOOL bMouse, bKeyboard;
    char *msgstr;

  LONG rc;

    msgstr = GetTwinMsgCode(hWnd,Msg);

  //logstr(-1,"winwndproc: %x %x %x %x %s\n",
//      hWnd,Msg,wParam,lParam,msgstr);

  switch (Msg)
  {
    case WM_INITMENU:
        break;

    case WM_CREATE:
      bMouse = TRUE;
      bKeyboard = TRUE;

      bMouse = FALSE;
      bKeyboard = FALSE;
      break;

    case WM_TIMER:
      if(wParam == 1)
        UpdateMessage(hWnd,"timer 1");
      else
        UpdateMessage(hWnd,"timer 2");
      break;

    case WM_CHAR:
    case WM_KEYDOWN:
    case WM_SYSKEYDOWN:
    case WM_SYSKEYUP:
        if( bKeyboard )
        {
          ShowKey( hWnd, msgstr, lParam );
        }
        break;

    case WM_KEYUP:

        switch(wParam) {
#ifdef LATER
            case 'H':
                ShowWindow(hWnd,SW_HIDE);
                break;
            case 'S':
                ShowWindow(hWnd,SW_NORMAL);
                break;
            case 'M':
                domsgbeep(hWnd);
                break;
            case 'S':
                dobeep(hWnd);
                break;
#endif
            case 'C':
                doclr(hWnd);
                break;
            case 'E':
                doblt(hWnd);
                break;
            case 'M':
                domemdc(hWnd);
                break;
            case 'T':
                dotimer(hWnd);
                break;
            case 'F':
                dofont(hWnd);
                break;
            case 'Q':
                PostQuitMessage(0);
                break;
            case 'R':
                doregion(hWnd);
                break;
            case 'V':
                dovcr(hWnd);
                break;
            case 'W':
                dowin(hWnd);
                break;
            case 'L':
                dotext(hWnd,0,"direct");
                break;
           default:
                if( bKeyboard )
                {
                  ShowKey( hWnd, msgstr, lParam );
                }
                break;

        }
    break;

    case WM_MOUSEMOVE:
    if( bMouse )
    {
      ShowMouse( hWnd, msgstr, Msg , lParam);
    }
    break;
    case WM_LBUTTONDBLCLK:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    if( bMouse )
    {
      ShowMouse( hWnd, msgstr, Msg , lParam);
    }
    break;

    case WM_COMMAND:
      switch (LOWORD(wParam))
      {
        case IDM_ABOUT:
          //hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
          //ShellAbout(hWnd, Program, Title, hIcon);
          //DestroyIcon(hIcon);
          break;

        case IDM_EXIT:
          DestroyWindow(hWnd);
          break;
        case IDM_EXIT+1:
        case IDM_EXIT+2:
        case IDM_EXIT+3:
        case IDM_EXIT+4:
        case IDM_EXIT+5:
        case IDM_EXIT+6:
        case IDM_EXIT+7:
        case IDM_EXIT+8:
        case IDM_EXIT+9:
        case IDM_EXIT+10:
        case IDM_EXIT+11:
        case IDM_EXIT+12:
                {
                        char buffer[128];
                        wsprintf(buffer,"accelerator IDM_EXIT+%d lparam=%d",
                                LOWORD(wParam) - IDM_EXIT,lParam);
                        MessageBox(hWnd,buffer,
                                "Accelerator Test", MB_OK);
                        break;
                }

        case IDM_MOUSEEVENTS:
          bMouse = !bMouse;
          if( bMouse )
          {
            CheckMenuItem( GetMenu(hWnd), IDM_MOUSEEVENTS,      MF_BYCOMMAND | MF_CHECKED );
          }
          else
          {
            CheckMenuItem( GetMenu(hWnd), IDM_MOUSEEVENTS,      MF_BYCOMMAND | MF_UNCHECKED );
          }
          break;

        case IDM_KEYBOARDEVENTS:
          bKeyboard = !bKeyboard;
          if( bKeyboard )
          {
            CheckMenuItem( GetMenu(hWnd), IDM_KEYBOARDEVENTS,   MF_BYCOMMAND | MF_CHECKED );
          }
          else
          {
            CheckMenuItem( GetMenu(hWnd), IDM_KEYBOARDEVENTS,   MF_BYCOMMAND | MF_UNCHECKED );
          }
          break;
      }
      break;

    case WM_SIZE:
      switch (wParam)
      {
        case SIZEICONIC:
          MessageBeep(0);
          break;
      }
      break;

    case WM_ERASEBKGND:
        {
            RECT        r;
            //logstr(-1,"WINDEMO: wm_erasebkgnd\n");
            hDC = (HDC) wParam;
            //hDC = GetDC(hWnd);
            GetUpdateRect(hWnd,&r,0);
            //logstr(-1,"WINDEMO: update rect erase = %d,%d,%d,%d\n",
            //  r.left,r.top,r.right,r.bottom);

            GetClientRect(hWnd,&r);

            //ValidateRect(hWnd,&r);

            rc = DefWindowProc(hWnd, Msg, wParam, lParam);

            GetUpdateRect(hWnd,&r,0);
            //logstr(-1,"WINDEMO: update rect after erase = %d,%d,%d,%d\n",
            //  r.left,r.top,r.right,r.bottom);
#ifdef LATER
            MoveTo(hDC,r.left,r.top);
            LineTo(hDC,r.left,r.bottom);
            LineTo(hDC,r.right,r.bottom);
            LineTo(hDC,r.right,r.top);
            LineTo(hDC,r.left,r.top);
            FillRect(hDC,&r,GetStockObject(WHITE_BRUSH));

            MoveTo(hDC,r.left,r.top);
            LineTo(hDC,r.right,r.bottom);
            MoveTo(hDC,r.right,r.top);
            LineTo(hDC,r.left,r.bottom);
#endif
            //ReleaseDC(hWnd,hDC);

            //logstr(-1,"WINDEMO: after erase background\n");

        }
        return rc;
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            RECT r;

            GetUpdateRect(hWnd,&r,0);

            hDC = BeginPaint(hWnd,&ps);

            if(GetParent(hWnd) == 0)
                dotext(hWnd,hDC,"parent");
            else
                dotext(hWnd,hDC,"child");

            EndPaint(hWnd,&ps);

            GetUpdateRect(hWnd,&r,0);

        }
        return 1;

    case WM_DESTROY:
      PostQuitMessage(0);
      break;

    default:
      return (DefWindowProc(hWnd, Msg, wParam, lParam));
  }

  /* By default return 0. */
  return (0);
}

extern int __argc;
extern char **__argv;

/*********************************************************************
**
** WinMain()
**
** Entry Point
**
**********************************************************************/
BOOL PASCAL
WinMain( HANDLE hInst, HANDLE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  HACCEL hAccelTable;
  HWND   hWnd,hWndParent;
  MSG    Msg;
  int    s;
  char   msgbuf[80];
  RECT   r;

  if (__argc > 1)
        s = atoi(__argv[1]);
  else
        s = 1;

  s = 0;
  if (s) {
        wsprintf(msgbuf,"Willows Demo Program\nMessageBox test: root\nExecuting tests %x",
                s);
        MessageBox(0,msgbuf, "MessageBox Test", MB_OK);
  }

  //debuggerbreak();

  if (!hPrevInstance)
  {
    WNDCLASS wc;

    wc.style         = 0;
    wc.lpfnWndProc   = WinWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor       = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName  = (LPSTR) "WinMenu";
    wc.lpszClassName = (LPSTR) Program;

    if ( !RegisterClass(&wc) )
    {
        return FALSE;
    }

    wc.style         = CS_OWNDC | CS_GLOBALCLASS | CS_DBLCLKS;
    wc.style         = 0;
    wc.lpfnWndProc   = WinWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInst;
    wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
    wc.hCursor       = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH);
    wc.lpszMenuName  = (LPSTR) 0;
    wc.lpszClassName = (LPSTR) Child;

    if ( !RegisterClass(&wc) )
    {
        return FALSE;
    }
  }

  LoadString(hInst,IDS_TITLE,Title,256);
  hInstance = hInst;


  hAccelTable = LoadAccelerators( hInstance, (LPSTR)"RESOURCE" );

  hWndParent = CreateWindow( Program,
                             Title,
                             WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL,
                             200,
                             25,
                             500,
                             400,
                             0,
                             0,
                             hInst,
                             NULL);

  if (!hWndParent)
  {
    return FALSE;
  }

  ShowWindow(hWndParent, nCmdShow);
  UpdateWindow(hWndParent);

  GetClientRect(hWndParent,&r);
  hWnd = CreateWindow( Child,
                             Title,
                             WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE,
                             0,
                             0,
                             r.right,
                             r.bottom,
                             hWndParent,
                             0,
                             hInst,
                             NULL);


  UpdateWindow(hWnd);
  SetFocus(hWnd);

  if (s & 2)
        MessageBox(0,"Test MessageBox #2\nowned by root",
              "MessageBox Test", MB_OK);
  if (s & 4)
        MessageBox(hWnd,"Test MessageBox #3\nowned by window...",
              "MessageBox Test", MB_OK);

  /* Do a test to see how the client is drawing. */
  if (s & 8)
        CheckClient( hWnd );

  while (GetMessage(&Msg, 0, 0, 0))
  {
    if (TranslateAccelerator( hWnd, hAccelTable, (LPMSG)&Msg ) == 0)
    {
      TranslateMessage(&Msg);
      DispatchMessage(&Msg);
    }
  }

  if (s & 16)
        MessageBox(hWnd,"Test MessageBox #4\nowned by root\ndemo exiting...",
              "MessageBox Test", MB_OK);

  return Msg.wParam;
}



/*********************************************************************
**
** CheckClient()
**
** Test positioning of client area...
**
**********************************************************************/
static void CheckClient( HWND hWnd )
{
  HDC hDC;
  RECT r;
  HPEN hPen, hOldPen;

  hDC = GetDC(hWnd);

  GetClientRect( hWnd, &r );
  hPen = CreatePen(PS_SOLID, 0, RGB(0xff,0,0));
  hOldPen = SelectObject(hDC,hPen);

  /* Draw an x first */
  MoveToEx( hDC, r.left, r.top, NULL );
  LineTo( hDC, r.right, r.bottom );
  MoveToEx( hDC, r.right, r.top, NULL );
  LineTo( hDC, r.left, r.bottom );

  /* Draw a diamond around client area */
  MoveToEx( hDC, r.right/2, r.top, NULL );
  LineTo( hDC, r.right, r.bottom/2 );
  LineTo( hDC, r.right/2, r.bottom );
  LineTo( hDC, r.left, r.bottom/2 );
  LineTo( hDC, r.right/2, r.top );

  SelectObject(hDC,hOldPen);

  ReleaseDC(hWnd,hDC);

}

void
UpdateMessage(HWND hWnd, char *msgstr)
{
  HDC hDC;
  SIZE  dwSize;
  /* Print out key. */

  hDC = GetDC( hWnd );

  GetTextExtentPoint( hDC, msgstr, strlen(msgstr), &dwSize );

  ScrollWindowEx( hWnd, 0, dwSize.cy, NULL, NULL, 0, NULL, SW_ERASE|SW_INVALIDATE);

  UpdateWindow(hWnd);

  TextOut( hDC, 0, 0, msgstr, strlen(msgstr) );

  ReleaseDC( hWnd, hDC );
}

static int TotalMessages;

/*********************************************************************
**
** ShowKey()
**
** Shows keyboard input in client area
**
**********************************************************************/
static void ShowKey( HWND hWnd, char *lpstr, long scancode )
{
  char szBuffer[100];
  char msg[80];

  szBuffer[0] = 0;
  GetKeyNameText(scancode,szBuffer,100);

  wsprintf( msg, "%4.4d: %s scan=%x key=%s",++TotalMessages,lpstr,scancode,szBuffer );

  UpdateMessage(hWnd,msg);

}



/*********************************************************************
**
** ShowMouse()
**
** Shows mouse input in client area
**
**********************************************************************/
static void ShowMouse( HWND hWnd, char *lpstr, UINT Message , LPARAM lParam)
{
  char szBuffer[100];
  char *p;
  short x,y;

  switch( Message )
  {
    case WM_MOUSEMOVE:
    {
      p = "Mouse Moved" ;
      break;
    }
    case WM_LBUTTONDBLCLK:
    {
      p = "Left Button Double Clicked" ;
      break;
    }
    case WM_LBUTTONDOWN:
    {
      p = "Left Button Down" ;
      break;
    }
    case WM_LBUTTONUP:
    {
      p =  "Left Button Up" ;
      break;
    }
    case WM_MBUTTONDBLCLK:
    {
      p = "Middle Button Double Clicked" ;
      break;
    }
     case WM_MBUTTONDOWN:
    {
      p = "Middle Button Down" ;
      break;
    }
   case WM_MBUTTONUP:
    {
      p = "Middle Button Up" ;
      break;
    }
    case WM_RBUTTONDBLCLK:
    {
      p = "Right Button Double Clicked" ;
      break;
    }
    case WM_RBUTTONDOWN:
    {
      p = "Right Button Down" ;
      break;
    }
    case WM_RBUTTONUP:
    {
      p = "Right Button Up" ;
      break;
    }
  }
  x = LOWORD(lParam);
  y = HIWORD(lParam);
  wsprintf( szBuffer, "%4.4d: %s %s %4d,%4d",
        ++TotalMessages,p ,lpstr,x,y);

  UpdateMessage(hWnd,szBuffer);
}
