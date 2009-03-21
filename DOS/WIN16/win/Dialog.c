/*    
	Dialog.c	2.59
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

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "windows.h"
#include "windowsx.h"

#include "Resources.h"
#include "kerndef.h"
#include "Log.h"
#include "Dialog.h"
#include "Button.h"
#include "ConvertRes.h"

/* external routines */
extern BOOL TestWF(HWND, DWORD);
extern void SetWF(HWND, DWORD);
extern HBRUSH GetControlBrush(HWND, HDC, WORD);
extern HCLASS32 FindClass(LPCSTR,HINSTANCE);

/* internal routines */
static HWND GetFirstDlgTabItem(HWND);
HWND InternalCreateDialog(HINSTANCE,LPSTR,HWND,DLGPROC,LPARAM);
static HWND GetNextDlgItem(HWND, HWND);
static HWND GetNextDlgControl(HWND ,HWND,HWND);
static HWND GetFirstDlgTabItem(HWND);
static HWND DlgSetFocus(HWND);
static int  ControlByName(HWND ,HWND ,WORD );
static DWORD GetDialogFontUnits(HFONT);
static HWND SaveDlgFocus(HWND);
static HWND RestoreDlgFocus(HWND);
static LPBYTE ConvertDlgTemplate(HINSTANCE, LPDLGTEMPLATE);

/* exported routines */
int InternalDialog(HWND );
void CheckDefPushButton(HWND, HWND, HWND);

extern ATOM atmGlobalLookup[];

static LPSTR std_controls[] = {
        "BUTTON",
        "EDIT",
        "STATIC",
        "LISTBOX",
        "SCROLLBAR",
        "COMBOBOX"
}; 


/*******************************************************************

	DialogBox Family

*******************************************************************/

int WINAPI
DialogBox(HINSTANCE hInst, LPCSTR lpDialog, HWND hWnd, DLGPROC lpDlgFunc)
{
	return DialogBoxParam(hInst, lpDialog, hWnd, lpDlgFunc, 0L);
}

int WINAPI
DialogBoxParam(HINSTANCE hInst, LPCSTR lpDlg, HWND hWnd,
		DLGPROC lpFunc, LPARAM lParam)
{
	HWND 	hDlg;
	HRSRC	hDialog;
	HGLOBAL  hTemplate;
	DIALOGDATA *lpdd;
	LPDLGTEMPLATE lpTemplate;

	if (!(hDialog = FindResource(hInst, lpDlg, RT_DIALOG)))
	    return -1;

	if (!(hTemplate = LoadResource(hInst, hDialog)))
	    return -1;

	if (!(lpTemplate = (LPDLGTEMPLATE)LockResource(hTemplate)))
	    return -1;

	/* modal dialogs are always visible */
	lpTemplate->style |= WS_VISIBLE;

	lpdd = (DIALOGDATA *)ConvertDlgTemplate(hInst, lpTemplate);
	hDlg = InternalCreateDialog(hInst,(LPSTR)lpdd,hWnd,lpFunc,lParam);
	CleanupDialog((LPBYTE)lpdd);

	if(UnlockResource(hTemplate) == 0)
		FreeResource(hTemplate);

	if (!hDlg)
	    return -1;

	if (hWnd)
	    EnableWindow(hWnd,FALSE);

	return InternalDialog(hDlg);
}

#ifdef TWIN32
int WINAPI
DialogBoxIndirect(HINSTANCE hInst, LPDLGTEMPLATE lpdt,
		  HWND hWnd, DLGPROC lpDlgFunc)
{
	return DialogBoxIndirectParam(hInst, lpdt, hWnd, lpDlgFunc, 0L);
}
#else
int WINAPI
DialogBoxIndirect(HINSTANCE hInst, HGLOBAL hDialog, HWND hWnd, 
			DLGPROC lpDlgFunc)
{
	return DialogBoxIndirectParam(hInst, hDialog, hWnd, lpDlgFunc, 0L);
}
#endif

#ifdef TWIN32
int WINAPI
DialogBoxIndirectParam(HINSTANCE hInst, LPDLGTEMPLATE lpDlg,
		       HWND hWnd, DLGPROC lpFunc, LPARAM lParam)
{
	HWND 	hDlg;

	/* modal dialogs are always visible */
	lpDlg->style |= WS_VISIBLE;

	hDlg = CreateDialogIndirectParam(hInst,(LPCSTR)lpDlg,
					hWnd,lpFunc,lParam);

	if (!hDlg)
	    return -1;

	if (hWnd)
	    EnableWindow(hWnd,FALSE);

	return InternalDialog(hDlg);
}
#else
int WINAPI
DialogBoxIndirectParam(HINSTANCE hInst, HGLOBAL hDialog, HWND hWnd,
			DLGPROC lpFunc, LPARAM lParam)
{
	HWND 	hDlg;
	LPDLGTEMPLATE lpDialog;
	
	lpDialog= (LPDLGTEMPLATE) LockResource(hDialog);

	/* modal dialogs are always visible */
	lpDialog->style |= WS_VISIBLE;

	hDlg = CreateDialogIndirectParam(hInst,(LPCSTR)lpDialog,
					hWnd,lpFunc,lParam);

	if (!hDlg)
	    return -1;

	if (hWnd)
	    EnableWindow(hWnd,FALSE);
	    
	UnlockResource(hDialog);

	return InternalDialog(hDlg);
}
#endif

/*******************************************************************

	CreateDialog Family

*******************************************************************/

HWND WINAPI
CreateDialog(HINSTANCE hInst, LPCSTR lpDialog, HWND hWnd, DLGPROC lpFunc)
{
	return CreateDialogParam(hInst,lpDialog,hWnd,lpFunc,0L);
}

/*
 *	this routine takes a pointer to a dialog type resource
 *	handle1 = find resource(string)
 *	handle2 = load resource(handle1)
 *	pointer =      lock resource(handle2)
 *		            uses resource(pointer)
 *		       unlock resource(handle2)
 *		  free resource (handle2)
 *
 */

HWND WINAPI
CreateDialogParam(HINSTANCE hInst, LPCSTR lpDlg, HWND hWnd,
		  DLGPROC lpFunc, LPARAM lP)
{
	HRSRC	hDialog;
	HGLOBAL hTemplate;
	LPDLGTEMPLATE	lpTemplate;
	HWND    hDlg;
	DIALOGDATA *lpdd;

	hDialog = FindResource(hInst, lpDlg, RT_DIALOG);

	if(hDialog == 0)
		return 0;

	hTemplate = LoadResource(hInst, hDialog);

	lpTemplate = (LPDLGTEMPLATE)LockResource(hTemplate);
	if(lpTemplate == 0) {
		return 0;
	}

	lpdd = (DIALOGDATA *)ConvertDlgTemplate(hInst, lpTemplate);
	hDlg = InternalCreateDialog(hInst,(LPSTR)lpdd,hWnd,lpFunc,lP);
	CleanupDialog((LPBYTE)lpdd);

	if(UnlockResource(hTemplate) == 0)
		FreeResource(hTemplate);

	return hDlg;
}

HWND WINAPI
CreateDialogIndirect(HINSTANCE hInst, const VOID *lpDlg,
			HWND hWnd, DLGPROC lpFunc)
{
	return CreateDialogIndirectParam(hInst,lpDlg,hWnd,lpFunc,0L);
}

/*
 * this takes a pointer to a memory template
 * converts to c structures, and calls InternalCreateDialog
 */

