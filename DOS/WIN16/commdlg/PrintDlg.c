/*    
	PrintDlg.c	1.7 Common Dialogs Print Routine Dialog
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
//#include "windowsx.h"
#include "dlgs.h"
#include "print.h"
#include "cderr.h"
#include "commdlg.h"

#include "Log.h"
#include "Dialog.h"
#include "GdiDDK.h"
#include "PrinterDC.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>


#define	ICON_PORTRAIT		0x210
#define	ICON_LANDSCAPE		0x211
#define	BINNAME_LEN		24
#define	PAPERS_STRING_ID_BASE	1153
#define	NO_DEFAULT_PRN_MSG	1114

/* Dialog procedures for CommDlg, these are NOT exported */
static LRESULT FAR PASCAL PrintDlgProc     (HWND, UINT, WPARAM, LPARAM);
static LRESULT FAR PASCAL PrintSetupDlgProc(HWND, UINT, WPARAM, LPARAM);

BOOL MakeDialogBox(HINSTANCE, HWND, LPCSTR, BOOL, BOOL, LPCSTR, 
			void *, DLGPROC);
static void InitPrintSetupControls(HWND, LPSTR, LPSTR, LPSTR, PRINTDLG *);
static int MakeDEVStruct(HWND, HWND, PRINTDLG FAR*);

HINSTANCE LoadDriver(LPCSTR);

extern DWORD	LastCommonDialogError;

/*----------------------------------------------------------------------------
				PrintDlg()
  ----------------------------------------------------------------------------*/
BOOL
WINAPI PrintDlg(PRINTDLG FAR *lppd)
{
#ifdef macintosh
	DEVMODE*		lpDevMode;
#endif

	LPDEVNAMES      lpDevNames;
	LPSTR           lpszDriver, lpszDevice, lpszPort;
	char            szBuf[128];
	char            szDevice[32];
	char            szPort[128];


	if ( lppd->Flags & PD_RETURNDEFAULT ) {
		if ( lppd->hDevMode  ||  lppd->hDevNames  || 
			MakeDEVStruct(0, lppd->hwndOwner, lppd) == -1 ) {
			LastCommonDialogError = PDERR_RETDEFFAILURE;
			return FALSE;
		}
		if ( lppd->Flags & (PD_RETURNDC | PD_RETURNIC) ) {
			lpDevNames = (DEVNAMES *)GlobalLock(lppd->hDevNames);
			if (!lpDevNames) {
				LastCommonDialogError = CDERR_MEMALLOCFAILURE;
				return FALSE;
			}
			_fstrcpy(szBuf,(LPSTR)lpDevNames +
					lpDevNames->wDriverOffset);
			lpszDriver = szBuf;
			_fstrcpy(szDevice,(LPSTR)lpDevNames +
					lpDevNames->wDeviceOffset);
			lpszDevice = szDevice;
			_fstrcpy(szPort,(LPSTR)lpDevNames +
					lpDevNames->wOutputOffset);
			lpszPort = szPort;
			GlobalUnlock(lppd->hDevNames);
			if ( lppd->Flags & PD_RETURNDC )
				lppd->hDC = CreateDC(lpszDriver, lpszDevice,
						lpszPort, NULL);
			else
				lppd->hDC = CreateIC(lpszDriver, lpszDevice,
						lpszPort, NULL);
			if ( !lppd->hDC ) {
				LastCommonDialogError = PDERR_CREATEICFAILURE;
				return FALSE;
			}
		}
		return TRUE;
	}

	if ( lppd->Flags & PD_PRINTSETUP ) {
		if ( lppd->Flags & PD_ENABLESETUPHOOK  &&
			!lppd->lpfnSetupHook ) {
			LastCommonDialogError = CDERR_NOHOOK;
			return 0;
		}
#ifdef	macintosh
/*	TODO:	Do we need to determine if lppd->hDevMode == NULL before calling GlobalLock()?? */
		lppd->hDevMode = TWIN_CreateDevMode( lppd->hDevMode );
		if ( lppd->hDevMode == NULL )
			return( FALSE );
		return( TWIN_PrintSetup( lppd->hDevMode ));
#else	/* macintosh */
		return MakeDialogBox( 
			(lppd->Flags & PD_ENABLESETUPTEMPLATEHANDLE)?
				lppd->hSetupTemplate:lppd->hInstance, 
			lppd->hwndOwner, lppd->lpSetupTemplateName, 
			lppd->Flags & PD_ENABLESETUPTEMPLATE,
			lppd->Flags & PD_ENABLESETUPTEMPLATEHANDLE,
			MAKEINTRESOURCE(PRNSETUPDLGORD), (void *)lppd, 
			(DLGPROC)PrintSetupDlgProc);
#endif	/* else macintosh */
	}
	else {
		if ( lppd->Flags & PD_ENABLEPRINTHOOK  &&
			!lppd->lpfnPrintHook ) {
			LastCommonDialogError = CDERR_NOHOOK;
			return 0;
		}
#ifdef	macintosh
/*	TODO:	Can we do this with an ESCAPE command like in MakeDEVStruct */
		lppd->hDevMode = TWIN_CreateDevMode( lppd->hDevMode );
		if ( lppd->hDevMode == NULL )
			return( FALSE );
		lpDevMode = (DEVMODE* )GlobalLock( lppd->hDevMode );
		lppd->hDC = CreateDC( "PSCRIPT", NULL, NULL, lpDevMode );
		if ( lppd->hDC == NULL )
		{
			GlobalUnlock( lppd->hDevMode );
			GlobalFree( lppd->hDevMode );
			return( FALSE );
		}
		GlobalUnlock( lppd->hDevMode );
		TWIN_FillPrintDlg( lppd, lppd->hDevMode );
		return( lppd->hDC != NULL );
#else	/* macintosh */
		return MakeDialogBox(
			(lppd->Flags & PD_ENABLEPRINTTEMPLATEHANDLE)?
				lppd->hPrintTemplate:lppd->hInstance, 
			lppd->hwndOwner, lppd->lpPrintTemplateName, 
			lppd->Flags & PD_ENABLEPRINTTEMPLATE,
			lppd->Flags & PD_ENABLEPRINTTEMPLATEHANDLE,
			MAKEINTRESOURCE(PRINTDLGORD), (void *)lppd, 
			(DLGPROC)PrintDlgProc);
#endif	/* macintosh */
	}
	return( 0 );
}


