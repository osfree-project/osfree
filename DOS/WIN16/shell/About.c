/*    
	About.c	2.21
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

#include "windows.h"
#include "windowsx.h"
#include "commdlg.h"
#include "Log.h"
#include "Dialog.h"
#include "About.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	char *win_version;
	char *ver_info;
	char *bld_info;
	char *win_info;
} TWIN_VersionStructure;

TWIN_VersionStructure * TWIN_GetVersionInfo();

static BOOL ShellAboutHandler(HWND, unsigned, WPARAM, LPARAM);

void
ShellAbout(HWND hWnd, LPCSTR lpszCaption, LPCSTR lpszAboutText,
		HICON hIcon)
{
	FARPROC	lpProc;
	SHELLABOUTDATA sad;
	HINSTANCE  hInst;
	int     rc;
	

	if(hWnd)
		hInst = GetWindowInstance(hWnd);
	else    hInst = 0;

	lpProc= MakeProcInstance((FARPROC)ShellAboutHandler,hInst);

	sad.lpszText    = (LPSTR)lpszAboutText;
	sad.lpszCaption = (LPSTR) lpszCaption;
	sad.hIcon = hIcon;

	rc = DialogBoxParam(
	        0, 
		"ShellAbout",
		hWnd,
		lpProc,
		(LPARAM)&sad		
		);
	FreeProcInstance(lpProc); 
}

DWORD TWIN_GetModuleFileName(HMODULE ,LPTSTR , DWORD ,int );

static BOOL FAR PASCAL
ShellAboutHandler(HWND hDlg, unsigned msg, WPARAM wParam, LPARAM lParam)
{
	SHELLABOUTDATA *sad;
	HWND	        hWnd;
	HDC	        hDC;
	char		abouttext[256];
	char		dirname[256];
	int		bpp;
	TWIN_VersionStructure *vs = TWIN_GetVersionInfo();

	switch(msg) {
	case WM_INITDIALOG:
		sad = (SHELLABOUTDATA *) lParam;
		
		/************************************************/
		/*	Caption first				*/
		/************************************************/

		sprintf(abouttext,"About %s",sad->lpszCaption);
	    	SetWindowText(hDlg,abouttext);

		/************************************************/
		/*	Default first two lines			*/
		/************************************************/
		SendDlgItemMessage (hDlg, SAB_ABOUT, WM_SETTEXT, 0,
		                    ( LONG )sad->lpszCaption);
		SendDlgItemMessage (hDlg, SAB_TEXT, WM_SETTEXT, 0,
		                    ( LONG )sad->lpszText);

		/************************************************/
		/*	Twin specific 5 lines			*/
		/************************************************/
		TWIN_GetModuleFileName(0,abouttext,256,0);

		/* add any shell about specific string */
		SendDlgItemMessage(hDlg, SAB_USER, WM_SETTEXT, 0,
			(LONG) abouttext);

		GetWindowsDirectory(dirname,256);
		sprintf(abouttext,"Windows: %s",dirname);
		SendDlgItemMessage(hDlg, SAB_WINDOW, WM_SETTEXT, 0,
			(LONG) abouttext);

		GetSystemDirectory(dirname,256);
		sprintf(abouttext,"System: %s",dirname);
		SendDlgItemMessage(hDlg, SAB_SYSTEM, WM_SETTEXT, 0,
			(LONG) abouttext);

		/************************************************/
		/*	Host specific 2 lines			*/
		/************************************************/
#ifdef LATER
	Add platform specific string to platform.h
	Use to fill out dialog box...
#endif
		/* host system information */
		sprintf ( abouttext, "Host: %s",TWIN_PLATFORM);
		SendDlgItemMessage(hDlg, SAB_HOST, WM_SETTEXT, 0,
			(LONG)abouttext);

		/* workstation information */
		hDC = GetDC(hDlg);
		bpp = GetDeviceCaps(hDC, BITSPIXEL);
		ReleaseDC(hDlg, hDC);

		sprintf(abouttext,"Mode: %s  %d bpp  %s",
			vs->win_info, bpp, vs->bld_info);
		SendDlgItemMessage(hDlg, SAB_TERM, WM_SETTEXT, 0,
			(LONG)abouttext);

		/* add the current mode... */
		sprintf(abouttext,"Version:%s", vs->ver_info);
		SendDlgItemMessage(hDlg, SAB_VERSION, WM_SETTEXT, 0,
			(LONG)abouttext);
	
		/************************************************/
		/*	Icon passed in?				*/
		/************************************************/
		if (sad->hIcon) 
	    		SendDlgItemMessage(hDlg, SAB_ICON, STM_SETICON,
				sad->hIcon, 0L);
		 else  {
			hWnd = GetDlgItem(hDlg,SAB_ICON);
			ShowWindow(hWnd,SW_HIDE);
		}

		return TRUE;

	case WM_COMMAND:
		switch ( GET_WM_COMMAND_ID(wParam, lParam) )
		{
			case IDOK:
				EndDialog(hDlg, IDOK);
				break;
			default:
				return ( FALSE );
		}
		return ( TRUE );
	}
	return FALSE;
}
