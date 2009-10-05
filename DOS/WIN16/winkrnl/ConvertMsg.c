/*    
	ConvertMsg.c	2.25
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

#include <string.h>

#include "windows.h"
#include "windowsx.h"
#include "commdlg.h"
#include "kerndef.h"
#include "Endian.h"
#include "Kernel.h"
#include "Log.h"
#include "Dialog.h"
#include "DPMI.h"
#include "BinTypes.h"
#include "dos.h"
#include "WinHelp.h"

extern ENV *envp_global;
extern DSCR *LDT;

extern UINT GetTDBSelector(HTASK);
extern LPSTR strpbrkr(LPSTR,LPSTR);
extern LPINT hsw_ConvertArrayToInt(LPBYTE, int);
extern void invoke_binary(void);
extern void CleanupDialog(LPBYTE);

extern BOOL TestWF(HWND, DWORD);

extern ATOM atmGlobalLookup[];

#define BUFFSIZE	0x40

#define WM_FILEOK	WM_USER+0xcc1b
#define WM_FINDMSG	WM_FILEOK + 2
#define WM_SHAREVI	WM_FILEOK + 3
#define WM_LBSELCHANGE	WM_FILEOK + 4
#define WM_COLOROK	WM_FILEOK + 5
#define WM_SETRGB	WM_FILEOK + 6
#define WM_HELPMSG	WM_FILEOK + 7
#define WM_WINHELP	WM_FILEOK + 8

static struct wellknown_atoms_s
{
    ATOM atom;
    char *string;
    UINT message;
} 
wellknown_atoms[] =
{
    { 0, FILEOKSTRING, WM_FILEOK },
    { 0, FINDMSGSTRING, WM_FINDMSG },
    { 0, SHAREVISTRING, WM_SHAREVI },
    { 0, LBSELCHSTRING, WM_LBSELCHANGE },
    { 0, COLOROKSTRING, WM_COLOROK },
    { 0, SETRGBSTRING, WM_SETRGB },
    { 0, HELPMSGSTRING, WM_HELPMSG },
    { 0, MSWIN_HELP_MSG_STRING, WM_WINHELP },
    { 0, NULL, 0 },
};

/*
 * forward declaration of statics.
 */
static void TWIN_nat_to_bin_openfilename(OPENFILENAME *nat_ofn);
static void TWIN_bin_to_nat_openfilename(LPARAM lParam, char *pBuffer);

void
FailConversion(ENV *envp, LONGPROC f)
{
    FatalAppExit(0,"FailConversion should never be called. Mike.");
}

void
TWIN_InitializeConvertMsg()
{
    struct wellknown_atoms_s *wka;
    
    for (wka = wellknown_atoms; wka->string; wka++)
	wka->atom = GlobalAddAtom(wka->string);
}