static LRESULT FAR PASCAL 
PrintDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static UINT		uiHelpMsg;
    HGLOBAL		hDMode;
    HDC 		hDC;
    HBRUSH 		hBrush;
    HINSTANCE		hDrvInst;
    void FAR		*lpMem;
    LONG		*lpLong;
    PRINTDLG		*cf;
    DEVMODE		*lpDevMode;
    DEVNAMES		*lpDevNames;
    UINT		uiRet = 0;
    LPSTR		lpszDriver, lpszDevice, lpszPort;
    int			i, n;
    PDEVCAPSPROC	lpfnDeviceCapabilities;
    PRINTDLG		printDlgStruct;
    RECT		rcClient;
    char		buf[80], szPrinterControl[128];

	cf = (PRINTDLG *)GetWindowLong(hDlg,DWL_LPARAM); 
	if ( message != WM_INITDIALOG  &&  cf->Flags & PD_ENABLEPRINTHOOK ) {
		uiRet = cf->lpfnPrintHook(hDlg,message,wParam,lParam);
		if ( uiRet ) 
			return (LRESULT)uiRet;
	}
	SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);

    switch ( message ) {
	case WM_INITDIALOG:
		if ( MakeDEVStruct(hDlg, 0, cf) == -1 ) {
			EndDialog(hDlg, FALSE);
			return 0;
		}

		if ( (cf->Flags & PD_ALLPAGES)  ||  
		     (!(cf->Flags & PD_PAGENUMS) && 
		      !(cf->Flags & PD_SELECTION)) )
			CheckRadioButton(hDlg, rad1, rad3, rad1);
		else
			if ( cf->Flags & PD_SELECTION )
				CheckRadioButton(hDlg, rad1, rad3, rad2);
			else
				if ( cf->Flags & PD_PAGENUMS )
					CheckRadioButton(hDlg, rad1, rad3, rad3);

		if ( cf->Flags & PD_NOPAGENUMS ) {
			SendDlgItemMessage(hDlg, rad3, WM_ENABLE, FALSE, 0);
			SendDlgItemMessage(hDlg, stc2, WM_ENABLE, FALSE, 0);
			SendDlgItemMessage(hDlg, stc3, WM_ENABLE, FALSE, 0);
			SendDlgItemMessage(hDlg, edt1, WM_ENABLE, FALSE, 0);
			SendDlgItemMessage(hDlg, edt2, WM_ENABLE, FALSE, 0);
		}

		if ( cf->Flags & PD_NOSELECTION )
			SendDlgItemMessage(hDlg, rad2, WM_ENABLE, FALSE, 0);

		if ( cf->Flags & PD_DISABLEPRINTTOFILE )
			SendDlgItemMessage(hDlg, chx1, WM_ENABLE, FALSE, 0);

		if ( cf->Flags & PD_COLLATE )
			SendDlgItemMessage(hDlg, chx2, BM_SETCHECK, TRUE, 0);

		if ( cf->Flags & PD_HIDEPRINTTOFILE )
			ShowWindow(GetDlgItem(hDlg, chx1), SW_HIDE);

		if ( cf->Flags & PD_PRINTTOFILE )
			SendDlgItemMessage(hDlg, chx1, BM_SETCHECK, TRUE, 0);

		if ( cf->Flags & PD_USEDEVMODECOPIES ) {
			/*  ?????????????  */
		}
		if ( cf->nFromPage != 0xFFFF )
			SetDlgItemInt(hDlg, edt1, cf->nFromPage, FALSE);
		
		if ( cf->nToPage != 0xFFFF )
			SetDlgItemInt(hDlg, edt2, cf->nToPage, FALSE);
		
		if ( cf->hDevMode ) {
			lpMem = GlobalLock(cf->hDevMode);
			SetDlgItemInt(hDlg, edt3, 
					((DEVMODE *)lpMem)->dmCopies, TRUE);
			GlobalUnlock(cf->hDevMode);
		}
		else
			SetDlgItemInt(hDlg, edt3, cf->nCopies, TRUE);

		/* Initialize stc1 */
		lpDevNames = (DEVNAMES *)GlobalLock(cf->hDevNames);
		lpszDriver = ((LPSTR)lpDevNames) + lpDevNames->wDriverOffset;
		lpszDevice = ((LPSTR)lpDevNames) + lpDevNames->wDeviceOffset;
		lpszPort   = ((LPSTR)lpDevNames) + lpDevNames->wOutputOffset;
		lpDevNames = (DEVNAMES *)GlobalLock(cf->hDevNames);
		if ( lpDevNames->wDefault & DN_DEFAULTPRN ) {
			sprintf(szPrinterControl,
				"Default Printer (%s on (%s))",
				lpszDevice, lpszPort);
			SetDlgItemText(hDlg, stc1, szPrinterControl);
		}
		else {
			/*  ?????  */
		}
		GlobalUnlock(cf->hDevNames);

		/* Initialize cmb1 */
		hDrvInst = LoadDriver(lpszDriver);
		lpfnDeviceCapabilities = (PDEVCAPSPROC)GetProcAddress(hDrvInst, 
					ORD_DEVICECAPS);
		n = lpfnDeviceCapabilities(lpszDevice, lpszPort,
					DC_ENUMRESOLUTIONS, 0, 0);
		lpLong = (LONG *)WinMalloc(n * ( 2 * sizeof(LONG) ));
		lpfnDeviceCapabilities(lpszDevice, lpszPort, DC_ENUMRESOLUTIONS,
				(LPVOID)lpLong, 0);
		FreeLibrary(hDrvInst);
		for ( i = 0; i < n * (2 * sizeof(LONG)); i += 2 * sizeof(LONG) ) {
			/* Only X-resolution is taken! */
			sprintf(buf, "%d dpi", (int) lpLong[i]);
			SendDlgItemMessage(hDlg, cmb1, CB_INSERTSTRING, -1,
						(LPARAM)buf);
			SendDlgItemMessage(hDlg, cmb1, CB_SETITEMDATA, i,
				MAKELONG(LOWORD(lpLong[i]),LOWORD(lpLong[i+1])));
		}
		SendDlgItemMessage(hDlg, cmb1, CB_SETCURSEL, 0, 0);
		WinFree((void *)lpLong);

		if ( !(cf->Flags & PD_SHOWHELP) )
			ShowWindow(GetDlgItem(hDlg, pshHelp), SW_HIDE);
		else
			uiHelpMsg = RegisterWindowMessage(HELPMSGSTRING);

		/* For WM_INITDIALOG hook is invoked after default processing */

		if ( cf->Flags & PD_ENABLEPRINTHOOK )
			cf->lpfnPrintHook(hDlg,message,wParam,lParam);

		return TRUE;

	case WM_COMMAND:
	    SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)1);

	    hDMode = (HGLOBAL)GetWindowWord(hDlg, DWW_RESULT);
	    if ( GET_WM_COMMAND_ID(wParam,lParam) == IDOK ) {
			lpDevMode = (DEVMODE *)GlobalLock(cf->hDevMode);
#ifndef LATER
			n = (int)SendDlgItemMessage(hDlg, cmb1, 
						CB_GETCURSEL, 0, 0);
			lpDevMode->dmPrintQuality = 
				(int)SendDlgItemMessage(hDlg, cmb1, 
						CB_GETITEMDATA, n, 0);
#else
			lpDevMode->dmPrintQuality = 300; /* Temp. patch !!!! */
#endif

			n = GetDlgItemInt(hDlg, edt3, (BOOL FAR *)&i, FALSE);
			if ( cf->Flags & PD_USEDEVMODECOPIES ) {
				cf->nCopies = 1;
				lpDevMode->dmCopies = n;
			}
			else
				cf->nCopies = n;

			cf->Flags |= PD_ALLPAGES;	/* Temp. patch !!!! */

			if ( IsDlgButtonChecked(hDlg, rad1) )
				cf->Flags |= PD_ALLPAGES;
			if ( IsDlgButtonChecked(hDlg, rad2) )
				cf->Flags |= PD_SELECTION;
			if ( IsDlgButtonChecked(hDlg, rad3) )
				cf->Flags |= PD_PAGENUMS;
			if ( IsDlgButtonChecked(hDlg, chx1) )
				cf->Flags |= PD_PRINTTOFILE;
			if ( IsDlgButtonChecked(hDlg, chx2) )
				cf->Flags |= PD_COLLATE;
			
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			if ( cf->Flags & (PD_RETURNDC | PD_RETURNIC) ) {
				lpDevNames = 
					(DEVNAMES *)GlobalLock(cf->hDevNames);
				lpszDriver = ((LPSTR)lpDevNames) + lpDevNames->wDriverOffset;
				lpszDevice = ((LPSTR)lpDevNames) + lpDevNames->wDeviceOffset;
				lpszPort   = ((LPSTR)lpDevNames) + lpDevNames->wOutputOffset;

				GlobalUnlock(cf->hDevNames);
				if ( cf->Flags & PD_RETURNDC )
					cf->hDC = CreateDC(
						lpszDriver, lpszDevice,
						lpszPort, lpDevMode);
				else
					cf->hDC = CreateIC(
						lpszDriver, lpszDevice,
						lpszPort, lpDevMode);
				if ( !cf->hDC ) {
					LastCommonDialogError =
							PDERR_CREATEICFAILURE;
					EndDialog(hDlg, FALSE);
					return 0;
				}
			}
			GlobalUnlock(cf->hDevMode);
			if ( cf->hDevMode != hDMode )
				GlobalFree(hDMode);
			EndDialog(hDlg, TRUE);
			return TRUE;
		}

	    if ( GET_WM_COMMAND_ID(wParam,lParam) == IDCANCEL ) {
		if ( cf->hDevMode != hDMode ) {
			GlobalFree(cf->hDevMode);
			cf->hDevMode = hDMode;
		}
		SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
		EndDialog(hDlg, FALSE);
		return FALSE;
	    }

	    switch ( GET_WM_COMMAND_ID(wParam,lParam) ) {
		case psh1:		/*  Setup...  */
			/* Make sure that we don't need to call PrintDlg */
			/* twice: first time w/Flags |= PD_RETURNDEFAULT */
			memcpy((LPSTR)&printDlgStruct, (LPSTR)cf, cf->lStructSize);
			printDlgStruct.Flags |= PD_PRINTSETUP;
			PrintDlg(&printDlgStruct);
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			return TRUE;
		case pshHelp:
			SendMessage(cf->hwndOwner, uiHelpMsg, 0, 0);
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			return TRUE;
		case rad1:
			CheckRadioButton(hDlg, rad1, rad3, rad1);
			SendDlgItemMessage(hDlg, rad1, WM_SETFOCUS, 0, 0);
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			return TRUE;
		case rad2:
			CheckRadioButton(hDlg, rad1, rad3, rad2);
			SendDlgItemMessage(hDlg, rad2, WM_SETFOCUS, 0, 0);
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			return TRUE;
		case rad3:
			CheckRadioButton(hDlg, rad1, rad3, rad3);
			SendDlgItemMessage(hDlg, edt1, WM_SETFOCUS, 0, 0);
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);
			return TRUE;
	    }
	    break;

	case WM_ERASEBKGND: {
	    hDC = GetDC(hDlg);
	    hBrush = (HBRUSH)SendMessage(hDlg,GET_WM_CTLCOLOR_MSG(CTLCOLOR_DLG),
				GET_WM_CTLCOLOR_MPS(hDC,hDlg,CTLCOLOR_DLG));
	    if (hBrush) {
		GetClientRect(hDlg, &rcClient);
		FillRect((HDC)wParam, &rcClient, hBrush);
		ReleaseDC(hDlg, hDC);
		SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)TRUE);
		return TRUE;
	    }
	    else {
		ReleaseDC(hDlg,hDC);
		SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)FALSE);
		return FALSE;
	    }
	}
		break;

	default:
		break;
    }
    return FALSE;
}