HWND WINAPI
CreateDialogIndirectParam(HINSTANCE hInst, const VOID *lpDlg, HWND hWnd,
			  DLGPROC lpFunc, LPARAM lParam)
{
	HWND	hDlg;
	DIALOGDATA *lpdd;
	/* do all the work... */
    	APISTR((LF_APICALL,
	 "CreateDialogIndirectParam(HINSTANCE=%x,VOID *=%p,HWND=%x,DLGPROC=%p,LPARAM=%x)\n",
	 hInst,lpDlg,hWnd,lpFunc,lParam));
	
	lpdd = (DIALOGDATA *)ConvertDlgTemplate(hInst, (LPDLGTEMPLATE)lpDlg);

	hDlg = InternalCreateDialog(hInst,(LPSTR)lpdd,hWnd,lpFunc,lParam);

	CleanupDialog((LPBYTE)lpdd);

    	APISTR((LF_APIRET,"CreateDialogIndirectParam: return HWND %x\n",hDlg));
	return hDlg;
}

HWND 
InternalCreateDialogIndirectParam(HINSTANCE hInst, const VOID *lpDlg, HWND hWnd,
                          DLGPROC lpFunc, LPARAM lParam)
{
        HWND    hDlg;
        
        hDlg = InternalCreateDialog(hInst,(LPSTR)lpDlg,hWnd,lpFunc,lParam);

        return hDlg;
}

/*******************************************************************/

static DWORD
GetDialogFontUnits(HFONT hFont)
{
    HDC hDC;
    TEXTMETRIC TextMetrics;
    HWND hWndRoot;
    HFONT last;
    int width,height;
    static DWORD dwFontUnits;

    hWndRoot = GetDesktopWindow();

    hDC = GetDC(hWndRoot);

    if(hFont)
    	last = SelectObject(hDC,hFont);
    else 
	last = 0;

    GetTextMetrics(hDC,&TextMetrics);

    if(last)
    	SelectObject(hDC,last);

    ReleaseDC(hWndRoot,hDC);
    width = TextMetrics.tmAveCharWidth;
    height = TextMetrics.tmHeight;
    dwFontUnits = MAKELONG(width,height);


    return dwFontUnits;
}

void    WINAPI 
MapDialogRect(HWND hDlg, RECT FAR *lpRect)
{
    HDC hDC;
    HFONT hFont, hFontOld = 0;
    TEXTMETRIC TextMetrics;
    DWORD dwFontUnits;
    int width, height;

    hDC = GetDC(hDlg);
    hFont = (HFONT)SendMessage(hDlg, WM_GETFONT, 0, 0);
    if ( hFont )
	hFontOld = SelectObject(hDC, hFont);

    if ( GetTextMetrics(hDC, &TextMetrics) ) {
	width  = TextMetrics.tmAveCharWidth;
	height = TextMetrics.tmHeight;
    } else {
	dwFontUnits = GetDialogBaseUnits();
	width  = (int)LOWORD(dwFontUnits);
	height = (int)HIWORD(dwFontUnits);
    }

    if ( hFontOld )
	SelectObject(hDC, hFontOld);
    ReleaseDC(hDlg,hDC);

    lpRect->left    = MulDiv(lpRect->left,   width,  4);
    lpRect->top     = MulDiv(lpRect->top,    height, 8);
    lpRect->right   = MulDiv(lpRect->right,  width,  4);
    lpRect->bottom  = MulDiv(lpRect->bottom, height, 8);
}

DWORD WINAPI
GetDialogBaseUnits(void)
{
	return GetDialogFontUnits(0);
}

