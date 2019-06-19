/*    	
	ComboBox.c	2.41
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */

#ifndef OEMRESOURCE
#define OEMRESOURCE     /* for OBM_CLOSE */
#endif

#include "windows.h"
#include "windowsx.h"

#undef OEMRESOURCE

#include "WinDefs.h"    /* this file now knows about HWND32 */
#include "WinData.h"
#include "Driver.h"
#include "Log.h"

#include <string.h>

typedef struct  {
    HFONT   hFont;          /* hFont used */
    HWND    hWndParent;     /* parent window */
    UINT    nID;            /* control ID */
    WORD    wStateFlags;    /* combobox state flags */
    UINT    wStyle;         /* this is a copy of LOWORD(style) */
    BOOL    bExtended;      /* extended UI flag */
    BOOL    bRedraw;        /* MiD - redraw flag, draw only if it's 1 */
    HWND    EditControl;    /* edit/static control hWnd */
    HWND    ListBoxControl; /* listbox control hWnd */
    RECT    ButtonRect;     /* local button rect (client) */
    RECT    ListBoxRect;    /* listbox rect (screen) */
    UINT    uHeight;        /* height of the normal state */
    WNDPROC lpfnOldStatic;  /* previous static wndproc */
    UINT    nListItems;     /* ecw */
} COMBOBOX;

#define CWD_LPCBDATA  0
#define CBC_EDITID    1

#define CSF_CAPTUREACTIVE   0x0001
#define CSF_LOCALBUTTONDOWN 0x0002
#define CSF_BUTTONDOWN      0x0004
#define CSF_LBOXBUTTONDOWN  0x0008
#define CSF_FOCUS           0x0010 /* MiD */
#define CSF_HASDROPPED	    0x0020 /* weav */

#define SET_STATE(lp, wMask)   (lp->wStateFlags |= (wMask))
#define CLEAR_STATE(lp, wMask) (lp->wStateFlags &= ~(wMask))
#define IS_SET(lp, wMask)      (lp->wStateFlags & (wMask))

#define BOWNERDRAW(l) ((l)->wStyle & (CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE))

/**********************************************
    Styles:

    CBS_AUTOHSCROLL     passed to the edit control
    CBS_DISABLENOSCROLL passed to the listbox control
    CBS_DROPDOWN
    CBS_DROPDOWNLIST
    CBS_HASSTRINGS      passed to the listbox control
    CBS_NOINTEGRALHEIGHT    passed to the listbox control
    CBS_OEMCONVERT      passed to the edit control
    CBS_OWNERDRAWFIXED  passed to the listbox control
    CBS_OWNERDRAWVARIABLE   passed to the listbox control
    CBS_SIMPLE      TODO
    CBS_SORT        passed to the listbox control

    WS_VSCROLL      passed to the listbox control

*********************************************/

/**********************************************
    CBN_xxx messages to be added

    from mouse tracking...
    CBN_SELENDCANCEL    TODO
    CBN_SELENDOK        TODO

*********************************************/

/* imported stuff */
extern void     Draw3DButtonRect(HDC,HPEN,HPEN,RECT,UINT);
extern LRESULT  DefLISTBOXProc(HWND, UINT, WPARAM, LPARAM);
extern HPEN     GetSysColorPen(int);
extern HCLASS32 FindClass(LPCSTR, HINSTANCE);
extern HWND TWIN_ConvertToSysScroll(HWND, BOOL, LPPOINT);

/* internal stuff */
static void CBoxDrawButton(HWND,UINT,COMBOBOX *);
static void CBoxSendMouseToLBox(COMBOBOX *, UINT, WPARAM, POINT);
static void CBoxCapture(HWND, WORD);
static void CBoxDrawEdit(COMBOBOX *, HWND, UINT);
static void CBoxDrawStatic(COMBOBOX *, HWND, UINT); /* MiD */

/* handle specific CB messages */
static LRESULT DefCBProc(HWND , UINT , WPARAM , LPARAM );

static WNDPROC lpComboBinToNat = 0;