static LRESULT FAR PASCAL 
PrintSetupDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    static UINT		uiHelpMsg;
    HGLOBAL		hDMode;
    HMODULE		hModule;
    HICON		hIcon;
    HDC			hDC;
    HBRUSH 		hBrush;
    PRINTDLG		*cf;
    UINT		uiRet = 0;
    int			n;
    LPSTR		lpszDriver, lpszDevice, lpszPort, lp;
    PEXTDEVMODEPROC	lpfnExtDevMode;
    PADVSETUPDLGPROC	lpfnAdvSetupDlg;
    DEVMODE		*lpDevMode, *lpDevModeOut;
    DEVNAMES		*lpDevNames;
    RECT		rcClient;
    char		buf1[80], buf2[80], szDevBuf[512];
    int			wID;

	cf = (PRINTDLG *)GetWindowLong(hDlg,DWL_LPARAM); 
	if ( message != WM_INITDIALOG  &&  cf->Flags & PD_ENABLESETUPHOOK ) {
		uiRet = cf->lpfnSetupHook(hDlg,message,wParam,lParam);
		if ( uiRet ) 
			return (LRESULT)uiRet;
	}
	SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);

    switch ( message ) {
	case WM_INITDIALOG:
/* #ifndef	macintosh */
		if ( MakeDEVStruct(hDlg, 0, cf) == -1 ) {
			EndDialog(hDlg, FALSE);
			return 0;
		}
/* #endif	//	macintosh */

		/* Initialize rad3, rad4 (grp3) */
		if ( GetProfileString("windows", "device", "", buf1, 
			sizeof(buf1)) ) {
			if ( (lpszDevice    = strtok(buf1, ","))  &&
				(lpszDriver = strtok(NULL, ","))  &&
				(lpszPort   = strtok(NULL, ",")) ) {
				sprintf(buf2, "(currently %s on %s)", 
					lpszDevice, lpszPort);
			}
			SetDlgItemText(hDlg, stc1, buf2);
	    		CheckRadioButton(hDlg, rad3, rad4, rad3);
			n = 0;
		}
		else {
			EnableWindow(GetDlgItem(hDlg, rad3), FALSE);
			EnableWindow(GetDlgItem(hDlg, stc1), FALSE);
			n = 1;
		}
		if ( GetProfileString("devices", "", "", szDevBuf, 
			sizeof(szDevBuf)) ) {
			lp = szDevBuf;
			while ( *lp ) {
				GetProfileString("devices", lp, "", buf1,
						sizeof(buf1));
				lpszDriver = strtok(buf1, ",");  /* Skip */
				lpszPort = strtok(buf1, ",");
				sprintf(buf2, "%s on %s", lp, lpszPort);
				SendDlgItemMessage(hDlg, cmb1, CB_INSERTSTRING,
					 	-1, (LPARAM)buf2);
				lp += strlen(lp) + 1;
			}
			SendDlgItemMessage(hDlg, cmb1, CB_SETCURSEL, 0, 0);
			if ( n )
	    			CheckRadioButton(hDlg, rad3, rad4, rad4);
		}
		else {
			EnableWindow(GetDlgItem(hDlg, rad4), FALSE);
			EnableWindow(GetDlgItem(hDlg, cmb1), FALSE);
			if ( n )
				EnableWindow(GetDlgItem(hDlg, grp3), FALSE);
		}

		lpDevNames = (DEVNAMES *)GlobalLock(cf->hDevNames);
		lpszDriver = ((LPSTR)lpDevNames) + lpDevNames->wDriverOffset;
		lpszDevice = ((LPSTR)lpDevNames) + lpDevNames->wDeviceOffset;
		lpszPort   = ((LPSTR)lpDevNames) + lpDevNames->wOutputOffset;
		InitPrintSetupControls(hDlg, lpszDriver, lpszDevice, lpszPort,
					cf);

		if ( !(cf->Flags & PD_SHOWHELP) )
			ShowWindow(GetDlgItem(hDlg, pshHelp), SW_HIDE);
		else
			uiHelpMsg = RegisterWindowMessage(HELPMSGSTRING);

		/* For WM_INITDIALOG hook is invoked after default processing */

		if ( cf->Flags & PD_ENABLESETUPHOOK )
			cf->lpfnSetupHook(hDlg,message,wParam,lParam);

	    return TRUE;

	case WM_COMMAND:
	    wID = GET_WM_COMMAND_ID(wParam,lParam);
	    if ( wID == IDOK     ||  wID == IDCANCEL  ||
		 wID == pshHelp  ||  wID == psh1      ||
		 wID == rad1     ||  wID == rad2      || 
		 wID == rad3     ||  wID == cmb1      ||  wID == cmb2 )
			SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)0);

	    hDMode = (HGLOBAL)GetWindowWord(hDlg, DWW_RESULT);
	    lpDevNames = (DEVNAMES *)GlobalLock(cf->hDevNames);
	    lpszDriver = ((LPSTR)lpDevNames) + lpDevNames->wDriverOffset;
	    lpszDevice = ((LPSTR)lpDevNames) + lpDevNames->wDeviceOffset;
	    lpszPort   = ((LPSTR)lpDevNames) + lpDevNames->wOutputOffset;

	    if ( wID == IDOK ) {
	    	lpDevMode = (DEVMODE *)GlobalLock(cf->hDevMode);
		if ( IsWindowEnabled(GetDlgItem(hDlg, rad1))  &&  
		     IsWindowEnabled(GetDlgItem(hDlg, rad2)) )
			if ( IsDlgButtonChecked(hDlg, rad1) )
				lpDevMode->dmOrientation = DMORIENT_PORTRAIT;
			else
				lpDevMode->dmOrientation = DMORIENT_LANDSCAPE;

		n = (int)SendDlgItemMessage(hDlg,cmb2,CB_GETCURSEL,0,0);
		lpDevMode->dmPaperSize = (int)SendDlgItemMessage(hDlg,cmb1, 
					CB_GETITEMDATA,n,0);

		n = (int)SendDlgItemMessage(hDlg,cmb3,CB_GETCURSEL,0,0);
		lpDevMode->dmDefaultSource = (int)SendDlgItemMessage(hDlg,cmb3, 
					CB_GETITEMDATA,n,0);

		GlobalUnlock(cf->hDevMode);
		if ( cf->hDevMode != hDMode )
			GlobalFree(hDMode);

		EndDialog(hDlg, TRUE);
		return TRUE;
	    }

	    if ( wID == IDCANCEL ) {
		if ( cf->hDevMode != hDMode ) {
			GlobalFree(cf->hDevMode);
			cf->hDevMode = hDMode;
		}
		EndDialog(hDlg, 0);
		return FALSE;
	    }

	    switch ( wID ) {
		case rad1:
			if ( IsWindowEnabled(GetDlgItem(hDlg, rad1)) ) {
				CheckRadioButton(hDlg, rad1, rad2, rad1);
				hModule = GetModuleHandle("COMMDLG");
				hIcon = LoadIcon(hModule, 
					MAKEINTRESOURCE(ICON_PORTRAIT));
				SendDlgItemMessage(hDlg, ico1, STM_SETICON,
					(WPARAM)hIcon, 0);
				return TRUE;
			}
			break;
		case rad2:
			if ( IsWindowEnabled(GetDlgItem(hDlg, rad2)) ) {
				CheckRadioButton(hDlg, rad1, rad2, rad2);
				hModule = GetModuleHandle("COMMDLG");
				hIcon = LoadIcon(hModule,
					MAKEINTRESOURCE(ICON_LANDSCAPE));
				SendDlgItemMessage(hDlg, ico1, STM_SETICON,
					(WPARAM)hIcon,0);
				return TRUE;
			}
			break;
		case rad3:
			CheckRadioButton(hDlg, rad3, rad4, rad3);
			SendDlgItemMessage(hDlg, rad3, WM_SETFOCUS, 0, 0);
			InitPrintSetupControls(hDlg, lpszDriver, lpszDevice,
						lpszPort, cf);
			return TRUE;
		case rad4:
			CheckRadioButton(hDlg, rad3, rad4, rad4);
			SendDlgItemMessage(hDlg, cmb1, WM_SETFOCUS, 0, 0);
			InitPrintSetupControls(hDlg, lpszDriver, lpszDevice,
						lpszPort, cf);
			return TRUE;
		case cmb1:
			/* Code for change printer is required */
			InitPrintSetupControls(hDlg, lpszDriver, lpszDevice,
						lpszPort, cf);
			return TRUE;
		case cmb2:
			return TRUE;

		case psh1:		/* <Options...> button */
			if ( !(hModule = LoadDriver(lpszDriver)) ) {
				LastCommonDialogError = PDERR_LOADDRVFAILURE;
				return FALSE;
			}
			lpfnExtDevMode = (PEXTDEVMODEPROC)GetProcAddress(
					hModule,ORD_EXTDEVMODE);
        		n = lpfnExtDevMode(0, 0, (DEVMODE *)0, lpszDevice, 
				lpszPort, (DEVMODE *)0, 0, 0);
			if ( !n ) {
				LastCommonDialogError = PDERR_GETDEVMODEFAIL;
				return FALSE;
			}

        		if ( !(hDMode = GlobalAlloc(GHND, (DWORD)n)) ) {
				LastCommonDialogError = CDERR_MEMALLOCFAILURE;
				return FALSE;
			}
        		lpDevModeOut = (DEVMODE *)GlobalLock(hDMode);
	    		lpDevMode    = (DEVMODE *)GlobalLock(cf->hDevMode);

			if ( !(lpfnAdvSetupDlg = (PADVSETUPDLGPROC)
				GetProcAddress(hModule,ORD_ADVSETUPDLG)) ) {
				LastCommonDialogError = PDERR_GETDEVMODEFAIL;
				return FALSE;
			}
			/* Invoke AdvancedSetupDialog() */
			n = (int)lpfnAdvSetupDlg(hDlg, hModule, lpDevMode,
						lpDevModeOut);
			FreeLibrary(hModule);

			if ( n != -1 ) {
				GlobalUnlock(cf->hDevMode);
				GlobalFree(cf->hDevMode);
				GlobalUnlock(hDMode);
				cf->hDevMode = hDMode;
				return TRUE;
			}	
			/* In case of error (n == -1)... */
			LastCommonDialogError = PDERR_GETDEVMODEFAIL;
			return FALSE;

		case pshHelp:
			SendMessage(cf->hwndOwner, uiHelpMsg, 0, 0);
			return TRUE;
		default:
			break;
	}
	break;

	case WM_ERASEBKGND:
	  {
	    hDC = GetDC(hDlg);
	    hBrush = (HBRUSH)SendMessage(hDlg,GET_WM_CTLCOLOR_MSG(CTLCOLOR_DLG),
				GET_WM_CTLCOLOR_MPS(hDC,hDlg,CTLCOLOR_DLG));
	    if (hBrush) {
		GetClientRect(hDlg, &rcClient);
		FillRect((HDC)wParam, &rcClient, hBrush);
		ReleaseDC(hDlg, hDC);
		SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)TRUE);
		return TRUE;
	    }
	    else {
		ReleaseDC(hDlg,hDC);
		SetWindowLong(hDlg, DWL_MSGRESULT, (LPARAM)FALSE);
		return FALSE;
	    }
	  }
	    break;

	default:
	    break;
    }
    return FALSE;
}