HWND
InternalCreateDialog(HINSTANCE hInst, LPSTR lpDlg,
			HWND hWnd, DLGPROC lpFunc, LPARAM lParam)
{
	DIALOGDATA *dp = (DIALOGDATA *) lpDlg;
	CONTROLDATA *cp;
	HWND  	hDlg;
	int     i;
	HWND    hCtl;
	HWND    hFirst = 0,hFirstButton;
	int	X,Y;
	HFONT	hFont = (HFONT)0;
	HFONT	ctlFont;
	DWORD	dwBaseUnits;
	DWORD	dwRet;
	WORD	wDialogWidth, wDialogHeight;
	WORD	wDlgCode;
	int	nWidth, nHeight;
	int	nXBorder = 0, nYBorder = 0;
	int	cyCaption;
	LOGFONT LogFont;
	BOOL	bVisible;
	DWORD	dwStyle;
#if 0
	HDC hDC;
	HCURSOR	hCursOld, hCursHourGlass;
#endif

	dwBaseUnits = GetDialogFontUnits(hFont);
	wDialogWidth  = LOWORD(dwBaseUnits);
	wDialogHeight = HIWORD(dwBaseUnits);

	dwStyle = dp->lStyle;
	/* force WS_POPUP style for top-level dialogs */
	if (!(dwStyle & WS_CHILD))
	    dwStyle |= WS_POPUP;

	if (dwStyle & DS_SETFONT  &&  
	    dp->szFaceName != NULL  && strlen(dp->szFaceName)) {

		memset((LPVOID)&LogFont,0,sizeof(LOGFONT));

		/* dp->wPointSize is defined as the typographical point size,*/
		/*  it must be converted to the logical font size.  */
#if 0
		LogFont.lfHeight = -1 * MulDiv(dp->wPointSize,
			GetDeviceCaps (hDC, LOGPIXELSY), 72);
#else
		/* dp->wPointSize is NOT a typographical point size.  It is
		 * the font height in dialog base units.  It was misnamed.
		 */
		LogFont.lfHeight = (dp->wPointSize * wDialogHeight / 8);
#endif
		LogFont.lfWeight = 700;
		strcpy(LogFont.lfFaceName,dp->szFaceName);

		hFont = CreateFontIndirect(&LogFont);
	}

	/* NOTE: 
	 *	 the coordinates here are from the resource file
	 * 	 we should not change them here, but in Windows.c.
 	 * 	 this was converted from a straight muliply then
 	 * 	 divide, to MulDiv, this will round up both the x,y
	 *	 and the width.
 	 */
	X       = MulDiv(dp->x,wDialogWidth,4);
	Y       = MulDiv(dp->y,wDialogHeight,8);
	nWidth  = MulDiv(dp->cx,wDialogWidth,4);
	nHeight = MulDiv(dp->cy,wDialogHeight,8);


	/* final window size and coordinates should be adjusted by */
	/* border and caption size depending on the style	   */

	if (dwStyle & WS_DLGFRAME) {
	    nXBorder = GetSystemMetrics(SM_CXDLGFRAME);
	    nYBorder = GetSystemMetrics(SM_CYDLGFRAME);
	}
	else if (dwStyle & WS_BORDER) {
	    nXBorder = GetSystemMetrics(SM_CXBORDER);
	    nYBorder = GetSystemMetrics(SM_CYBORDER);
	}
	if (dwStyle & WS_THICKFRAME) {
	    nXBorder = GetSystemMetrics(SM_CXFRAME);
	    nYBorder = GetSystemMetrics(SM_CYFRAME);
	}
	X -= nXBorder;
	Y -= nYBorder;
	nWidth += 2*nXBorder;
	nHeight += 2*nYBorder;
	if ((dwStyle & WS_CAPTION) == WS_CAPTION) {
	    cyCaption = GetSystemMetrics(SM_CYCAPTION);
	    Y -= cyCaption;
	    nHeight += cyCaption;
	}

	if (dwStyle & WS_VISIBLE) {
	    bVisible = TRUE;
	    dwStyle &= ~WS_VISIBLE;
	}
	else
	    bVisible = FALSE;
	
	hDlg = CreateWindow(
			(!HIWORD(dp->szClassName) || !strlen(dp->szClassName))?
				TWIN_DIALOGCLASS:dp->szClassName,
			(HIWORD(dp->szCaption) && strlen(dp->szCaption))?
				dp->szCaption:" ",
			dwStyle,
			X,Y,nWidth,nHeight,
			hWnd,
			(HMENU)0,
			(hInst)?hInst:
				(hWnd?GetWindowInstance(hWnd):0),
			NULL
		);
    
	if(hDlg == 0) {
		return 0;
	}

	/*
	 * The following code was removed because it needlessly leaves
	 * an hour glass on all of the applications windows while the dialog
	 * box is up.  Another call to SetCursor() was also commented out
	 * at the end of this function.
	 */
#if 0
	hCursHourGlass = LoadCursor((HINSTANCE)0, IDC_WAIT);
	hCursOld       = SetCursor(hCursHourGlass);
#endif

	SetWF(hDlg,WFDIALOGWINDOW);
	SetWindowWord(hDlg,DWW_STATUS,0);
	SetWindowWord(hDlg,DWW_PARENT,hWnd);
	SetWindowLong(hDlg,DWL_DLGPROC,(LONG)lpFunc);
	SetWindowLong(hDlg,DWL_LPARAM,lParam);
	SetWindowWord(hDlg,DWW_DEFID,(WORD)0L);
	if (hFont)
	    SendMessage(hDlg, WM_SETFONT, (WPARAM)hFont,
			     (LPARAM)MAKELONG(FALSE,0));

	for(i=0,cp=dp->controlinfo;i<(int)(dp->bNumberOfItems);i++,cp++) {

		hCtl = CreateWindow(
			cp->szClass?cp->szClass:"STATIC",
			((!cp->szClass || !lstrcmpi(cp->szClass,"STATIC"))
			 && ((cp->lStyle & 0xf) == SS_ICON))?
			NULL:cp->szText,
			cp->lStyle,
			(int)(cp->x*wDialogWidth)/4,
			(int)(cp->y*wDialogHeight)/8,
			(int)(cp->cx*wDialogWidth)/4,
			(int)(cp->cy*wDialogHeight)/8,
			hDlg,
			cp->wID,
			hWnd?GetWindowInstance(hWnd):hInst,
			NULL
		);
		if(!hFirst && (cp->lStyle & WS_TABSTOP))
			hFirst = hCtl;

		if ((!cp->szClass || !lstrcmpi(cp->szClass,"STATIC")) &&
			 ((cp->lStyle & 0xf) == SS_ICON)) {
		    /* icon for the SS_ICON control comes from the same place,
			as dialog template, note, only send message if we
			actually have one */
		    if(cp->szText)
		    	SendMessage(hCtl,
			    STM_SETICON,
			    (WPARAM)LoadIcon(hInst,cp->szText),
			    (LPARAM)0);
		}

		/* if a dialog class font was specified, and the control */
		/* has not done a WM_SETFONT, then set this font.        */
		if(hFont) {
			ctlFont = (HFONT)SendMessage(hCtl, WM_GETFONT, 0, 0);
			if(ctlFont == 0)
				SendMessage( hCtl, WM_SETFONT, (WPARAM)hFont,
				     (LPARAM)MAKELONG(FALSE,0) );
		}
	}

	dwRet =  SendMessage(hDlg,WM_INITDIALOG,(WPARAM)hFirst,lParam);

	for (hCtl = GetWindow(hDlg,GW_CHILD),hFirstButton = 0; hCtl;
	     hCtl = GetWindow(hCtl,GW_HWNDNEXTSIB)) {
	    wDlgCode = (WORD)SendMessage(hCtl,WM_GETDLGCODE,0,0L);
	    if (wDlgCode & DLGC_DEFPUSHBUTTON)
		break;
	    if ((wDlgCode & DLGC_UNDEFPUSHBUTTON) && (hFirstButton == 0))
		hFirstButton = hCtl;
	}
	if (hCtl != 0)
	    SetWindowWord(hDlg,DWW_DEFID,GetWindowID(hCtl));
	else
	    if (hFirstButton != 0)
		SetWindowWord(hDlg,DWW_DEFID,GetWindowID(hFirstButton));
	    else
		SetWindowWord(hDlg,DWW_DEFID,0);

	if (dwRet) {
	    	DlgSetFocus(hFirst);
	    
		if ((WORD)SendMessage(hFirst,WM_GETDLGCODE,0,0L) &
		    DLGC_HASSETSEL)
			Edit_SetSel(hFirst, 0, -1);
	}
	
	hCtl = GetFocus();

	if(hCtl == 0) {
		hCtl = GetFirstDlgTabItem(hDlg);
		if(hCtl) {
			DlgSetFocus(hCtl);

			if ((WORD)SendMessage(hCtl,WM_GETDLGCODE,0,0L) &
			    DLGC_HASSETSEL)
				Edit_SetSel(hCtl, 0, -1);
		}
	}

	if (bVisible) {
		ShowWindow(hDlg,SW_SHOWNORMAL);
	}

	if (bVisible) {
	    	UpdateWindow(hDlg);
	}

#ifdef CURSORWARPING
	/* this will warp the cursor to the default button */
	/* it only should be activated by a user preference */
	/* and not automatically */
        {
               HWND hWnd;
               int     width ,height;
               RECT rcDlg;
 
               hWnd = hCtl;
               GetWindowRect(hWnd,&rcDlg);
               width = (rcDlg.left + rcDlg.right)/2;
               height = (rcDlg.bottom + rcDlg.top)/2;
               SetCursorPos(width,height);
	}
#endif

	/* SetCursor(hCursOld); */
	return hDlg;
}


/*******************************************************************

	Dialog Control Functions 

*******************************************************************/

void
CheckDefPushButton(HWND hDlg, HWND hWndCurrent, HWND hWndNextItem)
{
    WORD wDlgCode;
    UINT wDefID;
    DWORD dwDefID;
    DWORD dwStyle;
    HWND hWndTemp, hDefItem;

    wDlgCode = (hWndNextItem)?
	(WORD)SendMessage(hWndNextItem,WM_GETDLGCODE,0,0L):0;

    if (hWndNextItem == hWndCurrent) {
	if (wDlgCode & DLGC_UNDEFPUSHBUTTON)
	    SendMessage(hWndCurrent,BM_SETSTYLE,
		(WPARAM)BS_DEFPUSHBUTTON,MAKELPARAM(TRUE,0));
    }
    else {
	if ((hWndCurrent && ((WORD)SendMessage(hWndCurrent,WM_GETDLGCODE,0,0L) &
			(DLGC_UNDEFPUSHBUTTON|DLGC_DEFPUSHBUTTON))) ||
	    (hWndNextItem && (wDlgCode &
			(DLGC_UNDEFPUSHBUTTON|DLGC_DEFPUSHBUTTON)))) {
	/* we have to find an ancestor of hWndNextItem whose parent is hDlg */
	    if (wDlgCode & DLGC_DEFPUSHBUTTON)
		SendMessage(hWndNextItem,BM_SETSTYLE,
			(WPARAM)BS_PUSHBUTTON,MAKELPARAM(TRUE,0));
	    for (hWndTemp = GetNextDlgItem(hDlg,hWndNextItem);
		 hWndTemp != hWndNextItem;
		 hWndTemp = GetNextDlgItem(hDlg,hWndTemp))
	        if ((WORD)SendMessage(hWndTemp,WM_GETDLGCODE,0,0L) &
			 DLGC_DEFPUSHBUTTON)
		    SendMessage(hWndTemp,BM_SETSTYLE,
				(WPARAM)BS_PUSHBUTTON,MAKELPARAM(TRUE,0));
	}
	if (wDlgCode & DLGC_UNDEFPUSHBUTTON)
	    SendMessage(hWndNextItem,BM_SETSTYLE,
			(WPARAM)BS_DEFPUSHBUTTON,MAKELPARAM(TRUE,0));
	else {
	    dwDefID = SendMessage(hDlg,DM_GETDEFID,0,0L);
	    if (HIWORD(dwDefID) != DC_HASDEFID)
		wDefID = IDOK;
	    else 
		wDefID = LOWORD(dwDefID);
	    if ((hDefItem = GetDlgItem(hDlg,wDefID))) {
		wDlgCode = (WORD)SendMessage(hDefItem,WM_GETDLGCODE,0,0L);
		dwStyle = GetWindowLong(hDefItem,GWL_STYLE);
		if (!(wDlgCode & DLGC_DEFPUSHBUTTON) &&
		    (wDlgCode & DLGC_UNDEFPUSHBUTTON) &&
		    !(dwStyle & WS_DISABLED))
		    SendMessage(hDefItem,BM_SETSTYLE,
			(WPARAM)BS_DEFPUSHBUTTON,MAKELPARAM(TRUE,0));
	    }
	}
    }
}