LRESULT
hsw_common_nat_to_bin(HWND hWnd, UINT message, WPARAM w, LPARAM l)
{
	WMCONVERT *lpwmc = NULL;
	LPCREATESTRUCT lpcs;
	MDICREATESTRUCT * lpmcs;
	COMPAREITEMSTRUCT * lpcis;
	DELETEITEMSTRUCT * lpdeli;
	DRAWITEMSTRUCT * lpditm;
	MINMAXINFO * lpmmi;
	MEASUREITEMSTRUCT * lpmi;
	PAINTSTRUCT *lpps;
	WINDOWPOS * lpwp;
	FINDREPLACE *lpfr;
	RECT * lprc;
	LPBYTE lpData;
	FPHELPPARAMS lphp;
	HGLOBAL hGlobal;
	WORD sel;
	WORD Sel[8];
	BOOL fBinFlag = FALSE;
	DWORD dwBinAddr;
	DWORD dwTemp;
	UINT msg = message;
        HINSTANCE hinst;
        HINSTANCE oldds;
        HINSTANCE newds;
#ifdef LATER
	ATOM atmClass;
#endif

#ifdef SEVERE
    LOGSTR((LF_LOG,"nattobin: h=%x m=%x w=%x l=%x\n", hWnd, message, w, l));
    LOGSTR((LF_LOG,"nattobin: cs=%4.4x ax=%4.4x ds=%4.4x es=%4.4x ss=%4.4x\n",
	envp_global->reg.cs,
	envp_global->reg.ax,
	envp_global->reg.ds,
	envp_global->reg.es,
	envp_global->reg.ss));
#endif

    oldds = envp_global->reg.ds; 
    if(IsWindow(hWnd)) {
#ifdef TWIN32
	    hinst = GetWindowLong(hWnd,GWL_HINSTANCE);
#else
	    hinst = GetWindowWord(hWnd,GWW_HINSTANCE);
#endif
	    newds = GetDataSelectorFromInstance(hinst);
	    if(newds != envp_global->reg.ds)
		envp_global->reg.ds = newds;
    }

    envp_global->reg.sp -= 72;
    lpData = (LPBYTE)envp_global->reg.sp;
    envp_global->reg.sp -= HANDLE_86 + UINT_86 + WORD_86 + LONG_86;
    dwTemp = (DWORD)MAKELP(LOWORD(envp_global->reg.ss),
	(WORD)(lpData-GetPhysicalAddress((WORD)envp_global->reg.ss)));
    if (message == WM_CONVERT) {
	lpwmc = (WMCONVERT *)l;
	if (w & HSWMC_DEST_MASK) {
	    PUTWORD(lpData,hWnd);
	    PUTWORD(lpData+2,lpwmc->uMsg);
	    PUTWORD(lpData+4,lpwmc->wParam);
	    PUTDWORD(lpData+8,lpwmc->lParam);
	    PUTWORD((LPBYTE)envp_global->reg.sp+6,lpwmc->hook_code);
	    if (w & HSWMC_DEST_CALLWNDPROC) {
		PUTWORD((LPBYTE)envp_global->reg.sp+4,lpwmc->hook_wParam);
	    }
	    else {		/* HSWMC_DEST_FILTER */
		PUTWORD((LPBYTE)envp_global->reg.sp+4,0);
	    }
	    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
	    invoke_binary();
	    envp_global->reg.sp += 72;
	    envp_global->reg.ds = oldds;
	    return MAKELONG(envp_global->reg.ax,envp_global->reg.dx);
	}
	if (w & HSWMC_BINMSG)
	    fBinFlag = TRUE;
	message = lpwmc->uMsg;
	w = lpwmc->wParam;
	l = lpwmc->lParam;
    }

    if (message >= 0xc000) {
	struct wellknown_atoms_s *wka;

	for (wka = wellknown_atoms; wka->string; wka++) {
	    if (wka->atom == (ATOM) message) {
		message = wka->message;
		break;
	    }
	}
    }

    if (!fBinFlag) {
	PUTWORD((LPBYTE)envp_global->reg.sp+8,(WORD)hWnd);
	PUTWORD((LPBYTE)envp_global->reg.sp+6,(WORD)msg);
	PUTWORD((LPBYTE)envp_global->reg.sp+4,(WORD)w);

	switch (message) {
	case WM_CREATE:
	case WM_NCCREATE:
	    lpcs = (LPCREATESTRUCT)l;
	    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
	    Sel[0] = (HIWORD(lpcs->lpszClass))?
		AssignSelector((LPBYTE)lpcs->lpszClass,0,
			TRANSFER_DATA,(DWORD)strlen(lpcs->lpszClass)+2):
		LOWORD((DWORD)lpcs->lpszClass);
	    if (HIWORD(lpcs->lpszName)) {
		LPBYTE lpID;
		if (HIWORD(lpcs->lpszName) == 0xffff) { /* loword is icon id */
		    lpID = (LPBYTE)&dwTemp;		/* for static control*/
		    *lpID = 0xff;			/* with SS_ICON	     */
		    PUTWORD(lpID+1,LOWORD((DWORD)lpcs->lpszName));
		    Sel[1] = AssignSelector(lpID,0,TRANSFER_DATA,sizeof(DWORD)+2);
		}
		else
		    Sel[1] = AssignSelector((LPBYTE)lpcs->lpszName,0,
			TRANSFER_DATA,(DWORD)strlen(lpcs->lpszName)+2);
	    }
	    else
		Sel[1] = LOWORD((DWORD)lpcs->lpszName);
	    if (lpcs->hwndParent) {
#ifdef LATER
/*		atmClass = GetClassWord(lpcs->hwndParent,GCW_ATOM);
	        if ((atmClass == atmGlobalLookup[LOOKUP_MDICLIENT]) && */
#else
		if (TestWF(lpcs->hwndParent,WFMDICLIENT) &&
#endif
		    (lpcs->lpCreateParams)) {
		    lpmcs = (MDICREATESTRUCT *)lpcs->lpCreateParams;
		    PUTDWORD(lpData+34,MAKELONG(0,Sel[0]));
		    PUTDWORD(lpData+38,MAKELONG(0,Sel[1]));
		    PUTWORD(lpData+42,(WORD)lpmcs->hOwner);
		    PUTWORD(lpData+44,(WORD)lpmcs->x);
		    PUTWORD(lpData+46,(WORD)lpmcs->y);
		    PUTWORD(lpData+48,(WORD)lpmcs->cx);
		    PUTWORD(lpData+50,(WORD)lpmcs->cy);
		    PUTDWORD(lpData+52,lpmcs->style);
		    PUTDWORD(lpData+56,(DWORD)lpmcs->lParam);
		    dwTemp = (DWORD)MAKELP(LOWORD(envp_global->reg.ss),
		        (WORD)(lpData+34-GetPhysicalAddress
		        ((WORD)envp_global->reg.ss)));
		    PUTDWORD(lpData,dwTemp);
	        }
	        else
	            PUTDWORD(lpData,(DWORD)(lpcs->lpCreateParams));
	    }
	    else
	        PUTDWORD(lpData,(DWORD)(lpcs->lpCreateParams));
	    PUTWORD(lpData+4,
		    (WORD)GetDataSelectorFromInstance(lpcs->hInstance));
	    PUTWORD(lpData+6,(WORD)(lpcs->hMenu));
	    PUTWORD(lpData+8,(WORD)(lpcs->hwndParent));
	    PUTWORD(lpData+10,(WORD)((short)(lpcs->cy)));
	    PUTWORD(lpData+12,(WORD)((short)(lpcs->cx)));
	    PUTWORD(lpData+14,(WORD)((short)(lpcs->y)));
	    PUTWORD(lpData+16,(WORD)((short)(lpcs->x)));
	    PUTDWORD(lpData+18,(DWORD)(lpcs->style));
	    PUTDWORD(lpData+22,MAKELONG(0,Sel[1]));
	    PUTDWORD(lpData+26,MAKELONG(0,Sel[0]));
	    PUTDWORD(lpData+30,(DWORD)(lpcs->dwExStyle));

	    invoke_binary();

	    /* the only thing we need to take back is window style */
	    lpcs->style = GETDWORD(lpData+18);

	    if (HIWORD(lpcs->lpszClass))
		FreeSelector(Sel[0]);
	    if (HIWORD(lpcs->lpszName))
		FreeSelector(Sel[1]);
	    break;

	case WM_ASKCBFORMATNAME:
	case WM_GETTEXT:
	    Sel[0] = (HIWORD(l))?AssignSelector((LPBYTE)l,0,
					TRANSFER_DATA,(DWORD)w+2):
				LOWORD(l);
	    PUTDWORD((LPBYTE)envp_global->reg.sp,MAKELONG(0,Sel[0]));
	    invoke_binary();
	    if (HIWORD(l))
		FreeSelector(Sel[0]);
	    break;

	case WM_DEVMODECHANGE:
	case WM_SETTEXT:
	case WM_WININICHANGE:
	    Sel[0] = (HIWORD(l))?AssignSelector((LPBYTE)l,0,
					TRANSFER_DATA,
					(DWORD)strlen((LPSTR)l)+2):
				LOWORD(l);
	    PUTDWORD((LPBYTE)envp_global->reg.sp,MAKELONG(0,Sel[0]));
	    invoke_binary();
	    if (HIWORD(l))
		FreeSelector(Sel[0]);
	    break;

	case WM_COMPAREITEM:
	    lpcis = (COMPAREITEMSTRUCT *)l;
	    PUTWORD(lpData,(WORD)lpcis->CtlType);
	    PUTWORD(lpData+2,(WORD)lpcis->CtlID);
	    PUTWORD(lpData+4,(WORD)lpcis->hwndItem);
	    PUTWORD(lpData+6,(WORD)lpcis->itemID1);
	    PUTDWORD(lpData+8,(DWORD)lpcis->itemData1);
	    PUTWORD(lpData+12,(WORD)lpcis->itemID2);
	    PUTDWORD(lpData+14,(DWORD)lpcis->itemData2); 
	    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
	    invoke_binary();
	    break;
	case WM_DELETEITEM:
	    lpdeli = (DELETEITEMSTRUCT *)l;
	    PUTWORD(lpData,(WORD)lpdeli->CtlType);
	    PUTWORD(lpData+2,(WORD)lpdeli->CtlID);
	    PUTWORD(lpData+4,(WORD)lpdeli->itemID);
	    PUTWORD(lpData+6,(WORD)lpdeli->hwndItem);
	    PUTDWORD(lpData+8,(DWORD)lpdeli->itemData);
	    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
	    invoke_binary();
	    break;
	case WM_DRAWITEM:
	    lpditm = (DRAWITEMSTRUCT *)l;
	    PUTWORD(lpData,(WORD)lpditm->CtlType);
	    PUTWORD(lpData+2,(WORD)lpditm->CtlID);
	    PUTWORD(lpData+4,(WORD)lpditm->itemID);
	    PUTWORD(lpData+6,(WORD)lpditm->itemAction);
	    PUTWORD(lpData+8,(WORD)lpditm->itemState);
	    PUTWORD(lpData+10,(WORD)lpditm->hwndItem);
	    PUTWORD(lpData+12,(WORD)lpditm->hDC);
	    RECT_TO_86(lpData+14,lpditm->rcItem);
	    PUTDWORD(lpData+22,(DWORD)lpditm->itemData);
	    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
	    invoke_binary();
	    break;
	case WM_GETMINMAXINFO:
	    lpmmi = (MINMAXINFO *)l;
	    PutPOINT(lpData,lpmmi->ptReserved);
	    PutPOINT(lpData+4,lpmmi->ptMaxSize);
	    PutPOINT(lpData+8,lpmmi->ptMaxPosition);
	    PutPOINT(lpData+12,lpmmi->ptMinTrackSize);
	    PutPOINT(lpData+16,lpmmi->ptMaxTrackSize);
	    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
	    invoke_binary();
	    GetPOINT(lpmmi->ptReserved,lpData);
	    GetPOINT(lpmmi->ptMaxSize,lpData+4);
	    GetPOINT(lpmmi->ptMaxPosition,lpData+8);
	    GetPOINT(lpmmi->ptMinTrackSize,lpData+12);
	    GetPOINT(lpmmi->ptMaxTrackSize,lpData+16);
	    break;

	case WM_MDICREATE:
	    lpmcs = (MDICREATESTRUCT *)l;
	    Sel[0] = (HIWORD(lpmcs->szClass))?
		AssignSelector((LPBYTE)lpmcs->szClass,0,
			TRANSFER_DATA,(DWORD)strlen(lpmcs->szClass)+2):
		LOWORD((DWORD)lpmcs->szClass);
	    Sel[1] = (HIWORD(lpmcs->szTitle))?
		AssignSelector((LPBYTE)lpmcs->szTitle,0,
			TRANSFER_DATA,(DWORD)strlen(lpmcs->szTitle)+2):
		LOWORD((DWORD)lpmcs->szTitle);
	    PUTDWORD(lpData,MAKELONG(0,Sel[0]));
	    PUTDWORD(lpData+4,MAKELONG(0,Sel[1]));
	    PUTWORD(lpData+8,(WORD)lpmcs->hOwner);
	    PUTWORD(lpData+10,(WORD)lpmcs->x);
	    PUTWORD(lpData+12,(WORD)lpmcs->y);
	    PUTWORD(lpData+14,(WORD)lpmcs->cx);
	    PUTWORD(lpData+16,(WORD)lpmcs->cy);
	    PUTDWORD(lpData+18,lpmcs->style);
	    PUTDWORD(lpData+22,lpmcs->lParam);
	    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
	    invoke_binary();
	    if (HIWORD(lpmcs->szClass))
		FreeSelector(Sel[0]);
	    if (HIWORD(lpmcs->szTitle))
		FreeSelector(Sel[1]);
	    break;

	case WM_MEASUREITEM:
	    lpmi = (MEASUREITEMSTRUCT *)l;
	    PUTWORD(lpData,(WORD)lpmi->CtlType);
	    PUTWORD(lpData+2,(WORD)lpmi->CtlID);
	    PUTWORD(lpData+4,(WORD)lpmi->itemID);
	    PUTWORD(lpData+6,(WORD)lpmi->itemWidth);
	    PUTWORD(lpData+8,(WORD)lpmi->itemHeight);
	    PUTDWORD(lpData+10,(DWORD)lpmi->itemData);
	    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
	    invoke_binary();
	    lpmi->itemWidth = (UINT)((short)GETWORD(lpData+6));
	    lpmi->itemHeight = (UINT)((short)GETWORD(lpData+8));
	    break;
	case WM_NCCALCSIZE:
	    lprc = (RECT *)l;
	    RECT_TO_86(lpData,(*lprc));
#ifdef LATER
	    if (w == FALSE)
		/*	Convert NCCALCSIZE_PARAMS structure */ ;
#endif
	    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
	    invoke_binary();
	    RECT_TO_C((*lprc),lpData);
	    break;
	case WM_PAINTCLIPBOARD:
	    hGlobal = (HGLOBAL)(LOWORD(l));
	    lpps = (PAINTSTRUCT *)GlobalLock(hGlobal);
	    GlobalUnlock(hGlobal);
	    hGlobal = GlobalAlloc(GHND,0x20);
	    lpData = (LPBYTE)GlobalLock(hGlobal);
	    PUTWORD(lpData,(WORD)lpps->hdc);
	    PUTWORD(lpData+2,(WORD)lpps->fErase);
	    RECT_TO_86(lpData+4,lpps->rcPaint);
	    PUTWORD(lpData+12,(WORD)lpps->fRestore);
	    PUTWORD(lpData+14,(WORD)lpps->fIncUpdate);
	    memcpy((LPSTR)lpData+16,(LPSTR)&lpps->rgbReserved,16);
	    GlobalUnlock(hGlobal);
	    sel = AssignSelector(lpData,0,TRANSFER_DATA,0x20+2);
	    SetSelectorHandle(sel,hGlobal);
	    PUTDWORD((LPBYTE)envp_global->reg.sp,(DWORD)MAKELP(sel,0));
	    invoke_binary();
	    GlobalFree(hGlobal);
	    FreeSelector(sel);
	    break;
	case WM_WINDOWPOSCHANGED:
	    lpwp = (WINDOWPOS *)l;
	    PUTWORD(lpData,(WORD)lpwp->hwnd);
	    PUTWORD(lpData+2,(WORD)lpwp->hwndInsertAfter);
	    PUTWORD(lpData+4,(WORD)lpwp->x);
	    PUTWORD(lpData+6,(WORD)lpwp->y);
	    PUTWORD(lpData+8,(WORD)lpwp->cx);
	    PUTWORD(lpData+10,(WORD)lpwp->cy);
	    PUTWORD(lpData+12,(WORD)lpwp->flags);
	    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
	    invoke_binary();
	    break;
	case WM_WINDOWPOSCHANGING:
	    lpwp = (WINDOWPOS *)l;
	    PUTWORD(lpData,(WORD)lpwp->hwnd);
	    PUTWORD(lpData+2,(WORD)lpwp->hwndInsertAfter);
	    PUTWORD(lpData+4,(WORD)lpwp->x);
	    PUTWORD(lpData+6,(WORD)lpwp->y);
	    PUTWORD(lpData+8,(WORD)lpwp->cx);
	    PUTWORD(lpData+10,(WORD)lpwp->cy);
	    PUTWORD(lpData+12,(WORD)lpwp->flags);
	    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
	    invoke_binary();
	    lpwp->hwnd = (HWND)GETWORD(lpData);
	    lpwp->hwndInsertAfter = (HWND)GETWORD(lpData+2);
	    lpwp->x = (int)(short)GETWORD(lpData+4);
	    lpwp->y = (int)(short)GETWORD(lpData+6);
	    lpwp->cx = (int)(short)GETWORD(lpData+8);
	    lpwp->cy = (int)(short)GETWORD(lpData+10);
	    lpwp->flags = (UINT)GETWORD(lpData+12);
	    break;
	case WM_ACTIVATEAPP:
	    sel = GetTDBSelector((HTASK)LOWORD(l));
	    PUTDWORD((LPBYTE)envp_global->reg.sp,(DWORD)sel);
	    invoke_binary();
	    break;
#ifdef LATER
	    message = WM_SHAREVI;
	    message = WM_LBSELCHANGE;
	    message = WM_COLOROK;
	    message = WM_SETRGB;
	    message = WM_HELPMSG;
#endif
	case WM_FILEOK:
	    TWIN_nat_to_bin_openfilename((OPENFILENAME *) l);
	    break;
	case WM_FINDMSG:
	    dwBinAddr = (DWORD)(*((BINADDR *)((LPBYTE)l - sizeof(LPVOID))));
	    lpData = (LPBYTE)GetAddress
			(HIWORD(dwBinAddr),LOWORD(dwBinAddr));
	    lpfr = (FINDREPLACE *)l;
	    PUTDWORD(lpData+8,lpfr->Flags);
	    if (lpfr->Flags & FR_DIALOGTERM) {
		if (lpfr->Flags & FR_ENABLETEMPLATEHANDLE) {
		    CleanupDialog((LPBYTE)GlobalLock(lpfr->hInstance));
		    GlobalUnlock(lpfr->hInstance);
		    GlobalFree(lpfr->hInstance);
		}
		WinFree((LPSTR)l - sizeof(LPVOID));
	    }
	    PUTDWORD((LPBYTE)envp_global->reg.sp,dwBinAddr);
	    invoke_binary();
	    break;

	case WM_WINHELP:
	    lphp = (FPHELPPARAMS) GlobalLock((GLOBALHANDLE) l);
	    dwTemp = GlobalAlloc(0, lphp->Size);
	    lpData = (LPBYTE)GlobalLock(dwTemp);
	    Sel[0] = AssignSelector(lpData, 0, TRANSFER_DATA, lphp->Size);
	    SetSelectorHandle(Sel[0], (HGLOBAL) dwTemp);
	    
	    PUTWORD(lpData + 0, lphp->Size);
	    PUTWORD(lpData + 2, lphp->Message);
	    PUTDWORD(lpData + 4, lphp->Context);
	    PUTDWORD(lpData + 8, lphp->Unknown);
	    PUTWORD(lpData + 12, lphp->PathOffset);
	    PUTWORD(lpData + 14, lphp->ExtraDataOffset);
	    memcpy(lpData + 16, lphp->Path, lphp->Size - 16);

	    GlobalUnlock((GLOBALHANDLE) l);

	    PUTDWORD((LPBYTE)envp_global->reg.sp, Sel[0]);
	    invoke_binary();

	    FreeSelector(Sel[0]);
	    GlobalUnlock(dwTemp);
	    GlobalFree(dwTemp);

	    break;
#ifdef TWIN32
	case WM_ACTIVATE:
	case WM_CHARTOITEM:
	case WM_COMMAND:
	case WM_MENUCHAR:
	case WM_VKEYTOITEM:
	    PUTWORD((LPBYTE)envp_global->reg.sp+4,LOWORD(w));
	    PUTDWORD((LPBYTE)envp_global->reg.sp,MAKELONG((WORD)l,HIWORD(w)));
	    invoke_binary();
	    break;
	case WM_MENUSELECT:
	case WM_HSCROLL:
	case WM_VSCROLL:
	    PUTWORD((LPBYTE)envp_global->reg.sp+4,LOWORD(w));
	    PUTDWORD((LPBYTE)envp_global->reg.sp,MAKELONG(HIWORD(w),(WORD)l));
	    invoke_binary();
	    break;
	case WM_MDIACTIVATE:
	    PUTWORD((LPBYTE)envp_global->reg.sp+4,(WORD)(hWnd == (HWND)l))
	    PUTDWORD((LPBYTE)envp_global->reg.sp,MAKELONG(LOWORD(l),LOWORD(w)));
	    invoke_binary();
	    break;
	case WM_MDISETMENU:
	    PUTWORD((LPBYTE)envp_global->reg.sp+4,0);
	    PUTDWORD((LPBYTE)envp_global->reg.sp,MAKELONG(LOWORD(w),LOWORD(l)));
	    invoke_binary();
	    break;
	case WM_PARENTNOTIFY:
	    PUTWORD((LPBYTE)envp_global->reg.sp+4,LOWORD(w));
	    if ((LOWORD(w) == WM_CREATE) || (LOWORD(w) == WM_DESTROY)) {
		PUTDWORD((LPBYTE)envp_global->reg.sp,
			MAKELONG(LOWORD(l),HIWORD(w)));
	    }
	    else
		PUTDWORD((LPBYTE)envp_global->reg.sp,l);
	    invoke_binary();
	    break;
	case WM_CHANGECBCHAIN:
	    PUTDWORD((LPBYTE)envp_global->reg.sp,MAKELONG(LOWORD(l),0));
	    invoke_binary();
	    break;
	case WM_CTLCOLORMSGBOX:
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORLISTBOX:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORDLG:
	case WM_CTLCOLORSCROLLBAR:
	case WM_CTLCOLORSTATIC:
	    PUTWORD((LPBYTE)envp_global->reg.sp+6,WM_CTLCOLOR);
	    PUTDWORD((LPBYTE)envp_global->reg.sp,MAKELONG(LOWORD(l),
			message - WM_CTLCOLORMSGBOX));
	    invoke_binary();
	    break;
#endif

	default:
	    PUTDWORD((LPBYTE)envp_global->reg.sp,(DWORD)l);
	    invoke_binary();
	    break;
	} /* switch */
    } /* if !BinFlag */
    else {
	PUTWORD((LPBYTE)envp_global->reg.sp+8,(WORD)hWnd);
	PUTWORD((LPBYTE)envp_global->reg.sp+6,(WORD)lpwmc->uMsg);
	PUTWORD((LPBYTE)envp_global->reg.sp+4,(WORD)w);
	PUTDWORD((LPBYTE)envp_global->reg.sp,(DWORD)l);
	invoke_binary();
    }
    envp_global->reg.sp += 72;
    envp_global->reg.ds = oldds;
    return MAKELONG(envp_global->reg.ax,envp_global->reg.dx);
}