static void
InitPrintSetupControls(HWND hDlg, LPSTR lpszDriver, LPSTR lpszDevice,
			LPSTR lpszPort, PRINTDLG *pd)
{
	HMODULE		hInst;
	HINSTANCE	hDrvInst;
	HICON		hIcon;
	PDEVCAPSPROC	lpfnDeviceCapabilities;
	DEVMODE		*lpDevMode;
	WORD		*lpWord;
	LPSTR		lpszBinNames;
	int		i, n;	
	char		buf[80];
	
	hInst = GetModuleHandle("COMMDLG");
	hDrvInst = LoadDriver(lpszDriver);
	lpfnDeviceCapabilities = (PDEVCAPSPROC)GetProcAddress(
			hDrvInst, ORD_DEVICECAPS);
	lpDevMode = (DEVMODE *)GlobalLock(pd->hDevMode);

	/* Initialize cmb2 */
	/* Actually, we don't need to inquire about DC_PAPERNAMES, */
	/* because DC_PAPERS returns indices in string resources   */
	/* (in module COMMDLG). Indices are 1153-based.            */ 
	n = lpfnDeviceCapabilities(lpszDevice, lpszPort, DC_PAPERS, 0,
				lpDevMode);
	/* !!!!!!!!!!! n == 0 ??????????? */
	lpWord = (WORD *)WinMalloc(n * sizeof(WORD));
	lpfnDeviceCapabilities(lpszDevice,lpszPort,DC_PAPERS,(LPVOID)lpWord,
				lpDevMode);
	for ( i = 0; i < n; i++ ) {
		LoadString(hInst, PAPERS_STRING_ID_BASE+lpWord[i]-1,
				buf, sizeof(buf));
		SendDlgItemMessage(hDlg, cmb2, CB_INSERTSTRING, -1, 
				(LPARAM)buf);
		SendDlgItemMessage(hDlg, cmb2, CB_SETITEMDATA, i, 
				(LPARAM)lpWord[i]);
	}
	SendDlgItemMessage(hDlg, cmb2, CB_SETCURSEL, 0, 0);
	WinFree((LPSTR)lpWord);

	/* Initialize cmb3 */
	n = lpfnDeviceCapabilities(lpszDevice, lpszPort, DC_BINNAMES, 0,
				lpDevMode);
	lpszBinNames = (LPSTR)WinMalloc(n * BINNAME_LEN);
	lpWord       = (WORD *)WinMalloc(n * sizeof(WORD));
	lpfnDeviceCapabilities(lpszDevice, lpszPort, DC_BINS, (LPVOID)lpWord,
				lpDevMode);
	lpfnDeviceCapabilities(lpszDevice, lpszPort, DC_BINNAMES, 
				(LPVOID)lpszBinNames, lpDevMode);
	for ( i = 0; i < n; i++ ) {
	/*--- removed the +1 in BINNAME_LEN*i+1 tdd Oct. 27 ---*/
		SendDlgItemMessage(hDlg, cmb3, CB_INSERTSTRING, -1, 
				(LPARAM)(lpszBinNames+BINNAME_LEN*i));
		SendDlgItemMessage(hDlg, cmb3, CB_SETITEMDATA, i, 
				(LPARAM)lpWord[i]);
	}
	SendDlgItemMessage(hDlg, cmb3, CB_SETCURSEL, 0, 0);
	WinFree(lpszBinNames);
	WinFree((LPSTR)lpWord);

	/* Initialize rad1, rad2, grp1 */
	n = lpfnDeviceCapabilities(lpszDevice, lpszPort, DC_ORIENTATION,
			 0, 0);
	if ( !n ) {
		CheckDlgButton(hDlg, rad1, FALSE);
		CheckDlgButton(hDlg, rad2, FALSE);
	}
	else {
		n = 1;
		CheckRadioButton(hDlg, rad1, rad2, rad1);
		hIcon = LoadIcon(hInst, MAKEINTRESOURCE(ICON_PORTRAIT));
		SendDlgItemMessage(hDlg, ico1, STM_SETICON, (WPARAM)hIcon, 0);

	}
	EnableWindow(GetDlgItem(hDlg, rad1), n);
	EnableWindow(GetDlgItem(hDlg, rad2), n);
	EnableWindow(GetDlgItem(hDlg, grp1), n);

	FreeLibrary(hDrvInst);
}