void WINAPI
CheckDlgButton(HWND hDlg, int nIDButton, UINT wCheck)
{
	HWND hWnd;
	if ((hWnd = GetDlgItem(hDlg,nIDButton))) {
		SendMessage(hWnd,BM_SETCHECK,wCheck,0);
	}
}

void WINAPI
CheckRadioButton(HWND hDlg, int nIDFirst,int nIDLast,int nIDCheckButton)
{
	HWND	hWndCheck,hWndTemp;
	WORD wID;
	DWORD dwStyle;
	ATOM atmClassName;

	if (!(hWndCheck = GetDlgItem(hDlg,nIDCheckButton)))
	    return;
	hWndTemp = GetNextDlgGroupItem(hDlg,hWndCheck,FALSE);
	while(hWndTemp && (hWndCheck != hWndTemp)) {
	    wID = GetWindowID(hWndTemp);
	    dwStyle = GetWindowLong(hWndTemp,GWL_STYLE);
	    atmClassName = GetClassWord(hWndTemp,GCW_ATOM);
	    if ((wID >= (WORD)nIDFirst) && (wID <= (WORD)nIDLast) &&
		(atmClassName == atmGlobalLookup[LOOKUP_BUTTON]) &&
		((LOWORD(dwStyle) == BS_RADIOBUTTON) ||
		 (LOWORD(dwStyle) == BS_AUTORADIOBUTTON)))
		SendMessage(hWndTemp,BM_SETCHECK,FALSE,0);
	    hWndTemp = GetNextDlgGroupItem(hDlg,hWndTemp,FALSE);
	}
	SendMessage(hWndCheck,BM_SETCHECK,TRUE,0);
}

UINT WINAPI
IsDlgButtonChecked(HWND hDlg, int nID)
{
	HWND hWnd;
	if((hWnd = GetDlgItem(hDlg,nID)) == 0)
		return 0;
	return SendMessage(hWnd,BM_GETCHECK,0,0);
}

/***************************************************************************\
*
*	Dialog Communication Routines
*
\***************************************************************************/

HWND WINAPI
GetDlgItem(HWND hDlg, int nIDDlgItem)
{
	HWND	hWnd;

	APISTR((LF_APICALL, "GetDlgItem(%x,%x)\n",hDlg,nIDDlgItem));

	for( hWnd = GetWindow(hDlg,GW_CHILD);hWnd && IsWindow(hWnd);
	     hWnd = GetWindow(hWnd,GW_HWNDNEXTSIB)) {
 	     if ((short)GetWindowID(hWnd) == (short)nIDDlgItem)
 			break;
 	}

	APISTR((LF_APIRET, "GetDlgItem: returns HWND %x\n",hWnd));
	return hWnd;
}

int WINAPI
GetDlgCtrlID(HWND hWnd)
{
    return TWIN_GetWindowID(hWnd);
}

#define ASCTOI(ch) (ch - '0')

UINT WINAPI
GetDlgItemInt(HWND hDlg, int nID, BOOL *lpTranslated, BOOL bSigned)
{
	unsigned int usVal;
	int          sVal, nLen,i;
	char         lbuf[256];

	nLen = GetDlgItemText(hDlg,nID,(LPSTR) lbuf,256);
	if(nLen == 0) {
		if(lpTranslated) *lpTranslated = FALSE;
		return 0;
	}

	if (bSigned) {
	    i = sscanf(lbuf,"%d",&sVal);
	    if (i == 0 || sVal > 32767) {
		sVal = 0;
		if(lpTranslated) *lpTranslated = FALSE;
	    }
	    else
		if(lpTranslated) *lpTranslated = TRUE;
	    return (UINT)sVal;
	}
	else {
	    i = sscanf(lbuf,"%u",&usVal);
	    if (i == 0 || usVal >= 65535) {
		usVal = 0;
		if(lpTranslated) *lpTranslated = FALSE;
	    }
	    else
		if(lpTranslated) *lpTranslated = TRUE;
	    return usVal;
	}
}

int WINAPI
GetDlgItemText(HWND hDlg, int nID, LPSTR lpStr, int nMax)
{
	HWND hWnd;
	int  nlen;

	if(!(hWnd = GetDlgItem(hDlg,nID)))
		return 0;
	nlen = SendMessage(hWnd,WM_GETTEXTLENGTH,0,0);
	if(nlen > 0) {
		nlen = SendMessage(hWnd,WM_GETTEXT,
				min(nMax,nlen+1),(LONG) lpStr);
		if (nlen < nMax)
			lpStr[nlen] = '\0';
	}
	return nlen;
}

static HWND
GetNextDlgItem(HWND hWndDlg, HWND hWndCtrl)
{
    HWND hWndNextItem;

    if (!hWndCtrl || !(hWndNextItem = GetWindow(hWndCtrl,GW_HWNDNEXT)))
	return GetWindow(hWndDlg,GW_CHILD);
    else
	return hWndNextItem;
}

HWND WINAPI
GetNextDlgGroupItem(HWND hWndDlg, HWND hWndCtrl, BOOL fPrevious)
{
    if (!IsChild(hWndDlg,hWndCtrl))
	return (HWND)0;
    return GetWindow(hWndCtrl,(fPrevious)?GW_HWNDPREVGROUP:GW_HWNDNEXTGROUP);
}

static HWND
GetNextDlgControl(HWND hDlg,HWND hWnd,HWND hWndMsg)
{
	HWND hWndTemp;

	hWndTemp = GetWindow(hWnd,GW_HWNDNEXTSIB);
	if(hWndTemp == 0)
		hWndTemp = GetWindow(hDlg,GW_CHILD);
	if(hWndTemp == hWndMsg)
		return 0;
	return hWndTemp;
}

static HWND
GetFirstDlgTabItem(HWND hWndDlg)
{
    HWND hWndTemp;

    for (hWndTemp = GetWindow(hWndDlg,GW_CHILD);
	 hWndTemp;
	 hWndTemp = GetWindow(hWndTemp,GW_HWNDNEXTSIB))
	if (GetWindowLong(hWndTemp,GWL_STYLE) & WS_TABSTOP)
	    return hWndTemp;
    return (HWND)0;
}

HWND WINAPI
GetNextDlgTabItem(HWND hWndDlg, HWND hWndCtrl, BOOL fPrevious)
{
    HWND hWndTemp;

    if (!IsChild(hWndDlg,hWndCtrl))
	return (HWND)0;

    for (hWndTemp = GetWindow(hWndCtrl,
		(fPrevious)?GW_HWNDPREVSIB:GW_HWNDNEXTSIB);
	 hWndTemp;
	 hWndTemp = GetWindow(hWndTemp,
		(fPrevious)?GW_HWNDPREVSIB:GW_HWNDNEXTSIB))

	if (GetWindowLong(hWndTemp,GWL_STYLE) & WS_TABSTOP)
	    if(IsWindowEnabled(hWndTemp))
	    	return hWndTemp;
    
    return (HWND) GetFirstDlgTabItem(hWndDlg);
}

