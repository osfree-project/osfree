/*    
	CommdlgIF.c	2.16
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

#include "windows.h"
#include "print.h"
#include "commdlg.h"

#include "kerndef.h"
#include "Endian.h"
#include "Log.h"
#include "BinTypes.h"
#include "Kernel.h"		/* MEMORYINFO */
#include "PrinterBin.h"
#include "DPMI.h"
#include "dos.h"

/* exported routines */
void IT_PRINTDLG(ENV *,LONGPROC);
void IT_CHOOSECOLOR(ENV *,LONGPROC);
void IT_1LPOF(ENV *,LONGPROC);		/* GetOpen[Save]FileName */
void IT_FINDREPLACE(ENV *,LONGPROC);	/* Find[Replace]Text */
void IT_CHOOSEFONT(ENV *,LONGPROC);

/* imported routines */
extern DWORD make_native_thunk(DWORD, DWORD);
extern LPBYTE ExtractDialog(HINSTANCE,LPBYTE);
extern void CleanupDialog(LPBYTE);
extern LPSTR strpbrkr(LPSTR,LPSTR);

void
IT_PRINTDLG (ENV *envp,LONGPROC f)
{
	BINADDR dwBinAddr;
	DWORD retcode;
	PRINTDLG *lppd;
	LPBYTE lpStruct;
	HGLOBAL hPrintTemplate = 0;
	HGLOBAL hSetupTemplate = 0;
	HGLOBAL hDevMode,hDevNames,hGlobal;
	DEVMODE *lpdm;
	DEVNAMES *lpdn;
	LPBYTE ptr;
	int len1,len2,len3;
	DWORD dwProc,dwFlags;
	MEMORYINFO *lpMemory;
	LPSTR lpString;
	LPBYTE lpData;
	LPBYTE lpPrintData = 0;
	LPBYTE lpSetupData = 0;
	WORD wAllocSize;
	WORD wSel = 0;

	dwBinAddr = GETDWORD(SP+4);
	lpStruct = (LPBYTE)GetAddress(HIWORD(dwBinAddr),LOWORD(dwBinAddr));
	lppd = (PRINTDLG *)((LPBYTE)WinMalloc(sizeof(PRINTDLG) + sizeof(LPVOID))
				+ sizeof(LPVOID));
	*((BINADDR *)((LPBYTE)lppd - sizeof(LPVOID))) = dwBinAddr;
	memset((LPSTR)lppd,'\0',sizeof(PRINTDLG));

	lppd->lStructSize = sizeof(PRINTDLG);
	lppd->hwndOwner = GETWORD(lpStruct+4);
	if (GETWORD(lpStruct+6)) {
	    lpData = (LPBYTE)GetPhysicalAddress(GETWORD(lpStruct+6));
	    hDevMode = GlobalAlloc(GHND,sizeof(DEVMODE) + 
			GETWORD(lpData+CCHDEVICENAME+6));
	    lpdm = (DEVMODE *)GlobalLock(hDevMode);
		GetDEVMODE(lpdm,lpData);
	    GlobalUnlock(hDevMode);
	}
	else
	    hDevMode = (HGLOBAL)0;
	lppd->hDevMode = hDevMode;
	if (GETWORD(lpStruct+8)) {
	    lpData = (LPBYTE)GetPhysicalAddress(GETWORD(lpStruct+8));
	    wAllocSize = sizeof(DEVNAMES);
	    lpdn = (DEVNAMES *)WinMalloc(wAllocSize);
	    lpdn->wDriverOffset = GETWORD(lpData);
	    lpdn->wDeviceOffset = GETWORD(lpData+2);
	    lpdn->wOutputOffset = GETWORD(lpData+4);
	    lpdn->wDefault = GETWORD(lpData+6);
	    len1 = strlen((LPSTR)lpData+lpdn->wDriverOffset);
	    len2 = strlen((LPSTR)lpData+lpdn->wDeviceOffset);
	    len3 = strlen((LPSTR)lpData+lpdn->wOutputOffset);
	    wAllocSize += len1 + len2 + len3 + 3;
	    lpdn = (DEVNAMES *)WinRealloc((LPSTR)lpdn,wAllocSize);
	    hDevNames = GlobalAlloc(0,0);
	    lpMemory = GETHANDLEINFO(hDevNames);
	    lpMemory->lpCore = (LPSTR)lpdn;
	    lpMemory->dwSize = wAllocSize;
	    RELEASEHANDLEINFO(lpMemory);
	    ptr = (LPBYTE)lpdn + sizeof(DEVNAMES);
	    if (len1)
		strcpy((LPSTR)ptr,(LPSTR)lpData+lpdn->wDriverOffset);
	    else
		*ptr = '\0';
	    lpdn->wDriverOffset = (DWORD)ptr - (DWORD)lpdn;
	    ptr += len1 + 1;
	    if (len2)
		strcpy((LPSTR)ptr,(LPSTR)lpData+lpdn->wDeviceOffset);
	    else
		*ptr = '\0';
	    lpdn->wDeviceOffset = (DWORD)ptr - (DWORD)lpdn;
	    ptr += len2 + 1;
	    if (len3)
		strcpy((LPSTR)ptr,(LPSTR)lpData+lpdn->wOutputOffset);
	    else
		*ptr = '\0';
	    lpdn->wOutputOffset = (DWORD)ptr - (DWORD)lpdn;
	}
	else
	    hDevNames = (HGLOBAL)0;
	lppd->hDevNames = hDevNames;
	lppd->hDC = 0;
	dwFlags = GETDWORD(lpStruct+12);
	lppd->Flags = dwFlags;
	lppd->nFromPage = GETWORD(lpStruct+16);
	lppd->nToPage = GETWORD(lpStruct+18);
	lppd->nMinPage = GETWORD(lpStruct+20);
	lppd->nMaxPage = GETWORD(lpStruct+22);
	lppd->nCopies = GETWORD(lpStruct+24);
	lppd->lCustData = GETDWORD(lpStruct+28);
	if (dwFlags & PD_RETURNDEFAULT)
	    lppd->lpfnPrintHook = lppd->lpfnSetupHook = (LPFNHOOK)0;
	else {
	    dwProc = GETDWORD(lpStruct+32);
	    lppd->lpfnPrintHook = (LPFNHOOK)((dwProc)?make_native_thunk
			(dwProc,(DWORD)hsw_commdlg_nat_to_bin):0L);
	    dwProc = GETDWORD(lpStruct+36);
	    lppd->lpfnSetupHook = (LPFNHOOK)((dwProc)?make_native_thunk
			(dwProc,(DWORD)hsw_commdlg_nat_to_bin):0L);
	}
	lppd->hInstance = (HINSTANCE) GetSelectorHandle(GETWORD(lpStruct+26));
	if (dwFlags & PD_ENABLEPRINTTEMPLATE) {
	    lpString = (LPSTR)GetAddress
			(GETWORD(lpStruct+42),GETWORD(lpStruct+40));
	    lppd->lpPrintTemplateName = lpString;
	}
	if (dwFlags & PD_ENABLESETUPTEMPLATE) {
	    lpString = (LPSTR)GetAddress
			(GETWORD(lpStruct+46),GETWORD(lpStruct+44));
	    lppd->lpSetupTemplateName = lpString;
	}
	if (dwFlags & PD_ENABLEPRINTTEMPLATEHANDLE) {
	    lpData = GetPhysicalAddress(GETWORD(lpStruct+48));
	    lpPrintData = (LPBYTE)ExtractDialog(0,lpData);
	    hPrintTemplate = GlobalAlloc(0,0L);
	    lpMemory = GETHANDLEINFO(hPrintTemplate);
	    lpMemory->lpCore = (LPSTR)lpPrintData;
	    RELEASEHANDLEINFO(lpMemory);
	    lppd->hPrintTemplate = hPrintTemplate;
	}
	if (dwFlags & PD_ENABLESETUPTEMPLATEHANDLE) {
	    lpData = GetPhysicalAddress(GETWORD(lpStruct+50));
	    lpSetupData = (LPBYTE)ExtractDialog(0,lpData);
	    hSetupTemplate = GlobalAlloc(0,0L);
	    lpMemory = GETHANDLEINFO(hSetupTemplate);
	    lpMemory->lpCore = (LPSTR)lpSetupData;
	    RELEASEHANDLEINFO(lpMemory);
	    lppd->hSetupTemplate = hSetupTemplate;
	}
	retcode = (DWORD)(f)(lppd);
	if (hPrintTemplate) {
	    CleanupDialog(lpPrintData);
	    GlobalFree(hPrintTemplate);
	}
	if (hSetupTemplate) {
	    CleanupDialog(lpSetupData);
	    GlobalFree(hSetupTemplate);
	}
	PUTWORD(lpStruct+10,lppd->hDC);
	PUTDWORD(lpStruct+12,lppd->Flags);
	PUTWORD(lpStruct+16,lppd->nFromPage);
	PUTWORD(lpStruct+18,lppd->nToPage);
	PUTWORD(lpStruct+24,lppd->nCopies);

	if (lppd->hDevMode != hDevMode) {
	    lpdm = (DEVMODE *)GlobalLock(lppd->hDevMode);
	    wAllocSize = CCHDEVICENAME + 4*UINT_86 + DWORD_86 + 12*INT_86;
	    hGlobal = (HGLOBAL)GlobalAlloc(GHND,
			wAllocSize+lpdm->dmDriverExtra);
	    lpData = (LPBYTE)GlobalLock(hGlobal);
	    PutDEVMODE(lpData,lpdm);
	    GlobalUnlock(lppd->hDevMode);
	    wSel = ASSIGNSEL(lpData, wAllocSize + lpdm->dmDriverExtra);
	    SetSelectorHandle(wSel,hGlobal);
	    PUTWORD(lpStruct+6,wSel);
	}
	if (lppd->hDevMode)
		GlobalFree(lppd->hDevMode);

	if (lppd->hDevNames != hDevNames) {
	    lpdn = (DEVNAMES *)GlobalLock(lppd->hDevNames);
	    if (lpdn) {
		len1 = len2 = len3 = 0;
		if (lpdn->wDriverOffset)
		    len1 = strlen((LPSTR)lpdn+lpdn->wDriverOffset);
		if (lpdn->wDeviceOffset)
		    len2 = strlen((LPSTR)lpdn+lpdn->wDeviceOffset);
		if (lpdn->wOutputOffset)
		    len3 = strlen((LPSTR)lpdn+lpdn->wOutputOffset);
		wAllocSize = 4*UINT_86 + len1 + len2 + len3 + 3;
		hGlobal = GlobalAlloc(GHND,wAllocSize);
		lpData = (LPBYTE)GlobalLock(hGlobal);
		PUTWORD(lpData,4*UINT_86);
		PUTWORD(lpData+2,4*UINT_86+len1+1);
		PUTWORD(lpData+4,4*UINT_86+len1+len2+2);
		PUTWORD(lpData+6,lpdn->wDefault);
		ptr = lpData;
		ptr += 4*UINT_86;
		if (len1)
		    strcpy((LPSTR)ptr,(LPSTR)lpdn+lpdn->wDriverOffset);
		else 
		    *ptr = '\0';
		ptr += len1 + 1;
		if (len2)
		    strcpy((LPSTR)ptr,(LPSTR)lpdn+lpdn->wDeviceOffset);
		else 
		    *ptr = '\0';
		ptr += len2 + 1;
		if (len3)
		    strcpy((LPSTR)ptr,(LPSTR)lpdn+lpdn->wOutputOffset);
		else
		    *ptr = '\0';
		wSel = ASSIGNSEL(lpData, wAllocSize);
		SetSelectorHandle(wSel,hGlobal);
		PUTWORD(lpStruct+8,wSel);
	    }
	    GlobalUnlock(lppd->hDevNames);
	}
	if (lppd->hDevNames)
	    GlobalFree(lppd->hDevNames);

	WinFree((LPSTR)lppd - sizeof(LPVOID));

	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
	envp->reg.sp += LP_86 + RET_86;
}