LRESULT
hsw_common_bin_to_nat(HWND hWnd, UINT uiMsg, WPARAM w, LPARAM l)
{
    UINT	uiMessage;
    WMCONVERT *lpwmc;
    LPBYTE lpStruct=0;
    COMPAREITEMSTRUCT cis;
    DELETEITEMSTRUCT deli;
    DRAWITEMSTRUCT ditm;
    RECT rcRect;
    MINMAXINFO mmi;
    MEASUREITEMSTRUCT mi;
    WINDOWPOS wpos;
    CREATESTRUCT cs;
    MDICREATESTRUCT mdic;
    char ofn[sizeof(OPENFILENAME) + 4];
    MSG msg;
    DWORD retcode;
    WPARAM wParam;
    LPARAM lParam;
    WORD w1,w2,w3,w4; 
#ifdef LATER
    CLIENTCREATESTRUCT ccs;
#endif

    if (uiMsg != WM_CONVERT)
	    FatalAppExit(0,
		"non-WM_CONVERT message in hsw_common_bin_to_nat\n");
    lpwmc = (WMCONVERT *)l;
    uiMessage = lpwmc->uMsg;
    wParam = lpwmc->wParam;
    lParam = lpwmc->lParam;

    if (uiMessage >= 0xc000) {
	struct wellknown_atoms_s *wka;

	for (wka = wellknown_atoms; wka->string; wka++) {
	    if (wka->atom == (ATOM) uiMessage) {
		uiMessage = wka->message;
		break;
	    }
	}
    }

    switch(uiMessage) {
	case WM_FILEOK:
	    TWIN_bin_to_nat_openfilename(lParam, ofn);
	    lParam = (LPARAM) (ofn + 4);
	    break;
	case WM_ASKCBFORMATNAME:          /* lParam - string address */
	case WM_DEVMODECHANGE:
	case WM_GETTEXT:
	case WM_SETTEXT:
	case WM_WININICHANGE:
	    lParam = (LPARAM)GetAddress(HIWORD(lParam),LOWORD(lParam));
	    break;
	case WM_CREATE:
	case WM_NCCREATE:
	    lpStruct = (LPBYTE)GetAddress
				(HIWORD(lParam),LOWORD(lParam));
	    cs.hInstance = (HANDLE)GetSelectorHandle(GETWORD(lpStruct+4));
	    cs.hMenu = (HANDLE)GETWORD(lpStruct+6);
	    cs.hwndParent = (HANDLE)GETWORD(lpStruct+8);
	    w1 = GETWORD(lpStruct+10);
	    w2 = GETWORD(lpStruct+12);
	    w3 = GETWORD(lpStruct+14);
	    w4 = GETWORD(lpStruct+16);
	    cs.cy = (w1 == CW_USEDEFAULT16)?(int)CW_USEDEFAULT:(int)((short)w1);
	    cs.cx = (w2 == CW_USEDEFAULT16)?(int)CW_USEDEFAULT:(int)((short)w2);
	    cs.y = (w3 == CW_USEDEFAULT16)?(int)CW_USEDEFAULT:(int)((short)w3);
	    cs.x = (w4 == CW_USEDEFAULT16)?(int)CW_USEDEFAULT:(int)((short)w4);
	    cs.style = GETDWORD(lpStruct+18);
	    cs.lpszName = (LPSTR)GetAddress(
				GETWORD(lpStruct+24),GETWORD(lpStruct+22));
	    cs.lpszClass = (LPSTR)GetAddress(
				GETWORD(lpStruct+28),GETWORD(lpStruct+26));
#ifdef LATER
	/* From now on this stuff is being taken care of in 	*/
	/* hsw_mdiclient_bin_to_nat				*/

	    if (strcasecmp(cs.lpszClass,"MDICLIENT") == 0) {
		lpStruct = (LPBYTE)GetAddress
				(GETWORD(lpStruct+2),GETWORD(lpStruct));
		ccs.hWindowMenu = (HANDLE)GETWORD(lpStruct);
		ccs.idFirstChild = (UINT)GETWORD(lpStruct+2);
		cs.lpCreateParams = &ccs;
	    }
	    else
#endif
		cs.lpCreateParams = (LPBYTE)GETDWORD(lpStruct);
	    lParam = (LPARAM)(&cs);
	    break;
	case WM_COMPAREITEM:
	    lpStruct = (LPBYTE)GetAddress
				(HIWORD(lParam),LOWORD(lParam));
	    w1 = GETWORD(lpStruct);
	    cis.CtlType = (w1 == (WORD)-1)?(UINT)-1:(UINT)w1;
	    w1 = GETWORD(lpStruct+2);
	    cis.CtlID   = (w1 == (WORD)-1)?(UINT)-1:(UINT)w1;
	    cis.hwndItem = (HWND)GETWORD(lpStruct+4);
	    w1 = GETWORD(lpStruct+6);
	    cis.itemID1 = (w1 == (WORD)-1)?(UINT)-1:(UINT)w1;
	    cis.itemData1 = GETDWORD(lpStruct+8);
	    w1 = GETWORD(lpStruct+12);
	    cis.itemID2 = (w1 == (WORD)-1)?(UINT)-1:(UINT)w1;
	    cis.itemData2 = GETDWORD(lpStruct+14);
	    lParam = (LPARAM)(&cis);
	    break;
	case WM_DELETEITEM:
	    lpStruct = (LPBYTE)GetAddress
				(HIWORD(lParam),LOWORD(lParam));
	    w1 = GETWORD(lpStruct);
	    deli.CtlType = (w1 == (WORD)-1)?(UINT)-1:(UINT)w1;
	    w1 = GETWORD(lpStruct+2);
	    deli.CtlID = (w1 == (WORD)-1)?(UINT)-1:(UINT)w1;
	    w1 = GETWORD(lpStruct+4);
	    deli.itemID = (w1 == (WORD)-1)?(UINT)-1:(UINT)w1;
	    deli.hwndItem = (HWND)GETWORD(lpStruct+6);
	    deli.itemData = GETDWORD(lpStruct+8);
	    lParam = (LPARAM)(&deli);
	    break;
	case WM_DRAWITEM:
	    lpStruct = (LPBYTE)GetAddress
				(HIWORD(lParam),LOWORD(lParam));
	    w1 = GETWORD(lpStruct);
	    ditm.CtlType = (w1 == (WORD)-1)?(UINT)-1:(UINT)w1;
	    w1 = GETWORD(lpStruct+2);
	    ditm.CtlID = (w1 == (WORD)-1)?(UINT)-1:(UINT)w1;
	    w1 = GETWORD(lpStruct+4);
	    ditm.itemID = (w1 == (WORD)-1)?(UINT)-1:(UINT)w1;
	    ditm.itemAction = (UINT)GETWORD(lpStruct+6);
	    ditm.itemState = (UINT)GETWORD(lpStruct+8);
	    ditm.hwndItem = (HWND)GETWORD(lpStruct+10);
	    ditm.hDC = (HDC)GETWORD(lpStruct+12);
	    RECT_TO_C(ditm.rcItem,lpStruct+14);
	    ditm.itemData = GETDWORD(lpStruct+22);
	    lParam = (LPARAM)(&ditm);
	    break;
	case WM_GETMINMAXINFO:
	    lpStruct = (LPBYTE)GetAddress
				(HIWORD(lParam),LOWORD(lParam));
	    GetPOINT(mmi.ptReserved,lpStruct);
	    GetPOINT(mmi.ptMaxSize,lpStruct+4);
	    GetPOINT(mmi.ptMaxPosition,lpStruct+8);
	    GetPOINT(mmi.ptMinTrackSize,lpStruct+12);
	    GetPOINT(mmi.ptMaxTrackSize,lpStruct+16);
	    lParam = (LPARAM)(&mmi);
	    break;
#ifdef WEIRD
	case WM_PAINTCLIPBOARD:
#endif
	case WM_MDICREATE:
	    lpStruct = (LPBYTE)GetAddress
				(HIWORD(lParam),LOWORD(lParam));
	    mdic.szClass = (LPSTR)GetAddress
			(GETWORD(lpStruct+2),GETWORD(lpStruct));
	    mdic.szTitle = (LPSTR)GetAddress
			(GETWORD(lpStruct+6),GETWORD(lpStruct+4));
	    mdic.hOwner = (HINSTANCE)GetSelectorHandle(GETWORD(lpStruct+8));
	    w1 = GETWORD(lpStruct+10);
	    w2 = GETWORD(lpStruct+12);
	    w3 = GETWORD(lpStruct+14);
	    w4 = GETWORD(lpStruct+16);
	    mdic.x = (w1 == CW_USEDEFAULT16)?
			(int)CW_USEDEFAULT:(int)((short)w1);
	    mdic.y = (w2 == CW_USEDEFAULT16)?
			(int)CW_USEDEFAULT:(int)((short)w2);
	    mdic.cx = (w3 == CW_USEDEFAULT16)?
			(int)CW_USEDEFAULT:(int)((short)w3);
	    mdic.cy = (w4 == CW_USEDEFAULT16)?
			(int)CW_USEDEFAULT:(int)((short)w4);
	    mdic.style = GETDWORD(lpStruct+18);
	    mdic.lParam = (LPARAM)GETDWORD(lpStruct+22);
	    lParam = (LPARAM)(&mdic);
	    break;
	case WM_MEASUREITEM:
	    lpStruct = (LPBYTE)GetAddress
				(HIWORD(lParam),LOWORD(lParam));
	    w1 = GETWORD(lpStruct);
	    mi.CtlType = (w1 == (WORD)-1)?(UINT)-1:(UINT)w1;
	    w1 = GETWORD(lpStruct+2);
	    mi.CtlID = (w1 == (WORD)-1)?(UINT)-1:(UINT)w1;
	    w1 = GETWORD(lpStruct+4);
	    mi.itemID = (w1 == (WORD)-1)?(UINT)-1:(UINT)w1;
	    mi.itemWidth = (UINT)GETWORD(lpStruct+6);
	    mi.itemHeight = (UINT)GETWORD(lpStruct+8);
	    mi.itemData = GETDWORD(lpStruct+10);
	    lParam = (LPARAM)(&mi);
	    break;
	case WM_NCCALCSIZE:
	    lpStruct = (LPBYTE)GetAddress
				(HIWORD(lParam),LOWORD(lParam));
	    RECT_TO_C(rcRect,lpStruct);
#ifdef LATER
	    if ((WORD)GETWORD(SP+8) != FALSE)
		/* Convert NCCALCSIZE_PARAMS structure */ ;
#endif
	    lParam = (LPARAM)(&rcRect);
	    break;
	case WM_WINDOWPOSCHANGED:
	case WM_WINDOWPOSCHANGING:
	    lpStruct = (LPBYTE)GetAddress
				(HIWORD(lParam),LOWORD(lParam));
	    wpos.hwnd = (HWND)GETWORD(lpStruct);
	    wpos.hwndInsertAfter = (HWND)GETWORD(lpStruct+2);
	    wpos.x = (int)((short)GETWORD(lpStruct+4));
	    wpos.y = (int)((short)GETWORD(lpStruct+6));
	    wpos.cx = (int)((short)GETWORD(lpStruct+8));
	    wpos.cy = (int)((short)GETWORD(lpStruct+10));
	    wpos.flags = (UINT)GETWORD(lpStruct+12);
	    lParam = (LPARAM)(&wpos);
	    break;
	case WM_ACTIVATEAPP:
	    lParam = (LPARAM)GetSelectorHandle(lParam & 0xffff);
	    break;
#ifdef TWIN32
	case WM_ACTIVATE:
	case WM_CHARTOITEM:
	case WM_COMMAND:
	case WM_MENUCHAR:
	case WM_VKEYTOITEM:
	    wParam = (WPARAM)MAKELONG(wParam,HIWORD(lParam));
	    lParam = (LPARAM)LOWORD(lParam);
	    break;
	case WM_MENUSELECT:
	case WM_HSCROLL:
	case WM_VSCROLL:
	    wParam = (WPARAM)MAKELONG(wParam,LOWORD(lParam));
	    lParam = (LPARAM)HIWORD(lParam);
	    break;
	case WM_MDIACTIVATE:
	    wParam = (WPARAM)HIWORD(lParam);
	    lParam = (LPARAM)LOWORD(lParam);
	    break;
	case WM_MDISETMENU:
	    wParam = (WPARAM)LOWORD(lParam);
	    lParam = (LPARAM)HIWORD(lParam);
	    break;
	case WM_PARENTNOTIFY:
	    if (wParam == WM_CREATE || wParam == WM_DESTROY) {
		wParam = (WPARAM)MAKELONG(wParam,HIWORD(lParam));
		lParam = (LPARAM)LOWORD(lParam);
	    }
	    else
		wParam = (WPARAM)MAKELONG(0, wParam); /* lParam is (x,y) */
	    break;
	case WM_CHANGECBCHAIN:
	    lParam = (LPARAM)LOWORD(lParam);
	    break;
	case WM_CTLCOLOR:
	    uiMessage = WM_CTLCOLORMSGBOX + HIWORD(lParam);
	    lParam = (LPARAM)LOWORD(lParam);
	    break;
#endif
	default:
	    break;
    }

    if (!(w & HSWMC_DEST_MASK))		/* HSWMC_DEST_WINDOW_PROC */ 
	retcode = (DWORD)(lpwmc->targ)(hWnd,uiMessage,wParam,lParam);
    else 
	if ((w & HSWMC_DEST_MASK) == HSWMC_DEST_DEF_FRAME)
	    retcode = (DWORD)(lpwmc->targ)(hWnd,(HWND)lpwmc->hook_wParam,
				uiMessage,wParam,lParam);
	else {
	    msg.hwnd = hWnd;
	    msg.message = uiMessage;
	    msg.wParam = wParam;
	    msg.lParam = lParam;
	    if (w & HSWMC_DEST_CALLWNDPROC)
		retcode = (DWORD)(lpwmc->targ)(lpwmc->hook_code,
				lpwmc->hook_wParam,&msg);
	    else            /* HSWMC_DEST_FILTER */
		retcode = (DWORD)(lpwmc->targ)(lpwmc->hook_code,NULL,&msg);
	}

    switch(uiMessage) {
	case WM_CREATE:
	case WM_NCCREATE:
	    PUTDWORD(lpStruct+18,cs.style);
	    break;
	case WM_GETMINMAXINFO:
	    PutPOINT(lpStruct,mmi.ptReserved);
	    PutPOINT(lpStruct+4,mmi.ptMaxSize);
	    PutPOINT(lpStruct+8,mmi.ptMaxPosition);
	    PutPOINT(lpStruct+12,mmi.ptMinTrackSize);
	    PutPOINT(lpStruct+16,mmi.ptMaxTrackSize);
	    break;
	case WM_NCCALCSIZE:
	    RECT_TO_86(lpStruct,rcRect);
	    break;
	case WM_WINDOWPOSCHANGING:
	    PUTWORD(lpStruct,(WORD)wpos.hwnd);
	    PUTWORD(lpStruct+2,(WORD)wpos.hwndInsertAfter);
	    PUTWORD(lpStruct+4,(WORD)wpos.x);
	    PUTWORD(lpStruct+6,(WORD)wpos.y);
	    PUTWORD(lpStruct+8,(WORD)wpos.cx);
	    PUTWORD(lpStruct+10,(WORD)wpos.cy);
	    PUTWORD(lpStruct+12,(WORD)wpos.flags);
	    break;
	default:
	    break;
    }
    return retcode;
}
 
