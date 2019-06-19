/*    wmfdemo.c	1.2 10/10/96 
 *
 *  Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.
 *
 *  Demonstrates the following:
 *
 *  - registering classes, and creating windows.
 *  - standard message loop and message dispatch logic.
 *  - use of common filedialogs
 *  - use of metafile functions
 *  - use of resources, menues.
 *  - calling the ShellAbout API.
 */

#include "windows.h"
#include "commdlg.h"
#include "wmfdemo.h"
#include <stdio.h>

extern void FAR PASCAL ShellAbout(HWND, LPCSTR, LPCSTR, HICON);

HINSTANCE hThisInst = (HINSTANCE)NULL;
char Title[]="WMFDemo Windows Application";
char Program[]="WMFDemo";
HMETAFILE hmf= (HMETAFILE)NULL;

BOOL GetFileName(HWND hWnd, char *pszFilename, int nLenFile, BOOL bSaving)
{
	BOOL (WINAPI *pfnDialog)(OPENFILENAME FAR *);
	OPENFILENAME ofn;

	pszFilename[0]=0;
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = hThisInst;
	ofn.lpstrFilter = "Metafiles (*.wmf)\0*.wmf\0All files (*.*)\0*.*\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = pszFilename;
	ofn.nMaxFile = nLenFile;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrInitialDir = NULL;
	ofn.lpfnHook = NULL;
	if (bSaving) {
		ofn.lpstrTitle = "Create Metafile";
		ofn.Flags = OFN_HIDEREADONLY | OFN_NOREADONLYRETURN | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST ;
	}
	else {
		ofn.lpstrTitle = "Play Back Metafile";
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST ;
	}
	ofn.lpstrDefExt = "wmf";
	
	pfnDialog = bSaving ? GetSaveFileName : GetOpenFileName;
	return pfnDialog(&ofn);
} /* GetFileName() */

void MakeMyMetafile(char *pszFilename)
{
	HDC hdc;
	HBRUSH hBrush, hBrushOld;
	HPEN hPen, hPenOld;
	HFONT hFont, hFontOld;
	RECT rect;
	int nSpacing[] = {20,8,8,8,15,20,15};
	
	hdc = CreateMetaFile(pszFilename);
	SetMapMode(hdc,MM_ANISOTROPIC);
	SetWindowOrg(hdc,0,0);
	SetWindowExt(hdc,150,150);

	hBrush = CreateSolidBrush(RGB(128, 0, 255));
	hBrushOld = SelectObject(hdc, hBrush);
	Ellipse(hdc, 0, 25, 100, 75);
	SelectObject(hdc, hBrushOld);
	DeleteObject(hBrush);
	
	hBrush = CreateSolidBrush(RGB(192, 192, 192));
	hBrushOld = SelectObject(hdc, hBrush);
	hPen = CreatePen(PS_DASH, 1, RGB(0, 0, 0));
	hPenOld = SelectObject(hdc, hPen);
	Rectangle(hdc, 25, 50, 75, 100);
	SelectObject(hdc, hPenOld);
	DeleteObject(hPen);
	SelectObject(hdc, hBrushOld);
	DeleteObject(hBrush);
	
	MoveTo(hdc, 40, 40);
	LineTo(hdc, 60, 60);
	LineTo(hdc, 60, 40);
	LineTo(hdc, 40, 60);
	LineTo(hdc, 40, 40);
	
	hBrush = CreateSolidBrush(RGB(64, 255, 64));
	hBrushOld = SelectObject(hdc, hBrush);
	hPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
	hPenOld = SelectObject(hdc, hPen);
	Chord(hdc, 0, 0, 100, 100, 0, 50, 50, 100);
	SelectObject(hdc, hPenOld);
	DeleteObject(hPen);
	SelectObject(hdc, hBrushOld);
	DeleteObject(hBrush);
	
	hFont = CreateFont(20, 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_TT_ALWAYS, PROOF_QUALITY, VARIABLE_PITCH | 0x04 | FF_SWISS, "Arial");
	hFontOld = SelectObject(hdc, hFont);
	rect.top = rect.left = 0;
	rect.bottom = rect.right = 100;
	ExtTextOut(hdc, 0, 0, 0, &rect, "Willows", 7, nSpacing);
	SelectObject(hdc, hFontOld);
	DeleteObject(hFont);

	hmf = CloseMetaFile(hdc);
	DeleteMetaFile(hmf);
        hmf = (HMETAFILE)NULL;
} /* MakeMyMetafile() */