LRESULT WINAPI
SendDlgItemMessage(HWND hDlg,int nID,UINT wMsg,WPARAM wParam,LPARAM lParam)
{
	HWND hWnd;
	if((hWnd = GetDlgItem(hDlg,nID)) == 0)
		return 0;
	return SendMessage(hWnd,wMsg,wParam,lParam);
}


void WINAPI
SetDlgItemInt(HWND hDlg,int nID, UINT wParam,BOOL bSigned)
{
	char wBuf[8];
	sprintf(wBuf,"%d",wParam);
	SetDlgItemText(hDlg,nID,wBuf);
}

void WINAPI
SetDlgItemText(HWND hDlg,int nID, LPCSTR lpstr)
{
	HWND hWnd;
	if((hWnd = GetDlgItem(hDlg,nID)) == 0)
		return;
	SendMessage(hWnd, WM_SETTEXT,0,(LONG) lpstr);
}

int
InternalDialog(HWND hDlg)
{
	HWND    hWnd,hWndFocus,hWndCapture;
	int     rc = 0;
	MSG     msg;

	if(hDlg == 0)
		return 0;

	hWnd = GetWindowWord(hDlg,DWW_PARENT);

	if ((hWndCapture = GetCapture()))
	    ReleaseCapture();

	while (IsWindow(hDlg) && (GetWindowWord(hDlg,DWW_STATUS) == 0)) {

	    if(GetMessage(&msg, (HWND)0, 0, 0)) {
		if (msg.message == WM_CLOSE) {
		    DispatchMessage (&msg) ;
		    break;
		}
		if(IsDialogMessage(hDlg,&msg))
		    continue;
		TranslateMessage (&msg) ;
		DispatchMessage (&msg) ;
	    }
	    if (hWnd) {	/* if there is an owner */
		if (!PeekMessage(&msg,hDlg,0,0,PM_NOREMOVE))
		    SendMessage(hWnd,WM_ENTERIDLE,MSGF_DIALOGBOX,(LPARAM)hDlg);
	    }
	}

	if (IsWindow(hDlg)) {
	    rc = GetWindowWord(hDlg,DWW_RESULT);
	    DestroyWindow(hDlg);
	}

	if(hWnd && IsWindow(hWnd)) {
	    EnableWindow(hWnd, TRUE);
	    if ((hWndFocus = GetDialogFocus(hWnd)))
		SetFocus(hWndFocus);
	}

	if (hWndCapture)
	    SetCapture(hWndCapture);

	return rc;
}

#ifdef TWIN32
BOOL WINAPI
#else
void WINAPI
#endif
EndDialog(HWND hDlg,int Result)
{
	ShowWindow(hDlg,SW_HIDE);
	SetWindowWord(hDlg,DWW_STATUS,1);	
	SetWindowWord(hDlg,DWW_RESULT,Result);	
#ifdef TWIN32
	return TRUE;
#endif
}

static HWND
SaveDlgFocus(HWND hDlg)
{
    HWND hWnd;

    hWnd = GetFocus();	

    if (hWnd && IsChild(hDlg,hWnd))
	SetDialogFocus(hDlg,hWnd);
    else
	SetDialogFocus(hDlg,0);

    return hWnd;
}

static HWND
RestoreDlgFocus(HWND hDlg)
{
	HWND	hWnd, hWndFocus;
	
	hWnd = GetDialogFocus(hDlg);
	hWndFocus = GetFocus();
	if(hWnd && (hWnd != hWndFocus))
		SetFocus(hWnd);
	return hWnd;
}

static HWND
DlgSetFocus(HWND hWnd)
{
	HWND	hDlg;
	HWND	hChild;

	hDlg = GetParent(hWnd);	
	hChild = 0;
	if(hDlg) {
		hChild = GetDialogFocus(hDlg);
		SetDialogFocus(hDlg,hWnd);
		SetFocus(hWnd);
	}
	return hChild;
}

/***********************************************************************
	This is the internal dialog box routine used as part of the
	DIALOG window class.  It calls the users supplied routine as
	necessary.
************************************************************************/

static WNDPROC lpDialogBinToNat = 0;