void
hsw_encapsulate_nat_to_bin(LPBYTE lpMsgBin, LPMSG lpMsg)
{
#define	POOLSIZE	16
#define	POOLMASK	0x000f

    static WMCONVERT ConvPool[POOLSIZE];
    static int nPoolIndex = 0;
    WMCONVERT *lpConvert;

    lpConvert = &ConvPool[nPoolIndex];
    lpConvert->uMsg = lpMsg->message;
    lpConvert->wParam = lpMsg->wParam;
    lpConvert->lParam = lpMsg->lParam;
    lpConvert->targ = (LONGPROC)NULL;

    PUTWORD(lpMsgBin+2,WM_CONVERT);
    PUTWORD(lpMsgBin+4,HSWMC_NATMSG);
    PUTDWORD(lpMsgBin+6,(LPARAM)lpConvert);

    nPoolIndex = (nPoolIndex + 1) & POOLMASK;
}

void
hsw_getmessage_nat_to_bin(LPBYTE lpMsgBin, LPMSG lpMsg)
{

	if (lpMsg->message == WM_CONVERT) {
	    if (!(lpMsg->wParam & HSWMC_BINMSG))
		FatalAppExit(0,"non-binary WM_CONVERT in IT_GETMESSAGE\n");
	    else {
		PUTWORD(lpMsgBin,(HANDLE)lpMsg->hwnd);
		PUTWORD(lpMsgBin+2,
			(UINT)(((WMCONVERT *)lpMsg->lParam)->uMsg));
		PUTWORD(lpMsgBin+4,
			(WPARAM)(((WMCONVERT *)lpMsg->lParam)->wParam));
		PUTDWORD(lpMsgBin+6,
			(LPARAM)(((WMCONVERT *)lpMsg->lParam)->lParam));
		PUTDWORD(lpMsgBin+10,(DWORD)lpMsg->time);
		PutPOINT(lpMsgBin+14,lpMsg->pt);
	    }
	}
	else {
	    PUTWORD(lpMsgBin,(HANDLE)lpMsg->hwnd);
	    PUTWORD(lpMsgBin+2,(UINT)lpMsg->message);
	    PUTWORD(lpMsgBin+4,(WPARAM)lpMsg->wParam);
	    PUTDWORD(lpMsgBin+6,(LPARAM)lpMsg->lParam);
	    PUTDWORD(lpMsgBin+10,(DWORD)lpMsg->time);
	    PutPOINT(lpMsgBin+14,lpMsg->pt);

	    if (HIWORD(lpMsg->lParam))
		switch (lpMsg->message) {
		case WM_CREATE:
		case WM_NCCREATE:
			hsw_encapsulate_nat_to_bin(lpMsgBin,lpMsg);
			break;

		case WM_ASKCBFORMATNAME:
		case WM_DEVMODECHANGE:
		case WM_GETTEXT:
		case WM_SETTEXT:
		case WM_WININICHANGE:
			hsw_encapsulate_nat_to_bin(lpMsgBin,lpMsg);
			break;
		case WM_COMPAREITEM:
			hsw_encapsulate_nat_to_bin(lpMsgBin,lpMsg);
			break;
		case WM_DELETEITEM:
			hsw_encapsulate_nat_to_bin(lpMsgBin,lpMsg);
			break;
		case WM_DRAWITEM:
			hsw_encapsulate_nat_to_bin(lpMsgBin,lpMsg);
			break;
		case WM_GETMINMAXINFO:
			hsw_encapsulate_nat_to_bin(lpMsgBin,lpMsg);
			break;
		case WM_MDICREATE:
			hsw_encapsulate_nat_to_bin(lpMsgBin,lpMsg);
			break;
		case WM_MEASUREITEM:
			hsw_encapsulate_nat_to_bin(lpMsgBin,lpMsg);
			break;
		case WM_NCCALCSIZE:
			hsw_encapsulate_nat_to_bin(lpMsgBin,lpMsg);
			break;
		case WM_PAINTCLIPBOARD:
			hsw_encapsulate_nat_to_bin(lpMsgBin,lpMsg);
			break;
		case WM_WINDOWPOSCHANGED:
			hsw_encapsulate_nat_to_bin(lpMsgBin,lpMsg);
			break;
		case WM_WINDOWPOSCHANGING:
			hsw_encapsulate_nat_to_bin(lpMsgBin,lpMsg);
			break;
		default:
			if (lpMsg->message >= WM_USER)
			    hsw_encapsulate_nat_to_bin(lpMsgBin,lpMsg);
			break;
		}
	}
}

