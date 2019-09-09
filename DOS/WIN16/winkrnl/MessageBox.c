/*    
	MessageBox.c	2.17
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
#include "KrnTask.h"
#include "Messages.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>


static BOOL MessageBoxHandler(HWND, UINT, WPARAM, LPARAM);

static HWND hParentNew;
static MESSAGEBOXDATA mbdNew;

static void MBMain(void)
{
	DialogBoxParam( 0, "MessageBox", hParentNew,
		       (DLGPROC)MessageBoxHandler, (LPARAM)&mbdNew);
	TWIN_ReadyAllTasks();
	DeleteTask(GetCurrentTask());
}

int WINAPI
MessageBox(HWND hWnd, LPCSTR text, LPCSTR caption, UINT flag)
{
	MESSAGEBOXDATA mbd;
	char	msgtext[1024];
	int     rc;

	APISTR((LF_APICALL,"MessageBox(HWND=%x,LPCSTR=%s,LPCSTR=%s,UINT=%x)\n",
		hWnd,text?text:"NULL",caption?caption:"NULL",flag));

	rc = min(1020,(int)strlen(text));
	rc++;

	strncpy(msgtext,text,rc);
	strcat(msgtext," ");

	mbd.text    = (LPSTR)msgtext;

	if(caption)
		mbd.caption = (LPSTR)caption;
	else
		mbd.caption = (LPSTR) "Internal Window Error";
	mbd.flag    = flag;

	if (TWIN_IsSafeToMessageBox()) {
		rc = DialogBoxParam( 0,"MessageBox", hWnd,
			(DLGPROC)MessageBoxHandler, (LPARAM)&mbd);
	}
	else {
		HTASK hMBTask;
		
		hParentNew = hWnd;
		mbdNew = mbd;
		hMBTask = CreateTask();
		TWIN_InitializeAnotherNativeTask(hMBTask, (DWORD) MBMain);
		DirectedYield(hMBTask);
	}

	
	APISTR((LF_APIRET,"MessageBox: returns int %x\n",rc));
	return rc;
}

/*
 * LATER
 *	the constant strings should be a the resource file...
 *	
 *	w/ keys, we need to lookup the default and cancel codes
 *	and return the appropriate value
 */