LRESULT WINAPI
DefDlgProc(HWND hDlg, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	DLGPROC	f;
	LRESULT	rc;
	HBRUSH	hBrush;
	HFONT   hFont;
	RECT rcClient;
	HWND hWndChild, hWndFocus;
	DWORD dwStyle;
	WORD wTmp;
	HCLASS32 hDialog32;

    	APISTR((LF_APICALL,"DefDlgProc(HWND=%x,UINT=%x,WPARAM=%x,LPARAM=%lx)\n",
		hDlg,iMessage,wParam,lParam));

	if (!IsWindow(hDlg)) {
    	    APISTR((LF_APIFAIL,"DefDlgProc: returns LRESULT 0\n"));
	    return 0L;
        }

	if (iMessage == WM_CONVERT) {
	    if (!lpDialogBinToNat) {
		hDialog32 = FindClass(TWIN_DIALOGCLASS,(HINSTANCE)0);
		lpDialogBinToNat = (WNDPROC)GetClassHandleLong(
			hDialog32,GCL_BINTONAT);
	    }
	    if (lpDialogBinToNat) {
		rc = lpDialogBinToNat(hDlg,iMessage,wParam,lParam);
    	    	APISTR((LF_APIRET,"DefDlgProc: returns LRESULT %x\n",rc));
		return rc;
	    } else {
    	    	APISTR((LF_APIFAIL,"DefDlgProc: returns LRESULT 0\n"));
		return (LRESULT)0;
	    }
	}

	if ( (f = (DLGPROC) GetWindowLong(hDlg,DWL_DLGPROC)) ) {
		rc = CallWindowProc(
#ifdef	STRICT
			(WNDPROC)f,
#else
			(FARPROC)f,
#endif
			hDlg,iMessage,wParam,lParam);
	}
	else  rc = 0L; 

	if (!IsWindow(hDlg)) {	/* dialog has been destroyed in the callback */
    	    APISTR((LF_APIRET,"DefDlgProc: returns LRESULT %x\n",rc));
	    return rc;
	}

	if(LOWORD(rc) == 0) {
	    switch(iMessage) {
		case WM_ERASEBKGND:
			GetClientRect(hDlg, &rcClient);
			hBrush = (HBRUSH)SendMessage(hDlg,
				GET_WM_CTLCOLOR_MSG(CTLCOLOR_DLG),
				GET_WM_CTLCOLOR_MPS(
					(HDC)wParam,hDlg,CTLCOLOR_DLG));
			FillRect((HDC)wParam,&rcClient,hBrush);
    	    		APISTR((LF_APIRET,"DefDlgProc: returns LRESULT 1\n"));
			return (LRESULT)1;

		case WM_SHOWWINDOW:
		    /* if we are hiding, save the focus */
		    if (!wParam)
			SaveDlgFocus(hDlg);
		    rc =  DefWindowProc(hDlg,iMessage,wParam,lParam);
    	    	    APISTR((LF_APIRET,"DefDlgProc: returns LRESULT %x\n",rc));
		    return rc;

		case WM_SYSCOMMAND:
		    if ((wParam & 0xfff0) == SC_MINIMIZE)
			SaveDlgFocus(hDlg);

		    rc = DefWindowProc(hDlg,iMessage,wParam,lParam);
    	    	    APISTR((LF_APIRET,"DefDlgProc: returns LRESULT %x\n",rc));
		    return rc;

		case WM_ACTIVATE:
		    if GET_WM_ACTIVATE_STATE(wParam,lParam)
			RestoreDlgFocus(hDlg);
		    else
			SaveDlgFocus(hDlg);
		    break;

		case WM_SETFOCUS:
		    if (!RestoreDlgFocus(hDlg))
			DlgSetFocus(GetFirstDlgTabItem(hDlg));
		    break;

		case WM_CLOSE:
		    hWndChild = GetDlgItem(hDlg, IDCANCEL);
		    if (hWndChild) {
			dwStyle = GetWindowLong(hDlg,GWL_STYLE);
			if (dwStyle & WS_DISABLED) {
			    MessageBeep(0);
			} else
			    PostMessage(hDlg,WM_COMMAND,
				GET_WM_COMMAND_MPS(IDCANCEL,
					hWndChild,BN_CLICKED));
		    }
		    break;

		case WM_NCDESTROY:
		    SetWindowWord(hDlg,DWW_STATUS,1);	
		    if ((hFont = (HFONT)GetWindowWord(hDlg, DWW_HFONT))) {
			DeleteObject(hFont);
			SetWindowWord(hDlg,DWW_HFONT,0);
		    }
		    DefWindowProc(hDlg,iMessage,wParam,lParam);
		    break;
		    
  		case DM_SETDEFID:
	  	    SetWindowWord(hDlg,DWW_DEFID,wParam);
    	    	    APISTR((LF_APIRET,"DefDlgProc: returns LRESULT 1\n"));
		    return (LRESULT)1;

		case DM_GETDEFID:
		    wTmp = GetWindowWord(hDlg,DWW_DEFID);
		    if (wTmp) {
			rc = MAKELRESULT(wTmp, DC_HASDEFID);
    	    	    	APISTR((LF_APIRET,"DefDlgProc: returns LRESULT %x\n",rc));
		      	return rc;
		    } else {
    	    	    	APISTR((LF_APIRET,"DefDlgProc: returns LRESULT 0\n"));
			return (LRESULT)0;
		    }

		case WM_NEXTDLGCTL:
		    hWndFocus = GetFocus();
		    if (LOWORD(lParam)) {
			if (!hWndFocus)
			    hWndFocus = hDlg;
			hWndChild = (HWND)wParam;
		    }
		    else {
			if (!hWndFocus) {
			    /* set to the first tab item */
			    hWndChild = GetFirstDlgTabItem(hDlg);
			    hWndFocus = hDlg;
			}
			else {
			    if (!IsChild(hDlg,hWndFocus))
				return (LRESULT)1;
			    hWndChild = GetNextDlgTabItem(hDlg,
				hWndFocus,(BOOL)wParam);
			}
		    }

		    DlgSetFocus(hWndChild);
		    CheckDefPushButton(hDlg,hWndFocus,hWndChild);

    	    	    APISTR((LF_APIRET,"DefDlgProc: returns LRESULT 1\n"));
		    return (LRESULT)1;

		case WM_GETFONT:
		    rc = (LRESULT)GetWindowWord(hDlg,DWW_HFONT);
    	    	    APISTR((LF_APIRET,"DefDlgProc: returns LRESULT %x\n",rc));
		    return rc;

		case WM_SETFONT:
		    SetWindowWord(hDlg,DWW_HFONT,(HFONT)wParam);
		    if (LOWORD(lParam))
			InvalidateRect(hDlg,NULL,TRUE);
    	    	    APISTR((LF_APIRET,"DefDlgProc: returns LRESULT %x\n",0));
		    return 0L;

		case WM_VKEYTOITEM:
		case WM_COMPAREITEM:
		case WM_CHARTOITEM:
		case WM_INITDIALOG:
		    break;

                case WM_MOUSEACTIVATE:
		    
	            rc = (LRESULT)MA_ACTIVATE;
                    if ((int)(short)LOWORD(lParam) == HTCAPTION)
		      rc = (LRESULT)MA_NOACTIVATE;
    	    	    APISTR((LF_APIRET,"DefDlgProc: returns LRESULT %x\n",rc));
		    return rc;

#ifdef	TWIN32
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSTATIC:
		    rc =  GetStockObject(LTGRAY_BRUSH);
    	    	    APISTR((LF_APIRET,"DefDlgProc: returns LRESULT %x\n",rc));
		    return rc;
#endif
		default:
		    rc = DefWindowProc(hDlg,iMessage,wParam,lParam);
    	    	    APISTR((LF_APIRET,"DefDlgProc: returns LRESULT %x\n",rc));
		    return rc;
	    }
	}
	else {
	    switch (iMessage) {
#ifndef	TWIN32
		case WM_CTLCOLOR:
#else
		case WM_CTLCOLORMSGBOX:
		case WM_CTLCOLOREDIT:
		case WM_CTLCOLORLISTBOX:
		case WM_CTLCOLORBTN:
		case WM_CTLCOLORDLG:
		case WM_CTLCOLORSTATIC:
		case WM_CTLCOLORSCROLLBAR:
#endif
		case WM_COMPAREITEM:
		case WM_VKEYTOITEM:
		case WM_CHARTOITEM:
		case WM_QUERYDRAGICON:
		case WM_INITDIALOG:
    	    	    APISTR((LF_APIRET,"DefDlgProc: returns LRESULT %x\n",rc));
		    return (LRESULT)rc;
		default:
		    rc = GetWindowLong(hDlg,DWL_MSGRESULT);
	    }
	}

    	APISTR((LF_APIRET,"DefDlgProc: returns LRESULT %x\n",rc));
	return rc;
}