LRESULT
hsw_listbox_nat_to_bin(HWND hWnd, UINT message, WPARAM w, LPARAM l)
{
    LPARAM lParam;
    WORD wSel;
    LRESULT ret;
    LPRECT lpRect;
    RECT rc;
    DWORD dwLBStyle;
    int nTextLength;

    if (message >= WM_USER && HIWORD(l))
	switch (message) {
	    case LB_ADDSTRING:
	    case LB_FINDSTRING:
	    case LB_FINDSTRINGEXACT:
	    case LB_INSERTSTRING:
		dwLBStyle = GetWindowLong(hWnd,GWL_STYLE);
		if ((dwLBStyle & LBS_HASSTRINGS) ||
		    !(dwLBStyle & (LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE))) {
		    wSel = AssignSelector((LPBYTE)l,0,
			TRANSFER_DATA,(DWORD)strlen((LPSTR)l)+2);
		    lParam = (LPARAM)MAKELP(wSel,0);
		    ret = hsw_common_nat_to_bin(hWnd,message,w,lParam);
		    FreeSelector(wSel);
		    return ret;
		}
		else
		    return hsw_common_nat_to_bin(hWnd,message,w,l);
		
	    case LB_DIR:
	    case LB_SELECTSTRING:
		wSel = AssignSelector((LPBYTE)l,0,TRANSFER_DATA,
			strlen((LPSTR)l)+2);
		lParam = (LPARAM)MAKELP(wSel,0);
		ret = hsw_common_nat_to_bin(hWnd,message,w,lParam);
		FreeSelector(wSel);
		return ret;

	    case LB_GETTEXT:
		if ((nTextLength = SendMessage(hWnd,LB_GETTEXTLEN,w,0L)) <= 0)
		    nTextLength = sizeof(DWORD);
		wSel = AssignSelector((LPBYTE)l,0,TRANSFER_DATA,
			nTextLength+2);
		lParam = (LPARAM)MAKELP(wSel,0);
		ret = hsw_common_nat_to_bin(hWnd,message,w,lParam);
		FreeSelector(wSel);
		return ret;

	    case LB_GETITEMRECT:
		lpRect = (LPRECT)l;
		wSel = AssignSelector((LPBYTE)&rc,0,
			TRANSFER_DATA,sizeof(RECT)+2);
		lParam = (LPARAM)MAKELP(wSel,0);
		ret = hsw_common_nat_to_bin(hWnd,message,w,lParam);
		FreeSelector(wSel);
		RECT_TO_C((*lpRect),&rc);
		return ret;

#ifdef	LATER
	    case LB_GETSELITEMS:
	    case LB_SETTABSTOPS:
		/* ptr to array of ints */
#endif
	}
    return hsw_common_nat_to_bin(hWnd,message,w,l);
}