LRESULT DefCOMBOBOXProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;                   /* paint structure           */
    HDC      hDC;
    TEXTMETRIC tm;
    MEASUREITEMSTRUCT mis;
    COMBOBOX *lp = (COMBOBOX *)NULL;
    LRESULT   rc;
    HINSTANCE hInst;
    HCLASS32 hComboClass32;
    POINT     cp,cpScreen;
    UINT      uiKey;
    LPCREATESTRUCT lpcs;
    LPWINDOWPOS lpwp;
    LPMEASUREITEMSTRUCT lpmis;
    LPDRAWITEMSTRUCT lpdis;
    LPDELETEITEMSTRUCT lpdlis;
    DWORD dwStyle,dwExStyle;
    WORD wEditWidth = 0,wEditHeight;
    WORD wCBN;
    RECT rcClient;

    rc = CB_OKAY;
    if ((uMsg != WM_NCCREATE) && (uMsg != WM_CONVERT) &&
       !(lp = (COMBOBOX *)GetWindowLong(hWnd,CWD_LPCBDATA)))
    return rc;

    switch(uMsg) {
#ifdef  LATER
    case WM_SIZE:
    case WM_ENABLE:
    case WM_LBUTTONDBLCLK:
    case WM_COMPAREITEM:
    case WM_CUT:
    case WM_CLEAR:
#endif               

    case WM_SETFOCUS:
        SET_STATE(lp, CSF_FOCUS);
        if ((lp->wStyle & 0x0F) == CBS_DROPDOWNLIST)
           {
           uiKey = (UINT)SendMessage(lp->ListBoxControl, LB_GETCURSEL, 0, 0L);
           CBoxDrawStatic(lp, hWnd, uiKey);
           }
        break;
        
    case WM_KILLFOCUS:
        CLEAR_STATE(lp, CSF_FOCUS);
        if ((lp->wStyle & 0x0F) == CBS_DROPDOWNLIST)
           {
           uiKey = (UINT)SendMessage(lp->ListBoxControl, LB_GETCURSEL, 0, 0L);
           CBoxDrawStatic(lp, hWnd, uiKey);
           }
        /*
        **    Hide listbox when loosing focus to window other than 
        **    our own listbox... When wParam == 0 we "loose" the focus
        **    to the scrollbar in a listbox!
        */
        if ((lp->wStyle & 0x0F) != CBS_SIMPLE && wParam != lp->ListBoxControl && wParam != 0)
           SendMessage(hWnd, CB_SHOWDROPDOWN, 0, 0L);

        break;


    case WM_KEYDOWN:     /* MiD 08/14/95 */
        /*
        **   We have to process this message in order to show
        **   current selection in a static control for certain
        **   keys. This doesn't affect combobox with an edit
        **   control, since the edit traps all key messages.
        */
        {
        int nCur   = SendMessage(lp->ListBoxControl, LB_GETCURSEL,0, 0L);
	int nPrevCur = nCur;
        int nCount = SendMessage(lp->ListBoxControl, LB_GETCOUNT, 0, 0L);

        if (nCount == 0)
           break;

        switch(wParam)
            {
            case VK_HOME:
               nCur = 0;
               break;

            case VK_END:
               nCur = nCount - 1;
               break;

            case VK_UP:
               nCur--;
               break;

            case VK_DOWN:
               nCur++;
               break;

            default:
              return 0L;
            }

        if (nCur >= nCount)
           nCur = nCount - 1;  
        if (nCur < 0)
           nCur = 0;

        SendMessage(lp->ListBoxControl, LB_SETCURSEL, nCur, 0L);
        SendMessage(lp->hWndParent, WM_COMMAND, GET_WM_COMMAND_MPS(lp->nID, hWnd, CBN_SELCHANGE));
	if (nCur != nPrevCur)
/* ecw */  SendMessage(lp->hWndParent, WM_COMMAND, GET_WM_COMMAND_MPS(lp->nID, hWnd, CBN_SELENDOK));
        InvalidateRect(hWnd, NULL, 1);
        break;
        }

    case WM_CHAR:
        {
        int nNewCur;
        int nOldCur;

        if (lp->EditControl)
           {
           SendMessage(lp->EditControl, uMsg, wParam, lParam);
           }
        else {
             nOldCur = SendMessage(lp->ListBoxControl, LB_GETCURSEL,0, 0L);
             SendMessage(lp->ListBoxControl, uMsg, wParam, lParam);
             nNewCur = SendMessage(lp->ListBoxControl, LB_GETCURSEL, 0, 0L);
             if (nNewCur != nOldCur)
                {
                SendMessage(lp->hWndParent, WM_COMMAND, GET_WM_COMMAND_MPS(lp->nID, hWnd, CBN_SELCHANGE));
                InvalidateRect(hWnd, NULL, 1);
                }
             }
        break;
        }

    case WM_SETREDRAW: 
        lp->bRedraw = wParam;
        if (lp->EditControl)
           SendMessage(lp->EditControl, WM_SETREDRAW, wParam, lParam);
        if (lp->ListBoxControl)
           SendMessage(lp->ListBoxControl, WM_SETREDRAW, wParam, lParam);
        break;
        
    case WM_NCCREATE:
        lp = (COMBOBOX *)WinMalloc(sizeof(COMBOBOX));
        memset((LPSTR)lp,'\0',sizeof(COMBOBOX));

        /* save ptr to internal structure */
        SetWindowLong(hWnd, CWD_LPCBDATA, (LONG) lp);

        /* this is for CreateWindow calls */
        hInst = GetWindowInstance(hWnd);

        /* fill in the internal structure */
        lpcs = (LPCREATESTRUCT)lParam;
        lp->bRedraw = 1;
        lp->wStateFlags = 0;
        lp->wStyle  = (UINT)LOWORD(lpcs->style);
        if (!BOWNERDRAW(lp))
           lp->wStyle |= CBS_HASSTRINGS;
        lp->bExtended  = TRUE;
        lp->hFont = 0;
        lp->hWndParent = lpcs->hwndParent;
        lp->nID  = (UINT)lpcs->hMenu;

        /* calc the height of the edit/static control */
        if (BOWNERDRAW(lp)) 
           {
           mis.CtlType = ODT_COMBOBOX;
           mis.CtlID = (UINT)lpcs->hMenu;
           mis.itemID = (UINT)-1;
           mis.itemData = 0L;
           SendMessage(lpcs->hwndParent, WM_MEASUREITEM, (WPARAM)lpcs->hMenu, (LPARAM)&mis);
           /*** wEditHeight = (WORD)mis.itemHeight + 2; ***/
           }
        /* get system font dimensions */
        hDC = GetDC((HWND)0);
        GetTextMetrics(hDC,&tm);
        ReleaseDC((HWND)0,hDC);

        /* allow different fonts to fit, don't hard code */
        /* otherwise big fonts won't fit. */
        /*****wEditHeight = ((tm.tmHeight - tm.tmInternalLeading)*7)/4;*****/
        wEditHeight = tm.tmHeight + tm.tmInternalLeading * 3;

        lp->uHeight = (UINT)wEditHeight;

	if ((lp->wStyle & 0x0F) != CBS_SIMPLE)
           {
           lp->ButtonRect.top    = 0;
           lp->ButtonRect.left   = lpcs->cx - 1 - GetSystemMetrics(SM_CXVSCROLL);
           lp->ButtonRect.right  = lpcs->cx;
           lp->ButtonRect.bottom = wEditHeight;
           /* for CBS_DROPDOWN/DROPDOWNLIST resize the window  */
           SetWindowPos(hWnd, 0,
                        0, 0, lpcs->cx, (int)wEditHeight,
                        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
           }
        else SetRectEmpty(&lp->ButtonRect);

        if ((lp->wStyle & 0xf) != CBS_DROPDOWNLIST) 
           {  /* EDIT field - calc edit control style */
           dwStyle = WS_CHILD | WS_VISIBLE | WS_BORDER;
           if (lp->wStyle & CBS_AUTOHSCROLL)
              dwStyle |= ES_AUTOHSCROLL;
           if (lp->wStyle & CBS_OEMCONVERT)
              dwStyle |= ES_OEMCONVERT;

           if ((lp->wStyle & 0x0F) == CBS_SIMPLE)
             wEditWidth = lpcs->cx;
           else if ((lp->wStyle & 0xf) == CBS_DROPDOWN)
                   wEditWidth = lp->ButtonRect.left - 5;

           /* create edit control */
           lp->EditControl = CreateWindow("EDIT", NULL, dwStyle,
                                          0, 0, wEditWidth, wEditHeight,
                                          hWnd, CBC_EDITID,
                                          hInst,(LPVOID)NULL);
           }
        else /* CBS_DROPDOWN -- static instead of edit */
             lp->EditControl = 0;
             
        /* listbox style */
        dwExStyle = WS_EX_SAVEBITS | WS_EX_NOCAPTURE | WS_EX_POPUPMENU;
        dwStyle =   WS_BORDER | LBS_NOTIFY | LBS_COMBOLBOX;
        if ((lp->wStyle & 0xf) == CBS_SIMPLE)
            dwStyle |= WS_VISIBLE | WS_CHILD; 
        else
            dwStyle |= WS_POPUP;
        if (lp->wStyle & CBS_DISABLENOSCROLL)
            dwStyle |= LBS_DISABLENOSCROLL;
        if (lp->wStyle & CBS_HASSTRINGS)
            dwStyle |= LBS_HASSTRINGS;
        if (lp->wStyle & CBS_NOINTEGRALHEIGHT)
            dwStyle |= LBS_NOINTEGRALHEIGHT;
        if (lp->wStyle & CBS_OWNERDRAWFIXED)
            dwStyle |= LBS_OWNERDRAWFIXED;
        if (lp->wStyle & CBS_OWNERDRAWVARIABLE)
            dwStyle |= LBS_OWNERDRAWVARIABLE;
        if (lp->wStyle & CBS_SORT)
            dwStyle |= LBS_SORT;
        if (lpcs->style & WS_VSCROLL)
            dwStyle |= WS_VSCROLL;

        /* calc listbox dimensions and position */
        if ((lp->wStyle & 0xf) == CBS_SIMPLE) 
           { 
           lp->ListBoxRect.left = 5;
           lp->ListBoxRect.top = wEditHeight - 1;
           lp->ListBoxRect.right = lpcs->cx;
           lp->ListBoxRect.bottom = lpcs->cy - 2;
           }
        else {
             lp->ListBoxRect.left = lpcs->x; 
             lp->ListBoxRect.right = lp->ListBoxRect.left + lpcs->cx - 1;
             lp->ListBoxRect.top = lpcs->y + wEditHeight - 1;
             lp->ListBoxRect.bottom = lp->ListBoxRect.top + lpcs->cy + 1;
             if ((lp->wStyle & 0x0F) == CBS_DROPDOWN)
                {
                lp->ListBoxRect.left += 5;
                }

             }
#ifdef LATER
        cp.x = ((lp->wStyle & 0xf) == CBS_DROPDOWNLIST)?0:5;
        cp.y = wEditHeight - 1;
        if ((lp->wStyle & 0xf) != CBS_SIMPLE)
            ClientToScreen(hWnd,&cp);
        lp->ListBoxRect.left = cp.x;
        lp->ListBoxRect.top =  cp.y;
        lp->ListBoxRect.right = cp.x + lpcs->cx;
        if ((lp->wStyle & 0xf) != CBS_DROPDOWNLIST)
            lp->ListBoxRect.right -= 5;
        lp->ListBoxRect.bottom = lp->ListBoxRect.top + lpcs->cy -
                wEditHeight + 1;
#endif
        lp->ListBoxControl = CreateWindowEx(dwExStyle,"COMBOLBOX",
                                            NULL, dwStyle,
                                            lp->ListBoxRect.left, lp->ListBoxRect.top,
                                            lp->ListBoxRect.right - lp->ListBoxRect.left,
                                            lp->ListBoxRect.bottom - lp->ListBoxRect.top,
                                            hWnd, 0,
                                            hInst,(LPVOID)NULL);
           
#ifdef  LATER
        /* Microsoft Word 6.0 wants to see COMBOLBOX on top */
        /*  of Z-order... */
        if (dwStyle & WS_POPUP)
            SetWindowPos(lp->ListBoxControl, HWND_TOP,
                         0, 0, 0, 0,
                         SWP_NOREDRAW | SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOMOVE);
#endif
        /* physically expand client window,
           if there is a scroll style
        */
        if (lpcs->style & WS_VSCROLL) 
           {
           HWND32 hWnd32 = GETHWND32(hWnd);
           RECT rc;

           SetRectEmpty(&hWnd32->rcNC);

           SetRect(&rc, 0, 0,
                   hWnd32->rWnd.right-hWnd32->rWnd.left,
                   hWnd32->rWnd.bottom-hWnd32->rWnd.top);

           hWnd32->wWidth = (WORD)rc.right;
           hWnd32->wHeight = (WORD)rc.bottom;
	   RELEASEWININFO(hWnd32);
           }
        /* 
        **   Finally turn off border drawing and WM_?SCROLL styles to prevent creation
        **   of system scrollbars.
        */ 
        dwStyle = GetWindowLong(hWnd, GWL_STYLE);
        dwStyle &= ~(WS_VSCROLL | WS_HSCROLL | WS_BORDER | WS_DLGFRAME | WS_THICKFRAME);
        SetWindowLong(hWnd, GWL_STYLE, dwStyle);
        lp->nListItems = 0;
        return TRUE;

    case WM_NCDESTROY:
        if (IsWindow(lp->ListBoxControl))
           DestroyWindow(lp->ListBoxControl);
        if (IsWindow(lp->EditControl))
           DestroyWindow(lp->EditControl);
        WinFree((LPSTR)lp);
        return 0L;

    case WM_GETDLGCODE:
        return (LRESULT)(DLGC_WANTCHARS|DLGC_WANTARROWS);

    case WM_LBUTTONDOWN:
        if ((lp->wStyle & 0xf) == CBS_SIMPLE)
            break;

        cp.x = (int)(short)LOWORD(lParam);
        cp.y = (int)(short)HIWORD(lParam);

        if (!IS_SET(lp, CSF_CAPTUREACTIVE)) /* no listbox yet */
           {                                                                          
           /* click on a button or anywhere if it's dropdown combo */
           if (PtInRect(&lp->ButtonRect, cp) || 
              (lp->wStyle & 0x0F) == CBS_DROPDOWNLIST)
              {
              if (PtInRect(&lp->ButtonRect, cp))
                 CBoxDrawButton(hWnd, 1, lp);
              cp.x = ((lp->wStyle & 0xf) != CBS_DROPDOWNLIST) ? 5 : 0;
              cp.y = lp->uHeight - 1;
              ClientToScreen(hWnd, &cp);
              OffsetRect(&lp->ListBoxRect, cp.x - lp->ListBoxRect.left, cp.y - lp->ListBoxRect.top);
              SetWindowPos(lp->ListBoxControl, HWND_TOP, /*0,*/
                           cp.x, cp.y, 0, 0,
                           SWP_NOSIZE | /*SWP_NOZORDER |*/ SWP_NOACTIVATE);
              SendMessage(lp->hWndParent, WM_COMMAND, GET_WM_COMMAND_MPS(lp->nID,hWnd,CBN_DROPDOWN));
	      /*  ECW   added following conditional...  4/4/96 */
	      if (!IS_SET(lp, CSF_HASDROPPED)) {
		  /* first time it drops down, size it to hold all items */
		  int nitems = SendMessage(lp->ListBoxControl,LB_GETCOUNT,0,0L);
		  if (lp->ListBoxRect.bottom - lp->ListBoxRect.top <
		      ((lp->uHeight-2) * nitems)) {
		    nitems = (nitems > 12 ? 12 : nitems); /* a dozen, max */
		    lp->ListBoxRect.bottom =
		      lp->ListBoxRect.top + ((lp->uHeight-2) * nitems);
		    SetWindowPos(lp->ListBoxControl,HWND_TOP,0,0,
				 lp->ListBoxRect.right - lp->ListBoxRect.left,
				 lp->ListBoxRect.bottom - lp->ListBoxRect.top,
				 SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
		  }
		  SET_STATE(lp, CSF_HASDROPPED);
	      }
	      /*  End of addition */
              ShowWindow(lp->ListBoxControl, SW_SHOW);
	      SetFocus(lp->ListBoxControl);
              CBoxCapture(hWnd, 1);
              SET_STATE(lp, CSF_CAPTUREACTIVE);
              SET_STATE(lp, CSF_BUTTONDOWN);
              }
           }
        else { /* there is a listbox visible */
             HWND hwndNewFocus = 0;
             
             cpScreen = cp;
             if ((lp->wStyle & 0xf) != CBS_SIMPLE)
                {
                ClientToScreen(hWnd, &cpScreen);
                hwndNewFocus = WindowFromPoint(cpScreen);
                }
             if (PtInRect(&lp->ListBoxRect, cpScreen)) 
                {
                CBoxSendMouseToLBox(lp, WM_LBUTTONDOWN, wParam, cpScreen);
                }
             else {
                  if (PtInRect(&lp->ButtonRect, cp))
                     CBoxDrawButton(hWnd, 0, lp);
                  if ((lp->wStyle & 0x0F) == CBS_DROPDOWN && hwndNewFocus == lp->EditControl)
                     /* don't close listbox */;
                  else {
                       SendMessage(lp->hWndParent, WM_COMMAND, GET_WM_COMMAND_MPS(lp->nID,hWnd,CBN_CLOSEUP));
                       SetWindowPos(lp->ListBoxControl, 0,
                               0, 0, 0, 0,
                               SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_HIDEWINDOW);
                       CBoxCapture(hWnd, 0);
                       CLEAR_STATE(lp, CSF_BUTTONDOWN);
                       }
                  CLEAR_STATE(lp, CSF_CAPTUREACTIVE);
                  if (hwndNewFocus && hwndNewFocus != hWnd)
                     {                      
                     ScreenToClient(hwndNewFocus, &cpScreen);
                     SetFocus(hwndNewFocus);
                     SendMessage(hwndNewFocus, WM_LBUTTONDOWN, wParam, MAKELONG(cpScreen.x, cpScreen.y));
                     }
                  }
             }
        break;

    case WM_MOUSEMOVE:
        if (!IS_SET(lp,CSF_BUTTONDOWN))
            break;
        cp.x = (int)(short)LOWORD(lParam);
        cp.y = (int)(short)HIWORD(lParam);
        if (IS_SET(lp, CSF_CAPTUREACTIVE)) 
           {
           if (PtInRect(&lp->ButtonRect,cp))
              {
              if (!IS_SET(lp, CSF_LOCALBUTTONDOWN))
                 CBoxDrawButton(hWnd, 1, lp);
              break;
              }   
           if ((lp->wStyle & 0xf) != CBS_SIMPLE)
              ClientToScreen(hWnd,&cp);
           if (PtInRect(&lp->ListBoxRect,cp)) 
              {
              CBoxSendMouseToLBox(lp,WM_MOUSEMOVE,wParam,cp);
              }
           if (IS_SET(lp,CSF_LOCALBUTTONDOWN) && ((lp->wStyle & 0xf) != CBS_SIMPLE))
              CBoxDrawButton(hWnd,0,lp);
           }
        break;

    case WM_LBUTTONUP:
        if (!IS_SET(lp, CSF_CAPTUREACTIVE))
            break;
        cp.x = (int)(short)LOWORD(lParam);
        cp.y = (int)(short)HIWORD(lParam);

        CLEAR_STATE(lp,CSF_BUTTONDOWN);

        if (PtInRect(&lp->ButtonRect, cp))
           /*(lp->wStyle & 0x0F) == CBS_DROPDOWNLIST)*/
           {
           if (PtInRect(&lp->ButtonRect, cp))
               CBoxDrawButton(hWnd, 0, lp);
           if (IS_SET(lp, CSF_LBOXBUTTONDOWN)) 
              {
              if ((lp->wStyle & 0xf) != CBS_SIMPLE)
                 ClientToScreen(hWnd, &cp);
              CBoxSendMouseToLBox(lp, WM_LBUTTONUP, wParam, cp);
              CLEAR_STATE(lp,CSF_LBOXBUTTONDOWN);
              }
           break;
           }
        if ((lp->wStyle & 0xf) != CBS_SIMPLE)
           ClientToScreen(hWnd, &cp);
        if (PtInRect(&lp->ListBoxRect, cp)) 
           {
           uiKey = (UINT)SendMessage(lp->ListBoxControl, LB_GETCURSEL, 0, 0);
           if (uiKey != (UINT)LB_ERR) 
              { 
              if (lp->EditControl)
                 { 
                 SetFocus(lp->EditControl); 
                 CBoxDrawEdit(lp, hWnd, uiKey); 
                 }
              else { 
                   SetFocus(hWnd); 
                   CBoxDrawStatic(lp, hWnd, uiKey); 
                   }
              
              /*  LATER check the WS_EX_NOPARENTNOTIFY bit in ext style.*/
/* ecw */     SendMessage(lp->hWndParent, WM_COMMAND, GET_WM_COMMAND_MPS(lp->nID,hWnd,CBN_SELENDOK));
              SendMessage(lp->hWndParent, WM_COMMAND, GET_WM_COMMAND_MPS(lp->nID,hWnd,CBN_CLOSEUP));
              SetWindowPos(lp->ListBoxControl, 0,
                           0, 0, 0, 0,
                           SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_HIDEWINDOW);
              CBoxCapture(hWnd, 0);
              CLEAR_STATE(lp,CSF_CAPTUREACTIVE);
              }
              
           CBoxSendMouseToLBox(lp, WM_LBUTTONUP, wParam, cp);
           CLEAR_STATE(lp,CSF_LBOXBUTTONDOWN);
           }
        else /* clicked somewhere outside button or listbox -
             ** the listbox should stay intact... MiD
             */
             if (IS_SET(lp, CSF_LBOXBUTTONDOWN)) 
                {
                if ((lp->wStyle & 0xf) != CBS_SIMPLE)
                   ClientToScreen(hWnd, &cp);
                CBoxSendMouseToLBox(lp, WM_LBUTTONUP, wParam, cp);
                CLEAR_STATE(lp,CSF_LBOXBUTTONDOWN);
                }
        break;

    case WM_ERASEBKGND:
        return 1L;

    case WM_PAINT:
        BeginPaint(hWnd,&ps);
        EndPaint(hWnd,&ps);

        if (!IsWindowVisible(hWnd) || !lp->bRedraw)
           return 0L;

        if ((lp->wStyle & 0xf) != CBS_SIMPLE)
           CBoxDrawButton(hWnd, IS_SET(lp,CSF_LOCALBUTTONDOWN), lp);
        uiKey = (UINT)SendMessage(lp->ListBoxControl, LB_GETCURSEL, 0, 0);
        if (lp->EditControl) 
           CBoxDrawEdit(lp, hWnd, uiKey);
        else CBoxDrawStatic(lp, hWnd, uiKey);
        return 0L;
        
    case WM_COMMAND:
        if (GET_WM_COMMAND_ID(wParam,lParam) == CBC_EDITID) {
            /* edit/static control notifications */
            switch((short)GET_WM_COMMAND_CMD(wParam,lParam)) {
            case EN_SETFOCUS:
#ifdef  LATER
                wCBN = CBN_SETFOCUS;
#else
                wCBN = 0;
#endif
                break;
            case EN_KILLFOCUS:
                wCBN = CBN_KILLFOCUS;
                break;
            case EN_CHANGE:
                {
                int  index = 0;
                char sz[128];
                /*
                **   Advance listbox
                **   selection until there is string match. One first mismatch
                **   listbox advances to its first item.
                */
                SendMessage(lp->EditControl, WM_GETTEXT, sizeof(sz)-1, (LPARAM)sz);
                if (lstrlen(sz) > 0L)
                   index = (int)SendMessage(lp->ListBoxControl, LB_FINDSTRING, -1, (LPARAM)sz);
                if (index == LB_ERR)
                   index = 0;
                SendMessage(lp->ListBoxControl, LB_SETTOPINDEX, index, 0L);
                wCBN = CBN_EDITCHANGE;
                break;
                }
            case EN_UPDATE:
                wCBN = CBN_EDITUPDATE;
                break;
            case EN_ERRSPACE:
                wCBN = CBN_ERRSPACE;
                break;
            default:
                wCBN = 0;
                break;
            }
            if (wCBN)
            return SendMessage(lp->hWndParent,WM_COMMAND,
                GET_WM_COMMAND_MPS(lp->nID,hWnd,wCBN));
            else
            return rc;
        }
        if (GET_WM_COMMAND_ID(wParam,lParam) == 0) {
            /* listbox notifications */
            switch ((short)GET_WM_COMMAND_CMD(wParam,lParam)) {
            case LBN_ERRSPACE:
                wCBN = CBN_ERRSPACE;
                break;
            case LBN_SELCHANGE:
                if ((lp->wStyle & 0xf) == CBS_SIMPLE) 
                   {
                   uiKey = (UINT)SendMessage(lp->ListBoxControl, LB_GETCURSEL, 0, 0);
                   if (uiKey != (UINT)LB_ERR)
                      if (lp->EditControl)
                         {
                         CBoxDrawEdit(lp, hWnd, uiKey);
                         }
                   }
                wCBN = CBN_SELCHANGE;
                break;
            case LBN_DBLCLK:
                wCBN = CBN_DBLCLK;
                break;
            case LBN_SELCANCEL: /* TODO */
                wCBN = 0;
                break;
            case LBN_SETFOCUS:
                wCBN = CBN_SETFOCUS;
                break;
            case LBN_KILLFOCUS:
                wCBN = CBN_KILLFOCUS;
                break;
            default:
                wCBN = 0;
                break;
            }
            if (wCBN)
               return SendMessage(lp->hWndParent, WM_COMMAND, GET_WM_COMMAND_MPS(lp->nID,hWnd,wCBN));
            else
            return rc;
            }
        break;

    case WM_GETTEXT:
	if ( lp->EditControl )
	    return SendMessage(lp->EditControl,uMsg,wParam,lParam);
	else if ( lp->ListBoxControl ) {
	    WPARAM sel, len;

	    sel = (WPARAM)SendMessage(lp->ListBoxControl, LB_GETCURSEL, 0, 0);
	    if ( sel != (WPARAM)LB_ERR ) {
		len = (WPARAM)SendMessage(lp->ListBoxControl, LB_GETTEXTLEN, 0, 0);
		if ( len <= wParam )
		    return SendMessage(lp->ListBoxControl, LB_GETTEXT, sel, lParam);
	    }
	}
	return CB_ERR;

    case WM_GETTEXTLENGTH:
	if ( lp->EditControl )
	    return SendMessage(lp->EditControl,uMsg,wParam,lParam);
	else if ( lp->ListBoxControl ) {
	    WPARAM sel;

	    sel = (WPARAM)SendMessage(lp->ListBoxControl, LB_GETCURSEL, 0, 0);
	    if ( sel != (WPARAM)LB_ERR ) 
		return SendMessage(lp->ListBoxControl, LB_GETTEXTLEN, 0, 0);
	}
	return CB_ERR;

    case WM_SETTEXT:
	if ( lp->EditControl )
	    return SendMessage(lp->EditControl,uMsg,wParam,lParam);
	return CB_ERR;

    case WM_SETFONT:
        lp->hFont = (HFONT)wParam;

        hDC = GetDC(hWnd);
        SelectObject(hDC,lp->hFont);
        GetTextMetrics(hDC,&tm);
        ReleaseDC(hWnd,hDC);
        wEditHeight = tm.tmHeight + 3 * tm.tmInternalLeading;

        if (wEditHeight == lp->uHeight)
            return 0L;

        lp->uHeight = (UINT)wEditHeight;
        lp->ButtonRect.bottom = wEditHeight;
        /*
        **   The following SetWindowPos causes WM_WINDOWPOSCHANGING message
        **   where child windows are resized and/or moved.
        */
        ShowWindow(hWnd, SW_HIDE);
        GetClientRect(hWnd,&rcClient);
        if ((lp->wStyle & 0xf) != CBS_SIMPLE) 
           SetWindowPos(hWnd, 0,
                        0, 0, rcClient.right, (int)wEditHeight,
                        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
        else SetWindowPos(hWnd, 0,
                          0, 0, rcClient.right, (int)wEditHeight + lp->ListBoxRect.bottom - lp->ListBoxRect.top + 1,
                          SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOREDRAW);
        ShowWindow(hWnd, SW_SHOWNA);

        if (lp->EditControl)
           SendMessage(lp->EditControl, WM_SETFONT, wParam,lParam);
        SendMessage(lp->ListBoxControl, WM_SETFONT, wParam,lParam);

        if(LOWORD(lParam))
            RedrawWindow(hWnd,(const RECT *)0,(HRGN)0,
            RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW );
        return (LRESULT)0;

    case WM_GETFONT:
        return lp->hFont;

    case WM_WINDOWPOSCHANGING:
        lpwp = (LPWINDOWPOS)lParam;
        if (lpwp)
        {
        if (!(lpwp->flags & SWP_NOSIZE))
            {
            lp->ButtonRect.right  = lpwp->cx;
            if ((lp->wStyle & 0xf) == CBS_SIMPLE) 
               lp->ButtonRect.left = lp->ButtonRect.right;
            else lp->ButtonRect.left = lpwp->cx - 1 -
                    GetSystemMetrics(SM_CXVSCROLL);

            if (lp->EditControl) 
               {
               wEditWidth = lp->ButtonRect.left + 1;
               if ((lp->wStyle & 0xf) == CBS_SIMPLE)
                  wEditWidth --;
               if ((lp->wStyle & 0xf) == CBS_DROPDOWN)
                  wEditWidth -= 5;
               SetWindowPos(lp->EditControl,(HWND)0,
                            0,0,
                            wEditWidth, lp->uHeight,
                            SWP_NOACTIVATE|SWP_NOMOVE|SWP_NOZORDER);
               }
            if (lp->ListBoxControl) 
               {
               if ((lp->wStyle & 0x0F) == CBS_SIMPLE)
                  {
                  lp->ListBoxRect.left = 5;
                  lp->ListBoxRect.top = lp->uHeight - 1;
                  lp->ListBoxRect.right = lpwp->cx;
                  lp->ListBoxRect.bottom = lpwp->cy - 2;
                  }
               else {
                    POINT cp;
		    cp.x = 0;
		    cp.y = lp->uHeight - 1;
                    ClientToScreen(hWnd, &cp);
                    OffsetRect(&lp->ListBoxRect, cp.x - lp->ListBoxRect.left, cp.y - lp->ListBoxRect.top);

                    lp->ListBoxRect.right = lp->ListBoxRect.left + lpwp->cx;
                    if ((lp->wStyle & 0xf) != CBS_DROPDOWNLIST)
                       lp->ListBoxRect.right -= 5;
                    }
               SetWindowPos(lp->ListBoxControl,(HWND)0,
                            lp->ListBoxRect.left, lp->ListBoxRect.top, 
                            lp->ListBoxRect.right - lp->ListBoxRect.left,
                            lp->ListBoxRect.bottom - lp->ListBoxRect.top,
                            SWP_NOACTIVATE|SWP_NOZORDER);
               }
            /* the height of the normal state stays the same */
            if ((lp->wStyle & 0xf) != CBS_SIMPLE)
               lpwp->cy = (int)lp->uHeight;
            }
        }
        return (LRESULT)0;

    case WM_WINDOWPOSCHANGED:
        DefWindowProc(hWnd,uMsg,wParam,lParam);
        lpwp = (LPWINDOWPOS)lParam;
        if (lpwp) {
        if (!(lpwp->flags & SWP_NOSIZE)) /* TODO */
            RedrawWindow(hWnd,(const RECT *)0,(HRGN)0,
            RDW_INVALIDATE|RDW_ERASE
#ifdef  LATER
            |RDW_UPDATENOW
#endif
            );
        }
        return (LRESULT)0;

    /*********************************************/
    /* ownerdraw stuff               */
    /*********************************************/
    case WM_DRAWITEM:
        lpdis = (LPDRAWITEMSTRUCT)lParam;
        lpdis->CtlType = ODT_COMBOBOX;
        lpdis->CtlID = lp->nID;
        lpdis->hwndItem = hWnd;
        return SendMessage(lp->hWndParent,WM_DRAWITEM,
                (WPARAM)lp->nID,lParam);

    case WM_MEASUREITEM:
        lpmis = (LPMEASUREITEMSTRUCT)lParam;
        lpmis->CtlType = ODT_COMBOBOX;
        lpmis->CtlID = lp->nID;
        return SendMessage(lp->hWndParent,WM_MEASUREITEM,
                (WPARAM)lp->nID,lParam);

    case WM_DELETEITEM:
        lpdlis = (LPDELETEITEMSTRUCT)lParam;
        lpdlis->CtlType = ODT_COMBOBOX;
        lpdlis->CtlID = lp->nID;
        lpdlis->hwndItem = hWnd;
        return SendMessage(lp->hWndParent,WM_DELETEITEM,
                (WPARAM)lp->nID,lParam);

    case WM_CONVERT:
        if (!lpComboBinToNat) {
        hComboClass32 = FindClass("COMBOBOX",0);
        lpComboBinToNat = (WNDPROC)GetClassHandleLong(
                hComboClass32,GCL_BINTONAT);
        }
        if (lpComboBinToNat)
        return lpComboBinToNat(hWnd, uMsg, wParam, lParam);
        else
        return (LRESULT)0;

    default:
        return DefCBProc( hWnd, uMsg, wParam, lParam);
    }
    return rc;
}

/************************************************************************
**
************************************************************************/
static LRESULT DefCBProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    int       len,index;
    COMBOBOX *lp;
    char     *selection;
    int   rc;
    POINT   cp;

    lp = (COMBOBOX *) GetWindowLong(hWnd,CWD_LPCBDATA);
    switch(uMsg) {
        /*********************************************/
        /* messages specific to the list box control */
        /*********************************************/
        case CB_ADDSTRING:
            lp->nListItems++;  /* shd. test for successful return */
            return SendMessage(lp->ListBoxControl,LB_ADDSTRING,
                wParam,lParam);
            
        case CB_DELETESTRING:
	    if (lp->nListItems)
	      lp->nListItems--;
            return SendMessage(lp->ListBoxControl,LB_DELETESTRING,
                wParam,lParam);
            
        case CB_DIR:
            return SendMessage(lp->ListBoxControl,LB_DIR,
                wParam,lParam);
            
        case CB_FINDSTRING:
            return SendMessage(lp->ListBoxControl,LB_FINDSTRING,
                wParam,lParam);
            
        case CB_FINDSTRINGEXACT:
               return SendMessage(lp->ListBoxControl,LB_FINDSTRINGEXACT,
                wParam,lParam);
            
        case CB_GETCOUNT:
            return SendMessage(lp->ListBoxControl,LB_GETCOUNT,
                wParam,lParam);
            
        case CB_GETCURSEL:
            return SendMessage(lp->ListBoxControl,LB_GETCURSEL,
                wParam,lParam);
            
        case CB_GETITEMDATA:
            return SendMessage(lp->ListBoxControl,LB_GETITEMDATA,
                wParam,lParam);
            
        case CB_GETITEMHEIGHT:
            return SendMessage(lp->ListBoxControl,LB_GETITEMHEIGHT,
                wParam,lParam);
            
        case CB_GETLBTEXT:
            return SendMessage(lp->ListBoxControl,LB_GETTEXT,
                wParam,lParam);
            
        case CB_GETLBTEXTLEN:
            return SendMessage(lp->ListBoxControl,LB_GETTEXTLEN,
                wParam,lParam);
            
        case CB_INSERTSTRING:
            return SendMessage(lp->ListBoxControl,LB_INSERTSTRING,
                wParam,lParam);
            
        case CB_SETITEMDATA:
            return SendMessage(lp->ListBoxControl,LB_SETITEMDATA,
                wParam,lParam);
            
        /*********************************************/
        /* messages specific to the edit control */
        /*********************************************/
        case CB_GETEDITSEL:
            return SendMessage(lp->EditControl,EM_GETSEL,0,0);
                
        case CB_LIMITTEXT:
            return SendMessage(lp->EditControl,EM_LIMITTEXT,
                wParam,lParam);

        case CB_SETEDITSEL:
            return SendMessage(lp->EditControl,EM_SETSEL,
                wParam,lParam);

        /*********************************************/
        /* messages handled by the combobox          */
        /*********************************************/
        case CB_GETDROPPEDCONTROLRECT:
            CopyRect((LPRECT)lParam,&lp->ListBoxRect);
            break;
        case CB_GETDROPPEDSTATE:
            return IS_SET(lp,CSF_CAPTUREACTIVE);

        case CB_GETEXTENDEDUI:
            return (LRESULT)lp->bExtended;

        case CB_RESETCONTENT:
            SendMessage(lp->ListBoxControl,LB_RESETCONTENT,0,0);
            if (lp->EditControl)
               SendMessage(lp->EditControl,WM_SETTEXT,0,(LPARAM)(LPSTR)"");
            break;

        case CB_SELECTSTRING:
            index = (int)SendMessage(lp->ListBoxControl, LB_SELECTSTRING, wParam, lParam);
            if (index == LB_ERR)
               return CB_ERR;

            len = (int)SendMessage(lp->ListBoxControl, LB_GETTEXTLEN, index, 0);
            if (len <= 0)
               return CB_ERR;

            selection = (LPSTR)WinMalloc((UINT)len+1);  
            rc = (int)SendMessage(lp->ListBoxControl, LB_GETTEXT, (WPARAM)index, (LPARAM)selection);
            if (lp->EditControl)
               rc = (int)SendMessage(lp->EditControl, WM_SETTEXT, 0, (LPARAM)selection);
            else CBoxDrawStatic(lp, hWnd, index);
            WinFree(selection);
            break;
            
        case CB_SETCURSEL:
            rc = (int)SendMessage(lp->ListBoxControl, LB_SETCURSEL, wParam, lParam);
            if (rc == LB_ERR)
               return CB_ERR;
            len = (int)SendMessage(lp->ListBoxControl, LB_GETTEXTLEN, wParam, 0);
            if (len <= 0)
               return CB_ERR;

            selection = (LPSTR)WinMalloc((UINT)len+1);  
            rc = (int)SendMessage(lp->ListBoxControl, LB_GETTEXT, wParam, (LPARAM)selection);
            if (lp->EditControl)
               rc = (int)SendMessage(lp->EditControl, WM_SETTEXT, 0, (LPARAM)selection);
            else CBoxDrawStatic(lp, hWnd, wParam);
            WinFree(selection);
            return (LRESULT)wParam;

        case CB_SETEXTENDEDUI:
            lp->bExtended = (BOOL)wParam;
            break;  

        case CB_SETITEMHEIGHT:      /* TODO */
            break;

        case CB_SHOWDROPDOWN:
            if ((lp->wStyle & 0xf) == CBS_SIMPLE)
                return 1L;
            if (wParam) 
               {
               if (IS_SET(lp,CSF_CAPTUREACTIVE))
                  return 1L;
               cp.x = ((lp->wStyle & 0xf) != CBS_DROPDOWNLIST) ? 5 : 0;
               cp.y = lp->uHeight -1;
               ClientToScreen(hWnd, &cp);
               OffsetRect(&lp->ListBoxRect, cp.x - lp->ListBoxRect.left, cp.y - lp->ListBoxRect.top);
               SetWindowPos(lp->ListBoxControl, 0,
                            cp.x, cp.y, 0, 0,
                            SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
               SendMessage(lp->hWndParent,WM_COMMAND, GET_WM_COMMAND_MPS(lp->nID,hWnd,CBN_DROPDOWN));
               SetWindowPos(lp->ListBoxControl, HWND_TOP,
                            0, 0, 0, 0,
                            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
                CBoxCapture(hWnd, 1);
                SET_STATE(lp,CSF_CAPTUREACTIVE);
                }
            else {
                 if (!IS_SET(lp,CSF_CAPTUREACTIVE))
                    return 1L;
                 SendMessage(lp->hWndParent, WM_COMMAND, GET_WM_COMMAND_MPS(lp->nID,hWnd,CBN_CLOSEUP));
                 SetWindowPos(lp->ListBoxControl, 0,
                              0, 0, 0, 0,
                              SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_HIDEWINDOW);
                 CBoxCapture(hWnd, 0);
                 CLEAR_STATE(lp, CSF_CAPTUREACTIVE);
                 }
            return 1L;

        /*********************************************/
        /* messages handled by the defwindowproc.... */
        /*********************************************/
        default:
            return DefWindowProc( hWnd, uMsg, wParam, lParam);
    }
    return CB_OKAY;
}

static void
CBoxDrawButton(HWND hWnd,UINT wState,COMBOBOX *lp)
{
    HDC       hDC;
    int     x,y;
    int     dx,dy;
    int     cx,cy;
    static int nWidth,nHeight;
    BITMAP    bmpCombo;
    static HBITMAP hbmpCombo = 0; 
    HBITMAP   hbmpOld = 0;
    HDC       hdcSrc;
    COLORREF  rgbText, rgbBk;
    HBRUSH    hBrush;
    HPEN      hPenHigh,hPenShadow;
    RECT      rc;

    hDC = GetDC(hWnd);
    hdcSrc = CreateCompatibleDC(hDC);

    CopyRect(&rc,&lp->ButtonRect);
    x = rc.left;
    y = rc.top;
    dx = rc.right;
    dy = rc.bottom;

    hPenHigh = GetStockObject(WHITE_PEN);
    hPenShadow = GetSysColorPen(COLOR_BTNSHADOW);
    hBrush = GetSysColorBrush(COLOR_BTNFACE);
    FillRect(hDC, &rc, hBrush);
    hBrush = GetStockObject(BLACK_BRUSH);
    FrameRect(hDC, &lp->ButtonRect, hBrush);
    rc.left += 1; rc.right -= 1;
    rc.top += 1; rc.bottom -= 1;
    Draw3DButtonRect(hDC,hPenHigh,hPenShadow,rc,wState);

    if (hbmpCombo == 0) 
       {
       hbmpCombo = LoadBitmap(0,(LPSTR)OBM_COMBO);
       GetObject(hbmpCombo, sizeof(BITMAP), (LPVOID)&bmpCombo);
       nWidth  = bmpCombo.bmWidth;
       nHeight = bmpCombo.bmHeight;
       }
   /*
   **   MiD 08/15/95 changed to mono bitmap as it is in Windows. Convert
   **                it to colors on the fly
   */
   hdcSrc = CreateCompatibleDC(hDC);
   hbmpOld = SelectObject(hdcSrc, hbmpCombo);
   /*
   **   Source hdc ok. Prepare the target hdc, then BitBlt to it.
   */
   rgbText = SetTextColor(hDC,GetSysColor(COLOR_BTNTEXT));
   rgbBk = SetBkColor(hDC,GetSysColor(COLOR_BTNFACE));

   cx = (dx - x - nWidth)/2;
   cy = (dy - y - nHeight)/2;
   if (wState) 
      {  cx++; cy++;  }
   BitBlt(hDC, x+cx, y+cy, nWidth, nHeight, hdcSrc, 0, 0, SRCCOPY);

   SetTextColor(hDC, rgbText);
   SetBkColor(hDC, rgbBk);
   SelectObject(hdcSrc,hbmpOld);
   DeleteDC(hdcSrc);
   ReleaseDC(hWnd,hDC);

    if (wState)
        SET_STATE(lp,CSF_LOCALBUTTONDOWN);
    else
        CLEAR_STATE(lp,CSF_LOCALBUTTONDOWN);
}

/************************************************************************
**
************************************************************************/
LRESULT DefCOMBOLBOXProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return DefLISTBOXProc(hWnd, msg, wParam,lParam);
}

/************************************************************************
**
************************************************************************/
static void CBoxSendMouseToLBox(COMBOBOX *lp, UINT uiMsg, WPARAM wParam, POINT ptScreen)
{
    POINT pt;
    int nNCHit;
    HWND hWndScroll;

    pt = ptScreen;
    ScreenToClient(lp->ListBoxControl,&pt);

    nNCHit = LOSHORT(SendMessage(lp->ListBoxControl, WM_NCHITTEST, 0, MAKELPARAM(ptScreen.x,ptScreen.y)));

    switch (nNCHit) 
       {
       case HTCLIENT:
           if (uiMsg == WM_MOUSEMOVE && !IS_SET(lp,CSF_LBOXBUTTONDOWN)) 
              {
              SendMessage(lp->ListBoxControl, WM_LBUTTONDOWN, 0, MAKELONG((WORD)pt.x,(WORD)pt.y));
              SET_STATE(lp, CSF_BUTTONDOWN | CSF_LBOXBUTTONDOWN);
              }
           SendMessage(lp->ListBoxControl, uiMsg, wParam, MAKELONG((WORD)pt.x,(WORD)pt.y));
           break;
           
       case HTVSCROLL:
           if (0 != (hWndScroll = TWIN_ConvertToSysScroll(lp->ListBoxControl, TRUE /* vertical */, &pt)))
              SendMessage(hWndScroll, uiMsg, wParam, MAKELONG((WORD)pt.x,(WORD)pt.y));
           break;
           
       default:
           break;
    }
}

/************************************************************************
**
************************************************************************/
static void CBoxCapture(HWND hWnd, WORD wFunc)
{
    static HWND hWndCapture = (HWND)0;

    if (wFunc) 
       {
       hWndCapture = SetCapture(hWnd);
       SetFocus(hWnd);
       }
    else {
         if (!hWndCapture)
            ReleaseCapture();
         else {
#ifdef  LATER
              SetCapture(hWndCapture);
#else
              ReleaseCapture();
#endif
              hWndCapture = (HWND)0;
              }
         }
}

/************************************************************************
**
************************************************************************/
static void CBoxDrawEdit(COMBOBOX *lp, HWND hWnd, UINT uiKey)
{
    int    nLen;
    LPVOID lpData;
    HRGN   hRgn;
    DRAWITEMSTRUCT dis;
/*
    if (uiKey == (UINT)LB_ERR)
       return;

    if (!BOWNERDRAW(lp)) 
*/
    if (lp->wStyle & CBS_HASSTRINGS)
       {
       if (uiKey == (UINT)LB_ERR)
	  return;
       nLen = (int)SendMessage(lp->ListBoxControl, LB_GETTEXTLEN, uiKey, 0L);
       if (nLen <= 0)
           return;
       lpData = (LPVOID)WinMalloc(nLen+1);
       SendMessage(lp->ListBoxControl, LB_GETTEXT, uiKey, (LPARAM)lpData);
       SendMessage(lp->EditControl, WM_SETTEXT, strlen(lpData), (LPARAM)lpData);
       Edit_SetSel(lp->EditControl, 0, -1);
       WinFree((LPSTR)lpData);
       }
    else {
         dis.CtlType = ODT_COMBOBOX;
         dis.CtlID = (UINT)lp->nID;
         dis.itemID = -1; /* used to be uiKey */
         dis.itemAction = ODA_DRAWENTIRE;
         dis.itemState = ODS_FOCUS;
         dis.hwndItem = hWnd;
         dis.itemData = 0;
         GetClientRect(lp->EditControl,&dis.rcItem);
         dis.rcItem.left += 3;
         dis.rcItem.right -= 3;
         dis.rcItem.top += 2;
         dis.rcItem.bottom -= 2;

         dis.hDC = GetDC(lp->EditControl);
         hRgn = CreateRectRgnIndirect(&dis.rcItem);
         SelectClipRgn(dis.hDC,hRgn);
         SelectObject(dis.hDC, lp->hFont);
         SendMessage(lp->hWndParent, WM_DRAWITEM, (WPARAM)(UINT)lp->nID, (LPARAM)&dis);
         ReleaseDC(lp->EditControl,dis.hDC);
         DeleteObject(hRgn);
         }   
}

/************************************************************************
**
************************************************************************/
static void CBoxDrawStatic(COMBOBOX *lp, HWND hWnd, UINT uiKey)
{   
    int    nLen;
    HDC    hdc;
    LPVOID lpData;
    HRGN   hRgn;
    RECT   rcClient;
    HFONT  hfonOld = 0;
    DRAWITEMSTRUCT dis;
    HBRUSH hbrStatic, hbrOld;               
    
    /*   Draw rectangle regardless of ownerdraw style...
    */           
    hdc = GetDC(hWnd);         
    rcClient.left   = 0;
    rcClient.top    = 0;
    rcClient.right  = lp->ButtonRect.left+1;
    rcClient.bottom = lp->uHeight;
    hbrStatic = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
    hbrOld = SelectObject(hdc, hbrStatic);
    SelectObject(hdc, GetStockObject(BLACK_PEN));/* ??? COLOR_WINDOWFRAME */
    Rectangle(hdc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
    SelectObject(hdc, hbrOld);
    DeleteObject(hbrStatic);
    ReleaseDC(hWnd, hdc);

    if (uiKey == (UINT)LB_ERR)
       return;

    if (!BOWNERDRAW(lp))
       {
       /* if necessary, draw text */
       hdc = GetDC(hWnd);
       nLen = (int)SendMessage(lp->ListBoxControl, LB_GETTEXTLEN, (WPARAM)uiKey, 0L);
       if (nLen > 0)
          {
          lpData = (LPVOID)WinMalloc(nLen+1);
          SendMessage(lp->ListBoxControl, LB_GETTEXT, uiKey, (LPARAM)lpData);
          SetBkMode(hdc, TRANSPARENT);
          if (!IS_SET(lp, CSF_FOCUS))
             {
             SetTextColor(hdc, GetSysColor(COLOR_WINDOWTEXT));
             rcClient.left += 2;
             }
          else {
               InflateRect(&rcClient, -2, -2);
               hbrStatic = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
               hbrOld = SelectObject(hdc, hbrStatic);
               FillRect(hdc, &rcClient, hbrStatic);
               DrawFocusRect(hdc, &rcClient);
               SelectObject(hdc, hbrOld);
               DeleteObject(hbrStatic);
               SetTextColor(hdc, GetSysColor(COLOR_HIGHLIGHTTEXT));
               }
          if (lp->hFont)
             hfonOld = SelectObject(hdc, lp->hFont);
          DrawText(hdc, (LPSTR)lpData, nLen, &rcClient, DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
          if (lp->hFont)
             SelectObject(hdc, hfonOld);
          WinFree((LPVOID)lpData);
          }
       ReleaseDC(hWnd, hdc);
       }
    else { /* fill OWNERDRAWSTRUCT and send WM_DRAWITEM message */
         dis.CtlType    = ODT_COMBOBOX;
         dis.CtlID      = (UINT)lp->nID;
         dis.itemID     = uiKey;
         dis.itemAction = ODA_DRAWENTIRE;
         dis.itemState  = ODS_FOCUS;
         dis.hwndItem   = hWnd;
         dis.itemData   = SendMessage(lp->ListBoxControl, LB_GETITEMDATA, uiKey, 0L);
         GetClientRect(hWnd, &dis.rcItem);
         dis.rcItem.left += 3; 
         dis.rcItem.right = lp->ButtonRect.left - 2;  /* do not touch button */
         dis.rcItem.top += 2; 
         dis.rcItem.bottom -= 2; 

         dis.hDC = GetDC(hWnd);
         hRgn = CreateRectRgnIndirect(&dis.rcItem);
         SelectClipRgn(dis.hDC, hRgn);
         SelectObject(dis.hDC, lp->hFont);
         SendMessage(lp->hWndParent, WM_DRAWITEM, (WPARAM)(UINT)lp->nID, (LPARAM)&dis);
         ReleaseDC(hWnd, dis.hDC);
         DeleteObject(hRgn);
         }   
}