static BOOL
MessageBoxHandler(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	MESSAGEBOXDATA *mbd;
	HWND	        hWnd;
	HDC		hDC;
	HICON		hIcon;
	LPSTR		resid;
	int		rc,n;
	int		control;
	int		nItems;
	int		minwidth;
    	PAINTSTRUCT 	ps;
	RECT		rect;
	RECT		rcClient,rcDlg,crect;
	int		nNCHeight, nNCWidth;
	int		ctlid[3];
	int		len;
	int		cx,width,height,nlines;

	switch(msg) {
	case WM_INITDIALOG:
		mbd = (MESSAGEBOXDATA *) lParam;
	    	if(mbd->caption)
			SetWindowText(hDlg,mbd->caption);

		switch(mbd->flag & MB_ICONMASK) {
			case  MB_ICONHAND:
				MessageBeep(MB_ICONHAND);
				resid = (LPSTR)IDI_HAND;
				break;
			case  MB_ICONQUESTION:
				MessageBeep(MB_ICONQUESTION);
				resid = (LPSTR)IDI_QUESTION;
				break;
			case  MB_ICONEXCLAMATION:
				MessageBeep(MB_ICONEXCLAMATION);
				resid = (LPSTR)IDI_EXCLAMATION;
				break;
			case  MB_ICONASTERISK:
				MessageBeep(MB_ICONASTERISK);
				resid = (LPSTR)IDI_ASTERISK;
				break;
			default:
				resid = (LPSTR)0;
		}
		if(resid) {
			hIcon = LoadIcon(0,resid);
	    		SendDlgItemMessage(hDlg,105,STM_SETICON,hIcon,0L);
		} else {
			if((hWnd = GetDlgItem(hDlg,105)))
				ShowWindow(hWnd,SW_HIDE);
		}
		
		/* set default button to 1 */
		control = IDOK;
		nItems  = 1;

		switch(mbd->flag & MB_TYPEMASK) {
			case MB_OK:
				SetDlgItemText(hDlg,101,"&OK");
				hWnd = GetDlgItem(hDlg,101);
				ShowWindow(hWnd,SW_SHOW);
				SetWindowID(hWnd,IDOK);

				hWnd = GetDlgItem(hDlg,102);
				ShowWindow(hWnd,SW_HIDE);
	
				hWnd = GetDlgItem(hDlg,103);
				ShowWindow(hWnd,SW_HIDE);
				ctlid[0] = IDOK;
				break;

			case MB_OKCANCEL:
				SetDlgItemText(hDlg,101,"&OK");
				hWnd = GetDlgItem(hDlg,101);
				ShowWindow(hWnd,SW_SHOW);
				SetWindowID(hWnd,IDOK);

				SetDlgItemText(hDlg,102,"&CANCEL");
				hWnd = GetDlgItem(hDlg,102);
				ShowWindow(hWnd,SW_SHOW);
				SetWindowID(hWnd,IDCANCEL);

				hWnd = GetDlgItem(hDlg,103);
				ShowWindow(hWnd,SW_HIDE);
				nItems = 2;
				ctlid[0] = IDOK;
				ctlid[1] = IDCANCEL;
				break;
			case MB_ABORTRETRYIGNORE:
				SetDlgItemText(hDlg,101,"&ABORT");
				SetDlgItemText(hDlg,102,"&RETRY");
				SetDlgItemText(hDlg,103,"&IGNORE");

				hWnd = GetDlgItem(hDlg,101);
				ShowWindow(hWnd,SW_SHOW);
				SetWindowID(hWnd,IDABORT);

				hWnd = GetDlgItem(hDlg,102);
				ShowWindow(hWnd,SW_SHOW);
				SetWindowID(hWnd,IDRETRY);

				hWnd = GetDlgItem(hDlg,103);
				ShowWindow(hWnd,SW_SHOW);
				SetWindowID(hWnd,IDIGNORE);

				control = IDRETRY;
				nItems = 3;
				ctlid[0] = IDABORT;
				ctlid[1] = IDRETRY;
				ctlid[2] = IDIGNORE;
				break;
			case MB_YESNOCANCEL:
				SetDlgItemText(hDlg,101,"&YES");
				SetDlgItemText(hDlg,102,"&NO");
				SetDlgItemText(hDlg,103,"&CANCEL");

				hWnd = GetDlgItem(hDlg,101);
				ShowWindow(hWnd,SW_SHOW);
				SetWindowID(hWnd,IDYES);

				hWnd = GetDlgItem(hDlg,102);
				ShowWindow(hWnd,SW_SHOW);
				SetWindowID(hWnd,IDNO);

				hWnd = GetDlgItem(hDlg,103);
				ShowWindow(hWnd,SW_SHOW);
				SetWindowID(hWnd,IDCANCEL);
				control = IDYES;
				nItems = 3;
				ctlid[0] = IDYES;
				ctlid[1] = IDNO;
				ctlid[2] = IDCANCEL;

				break;
			case MB_YESNO:
				SetDlgItemText(hDlg,101,"&YES");
				hWnd = GetDlgItem(hDlg,101);
				ShowWindow(hWnd,SW_SHOW);
				SetWindowID(hWnd,IDYES);

				SetDlgItemText(hDlg,102,"&NO");
				hWnd = GetDlgItem(hDlg,102);
				ShowWindow(hWnd,SW_SHOW);
				SetWindowID(hWnd,IDNO);

				hWnd = GetDlgItem(hDlg,103);
				ShowWindow(hWnd,SW_HIDE);

				nItems = 2;
				control = IDYES;
				ctlid[0] = IDYES;
				ctlid[1] = IDNO;
				break;
			case MB_RETRYCANCEL:
				SetDlgItemText(hDlg,101,"&RETRY");
				hWnd = GetDlgItem(hDlg,101);
				ShowWindow(hWnd,SW_SHOW);
				SetWindowID(hWnd,IDRETRY);

				SetDlgItemText(hDlg,102,"&CANCEL");
				hWnd = GetDlgItem(hDlg,102);
				ShowWindow(hWnd,SW_SHOW);
				SetWindowID(hWnd,IDCANCEL);

				hWnd = GetDlgItem(hDlg,103);
				ShowWindow(hWnd,SW_HIDE);

				nItems = 2;
				control = IDRETRY;
				ctlid[0] = IDRETRY;
				ctlid[1] = IDCANCEL;
				break;
		}

		/* default id */
		mbd->lParam = control;

		hWnd = GetDlgItem(hDlg,control);
		GetClientRect(hWnd,&crect);
		rc = 32;
		minwidth = 32 + nItems * (rc + crect.right);

		/************************************************/
		/*						*/
		/* Calculate length of longest segment		*/
		/* currently, we take the whole thing, but we   */
		/* should see if it has multiple lines...	*/
		/*						*/
		/************************************************/
		hDC = GetDC(0);
		len = GetTextExtent(hDC,mbd->text,strlen(mbd->text));	
		width = LOWORD(len);
		height = HIWORD(len);

		cx = GetDeviceCaps( hDC, HORZRES ) / 2;

		nlines = width / cx;
		nlines++;

		/* we now know how big the static text field is */
		/* rect will hold left,top as position, */
		/*                right,bottom as distance */
		rect.left  = 0;
		rect.top   = 0;
		rect.right = width / nlines;	/* exact width 		*/
		rect.right += 8;		/* a pinch to spare...  */

		nlines++;			/* just so drawtext works */
		rect.bottom = height * nlines;	/* give us a guess... 	  */

		/* calculate given the desired width... */
		/* we have fixed the right edge, now we see exactly how deep */
		cx = DrawText(hDC,mbd->text,-1,&rect,DT_CALCRECT|DT_WORDBREAK);

		/* make sure all text is accounted for... */
		/* this is distance, not position */
		rect.bottom += 16;

		/* this is where it starts vertically in the dialog */
		/* this is the actual position */
		rect.top = 32;

		/* where should the text go... */
		/* should we move it to make room for icon? */
		if(mbd->flag & MB_ICONMASK) {
			rect.left = 64;
			/* now move the icon... */
			if((hWnd = GetDlgItem(hDlg,105)))
			     SetWindowPos(hWnd,0,
				16, rect.top + (rect.bottom-32)/2,
				32, 32 ,
				SWP_NOSIZE|SWP_NOREDRAW|
				SWP_NOZORDER|SWP_NOACTIVATE);
		} else	rect.left = 16;

		/* now resize/move the static field... */
		hWnd = GetDlgItem(hDlg,0x68);
		SetWindowPos(hWnd,0,
			rect.left, rect.top,
			rect.right, rect.bottom,
			SWP_NOREDRAW|SWP_NOZORDER|SWP_NOACTIVATE);

		/* and set text into it */
		SetWindowText(hWnd,mbd->text);

		ReleaseDC(0,hDC);

		/************************************************/
		/*						*/
		/*  calculate the size of the message box, given*/
		/*  the size of the static text in it 		*/
		/*						*/
		/************************************************/

		GetClientRect(hDlg,&rcClient);	/* this is the dialog */
		GetWindowRect(hDlg,&rcDlg);
		nNCWidth = rcDlg.right - rcDlg.left - rcClient.right;
		nNCHeight = rcDlg.bottom - rcDlg.top - rcClient.bottom;

		/* resize so as to center the static text... */
		rcClient.right = rect.right + rect.left + 16;

		/* make sure it can hold all buttons fully */
		if(rcClient.right < minwidth)
			rcClient.right = minwidth;

		/* allocate space for buttons   */
		/* get size of a button 	*/
		hWnd = GetDlgItem(hDlg,control);
		GetClientRect(hWnd,&crect);

		/* the actual bottom is bottom of text and button... */
		/* calculate using rect.top position and rect.bottom */
		/* as distance */
		rcClient.bottom = rect.top + rect.bottom + crect.bottom;
		
		/* plus space around the button */
		rcClient.bottom += 32;

		/* now resize the dialog box itself */
		/* we have accounted for icons, buttons and text */
		width = rcClient.right + nNCWidth;
		height = rcClient.bottom + nNCHeight;

		SetWindowPos(hDlg,0,
			(GetSystemMetrics(SM_CXSCREEN) - width) /2,
			(GetSystemMetrics(SM_CYSCREEN) - height) /3,
			width,height,
			SWP_NOZORDER|SWP_NOACTIVATE);
			
		/* now place the buttons correctly */
		GetClientRect(hDlg,&crect);

		/* get the size of a control */
		hWnd = GetDlgItem(hDlg,control);
		GetClientRect(hWnd,&rcClient);

		/* this is space after removing space for buttons */
		rc = crect.right - nItems * rcClient.right;
		/* this is the space per button */
		rc /= (nItems + 1);

		for(n=0;n<nItems;n++) {
	
			hWnd = GetDlgItem(hDlg,ctlid[n]);

			/* buttons are just below text... */
			SetWindowPos(hWnd,0,
				rc + n*(rc + rcClient.right),
				rect.top+rect.bottom+16,
				rcClient.right, rcClient.bottom,
				SWP_NOSIZE|SWP_NOREDRAW|
				SWP_NOZORDER|SWP_NOACTIVATE);
		}

		hWnd = GetDlgItem(hDlg,control);
		if(hWnd)
			SetFocus(hWnd);
		else {
			SetFocus(wParam);
			hWnd = wParam;
		}
		
#ifdef CURSORWARPING
		/* this will warp the cursor to the default button */
		/* it only should be activated by a user preference */
		/* and not automatically */
		GetWindowRect(hWnd,&rcDlg);
		width = (rcDlg.left + rcDlg.right)/2;
		height = (rcDlg.bottom + rcDlg.top)/2;
		height += 20;
		SetCursorPos(width,height);
#endif
		return FALSE;

	case WM_PAINT:
	    	BeginPaint(hDlg,&ps);
	    	EndPaint(hDlg,&ps);

		return TRUE;

	case DM_GETDEFID:
		mbd = (MESSAGEBOXDATA *) GetWindowLong(hDlg,DWL_LPARAM);
		SetWindowLong(hDlg,DWL_MSGRESULT, 
			MAKELONG(mbd->lParam,DC_HASDEFID));
		return  TRUE;   

	case WM_COMMAND:
		mbd = (MESSAGEBOXDATA *) GetWindowLong(hDlg,DWL_LPARAM);

		/* get default value */
		/* note: this may already have been set... */
		rc = GET_WM_COMMAND_ID(wParam,lParam);

		EndDialog(hDlg, rc);
		return TRUE;

/************
        case WM_KEYDOWN:
		printf("mbkey: %x %x\n",wParam,lParam); 

		GetDlgItemText(hDlg, 1, TitleStr, 80);
		printf("1: %s\n",TitleStr);
		GetDlgItemText(hDlg, 2, TitleStr, 80);
		printf("2: %s\n",TitleStr);
		GetDlgItemText(hDlg, 3, TitleStr, 80);
		printf("3: %s\n",TitleStr);

	        SendMessage(GetParent(hWnd), WM_COMMAND,
			GET_WM_COMMAND_MPS(GetWindowID(hWnd),hWnd,BN_CLICKED));

                break;
*************/

	case WM_GETDLGCODE:
		return DLGC_WANTALLKEYS|DLGC_DEFPUSHBUTTON;

	}
	return FALSE;
}

/*
 * Local Variables:
 * c-indent-level:8
 * c-continued-statement-offset:8
 * c-continued-brace-offset:0
 * c-brace-imaginary-offset:0
 * c-brace-offset:-8
 * c-label-offset:-8
 * End:
 */