LRESULT
hsw_listbox_bin_to_nat(HWND hWnd, UINT message, WPARAM w, LPARAM l)
{
    WMCONVERT *lpwmc = (WMCONVERT *)l;
    LPBYTE lpString;
    LRESULT ret;
    RECT rc;
    DWORD dwLBStyle;
    int i;

    if (message != WM_CONVERT)
	FatalAppExit(0,"non-WM_CONVERT in hsw_listbox_bin_to_nat\n");

    if ((w & HSWMC_BINMSG) && HIWORD(lpwmc->lParam) &&
	lpwmc->uMsg >= WM_USER)
	switch (lpwmc->uMsg) {
	    case LB_ADDSTRING:
	    case LB_FINDSTRING:
	    case LB_FINDSTRINGEXACT:
	    case LB_INSERTSTRING:
		dwLBStyle = GetWindowLong(hWnd,GWL_STYLE);
		if (((dwLBStyle & LBS_HASSTRINGS) ||
		    !(dwLBStyle & (LBS_OWNERDRAWFIXED|LBS_OWNERDRAWVARIABLE))))
		/*	&& (lpwmc->lParam & 0x70000 == 0x70000)) */
		 {
		    lpString = (LPBYTE)GetAddress(HIWORD(lpwmc->lParam),
					LOWORD(lpwmc->lParam));
		    lpwmc->lParam = (LPARAM)lpString;
		}
		return hsw_common_bin_to_nat(hWnd,message,w,l);

	    case LB_DIR:
	    case LB_GETTEXT:
	    case LB_SELECTSTRING:
		lpString = (LPBYTE)GetAddress(HIWORD(lpwmc->lParam),
					LOWORD(lpwmc->lParam));
		lpwmc->lParam = (LPARAM)lpString;
		return hsw_common_bin_to_nat(hWnd,message,w,l);

	    case LB_GETITEMRECT:
		lpString = (LPBYTE)GetAddress(HIWORD(lpwmc->lParam),
					LOWORD(lpwmc->lParam));
		lpwmc->lParam = (LPARAM)&rc;
		ret = hsw_common_bin_to_nat(hWnd,message,w,l);
		RECT_TO_86(lpString,rc);
		return ret;

	    case LB_GETSELITEMS:
	      {
		int nItems = (int)lpwmc->wParam;
		LPINT lpIntArray = (LPINT)WinMalloc(sizeof(int) * nItems);
		lpString = (LPBYTE)GetAddress(HIWORD(lpwmc->lParam),
					LOWORD(lpwmc->lParam));
		lpwmc->lParam = (LPARAM)lpIntArray;
		ret = hsw_common_bin_to_nat(hWnd,message,w,l);
		for (i=0; i<nItems; i++,lpString+=2)
		    PUTWORD(lpString,(WORD)lpIntArray[i]);
		WinFree((LPSTR)lpIntArray);
		return ret;
	      }
		    
#ifdef	LATER
	    case LB_SETTABSTOPS:
		/* ptr to array of ints */
#endif
	}
    return hsw_common_bin_to_nat(hWnd,message,w,l);
}

LRESULT
hsw_combobox_nat_to_bin(HWND hWnd, UINT message, WPARAM w, LPARAM l)
{
    LPARAM lParam;
    WORD wSel;
    LRESULT ret;
    LPRECT lpRect;
    RECT rc;
    DWORD dwCBStyle;
    int nTextLength;

    if (message >= WM_USER && HIWORD(l))
	switch (message) {
	    case CB_ADDSTRING:
	    case CB_FINDSTRING:
	    case CB_FINDSTRINGEXACT:
	    case CB_INSERTSTRING:
		dwCBStyle = GetWindowLong(hWnd,GWL_STYLE);
		if ((dwCBStyle & CBS_HASSTRINGS) ||
		    !(dwCBStyle & (CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE))) {
		    wSel = AssignSelector((LPBYTE)l,0,
			TRANSFER_DATA,(DWORD)strlen((LPSTR)l)+2);
		    lParam = (LPARAM)MAKELP(wSel,0);
		    ret = hsw_common_nat_to_bin(hWnd,message,w,lParam);
		    FreeSelector(wSel);
		    return ret;
		}
		else
		    return hsw_common_nat_to_bin(hWnd,message,w,l);
		
	    case CB_DIR:
	    case CB_SELECTSTRING:
		wSel = AssignSelector((LPBYTE)l,0,TRANSFER_DATA,
			strlen((LPSTR)l)+2);
		lParam = (LPARAM)MAKELP(wSel,0);
		ret = hsw_common_nat_to_bin(hWnd,message,w,lParam);
		FreeSelector(wSel);
		return ret;

	    case CB_GETLBTEXT:
		if ((nTextLength = SendMessage(hWnd,CB_GETLBTEXTLEN,w,0L)) <= 0)
		    nTextLength = sizeof(DWORD);
		wSel = AssignSelector((LPBYTE)l,0,TRANSFER_DATA,
			nTextLength+2);
		lParam = (LPARAM)MAKELP(wSel,0);
		ret = hsw_common_nat_to_bin(hWnd,message,w,lParam);
		FreeSelector(wSel);
		return ret;

	    case CB_GETDROPPEDCONTROLRECT:
		lpRect = (LPRECT)l;
		wSel = AssignSelector((LPBYTE)&rc,0,
			TRANSFER_DATA,sizeof(RECT)+2);
		lParam = (LPARAM)MAKELP(wSel,0);
		ret = hsw_common_nat_to_bin(hWnd,message,w,lParam);
		FreeSelector(wSel);
		RECT_TO_C((*lpRect),&rc);
		return ret;
	}
    return hsw_common_nat_to_bin(hWnd,message,w,l);
}

LRESULT
hsw_combobox_bin_to_nat(HWND hWnd, UINT message, WPARAM w, LPARAM l)
{
    WMCONVERT *lpwmc = (WMCONVERT *)l;
    LPBYTE lpString;
    LRESULT ret;
    RECT rc;
    DWORD dwCBStyle;

    if (message != WM_CONVERT)
	FatalAppExit(0,"non-WM_CONVERT in hsw_combobox_bin_to_nat\n");

    if ((w & HSWMC_BINMSG) && HIWORD(lpwmc->lParam) &&
	lpwmc->uMsg >= WM_USER)
	switch (lpwmc->uMsg) {
	    case CB_ADDSTRING:
	    case CB_FINDSTRING:
	    case CB_FINDSTRINGEXACT:
	    case CB_INSERTSTRING:
		dwCBStyle = GetWindowLong(hWnd,GWL_STYLE);
		if ((dwCBStyle & CBS_HASSTRINGS) ||
		    !(dwCBStyle & (CBS_OWNERDRAWFIXED|CBS_OWNERDRAWVARIABLE))) 
		/*	&& (lpwmc->lParam & 0x70000 == 0x70000))  */
		{
		    lpString = (LPBYTE)GetAddress(HIWORD(lpwmc->lParam),
					LOWORD(lpwmc->lParam));
		    lpwmc->lParam = (LPARAM)lpString;
		}
		return hsw_common_bin_to_nat(hWnd,message,w,l);

	    case CB_DIR:
	    case CB_GETLBTEXT:
	    case CB_SELECTSTRING:
		lpString = (LPBYTE)GetAddress(HIWORD(lpwmc->lParam),
					LOWORD(lpwmc->lParam));
		lpwmc->lParam = (LPARAM)lpString;
		return hsw_common_bin_to_nat(hWnd,message,w,l);

	    case CB_GETDROPPEDCONTROLRECT:
		lpString = (LPBYTE)GetAddress(HIWORD(lpwmc->lParam),
					LOWORD(lpwmc->lParam));
		lpwmc->lParam = (LPARAM)&rc;
		ret = hsw_common_bin_to_nat(hWnd,message,w,l);
		RECT_TO_86(lpString,rc);
		return ret;
	}
    return hsw_common_bin_to_nat(hWnd,message,w,l);
}