BOOL WINAPI
IsDialogMessage(HWND hDlg, LPMSG lpmsg)
{
    HWND hWndFocus,hDefItem,hWndMsg,hWndNextItem,hNext;
    WORD wDlgCode;
    BOOL bPrevious = FALSE;
    WORD wDefID;
    WORD wButtonState;
    DWORD dwDefID;
    DWORD dwStyle;
    char buffer[128];
    int  len;

    if (!IsWindow(hDlg))
	return FALSE;

    if (CallMsgFilter(lpmsg,MSGF_DIALOGBOX))
	return TRUE;

    if (!(hWndMsg = lpmsg->hwnd))
	return FALSE;
    if (!((hDlg == hWndMsg) || IsChild(hDlg,hWndMsg)))
	return FALSE;

    wDefID = IDCANCEL;

    wDlgCode = (WORD)SendMessage(hWndMsg,WM_GETDLGCODE,0,0L);
    if (wDlgCode & DLGC_WANTMESSAGE) {
		/* printf("dialog wants all messages\n"); */
    } else {
	
    switch(lpmsg->message) {
	case WM_KEYDOWN: 	{
	    if (wDlgCode & DLGC_WANTALLKEYS) {
		break;
	    }

	    switch(lpmsg->wParam) {
		case VK_RETURN:
		case VK_EXECUTE:
		    hWndFocus = GetFocus();
		    wDlgCode = (WORD)SendMessage(hWndFocus,WM_GETDLGCODE,0,0L);
		    wDefID = GetWindowID(hWndFocus);
		    if (!(wDlgCode & DLGC_DEFPUSHBUTTON)) {
			dwDefID = SendMessage(hDlg,DM_GETDEFID,0,0L);
			if (HIWORD(dwDefID) != DC_HASDEFID)
			    wDefID = IDOK;
			else 
			    wDefID = LOWORD(dwDefID);
		    }
		    /* fall into escape cancel code... */

		case VK_ESCAPE:
		case VK_CANCEL:
		    hDefItem = GetDlgItem(hDlg,wDefID);
		    if (hDefItem) {
		    	dwStyle = GetWindowLong(hDefItem,GWL_STYLE);
			if (dwStyle & WS_DISABLED) {
				MessageBeep(0);
		    		return 1L;
		        }
	            }	
		    SendMessage(hDlg,WM_COMMAND,
				GET_WM_COMMAND_MPS(wDefID,0,0));
		    return 1L;


		case VK_TAB:
		    if (wDlgCode & DLGC_WANTTAB) {
			break;
		    }

		    hWndNextItem = GetNextDlgTabItem(hDlg,hWndMsg,
			GetKeyState(VK_SHIFT) & 0x8000);
		
		    if ((WORD)SendMessage(hWndNextItem,WM_GETDLGCODE,0,0L) &
					DLGC_HASSETSEL)
			Edit_SetSel(hWndNextItem, 0, -1);

		    DlgSetFocus(hWndNextItem);

		    CheckDefPushButton(hDlg,hWndMsg,hWndNextItem);
		    return 1L;

		case VK_LEFT:
		case VK_UP:
		    bPrevious = TRUE;
		case VK_RIGHT:
		case VK_DOWN:
		    /* this relies on bPrevious set to FALSE by default */
		    if (wDlgCode & DLGC_WANTARROWS) {
			/*printf("dialog wants these arrows\n"); */
			break;
		    }

		    hNext = GetNextDlgGroupItem(hDlg,hWndMsg,bPrevious);
		    wDlgCode = SendMessage(hNext,WM_GETDLGCODE,0,0L);

		wDefID = GetDlgCtrlID(hNext);
		if (wDlgCode & (DLGC_DEFPUSHBUTTON|DLGC_UNDEFPUSHBUTTON))
			; /*--- No Operation ---*/
		else 
		if (wDlgCode & DLGC_RADIOBUTTON)
			SendMessage(hDlg,WM_COMMAND, GET_WM_COMMAND_MPS(wDefID,0,0));
		else 
		if (wDlgCode & DLGC_STATIC)
			; /*--- No Operation ---*/
		else 
		if (wDlgCode & DLGC_BUTTON)
			; /*--- No Operation ---*/
		DlgSetFocus(hNext);
		return 1L;
	    }
	    break;
	}

	case WM_SYSCHAR:
	    if (!GetFocus()) {
		if (!(GetKeyState(VK_MENU) & 0x8000)) {
		    /* printf("syschar...beep\n"); */
		    MessageBeep(0);
		    return 1L;
		}
	    }

	    if (lpmsg->wParam == VK_SPACE) {
		DefWindowProc(hDlg,lpmsg->message,lpmsg->wParam,
		    lpmsg->lParam);
		return 1L;
	    }

	case WM_CHAR:
	    if (hDlg == hWndMsg) {
		return 1L;
	    }
	    
	    wDlgCode = SendMessage(hWndMsg,WM_GETDLGCODE,0,0L);

	    if (lpmsg->message == WM_CHAR &&
		(wDlgCode & (DLGC_WANTCHARS | DLGC_WANTALLKEYS))) {

		buffer[0] = 0;
		len = GetWindowText(hWndMsg,buffer,128);
		/* printf("control[%s] wants char/all keys\n",buffer); */
		break;
	    }

	    if (lpmsg->wParam == VK_SPACE) {
		wDefID = GetWindowID(hWndMsg);

		/* button then send message */
		if (wDlgCode & (DLGC_DEFPUSHBUTTON|DLGC_UNDEFPUSHBUTTON)) {
			SendMessage(hDlg,WM_COMMAND,
				GET_WM_COMMAND_MPS(wDefID,0,0));
		        return 1L;
		} else if(wDlgCode & DLGC_RADIOBUTTON) {
			SendMessage(hDlg,WM_COMMAND,
				GET_WM_COMMAND_MPS(wDefID,0,0));
			return 1L;
		} else if(wDlgCode & DLGC_BUTTON) {
			wButtonState = SendMessage(hWndMsg,BM_GETCHECK,0,0);
			if(wButtonState) 
				wButtonState = 0;
			else	   
				wButtonState = 1;
			dwStyle = GetWindowStyle(hWndMsg) & 0x000f;
			if ( dwStyle == BS_AUTOCHECKBOX )
				SendMessage(hWndMsg,BM_SETCHECK,wButtonState,0);
			else if ( dwStyle == BS_AUTO3STATE )
			{
			    	TWIN_Cycle3StateButton(hWndMsg);
				wButtonState = SendMessage(hWndMsg,BM_GETCHECK,
							   0,0);
			}
			
			SendMessage(hDlg,WM_COMMAND,
				GET_WM_COMMAND_MPS(wDefID,hWndMsg,wButtonState));
			return 1L;
		}
	    }

	    /* we have the dialog box and some window
	     * does it want this character? 	    
	     */
	    wDefID = ControlByName(hDlg,hWndMsg,LOBYTE(lpmsg->wParam));

	    /* if we have syschar and no match then beep? */
	    if (!wDefID) {
		if (lpmsg->message == WM_SYSCHAR) {
		    DefWindowProc(hDlg,lpmsg->message,lpmsg->wParam,
			lpmsg->lParam);
		}
		else {
		    MessageBeep(0);
		}

		/* this case is handled... */
	        return 1L;
	    }

	    hWndMsg = GetDlgItem(hDlg, wDefID);
	    wDlgCode = SendMessage(hWndMsg,WM_GETDLGCODE,0,0L);

	    if (wDlgCode & (DLGC_DEFPUSHBUTTON|DLGC_UNDEFPUSHBUTTON)) {
		SendMessage(hDlg,WM_COMMAND,
			GET_WM_COMMAND_MPS(wDefID,0,0));
	    } else if(wDlgCode & DLGC_RADIOBUTTON) {
		CheckRadioButton(hDlg,0,0xffff, wDefID);
	    } else if(wDlgCode & DLGC_BUTTON) {
		dwStyle = GetWindowStyle(hWndMsg) & 0x000f;
		if ( dwStyle == BS_AUTO3STATE ||
		     dwStyle == BS_3STATE )
		{
		    TWIN_Cycle3StateButton(hWndMsg);
		}
		else
		{
		    wButtonState = SendMessage(hWndMsg,BM_GETCHECK,0,0);
		    if(wButtonState)
			wButtonState = 0;
		    else
			wButtonState = 1;
		    SendMessage(hWndMsg,BM_SETCHECK,wButtonState,0);
		}
	    }

	    /* change focus to this window... */
	    hWndMsg = GetDlgItem(hDlg, wDefID);
	    DlgSetFocus(hWndMsg);
	    return 1L;
       }
    }

    TranslateMessage(lpmsg);
    DispatchMessage(lpmsg);
    return TRUE;
}


static int
ControlByName(HWND hDlg,HWND hWndMsg,WORD wParam)
{
	int wDefID;
	HWND	hWnd;
	char	buffer[80];
	int	i,len;
	char	ch;

	ch = toupper(wParam);
	for(hWnd = hWndMsg;hWnd;hWnd = GetNextDlgControl(hDlg,hWnd,hWndMsg)) {
		wDefID = GetWindowID(hWnd);
		buffer[0] = 0;
		GetDlgItemText(hDlg, wDefID, buffer, 80);

		/* whats the length, and don't do till the end... */
		len = strlen(buffer);	
		len--;
		for(i=0;i<len;i++) {
			if(buffer[i] == '&' && toupper(buffer[i+1]) == ch)
				return wDefID;
		}
	}
	return 0;
}