#ifdef	macintosh

static int
MakeDEVStruct( HWND hDlg, HWND hWndOwner, PRINTDLG FAR* cf )
{

#ifdef	COMMENT
	DEVMODE*	lpDevMode;

/*	TODO:	Handle the case that the caller provided default values */

	cf->hDevMode = GlobalAlloc( GHND, (DWORD )sizeof( TWINDEVMODE ));
	if ( cf->hDevMode == NULL )
		return( -1 );
	lpDevMode = (TWINDEVMODE* )GlobalLock( cf->hDevMode );
	if ( lpDevMode == NULL )
	{
		GlobalFree( cf->hDevMode );
		cf->hDevMode = NULL;
		return( -1 );
	}
	strcpy((char* )lpDevMode->devMode.dmDeviceName, "Macintosh Default Printer" );
	lpDevMode->devMode.dmSpecVersion	= 0x30A;
	lpDevMode->devMode.dmSize			= sizeof( DEVMODE );
	lpDevMode->devMode.dmDriverExtra	= sizeof( LPVOID );
	lpDevMode->devMode.dmFields			= 0x00007F1F;	/* everything initialized */
	lpDevMode->devMode.dmOrientation	= DMORIENT_PORTRAIT;
	lpDevMode->devMode.dmPaperSize		= 0;			/* use length and width fields */

/*	lpDevMode->devMode.dmPaperLength	= set by the driver */
/*	lpDevMode->devMode.dmPaperWidth		= set by the driver */

	lpDevMode->devMode.dmScale		= 100;

/*	lpDevMode->devMode.dmCopies		= set by the driver */

	lpDevMode->devMode.dmDefaultSource	= DMBIN_AUTO;

/*	lpDevMode->devMode.dmPrintQuality	= set by the driver */

	lpDevMode->devMode.dmColor			= DMCOLOR_COLOR;
	lpDevMode->devMode.dmDuplex			= DMDUP_HORIZONTAL;

/*	lpDevMode->devMode.YResolution 		= set by the driver */

	lpDevMode->devMode.dmTTOption		= DMTT_SUBDEV;

/*	TODO:	Fill in hDefNames to best of ability */

	cf->hDevNames = GlobalAlloc( GHND, (DWORD )sizeof( DEVNAMES ));
	if ( cf->hDevNames == NULL )
		return( -1 );

#endif	/*	COMMENT */
	return( 0 );
}
#else	/*	macintosh */
static int
MakeDEVStruct(HWND hDlg, HWND hWndOwner, PRINTDLG FAR*cf)
{
	int		n, iRet;
	HGLOBAL		hDevMode, hDevNames;
	HINSTANCE	hInst;
/*
	HMODULE		hDriverModule;
*/
	DEVMODE		*lpDevMode, *lpDevModeInit;
	DEVNAMES	*lpDevNames;
	PEXTDEVMODEPROC	lpfnExtDevMode;
	LPSTR		lpszDriver = (LPSTR)NULL;
	LPSTR		lpszDevice = (LPSTR)NULL;
	LPSTR		lpszPort = (LPSTR)NULL;
	char		szBuf[128];
	char		szDevice[32];
	char		szPort[128];
	UINT		uDefault;

	if ( cf->hDevMode ) {
		lpDevModeInit = (DEVMODE *)GlobalLock(cf->hDevMode);
		strcpy(szDevice,(const char *)lpDevModeInit->dmDeviceName);
		lpszDevice = szDevice;
		GlobalUnlock(cf->hDevMode);
		/* Get information about specified printer */
		if ( GetProfileString("devices", lpszDevice, "", szBuf, 80) ) {
				if ( !((lpszDriver = strtok(szBuf, ","))  &&
					(lpszPort = strtok(NULL, ","))) ) {
						LastCommonDialogError = PDERR_PARSEFAILURE;
						return -1;
				}
		}
		else {
			LastCommonDialogError = PDERR_PRINTERNOTFOUND;
			return -1;
		}
	}
	else {
		if ( !cf->hDevNames ) {
			/* Get information about default printer */
			if ( GetProfileString("windows", "device", "", szBuf, 80) ) {
				if ( !((lpszDevice = strtok(szBuf, ","))  &&
			     	(lpszDriver   = strtok(NULL,  ","))  &&
			     	(lpszPort     = strtok(NULL,  ","))) ) {
					LastCommonDialogError = PDERR_PARSEFAILURE;
					return -1;
				}
			}
			else {
				if ( !(cf->Flags & PD_NOWARNING) ) {
					hInst = GetModuleHandle("COMMDLG");
					LoadString(hInst, NO_DEFAULT_PRN_MSG,
						szBuf, sizeof(szBuf));
					MessageBox((hDlg)?hDlg:hWndOwner,
						szBuf, "Print Setup",
						MB_ICONEXCLAMATION);
				}
				LastCommonDialogError = PDERR_NODEFAULTPRN;
				return -1;
			}
		}
		else {
			lpDevNames = (DEVNAMES *)GlobalLock(cf->hDevNames);
			if (!lpDevNames) {
				LastCommonDialogError = PDERR_NODEVICES;
				return -1;
			}
			strcpy(szBuf,(LPSTR)lpDevNames +
					lpDevNames->wDriverOffset);
			lpszDriver = szBuf;
			strcpy(szDevice,(LPSTR)lpDevNames +
					lpDevNames->wDeviceOffset);
			lpszDevice = szDevice;
			strcpy(szPort,(LPSTR)lpDevNames +
					lpDevNames->wOutputOffset);
			lpszPort = szPort;
			uDefault = lpDevNames->wDefault;
			GlobalUnlock(cf->hDevNames);
		}
	}

	if ( hDlg )	/* Save previous handler */
		SetWindowWord(hDlg, DWW_RESULT, (WORD)cf->hDevMode);
	

	/* Load driver */
	if ( (hInst = LoadDriver(lpszDriver)) <= HINSTANCE_ERROR ) {
		LastCommonDialogError = PDERR_LOADDRVFAILURE;
		return -1; 
	}
	lpfnExtDevMode = (PEXTDEVMODEPROC)GetProcAddress(hInst,ORD_EXTDEVMODE);
/*
	hDriverModule = GetModuleHandle(lpszDriver);
*/

	/* Get sizeof(DEVMODE) */
	n = lpfnExtDevMode(0, 0, (DEVMODE *)0, lpszDevice, lpszPort,
				(DEVMODE *)0, 0, 0);
	hDevMode = GlobalAlloc(GHND, (DWORD)n);
	lpDevMode = (DEVMODE *)GlobalLock(hDevMode);

	/* Make DEVMODE structure */
	if ( cf->hDevMode ) {
		lpDevModeInit = (DEVMODE *)GlobalLock(cf->hDevMode);
		iRet = lpfnExtDevMode(hDlg, hInst, lpDevMode,
				lpszDevice,lpszPort,lpDevModeInit,
				0, DM_COPY | DM_MODIFY);
		GlobalUnlock(cf->hDevMode);
	}
	else
		iRet = lpfnExtDevMode(hDlg, hInst, lpDevMode,
				lpszDevice,lpszPort,(DEVMODE *)0,0,DM_COPY);

	/*  !!!!!!!!!!   Tmp PATCH  !!!!!!!!!!!!!! */
	lpDevMode->dmDeviceName[17]='\0';

	if ( iRet < 0 ) {
		GlobalUnlock(hDevMode);
		GlobalFree(hDevMode);
		FreeLibrary(hInst);
		LastCommonDialogError = PDERR_GETDEVMODEFAIL;
		return iRet;
	}


	/* Make DEVNAMES structure */
	if ( !cf->hDevNames ) {
		/*  Find the place somewhere to preserve the old 
		    cf->hDevNames, and 
			- restore it in IDCANCEL;
			- free the memory associated w/old hDevNames
			  in IDOK.
					??????????????????????
		*/
	}
	else
		/* For now, we free this memory here.  */
		GlobalFree(cf->hDevNames);

	hDevNames = GlobalAlloc(GHND, sizeof(DEVNAMES) +
		strlen(lpszDriver) + strlen(lpszDevice) +
		strlen(lpszPort) + 3);
	if ( hDevNames == 0 )
		return -1;
	lpDevNames = (DEVNAMES *)GlobalLock(hDevNames);

	lpDevNames->wDeviceOffset = sizeof(DEVNAMES);
	lpDevNames->wDriverOffset = sizeof(DEVNAMES) +
			strlen(lpszDevice) + 1;
	lpDevNames->wOutputOffset = lpDevNames->wDriverOffset +
			strlen(lpszDriver) + 1;
	lpDevNames->wDefault = DN_DEFAULTPRN;
	strcpy(((LPSTR)lpDevNames)+lpDevNames->wDriverOffset, lpszDriver);
	strcpy(((LPSTR)lpDevNames)+lpDevNames->wDeviceOffset, lpszDevice);
	strcpy(((LPSTR)lpDevNames)+lpDevNames->wOutputOffset, lpszPort);
	GlobalUnlock(hDevNames);

	FreeLibrary(hInst);
	cf->hDevMode = hDevMode;
	cf->hDevNames = hDevNames;
	return iRet;
}
#endif	/*	else macintosh */