LRESULT
hsw_edit_nat_to_bin(HWND hWnd, UINT message, WPARAM w, LPARAM l)
{
    LPARAM lParam;
    WORD wSel;
    LRESULT ret;
    LPRECT lpRect;
    RECT rc;

    if (message >= WM_USER && HIWORD(l))
	switch (message) {
	    case EM_GETLINE:
		wSel = AssignSelector((LPBYTE)l,0,TRANSFER_DATA,
			*((LPWORD)l)+2);
		lParam = (LPARAM)MAKELP(wSel,0);
		ret = hsw_common_nat_to_bin(hWnd,message,w,lParam);
		FreeSelector(wSel);
		return ret;
	    case EM_REPLACESEL:
		wSel = AssignSelector((LPBYTE)l,0,TRANSFER_DATA,
			strlen((LPSTR)l)+2);
		lParam = (LPARAM)MAKELP(wSel,0);
		ret = hsw_common_nat_to_bin(hWnd,message,w,lParam);
		FreeSelector(wSel);
		return ret;
	    case EM_GETRECT:
		lpRect = (LPRECT)l;
		wSel = AssignSelector((LPBYTE)&rc,0,
			TRANSFER_DATA,sizeof(RECT)+2);
		lParam = (LPARAM)MAKELP(wSel,0);
		ret = hsw_common_nat_to_bin(hWnd,message,w,lParam);
		FreeSelector(wSel);
		RECT_TO_C((*lpRect),&rc);
		return ret;
	    case EM_SETRECT:
	    case EM_SETRECTNP:
		lpRect = (LPRECT)l;
		RECT_TO_86(&rc,(*lpRect));
		wSel = AssignSelector((LPBYTE)&rc,0,
			TRANSFER_DATA,sizeof(RECT)+2);
		lParam = (LPARAM)MAKELP(wSel,0);
		ret = hsw_common_nat_to_bin(hWnd,message,w,lParam);
		FreeSelector(wSel);
		return ret;
#ifdef	LATER
	    case EM_SETTABSTOPS:
		/* ptr to array of ints */
#endif
#ifdef	LATER
	    case EM_SETWORDBREAKPROC:
		/* ptr to EDITWORDBREAKPROC */
#endif
#ifdef TWIN32
	    case EM_SETSEL:
		lParam = (LPARAM)MAKELONG(w,l);
		return hsw_common_nat_to_bin(hWnd,message,w,lParam);
#endif
	    default:
		break;
	}
    return hsw_common_nat_to_bin(hWnd,message,w,l);
}

LRESULT
hsw_edit_bin_to_nat(HWND hWnd, UINT message, WPARAM w, LPARAM l)
{
    WMCONVERT *lpwmc = (WMCONVERT *)l;
    LPBYTE lpString;
    LRESULT ret;
    RECT rc;

    if (message != WM_CONVERT)
	FatalAppExit(0,"non-WM_CONVERT in hsw_edit_bin_to_nat\n");

    if ((w & HSWMC_BINMSG) && lpwmc->uMsg >= WM_USER)
	switch (lpwmc->uMsg) {
	    case EM_GETLINE:
	    case EM_REPLACESEL:
		if (HIWORD(lpwmc->lParam)) {
		    lpString = (LPBYTE)GetAddress(HIWORD(lpwmc->lParam),
					LOWORD(lpwmc->lParam));
		    lpwmc->lParam = (LPARAM)lpString;
		}
		break;
	    case EM_GETRECT:
		if (HIWORD(lpwmc->lParam)) {
		    lpString = (LPBYTE)GetAddress(HIWORD(lpwmc->lParam),
					LOWORD(lpwmc->lParam));
		    lpwmc->lParam = (LPARAM)&rc;
		    ret = hsw_common_bin_to_nat(hWnd,message,w,l);
		    RECT_TO_86(lpString,rc);
		    return ret;
		}
		else
		    break;
	    case EM_SETRECT:
	    case EM_SETRECTNP:
		if (HIWORD(lpwmc->lParam)) {
		    lpString = (LPBYTE)GetAddress(HIWORD(lpwmc->lParam),
					LOWORD(lpwmc->lParam));
		    RECT_TO_C(rc,lpString)
		    lpwmc->lParam = (LPARAM)&rc;
		}
		break;

	    case EM_SETTABSTOPS:
		if ((lpwmc->wParam != 0) && HIWORD(lpwmc->lParam)) {
		    lpString = (LPBYTE)GetAddress(HIWORD(lpwmc->lParam),
					LOWORD(lpwmc->lParam));
		    lpwmc->lParam = (LPARAM)hsw_ConvertArrayToInt(lpString,
						lpwmc->wParam);
		}
		break;
#ifdef	LATER
	    case EM_SETWORDBREAKPROC:
		/* ptr to EDITWORDBREAKPROC */
#endif
#ifdef TWIN32
	    case EM_SETSEL:
		lpwmc->wParam = (WPARAM)(int)LOSHORT(lpwmc->lParam);
		lpwmc->lParam = (LONG)HISHORT(lpwmc->lParam);
		break;
#endif
	    default:
		break;
	}
    return hsw_common_bin_to_nat(hWnd,message,w,l);
}

LRESULT
hsw_commdlg_nat_to_bin(HWND hWnd, UINT message, WPARAM w, LPARAM l)
{
    return LOWORD(hsw_common_nat_to_bin(hWnd,message,w,
	(message!=WM_INITDIALOG)?
		l:
		(LPARAM)(*((BINADDR *)((LPBYTE)l - sizeof(LPVOID))))));
}

LRESULT
hsw_mdiclient_bin_to_nat(HWND hWnd, UINT message, WPARAM w, LPARAM l)
{
    WMCONVERT *lpwmc = (WMCONVERT *)l;
    LPARAM lParam;
    LPBYTE lpStruct,lpStruct1;
    WORD w1,w2,w3,w4;
    CREATESTRUCT cs;
    CLIENTCREATESTRUCT ccs;
    DWORD retcode;

    if (message != WM_CONVERT) 
	FatalAppExit(0,"non-WM_CONVERT in hsw_mdiclient_bin_to_nat\n");

    if ((w & HSWMC_BINMSG) &&
		(lpwmc->uMsg == WM_CREATE || lpwmc->uMsg == WM_NCCREATE)) {
	lParam = lpwmc->lParam;
	lpStruct = (LPBYTE)GetAddress(HIWORD(lParam),LOWORD(lParam));
	cs.hInstance = (HANDLE)GetSelectorHandle(GETWORD(lpStruct+4));
	cs.hMenu = (HANDLE)GETWORD(lpStruct+6);
	cs.hwndParent = (HANDLE)GETWORD(lpStruct+8);
	w1 = GETWORD(lpStruct+10);
	w2 = GETWORD(lpStruct+12);
	w3 = GETWORD(lpStruct+14);
	w4 = GETWORD(lpStruct+16);
	cs.cy = (w1 == CW_USEDEFAULT16)?(int)CW_USEDEFAULT:(int)((short)w1);
	cs.cx = (w2 == CW_USEDEFAULT16)?(int)CW_USEDEFAULT:(int)((short)w2);
	cs.y = (w3 == CW_USEDEFAULT16)?(int)CW_USEDEFAULT:(int)((short)w3);
	cs.x = (w4 == CW_USEDEFAULT16)?(int)CW_USEDEFAULT:(int)((short)w4);
	cs.style = GETDWORD(lpStruct+18);
	cs.lpszName = (LPSTR)GetAddress(
				GETWORD(lpStruct+24),GETWORD(lpStruct+22));
	cs.lpszClass = (LPSTR)GetAddress(
				GETWORD(lpStruct+28),GETWORD(lpStruct+26));
	lpStruct1 = (LPBYTE)GetAddress(GETWORD(lpStruct+2),GETWORD(lpStruct));
	ccs.hWindowMenu = (HANDLE)GETWORD(lpStruct);
	ccs.idFirstChild = (UINT)GETWORD(lpStruct+2);
	cs.lpCreateParams = &ccs;
	retcode = (DWORD)(lpwmc->targ)
			(hWnd,lpwmc->uMsg,lpwmc->wParam,(LPARAM)(&cs));
	PUTDWORD(lpStruct+18,cs.style);
	return retcode;
    }
    else
	return hsw_common_bin_to_nat(hWnd,message,w,l);
}

LRESULT
hsw_mdiclient_nat_to_bin(HWND hWnd, UINT message, WPARAM w, LPARAM l)
{
    return hsw_common_nat_to_bin(hWnd,message,w,l);
}

static int
TWIN_string_array_length(char *s)
{
    int n;
    
    for (n = 0; *s || *(s+1); n++, s++)
	;
    
    return n + 2;
}