long __export CALLBACK
WmfWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    HICON hIcon;
	char szFilename[256];
	char NewTitle[256];
	PAINTSTRUCT	ps;
	HDC			hDC;
	RECT		r;

    switch(msg)  
    {
      case WM_PAINT :
		hDC = BeginPaint(hWnd, &ps);
		SetMapMode(hDC, MM_ISOTROPIC);
		GetClientRect(hWnd, &r);
		SetViewportOrg(hDC, r.left, r.top);
		SetViewportExt(hDC, r.right, r.bottom);
		if(hmf)
			PlayMetaFile(hDC, hmf);
		EndPaint(hWnd, &ps);
        break;

	  case WM_SIZE:
		InvalidateRect(hWnd,0,TRUE);
		break;

      case WM_CLOSE :
      case WM_DESTROY :
		if (hmf != (HMETAFILE)NULL) {
	  	  DeleteMetaFile(hmf);
		  hmf = (HMETAFILE)NULL;
		}
    	PostQuitMessage(0);
        break;
        
      case WM_COMMAND :
        switch(wParam) 
        {
    		case IDM_EXIT:
    			PostMessage(hWnd,WM_CLOSE,0,0);
    			break;

    		case IDM_CLEAR:
    		    	if(hmf != (HMETAFILE)NULL) {
    		      	  DeleteMetaFile(hmf);
    		      	  hmf=(HMETAFILE)NULL;
    		      	  SetWindowText(hWnd,Title);
    		    	}
			InvalidateRect(hWnd, NULL, TRUE);	    		
    		     	break;

    		case IDM_ABOUT:
		   	hIcon = LoadIcon(hThisInst, MAKEINTRESOURCE(IDI_WILLOWS));
		   	ShellAbout(hWnd, Program, Title, hIcon);
		   	DestroyIcon(hIcon);
       			break;
        		
        	case IDM_CREATE:
        		if (GetFileName(hWnd, szFilename, sizeof(szFilename), TRUE))
        		{
           		  MakeMyMetafile(szFilename);
        		  MessageBox(hWnd, szFilename, "Metafile created", MB_OK|MB_ICONINFORMATION);\
        		}
				break;
				        		
        	case IDM_PLAYBACK:
        		if (GetFileName(hWnd, szFilename, sizeof(szFilename), FALSE))
			{
			  if ( hmf ) {
				/* Delete previous metafile */
				DeleteMetaFile(hmf);
				hmf = (HMETAFILE)NULL;
			  }
			  if ( !(hmf = GetMetaFile(szFilename)) ) {
				MessageBox(hWnd,
					"Cannot get the metafile",
					szFilename, MB_OK);
			  }
			  else
			  { 
			    sprintf(NewTitle,"%s - %s",Title,szFilename);
			    SetWindowText(hWnd,NewTitle);
			    InvalidateRect(hWnd,0,TRUE);
			  }
        		}  
			break;
        }	
      default :
        return DefWindowProc(hWnd, msg, wParam, lParam);
	}       
	return 0;
} /* WmfWndProc() */

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
        wc.lpfnWndProc = WmfWndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = hInstance;
	wc.hIcon   = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WILLOWS));
        wc.hCursor = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
        wc.hbrBackground = GetStockObject(WHITE_BRUSH);
        wc.lpszMenuName = "WmfMenu";
        wc.lpszClassName = "WmfClass";

        if ( !RegisterClass(&wc) )
            return FALSE;
    }

    hThisInst = hInstance;

    hWnd = CreateWindow(
        "WmfClass",
        Title,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        400,
        200,
        (HWND)NULL,
        (HMENU)NULL,
        hInstance,
        NULL);

    if ( !hWnd )
        return FALSE;

    ShowWindow(hWnd, nShow);
    UpdateWindow(hWnd);
                

    while ( GetMessage(&Message, (HWND)NULL, 0, 0) ) {
        TranslateMessage(&Message);
        DispatchMessage(&Message);
    }
    
    return (BOOL)Message.wParam;

} /* WinMain() */