void
IT_FINDREPLACE(ENV *envp, LONGPROC f)
{
	FINDREPLACE *lpfr;
	DWORD retcode;
	DWORD dwProc;
	LPBYTE lpStruct;
	LPBYTE lpData,lpDlgData;
	DWORD dwBinAddr;
	HGLOBAL hGlobal = 0;
	MEMORYINFO *lpMemory;
	WORD sel;

	dwBinAddr = GETDWORD(SP+4);
	lpStruct = (LPBYTE)GetAddress(HIWORD(dwBinAddr),LOWORD(dwBinAddr));
	lpfr = (FINDREPLACE *)((LPBYTE)WinMalloc(sizeof(FINDREPLACE) + sizeof(LPVOID))
				+ sizeof(LPVOID));
	*((BINADDR *)((LPBYTE)lpfr - sizeof(LPVOID))) = dwBinAddr;
	memset((LPSTR)lpfr,'\0',sizeof(FINDREPLACE));
	lpfr->lStructSize = sizeof(FINDREPLACE);
	lpfr->hwndOwner = (HWND)GETWORD(lpStruct+4);
	lpfr->Flags = GETDWORD(lpStruct+8);
	lpfr->lpstrFindWhat = (LPSTR)GetAddress
			(GETWORD(lpStruct+14),GETWORD(lpStruct+12));
	lpfr->lpstrReplaceWith = (LPSTR)GetAddress
			(GETWORD(lpStruct+18),GETWORD(lpStruct+16));
	lpfr->wFindWhatLen = GETWORD(lpStruct+20);
	lpfr->wReplaceWithLen = GETWORD(lpStruct+22);
	lpfr->lCustData = GETDWORD(lpStruct+24);
	if (lpfr->Flags & FR_ENABLEHOOK) {
	    dwProc = GETDWORD(lpStruct+28);
	    lpfr->lpfnHook = (LPFNHOOK)((dwProc)?make_native_thunk
			(dwProc,(DWORD)hsw_commdlg_nat_to_bin):0);
	}
	else
	    lpfr->lpfnHook = (LPFNHOOK)0L;
	if (lpfr->Flags & FR_ENABLETEMPLATEHANDLE) {
	    lpData = (LPBYTE)GetPhysicalAddress(GETWORD(lpStruct+6));
	    lpDlgData = (LPBYTE)ExtractDialog(0,lpData);
	    hGlobal = GlobalAlloc(0,0L);
	    lpMemory = GETHANDLEINFO(hGlobal);
	    lpMemory->lpCore = (LPSTR)lpDlgData;
	    RELEASEHANDLEINFO(lpMemory);
	    lpfr->hInstance = (HINSTANCE)hGlobal;
	}
	else if (lpfr->Flags & FR_ENABLETEMPLATE) {
	    lpfr->hInstance = (HINSTANCE)GetSelectorHandle(GETWORD(lpStruct+6));
	    lpfr->lpTemplateName = (LPCSTR)((sel = GETWORD(lpStruct+34))?
		GetAddress(sel,GETWORD(lpStruct+32)):
		(LPSTR)GETDWORD(lpStruct+32));
	}
	retcode = (DWORD)(f)(lpfr);
	WinFree((LPSTR)lpfr - sizeof(LPVOID));
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
	envp->reg.sp += LP_86 + RET_86;
}