static void
TWIN_nat_to_bin_openfilename(OPENFILENAME *pNatOFN)
{
    char String[_MAX_PATH];
    DWORD dwBinOFN = 0;
    HGLOBAL hBinOFN = 0;
    LPBYTE lpBinOFN = NULL;
    LPSTR lpString;
    LPSTR lp;
    WORD wWord;
    WORD wSel = 0;

    /*
     * Get a pointer to the binary version.
     */
    if (pNatOFN->lStructSize != sizeof(OPENFILENAME)) 
    {
	dwBinOFN = *((DWORD *) ((LPBYTE) pNatOFN - sizeof(LPVOID)));
	lpBinOFN = (LPBYTE) GetAddress(HIWORD(dwBinOFN), LOWORD(dwBinOFN));
    }
    else
    {
	int extradataoffset;
	int n;
	
	hBinOFN = GlobalAlloc(0, 4096);
	lpBinOFN = (LPBYTE) GlobalLock(hBinOFN);

	wSel = AssignSelector(lpBinOFN, 0, TRANSFER_DATA, 
			      GlobalSize(hBinOFN) + 2);
	SetSelectorHandle(wSel, hBinOFN);
	GlobalUnlock(hBinOFN);

	dwBinOFN = (DWORD) MAKELP(wSel, 0);

	PUTDWORD(lpBinOFN +  0, 72);	/* lpStructSize */
	PUTWORD (lpBinOFN +  4, pNatOFN->hwndOwner);
	PUTWORD (lpBinOFN +  6, pNatOFN->hInstance);
	PUTDWORD(lpBinOFN + 16, pNatOFN->nMaxCustFilter);
	PUTDWORD(lpBinOFN + 28, pNatOFN->nMaxFile);
	PUTDWORD(lpBinOFN + 36, pNatOFN->nMaxFileTitle);
	PUTDWORD(lpBinOFN + 48, pNatOFN->Flags);
	PUTWORD (lpBinOFN + 52, pNatOFN->nFileOffset);
	PUTWORD (lpBinOFN + 54, pNatOFN->nFileExtension);
	PUTDWORD(lpBinOFN + 60, pNatOFN->lCustData);
	PUTDWORD(lpBinOFN + 64, pNatOFN->lpfnHook);

	extradataoffset = 128;

	if (pNatOFN->lpstrFilter)
	{
	    n = TWIN_string_array_length((char *) pNatOFN->lpstrFilter);
	    PUTDWORD(lpBinOFN + 8, dwBinOFN + extradataoffset);
	    memcpy(lpBinOFN + extradataoffset, pNatOFN->lpstrFilter, n);
	    extradataoffset += n;
	}
	else
	    PUTDWORD(lpBinOFN + 8, 0);
	
	if (pNatOFN->lpstrCustomFilter)
	{
	    PUTDWORD(lpBinOFN + 12, dwBinOFN + extradataoffset);
	    memcpy(lpBinOFN + extradataoffset, pNatOFN->lpstrCustomFilter, 
		   pNatOFN->nMaxCustFilter);
	    extradataoffset += pNatOFN->nMaxCustFilter;
	}
	else
	    PUTDWORD(lpBinOFN + 12, 0);
	
	if (pNatOFN->lpstrFile)
	{
	    PUTDWORD(lpBinOFN + 24, dwBinOFN + extradataoffset);
	    /* Filled in later */
	    extradataoffset += pNatOFN->nMaxFile;
	}
	else
	    PUTDWORD(lpBinOFN + 24, 0);
	
	if (pNatOFN->lpstrFileTitle)
	{
	    PUTDWORD(lpBinOFN + 32, dwBinOFN + extradataoffset);
	    /* Filled in later */
	    extradataoffset += pNatOFN->nMaxFileTitle;
	}
	else
	    PUTDWORD(lpBinOFN + 32, 0);
	
	if (pNatOFN->lpstrInitialDir)
	{
	    PUTDWORD(lpBinOFN + 40, dwBinOFN + extradataoffset);
	    strcpy((char *) (lpBinOFN + extradataoffset), 
		   pNatOFN->lpstrInitialDir);
	    extradataoffset += strlen(pNatOFN->lpstrInitialDir) + 1;
	}
	else
	    PUTDWORD(lpBinOFN + 40, 0);
	
	if (pNatOFN->lpstrTitle)
	{
	    PUTDWORD(lpBinOFN + 44, dwBinOFN + extradataoffset);
	    strcpy((char *) (lpBinOFN + extradataoffset), pNatOFN->lpstrTitle);
	    extradataoffset += strlen(pNatOFN->lpstrTitle) + 1;
	}
	else
	    PUTDWORD(lpBinOFN + 44, 0);
	
	if (pNatOFN->lpstrDefExt)
	{
	    PUTDWORD(lpBinOFN + 56, dwBinOFN + extradataoffset);
	    strcpy((char *) (lpBinOFN + extradataoffset), 
		   pNatOFN->lpstrDefExt);
	    extradataoffset += strlen(pNatOFN->lpstrDefExt) + 1;
	}
	else
	    PUTDWORD(lpBinOFN + 56, 0);
	
	if ((pNatOFN->Flags & OFN_ENABLETEMPLATE) && (pNatOFN->lpTemplateName))
	{
	    PUTDWORD(lpBinOFN + 68, dwBinOFN + extradataoffset);
	    strcpy((char *) lpBinOFN + extradataoffset, 
		   pNatOFN->lpTemplateName);
	    extradataoffset += strlen(pNatOFN->lpTemplateName) + 1;
	}
	else
	    PUTDWORD(lpBinOFN + 68, 0);
    }
    
    /*
     * Fill in the binary version.
     */
    PUTDWORD(lpBinOFN+20,pNatOFN->nFilterIndex);

    if (pNatOFN->lpstrFile) 
    {
	lpString = (LPSTR) GetAddress(GETWORD(lpBinOFN+26), 
				      GETWORD(lpBinOFN+24));
	xdoscall(XDOS_GETDOSNAME,0, (void *) String,
		 (void *) pNatOFN->lpstrFile);
	strncpy(lpString, String, min(strlen(String)+1, pNatOFN->nMaxFile));

	if ((lp = strpbrkr(lpString,"\\")))
	    wWord = (WORD)(lp - lpString + 1);
	else 
	{
	    wWord = 0;
	    lp = lpString;
	}

	PUTWORD(lpBinOFN+52, wWord);
	if ((lp = strpbrk(lp,".")))
	{
	    if (*(lp+1) == '\0')
		wWord = 0;
	    else
		wWord = (WORD)(lp - lpString + 1);
	}
	else
	    wWord = strlen(lpString);
	PUTWORD(lpBinOFN+54, wWord);
    }

    if (pNatOFN->lpstrFileTitle) 
    {
	lpString = (LPSTR) GetAddress(GETWORD(lpBinOFN+34), 
				      GETWORD(lpBinOFN+32));
	strcpy(lpString,pNatOFN->lpstrFileTitle);
    }

    /*
     * Transfer control to the binary.
     */
    PUTDWORD((LPBYTE) envp_global->reg.sp, dwBinOFN);
    invoke_binary();

    /*
     * Clean up any mess that we made.
     */
    if (hBinOFN)
    {
	GlobalFree(hBinOFN);
	FreeSelector(wSel);
    }
    
    return;

}    

static void TWIN_bin_to_nat_openfilename(LPARAM lParam, char *pBuffer)
{
    LPBYTE pBinOFN;
    OPENFILENAME *pNatOFN;

    pNatOFN = (OPENFILENAME *) (pBuffer + 4);
    pBinOFN = (LPBYTE) GetAddress(HIWORD(lParam), LOWORD(lParam));
    
    memset(pBuffer, '\0', sizeof(OPENFILENAME) + sizeof(LPVOID));
    *((BINADDR *) pBuffer) = lParam;

    pNatOFN->lStructSize = sizeof(OPENFILENAME) + sizeof(LPVOID);
    pNatOFN->hwndOwner = GETWORD(pBinOFN + 4);
    pNatOFN->hInstance = GETWORD(pBinOFN + 6);
    pNatOFN->nMaxCustFilter = GETDWORD(pBinOFN + 16);
    pNatOFN->nFilterIndex = GETDWORD(pBinOFN + 20);
    pNatOFN->nMaxFile = GETDWORD(pBinOFN + 28);
    pNatOFN->nMaxFileTitle = GETDWORD(pBinOFN + 36);
    pNatOFN->Flags = GETDWORD(pBinOFN + 48);
    pNatOFN->nFileOffset = GETWORD(pBinOFN + 52);
    pNatOFN->nFileExtension = GETWORD(pBinOFN + 54);
    pNatOFN->lCustData = GETDWORD(pBinOFN + 60);
    pNatOFN->lpfnHook = (UINT (CALLBACK *)(HWND, UINT, WPARAM, LPARAM))
	GETDWORD(pBinOFN + 64);

    pNatOFN->lpstrFilter = GetAddress(GETWORD(pBinOFN + 10), 
				      GETWORD(pBinOFN + 8));
    pNatOFN->lpstrCustomFilter = GetAddress(GETWORD(pBinOFN + 14), 
					    GETWORD(pBinOFN + 12));
    pNatOFN->lpstrFile = GetAddress(GETWORD(pBinOFN + 26), 
				    GETWORD(pBinOFN + 24));
    pNatOFN->lpstrFileTitle = GetAddress(GETWORD(pBinOFN + 34), 
					 GETWORD(pBinOFN + 32));
    pNatOFN->lpstrInitialDir = GetAddress(GETWORD(pBinOFN + 42), 
					  GETWORD(pBinOFN + 40));
    pNatOFN->lpstrTitle = GetAddress(GETWORD(pBinOFN + 46), 
				     GETWORD(pBinOFN + 44));
    pNatOFN->lpstrDefExt = GetAddress(GETWORD(pBinOFN + 58), 
				      GETWORD(pBinOFN + 56));
    pNatOFN->lpTemplateName = GetAddress(GETWORD(pBinOFN + 70), 
					 GETWORD(pBinOFN + 68));
}