static LPBYTE
ConvertDlgTemplate(HINSTANCE hInst, LPDLGTEMPLATE lpdt)
{
        DIALOGDATA *lpdd;
        LPBYTE lpcur;
        LPDLGITEMTEMPLATE lpdit;
        CONTROLDATA *lpcd;
        int i,len;
        WORD wNumControls;

        /* Begin to parse DLGTEMPLATE data into DIALOGDATA structure */

        if (!(lpdd = (DIALOGDATA *)WinMalloc(sizeof(DIALOGDATA))))
                return NULL;
        memset((LPSTR)lpdd,'\0',sizeof(DIALOGDATA));

        lpdd->lStyle = lpdt->style;

        lpdd->bNumberOfItems = lpdt->cdit;

        wNumControls = (WORD)lpdd->bNumberOfItems;

        lpdd->x = lpdt->x;

        lpdd->y = lpdt->y;

        lpdd->cx = lpdt->cx;
	
	lpdd->cy = lpdt->cy;
	
        lpcur = (LPBYTE)lpdt + sizeof(DLGTEMPLATE);
        /*  3 variable length strings follow DLGTEMPLATEs...position
            lpcur to the beginning of the first string */

        if (*lpcur)
        /* first array is menu data, if its 0x00 then no menu */
        {
                if (*lpcur == 0xff)
                /*  if lpcur = 0xff, then array contains ordinal value
                    of a menu resource */
                {
                        LoadString(hInst, *((WORD *)(lpcur+1)),
                                        lpdd->szMenuName, 256);
                        lpcur +=3;
                }
                else
                /*  else it is a null terminated unicode string that specifies
                    the name of a menu resource */
                {
                        len = strlen((LPSTR)lpcur)+1;
                        lpdd->szMenuName = WinMalloc(len);
                        strcpy(lpdd->szMenuName,(LPSTR)lpcur);
                        lpcur += len;
                }
        }
        else lpcur++;
	lpcur = (LPBYTE)ALIGN_16BIT(lpcur);

        if (*lpcur)
        /* Second array is a class array that identifies the window class
           of the control, if it is 0x00 then it uses the predefined
           dialog box */
        {
                /* if it is anything else, it is a null terminated unicode
                   string that specifies the name of a registered window
                   class */
                len = strlen((LPSTR)lpcur) + 1;
                lpdd->szClassName = WinMalloc(len);
                strcpy(lpdd->szClassName,(LPSTR)lpcur);
                lpcur += len;
        }
	else lpcur++;
	lpcur = (LPBYTE)ALIGN_16BIT(lpcur);

        if (*lpcur)
        /* Third array is the title array, if it is 0x00 then there is no
           title.  */
        {
                /*  if it is anything else, it is a null terminated unicode
                    string that specifies the title of the dialog */
                len = strlen((LPSTR)lpcur) + 1;
                lpdd->szCaption = WinMalloc(len);
                strcpy(lpdd->szCaption,(LPSTR)lpcur);
                lpcur += len;
        }
        else lpcur++;
	lpcur = (LPBYTE)ALIGN_16BIT(lpcur);

        if (lpdd->lStyle & DS_SETFONT)
        /*  The 16 bit point size value and typeface array follow the title
            array, but only if the style member specifies DS_SETFONT */
        {
                lpdd->wPointSize = *((WORD *)(lpcur));
                lpcur += 2;
                len = strlen((LPSTR)lpcur) + 1;
                lpdd->szFaceName = WinMalloc(len);
                strcpy(lpdd->szFaceName,(LPSTR)lpcur);
                lpcur += len;
        }

        /*  position lpdit to the beginning of the first DLGITEMTEMPLATE 
        lpdit = (LPDLGITEMTEMPLATE)ALIGN_32BIT(lpcur);*/

        if (wNumControls == 0)
        /*  if there are no controls then we're done */
        {
                lpdd->controlinfo = NULL;
                return (LPBYTE)lpdd;
        }

        lpcd = (CONTROLDATA *)WinMalloc(sizeof(CONTROLDATA) * wNumControls);
        if (!lpcd)
                return NULL;
        memset((LPSTR)lpcd,'\0',sizeof(CONTROLDATA) * wNumControls);

        for (i = 0; i < (int)wNumControls; i++)
        {
	        lpdit = (LPDLGITEMTEMPLATE)ALIGN_32BIT(lpcur);

                /* create a CONTROLDATA structure for each DLGITEMTEMPLATE */
                lpcd[i].x = lpdit->x;
                lpcd[i].y = lpdit->y;
                lpcd[i].cx = lpdit->cx;
                lpcd[i].cy = lpdit->cy;
                lpcd[i].wID = lpdit->id;
                lpcd[i].lStyle = lpdit->style;

                lpcur = (LPBYTE)lpdit + sizeof(DLGITEMTEMPLATE);

                /* this is the class array, if *lpcur is 0xffff then look
                   for an ordinal which specifies a predefined system class */
#ifdef TWIN32
                if (*(WORD *)lpcur == 0xffff)
                {
			lpcur += sizeof(WORD);
                        lpcd[i].szClass = (LPSTR)WinStrdup(
				std_controls[*(WORD *)lpcur & ~0x0080]);
			lpcur += sizeof(WORD);
                }
#else
                /* this is the class array, if *lpcur has 0x80 bit set then use
                   it as an ordinal which specifies a predefined system class */
                if (*lpcur & 0x80)
                {
                        lpcd[i].szClass = (LPSTR)WinStrdup(
				std_controls[*lpcur++ & ~0x80]);
                }
#endif
                else
                /* else it is a null-terminated unicode string that
                   specifies the name of a registered window class */
                {
                        len = strlen((LPSTR)lpcur) + 1;
                        lpcd[i].szClass = WinMalloc(len);
                        strcpy(lpcd[i].szClass,(LPSTR)lpcur);
                        lpcur += len+1;
                }
		lpcur = (LPBYTE)ALIGN_16BIT(lpcur);

#ifdef TWIN32
                if (*(WORD *)lpcur == 0xffff)
                /* this is the title array, if *lpcur is 0xffff then next
                   word is an ordinal which specifies resource id. */
                {
			lpcur += sizeof(WORD);
                        lpcd[i].szText = (LPSTR)(LONG) *(WORD *)lpcur;
                        lpcur += sizeof(WORD);
                }
                else
#endif
                /* else it is a null-terminated unicode string that
                   specifies the initial text. */
                {
                        len = strlen((LPSTR)lpcur) + 1;
                        lpcd[i].szText = WinMalloc(len);
                        strcpy(lpcd[i].szText,(LPSTR)lpcur);
                        lpcur += len;
                }
                lpcur = (LPBYTE)ALIGN_32BIT(lpcur);

        /* There needs to be code to handle the creation data array here */
                if (*lpcur)
			lpcur += (int)*lpcur;
		else
			lpcur++;
		lpcur = (LPBYTE)ALIGN_32BIT(lpcur);
        }
        lpdd->controlinfo = lpcd;
        return (LPBYTE)lpdd;
}

int WINAPI
InternalDlgIndirectParam(HINSTANCE hInst, HGLOBAL hTemplate,
                       HWND hWnd, DLGPROC lpFunc, LPARAM lParam)
{
        HWND    hDlg; 
        LPDLGTEMPLATE lpDlg;

        lpDlg = (LPDLGTEMPLATE)GlobalLock(hTemplate);
        
        /* modal dialogs are always visible */
        lpDlg->style |= WS_VISIBLE;

        hDlg = InternalCreateDialogIndirectParam(hInst,(LPCSTR)lpDlg,
                                        hWnd,lpFunc,lParam);

        GlobalUnlock(hTemplate);

        if (!hDlg)
            return -1;

        if (hWnd)
            EnableWindow(hWnd,FALSE);

        return InternalDialog(hDlg);
}