void
IT_CHOOSECOLOR (ENV *envp,LONGPROC f)
{
	BINADDR dwBinAddr;
	CHOOSECOLOR *lpcc;
	DWORD retcode;
	LPBYTE lpStruct,lpData;
	LPBYTE lpDlgData = 0;
	HGLOBAL hGlobal = 0;
	COLORREF CustColors[16];
	MEMORYINFO *lpMemory;
	WORD sel;
	DWORD dwProc;
	int i;

	dwBinAddr = GETDWORD(SP+4);
	lpStruct = (LPBYTE)GetAddress(HIWORD(dwBinAddr),LOWORD(dwBinAddr));
	lpcc = (CHOOSECOLOR *)((LPBYTE)WinMalloc(sizeof(CHOOSECOLOR) + sizeof(LPVOID))
				+ sizeof(LPVOID));
	*((BINADDR *)((LPBYTE)lpcc - sizeof(LPVOID))) = dwBinAddr;
	memset((LPSTR)lpcc,'\0',sizeof(CHOOSECOLOR));

	lpcc->lStructSize = sizeof(CHOOSECOLOR);
	lpcc->Flags = GETDWORD(lpStruct+16);
	lpcc->hwndOwner = (HWND)GETWORD(lpStruct+4);
	if (lpcc->Flags & CC_ENABLETEMPLATEHANDLE) {
	    lpData = GetPhysicalAddress(GETWORD(lpStruct+6));
	    lpDlgData = (LPBYTE)ExtractDialog(0,lpData);
	    hGlobal = GlobalAlloc(0,0L);
	    lpMemory = GETHANDLEINFO(hGlobal);
	    lpMemory->lpCore = (LPSTR)lpDlgData;
	    RELEASEHANDLEINFO(lpMemory);
	    lpcc->hInstance = (HINSTANCE)hGlobal;
	}
	else if (lpcc->Flags & CC_ENABLETEMPLATE) {
	    lpcc->hInstance = (HINSTANCE) GetSelectorHandle(GETWORD(lpStruct+6));
	    lpcc->lpTemplateName = (LPSTR)((sel = GETWORD(lpStruct+30)) ? 
		GetAddress(sel,GETWORD(lpStruct+28)):
		(LPSTR)GETDWORD(lpStruct+28));
	}
	lpcc->rgbResult = (COLORREF)GETDWORD(lpStruct+8);
	lpData = (LPBYTE)GetAddress
			(GETWORD(lpStruct+14),GETWORD(lpStruct+12));
	for (i=0; i<16; i++)
	    CustColors[i] = (COLORREF)GETDWORD(lpData+4*i);
	lpcc->lpCustColors = CustColors;
	lpcc->lCustData = GETDWORD(lpStruct+20);
	if (lpcc->Flags & CC_ENABLEHOOK) {
	    dwProc = GETDWORD(lpStruct+24);
	    lpcc->lpfnHook = (LPFNHOOK)((dwProc)?make_native_thunk
			(dwProc,(DWORD)hsw_commdlg_nat_to_bin):0L);
	}

	retcode = (DWORD)(f)(lpcc);

	PUTDWORD(lpStruct+8,(DWORD)lpcc->rgbResult);

	for (i=0; i<16; i++)
	    PUTDWORD(lpData+4*i,(DWORD)CustColors[i]);

	if (hGlobal) {
	    CleanupDialog(lpDlgData);
	    GlobalFree(hGlobal);
	}

	WinFree((LPSTR)lpcc - sizeof(LPVOID));

	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CHOOSEFONT (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	CHOOSEFONT *lpcf;
	BINADDR dwBinAddr;
	LOGFONT lf;
	DWORD dwProc;
	LPBYTE lpStruct;
	LPBYTE lpData,lpDlgData = 0;
	HGLOBAL hGlobal = 0;
	MEMORYINFO *lpMemory;
	WORD sel;

	dwBinAddr = GETDWORD(SP+4);
	lpStruct = (LPBYTE)GetAddress(HIWORD(dwBinAddr),LOWORD(dwBinAddr));
	lpcf = (CHOOSEFONT *)((LPBYTE)WinMalloc(sizeof(CHOOSEFONT) + sizeof(LPVOID))
				+ sizeof(LPVOID));
	*((BINADDR *)((LPBYTE)lpcf - sizeof(LPVOID))) = dwBinAddr;
	memset((LPSTR)lpcf,'\0',sizeof(CHOOSEFONT));

	lpcf->Flags = GETDWORD(lpStruct+14);
	lpcf->lStructSize = sizeof(CHOOSEFONT);
	lpcf->hwndOwner = (HWND)GETWORD(lpStruct+4);
	lpcf->hDC = (HDC)GETWORD(lpStruct+6);
	lpData = (LPBYTE)GetAddress
			(GETWORD(lpStruct+10),GETWORD(lpStruct+8));
	if (lpcf->Flags & CF_INITTOLOGFONTSTRUCT) {
	    lf.lfHeight = (int)((short)GETWORD(lpData));
	    lf.lfWidth = (int)((short)GETWORD(lpData+2));
	    lf.lfEscapement = (int)((short)GETWORD(lpData+4));
	    lf.lfOrientation = (int)((short)GETWORD(lpData+6));
	    lf.lfWeight = (int)((short)GETWORD(lpData+8));
	    memcpy((LPSTR)&lf.lfItalic,(LPSTR)lpData+10,LF_FACESIZE+8);
	}
	else
	    memset((LPSTR)&lf,'\0',sizeof(LOGFONT));
	lpcf->lpLogFont = &lf;
	lpcf->iPointSize = (int)((short)GETWORD(lpStruct+12));
	lpcf->rgbColors = (COLORREF)GETDWORD(lpStruct+18);
	lpcf->lCustData = GETDWORD(lpStruct+22);
	if (lpcf->Flags & CF_ENABLEHOOK) 
	    lpcf->lpfnHook = (LPFNHOOK)((dwProc = GETDWORD(lpStruct+26))?
			make_native_thunk(dwProc,(DWORD)hsw_commdlg_nat_to_bin):
			0L);
	else
	    lpcf->lpfnHook = (LPFNHOOK)0L;
	if (lpcf->Flags & CF_ENABLETEMPLATE) {
	    lpcf->lpTemplateName = (LPCSTR)((sel = GETWORD(lpStruct+32))?
		GetAddress(sel,GETWORD(lpStruct+30)):
		(LPSTR)GETDWORD(lpStruct+30));
	    lpcf->hInstance = (HINSTANCE)GetSelectorHandle(GETWORD(lpStruct+34));
	}
	else if (lpcf->Flags & CF_ENABLETEMPLATEHANDLE) {
	    lpDlgData = GetPhysicalAddress(GETWORD(lpStruct+34));
	    lpDlgData = (LPBYTE)ExtractDialog(0,lpDlgData);
	    hGlobal = GlobalAlloc(0,0L);
	    lpMemory = GETHANDLEINFO(hGlobal);
	    lpMemory->lpCore = (LPSTR)lpDlgData;
	    RELEASEHANDLEINFO(lpMemory);
	    lpcf->hInstance = (HINSTANCE)hGlobal;
	}
	lpcf->lpszStyle = (LPSTR)GetAddress
			(GETWORD(lpStruct+38),GETWORD(lpStruct+36));
	lpcf->nFontType = GETWORD(lpStruct+40);
	lpcf->nSizeMin = (int)((short)GETWORD(lpStruct+42));
	lpcf->nSizeMax = (int)((short)GETWORD(lpStruct+44));
	retcode = (DWORD)(f)(lpcf);
	PUTWORD(lpData,(WORD)lf.lfHeight);
	PUTWORD(lpData+2,(WORD)lf.lfWidth);
	PUTWORD(lpData+4,(WORD)lf.lfEscapement);
	PUTWORD(lpData+6,(WORD)lf.lfOrientation);
	PUTWORD(lpData+8,(WORD)lf.lfWeight);
	memcpy((LPSTR)lpData+10,(LPSTR)&lf.lfItalic,LF_FACESIZE+8);
	PUTDWORD(lpStruct+18,lpcf->rgbColors);
	WinFree((LPSTR)lpcf - sizeof(LPVOID));
	if (hGlobal) {
	    CleanupDialog(lpDlgData);
	    GlobalFree(hGlobal);
	}
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LPOF (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	OPENFILENAME *lpofn;
	DWORD dwProc;
	LPBYTE lpData;
	LPBYTE lpDlgData = 0;
	HGLOBAL hGlobal = 0;
	MEMORYINFO *lpMemory;
	WORD sel;
	BINADDR lpofnbin;
	static char File[_MAX_PATH];
	LPSTR lpszFileName,lpTemp;

	lpofnbin = (BINADDR)GETDWORD(SP+4);
	lpStruct = (LPBYTE)GetAddress
			(HIWORD(lpofnbin),LOWORD(lpofnbin));
	lpData = (LPBYTE)WinMalloc(sizeof(OPENFILENAME) + sizeof(LPVOID));
	memset((LPSTR)lpData,'\0',sizeof(OPENFILENAME) + sizeof(LPVOID));
	lpofn = (OPENFILENAME *)(lpData + sizeof(LPVOID));
	*((BINADDR *)lpData) = lpofnbin;

	lpofn->lStructSize = sizeof(OPENFILENAME) + sizeof(LPVOID);
	lpofn->hwndOwner = (HWND)GETWORD(lpStruct+4);
	lpofn->lpstrFilter = (LPSTR)GetAddress
		(GETWORD(lpStruct+10),GETWORD(lpStruct+8));
	lpofn->lpstrCustomFilter = (LPSTR)GetAddress
		(GETWORD(lpStruct+14),GETWORD(lpStruct+12));
	lpofn->nMaxCustFilter = GETDWORD(lpStruct+16);
	lpofn->nFilterIndex = GETDWORD(lpStruct+20);
	lpofn->lpstrFile = &File[0];
	lpszFileName = (LPSTR)GetAddress
		(GETWORD(lpStruct+26),GETWORD(lpStruct+24));
	strcpy(lpofn->lpstrFile,lpszFileName);
	lpofn->nMaxFile = GETDWORD(lpStruct+28);
	lpofn->lpstrFileTitle = (LPSTR)GetAddress
		(GETWORD(lpStruct+34),GETWORD(lpStruct+32));
	lpofn->nMaxFileTitle = GETDWORD(lpStruct+36);
	lpofn->lpstrInitialDir = (LPSTR)GetAddress
		(GETWORD(lpStruct+42),GETWORD(lpStruct+40));
	lpofn->lpstrTitle = (LPSTR)GetAddress
		(GETWORD(lpStruct+46),GETWORD(lpStruct+44));
	lpofn->Flags = GETDWORD(lpStruct+48);
	lpofn->lpstrDefExt = (LPSTR)GetAddress
		(GETWORD(lpStruct+58),GETWORD(lpStruct+56));
	lpofn->lCustData = GETDWORD(lpStruct+60);
	if (lpofn->Flags & OFN_ENABLEHOOK) {
	    dwProc = GETDWORD(lpStruct+64);
	    lpofn->lpfnHook = (LPFNHOOK)((dwProc)?
		make_native_thunk(dwProc,(DWORD)hsw_commdlg_nat_to_bin):0L);
	}
	if (lpofn->Flags & OFN_ENABLETEMPLATEHANDLE) {
	    lpDlgData = GetPhysicalAddress(GETWORD(lpStruct+6));
	    lpDlgData = (LPBYTE)ExtractDialog(0,lpDlgData);
	    hGlobal = GlobalAlloc(0,0L);
	    lpMemory = GETHANDLEINFO(hGlobal);
	    lpMemory->lpCore = (LPSTR)lpDlgData;
	    RELEASEHANDLEINFO(lpMemory);
	    lpofn->hInstance = (HINSTANCE)hGlobal;
	}
	else if (lpofn->Flags & OFN_ENABLETEMPLATE) {
	    lpofn->hInstance = (HANDLE)GetSelectorHandle(GETWORD(lpStruct+6));
	    lpofn->lpTemplateName = (LPCSTR)((sel = GETWORD(lpStruct+70)) ?
		GetAddress(sel,GETWORD(lpStruct+68)):
		(LPSTR)GETDWORD(lpStruct+68));
	}
	retcode = (f)(lpofn);
	if (retcode) {
    	    xdoscall(XDOS_GETDOSPATH,0,(void *) lpszFileName,(void *) File);

	    if ((lpTemp = strpbrkr(lpszFileName,"\\")))
		lpofn->nFileOffset = lpTemp - lpszFileName + 1;
	    else {
		lpofn->nFileOffset = 0;
		lpTemp = lpszFileName;
	    }
	    if ((lpTemp = strpbrk(lpTemp,"."))) {
		if (*(lpTemp+1) == '\0')
		    lpofn->nFileExtension = 0;
		else
		    lpofn->nFileExtension = lpTemp - lpszFileName + 1;
	    }
	    else
		lpofn->nFileExtension = strlen(lpszFileName);

	    PUTWORD(lpStruct+52,(WORD)lpofn->nFileOffset);
	    PUTWORD(lpStruct+54,(WORD)lpofn->nFileExtension);
	}
	PUTDWORD(lpStruct+48,lpofn->Flags);
	WinFree((LPSTR)lpData);
	if (hGlobal) {
	    CleanupDialog(lpDlgData);
	    GlobalFree(hGlobal);
	}
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}
