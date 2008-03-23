/* 
	Callback.c	2.12
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
#include "ole.h"

#include "kerndef.h"
#include "Endian.h"
#include "BinTypes.h"
#include "DPMI.h"
#include "Hook.h"
#include "Meta.h"
#include "invoke_binary.h"
#include "LoadDLL.h"

extern ENV *envp_global;
extern DSCR *LDT;

BOOL CALLBACK
hsw_enumprops(HWND hwnd,LPCSTR lpsz,HANDLE hData)
{
    WORD sel;

    envp_global->reg.sp -= 2*HANDLE_86 + LP_86;
    sel = ASSIGNSEL(lpsz, 512);
    PUTWORD((LPBYTE)envp_global->reg.sp+6,hwnd);
    PUTDWORD((LPBYTE)envp_global->reg.sp+2,MAKELONG(0,sel));
    PUTWORD((LPBYTE)envp_global->reg.sp,hData);
    invoke_binary();
    FreeSelector(sel);
    return (envp_global->reg.ax)?TRUE:FALSE;
}

void CALLBACK
hsw_timerproc(HWND hwnd,UINT msg,UINT idTimer,DWORD dwTime)
{
    envp_global->reg.sp -= HANDLE_86 + 2*UINT_86 + DWORD_86;
    PUTWORD((LPBYTE)envp_global->reg.sp+8,hwnd);
    PUTWORD((LPBYTE)envp_global->reg.sp+6,(WORD)msg);
    PUTWORD((LPBYTE)envp_global->reg.sp+4,(WORD)idTimer);
    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTime);
    invoke_binary();
}

void CALLBACK
hsw_mmtimer(UINT uTimerID,UINT msg,DWORD dwUser,DWORD dw1,DWORD dw2)
{
    envp_global->reg.sp -= 2*UINT_86 + 3*DWORD_86;
    PUTWORD((LPBYTE)envp_global->reg.sp+14,uTimerID);
    PUTWORD((LPBYTE)envp_global->reg.sp+12,msg);
    PUTDWORD((LPBYTE)envp_global->reg.sp+8,dwUser);
    PUTDWORD((LPBYTE)envp_global->reg.sp+4,dw1);
    PUTDWORD((LPBYTE)envp_global->reg.sp,dw2);
    invoke_binary();
}

BOOL CALLBACK
hsw_wndenumproc(HWND hwnd, LPARAM lParam)
{
    envp_global->reg.sp -= HANDLE_86 + LONG_86;
    PUTWORD((LPBYTE)envp_global->reg.sp+4,hwnd);
    PUTDWORD((LPBYTE)envp_global->reg.sp,lParam);
    invoke_binary();
    return (BOOL)envp_global->reg.ax;
}

int CALLBACK
hsw_enumbrushproc(LOGBRUSH *lplb,LPARAM lpData)
{
	LPBYTE lpBuf;

	envp_global->reg.sp -= sizeof(LOGBRUSH) + 0x10;
	lpBuf = (LPBYTE)envp_global->reg.sp;
	envp_global->reg.sp -= LP_86 + LONG_86;
	PUTWORD(lpBuf,lplb->lbStyle);
	PUTDWORD(lpBuf+2,lplb->lbColor);
	PUTWORD(lpBuf+6,(WORD)lplb->lbHatch);
	PUTDWORD((LPBYTE)envp_global->reg.sp+4,(DWORD)MAKELP
		(LOWORD(envp_global->reg.ss),
		(WORD)(lpBuf-GetPhysicalAddress((WORD)envp_global->reg.ss))));
	PUTDWORD((LPBYTE)envp_global->reg.sp,lpData);
	invoke_binary();
	envp_global->reg.sp += sizeof(LOGBRUSH) + 0x10;
	return (int)envp_global->reg.ax;
}

int CALLBACK
hsw_enumpenproc(LOGPEN *lplp,LPARAM lpData)
{
	BYTE buf[0x10];
	WORD sel;

	envp_global->reg.sp -= LP_86 + LONG_86;
	sel = AssignSelector(buf,0,TRANSFER_DATA,(DWORD)0x10);
	PUTWORD(buf,lplp->lopnStyle);
	PutPOINT(buf+2,lplp->lopnWidth);
	PUTDWORD(buf+6,lplp->lopnColor);
	PUTDWORD((LPBYTE)envp_global->reg.sp+4,MAKELONG(0,sel));
	PUTDWORD((LPBYTE)envp_global->reg.sp,lpData);
	invoke_binary();
	FreeSelector(sel);
	return (int)envp_global->reg.ax;
}

int CALLBACK
hsw_fontenumproc(LOGFONT *lplf, NEWTEXTMETRIC *lpntm, 
				int FontType, LPARAM lpData)
{
    LPBYTE lpBuf;

    envp_global->reg.sp -= 128;
    lpBuf = (LPBYTE)envp_global->reg.sp;
    envp_global->reg.sp -= 2*LP_86 + INT_86 + LONG_86;

    PUTWORD(lpBuf,(WORD)lplf->lfHeight);
    PUTWORD(lpBuf+2,(WORD)lplf->lfWidth);
    PUTWORD(lpBuf+4,(WORD)lplf->lfEscapement);
    PUTWORD(lpBuf+6,(WORD)lplf->lfOrientation);
    PUTWORD(lpBuf+8,(WORD)lplf->lfWeight);
    memcpy(lpBuf+10,&lplf->lfItalic,8+LF_FACESIZE);
    PUTDWORD((LPBYTE)envp_global->reg.sp+10,(DWORD)MAKELP
	(LOWORD(envp_global->reg.ss),
	(WORD)(lpBuf-GetPhysicalAddress((WORD)envp_global->reg.ss))));

    lpBuf += 32 + LF_FACESIZE;
    PUTWORD(lpBuf,(WORD)lpntm->tmHeight);
    PUTWORD(lpBuf+2,(WORD)lpntm->tmAscent);
    PUTWORD(lpBuf+4,(WORD)lpntm->tmDescent);
    PUTWORD(lpBuf+6,(WORD)lpntm->tmInternalLeading);
    PUTWORD(lpBuf+8,(WORD)lpntm->tmExternalLeading);
    PUTWORD(lpBuf+10,(WORD)lpntm->tmAveCharWidth);
    PUTWORD(lpBuf+12,(WORD)lpntm->tmMaxCharWidth);
    PUTWORD(lpBuf+14,(WORD)lpntm->tmWeight);
    memcpy(lpBuf+16,&lpntm->tmItalic,9);
    PUTWORD(lpBuf+25,(WORD)lpntm->tmOverhang);
    PUTWORD(lpBuf+27,(WORD)lpntm->tmDigitizedAspectX);
    PUTWORD(lpBuf+29,(WORD)lpntm->tmDigitizedAspectY);
    PUTDWORD(lpBuf+31,(DWORD)lpntm->ntmFlags);
    PUTWORD(lpBuf+35,(WORD)lpntm->ntmSizeEM);
    PUTWORD(lpBuf+37,(WORD)lpntm->ntmCellHeight);
    PUTWORD(lpBuf+39,(WORD)lpntm->ntmAvgWidth);
    PUTDWORD((LPBYTE)envp_global->reg.sp+6,(DWORD)MAKELP
	(LOWORD(envp_global->reg.ss),
	(WORD)(lpBuf-GetPhysicalAddress((WORD)envp_global->reg.ss))));

    PUTWORD((LPBYTE)envp_global->reg.sp+4,(WORD)FontType);
    PUTDWORD((LPBYTE)envp_global->reg.sp,lpData);
    invoke_binary();
    envp_global->reg.sp += 128;
    return (int)envp_global->reg.ax;
}


BOOL CALLBACK
hsw_abortproc(HDC hDC, int error)
{
	LPBYTE	lpSP;

	lpSP = (LPBYTE)envp_global->reg.sp;
	envp_global->reg.sp -= HANDLE_86 + INT_86;
	PUTWORD(lpSP,     hDC);
	PUTWORD(lpSP + 2, error);
	invoke_binary();
	return (BOOL)envp_global->reg.ax;
}

/****************************************************************/
/* 	Hook procedures						*/
/****************************************************************/

DWORD CALLBACK
hsw_msgfilterproc(int code, WPARAM wParam, LPARAM lParam)
{
    LPBYTE lpData;
    DWORD dwTemp;
    LPMSG lpmsg;

/* This filter proc accepts only mouse and keyboard messages -- */
/* no need to worry about fancy parameter conversion		*/

    envp_global->reg.sp -= 24;
    lpData = (LPBYTE)envp_global->reg.sp;
    dwTemp = (DWORD)MAKELP(LOWORD(envp_global->reg.ss),
		(WORD)(lpData-GetPhysicalAddress((WORD)envp_global->reg.ss)));
    lpmsg = (LPMSG)lParam;
    PUTWORD(lpData,lpmsg->hwnd);
    PUTWORD(lpData+2,lpmsg->message);
    PUTWORD(lpData+4,lpmsg->wParam);
    PUTDWORD(lpData+6,lpmsg->lParam);
    PUTDWORD(lpData+10,lpmsg->time);
    PutPOINT(lpData+14,lpmsg->pt);
    envp_global->reg.sp -= INT_86 + WORD_86 + LP_86;
    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
    PUTWORD((LPBYTE)envp_global->reg.sp+4,wParam);
    PUTWORD((LPBYTE)envp_global->reg.sp+6,(WORD)code);
    invoke_binary();
    lpmsg->message = GETWORD(lpData+2);
    lpmsg->wParam = GETWORD(lpData+4);
    lpmsg->lParam = GETDWORD(lpData+6);
    envp_global->reg.sp += 24;
    return (int)envp_global->reg.ax;
}

DWORD CALLBACK
hsw_hookproc(int code, WPARAM wParam, LPARAM lParam)
{
    LPBYTE lpData;
    DWORD dwTemp;
    LPMSG lpmsg;

    envp_global->reg.sp -= 24;
    lpData = (LPBYTE)envp_global->reg.sp;
    dwTemp = (DWORD)MAKELP(LOWORD(envp_global->reg.ss),
		(WORD)(lpData-GetPhysicalAddress((WORD)envp_global->reg.ss)));
    lpmsg = (LPMSG)lParam;
    PUTWORD(lpData,lpmsg->hwnd);
    PUTWORD(lpData+2,lpmsg->message);
    PUTWORD(lpData+4,lpmsg->wParam);
    PUTDWORD(lpData+6,lpmsg->lParam);
    PUTDWORD(lpData+10,lpmsg->time);
    PutPOINT(lpData+14,lpmsg->pt);
    envp_global->reg.sp -= INT_86 + WORD_86 + LP_86;
    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
    PUTWORD((LPBYTE)envp_global->reg.sp+4,wParam);
    PUTWORD((LPBYTE)envp_global->reg.sp+6,(WORD)code);
    invoke_binary();

/* Here we should update original lpmsg -- app might have changed it */

    envp_global->reg.sp += 24;
    return (int)envp_global->reg.ax;
}

DWORD CALLBACK
hsw_callwndproc(int code, WPARAM wParam, LPARAM lParam)
{
    LPBYTE lpData;
    DWORD dwTemp;
    HOOKINFO *lphki;

    envp_global->reg.sp -= 24;
    lpData = (LPBYTE)envp_global->reg.sp;
    dwTemp = (DWORD)MAKELP(LOWORD(envp_global->reg.ss),
		(WORD)(lpData-GetPhysicalAddress((WORD)envp_global->reg.ss)));
    lphki = (HOOKINFO *)lParam;
    PUTDWORD(lpData,lphki->lParam);
    PUTWORD(lpData+4,lphki->wParam);
    PUTWORD(lpData+6,lphki->msg);
    PUTWORD(lpData+8,lphki->hWnd);
    envp_global->reg.sp -= INT_86 + WORD_86 + LP_86;
    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
    PUTWORD((LPBYTE)envp_global->reg.sp+4,wParam);
    PUTWORD((LPBYTE)envp_global->reg.sp+6,(WORD)code);
    invoke_binary();

/* Here we should update original lpmsg -- app might have changed it */

    envp_global->reg.sp += 24;
    return (int)envp_global->reg.ax;
}

DWORD CALLBACK
hsw_keyboardproc(int code, WPARAM wParam, LPARAM lParam)
{
    envp_global->reg.sp -= INT_86 + WORD_86 + LP_86;
    PUTWORD((LPBYTE)envp_global->reg.sp+6,(WORD)code);
    PUTWORD((LPBYTE)envp_global->reg.sp+4,wParam);
    PUTDWORD((LPBYTE)envp_global->reg.sp,(DWORD)lParam);
    invoke_binary();
    return (int)envp_global->reg.ax;
}

DWORD CALLBACK
hsw_mouseproc(int code, WPARAM wParam, LPARAM lParam)
{
    LPBYTE lpData;
    DWORD dwTemp;
    MOUSEHOOKSTRUCT *lpmhs;

    envp_global->reg.sp -= 16;
    lpData = (LPBYTE)envp_global->reg.sp;
    dwTemp = (DWORD)MAKELP(LOWORD(envp_global->reg.ss),
		(WORD)(lpData-GetPhysicalAddress((WORD)envp_global->reg.ss)));
    lpmhs = (MOUSEHOOKSTRUCT *)lParam;
    PutPOINT(lpData,lpmhs->pt);
    PUTWORD(lpData+4,lpmhs->hwnd);
    PUTWORD(lpData+6,lpmhs->wHitTestCode);
    PUTDWORD(lpData+8,lpmhs->dwExtraInfo);
    envp_global->reg.sp -= INT_86 + WORD_86 + LP_86;
    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
    PUTWORD((LPBYTE)envp_global->reg.sp+4,wParam);
    PUTWORD((LPBYTE)envp_global->reg.sp+6,(WORD)code);
    invoke_binary();
    envp_global->reg.sp += 16;
    return (int)envp_global->reg.ax;
}

DWORD CALLBACK
hsw_cbtproc(int code, WPARAM wParam, LPARAM lParam)
{
    LPBYTE lpData;
    DWORD dwTemp;
    CBT_CREATEWND *lpcbtcs;
    WORD wSel1 = 0,wSel2 = 0;

    switch (code) {
	case HCBT_CREATEWND:
	    envp_global->reg.sp -= 48;
	    lpData = (LPBYTE)envp_global->reg.sp;
	    dwTemp = (DWORD)MAKELP(LOWORD(envp_global->reg.ss),
		(WORD)(lpData-GetPhysicalAddress((WORD)envp_global->reg.ss)));
	    lpcbtcs = (CBT_CREATEWND *)lParam;
	    /* this is a CREATESTRUCT made on the stack */
#ifdef	LATER
	/* special treatment for MDI children */
#else
	    PUTDWORD(lpData,lpcbtcs->lpcs->lpCreateParams);
#endif
	    PUTWORD(lpData+4,
		    (WORD)
		    GetDataSelectorFromInstance(lpcbtcs->lpcs->hInstance));
	    PUTWORD(lpData+6,(WORD)lpcbtcs->lpcs->hMenu);
	    PUTWORD(lpData+8,(WORD)lpcbtcs->lpcs->hwndParent);
	    PUTWORD(lpData+10,(WORD)
		((UINT)lpcbtcs->lpcs->cy != CW_USEDEFAULT)?
			lpcbtcs->lpcs->cy:CW_USEDEFAULT16);
	    PUTWORD(lpData+12,(WORD)
		((UINT)lpcbtcs->lpcs->cx != CW_USEDEFAULT)?
			lpcbtcs->lpcs->cx:CW_USEDEFAULT16);
	    PUTWORD(lpData+14,(WORD)
		((UINT)lpcbtcs->lpcs->y != CW_USEDEFAULT)?
			lpcbtcs->lpcs->y:CW_USEDEFAULT16);
	    PUTWORD(lpData+16,(WORD)
		((UINT)lpcbtcs->lpcs->x != CW_USEDEFAULT)?
			lpcbtcs->lpcs->x:CW_USEDEFAULT16);
	    PUTDWORD(lpData+18,(DWORD)lpcbtcs->lpcs->style);
	    if (lpcbtcs->lpcs->lpszName) {
		wSel1 = ASSIGNSEL(lpcbtcs->lpcs->lpszName,
				strlen(lpcbtcs->lpcs->lpszName)+1);
		PUTDWORD(lpData+22,MAKELP(wSel1,0));
	    }
	    else
		PUTDWORD(lpData+22,lpcbtcs->lpcs->lpszName);
	    if (HIWORD(lpcbtcs->lpcs->lpszClass)) {
		wSel2 = ASSIGNSEL(lpcbtcs->lpcs->lpszClass,
				strlen(lpcbtcs->lpcs->lpszClass)+1);
		PUTDWORD(lpData+26,MAKELP(wSel2,0));
	    }
	    else
		PUTDWORD(lpData+26,lpcbtcs->lpcs->lpszClass);
	    PUTDWORD(lpData+30,lpcbtcs->lpcs->dwExStyle);

	    /* this is a CBT_CREATEWND struct made on the stack */
	    PUTDWORD(lpData+36,dwTemp);
	    dwTemp += 36;
	    PUTWORD(lpData+40,lpcbtcs->hwndInsertAfter);

	    envp_global->reg.sp -= INT_86 + WORD_86 + LP_86;
	    PUTDWORD((LPBYTE)envp_global->reg.sp,dwTemp);
	    PUTWORD((LPBYTE)envp_global->reg.sp+4,wParam);
	    PUTWORD((LPBYTE)envp_global->reg.sp+6,(WORD)code);
	    invoke_binary();
	    if (wSel1)
		FreeSelector(wSel1);
	    if (wSel2)
		FreeSelector(wSel2);
	    envp_global->reg.sp += 48;
	    return (int)envp_global->reg.ax;

	default:
	    break;
    }
    return 0;
}

/****************************************************************/

BOOL CALLBACK
hsw_graystringproc(HDC hdc, LPARAM lpData, int cch)
{
    LPARAM lParam;
    WORD wSel = 0;

    if (lpData) 
	wSel = ASSIGNSEL(lpData, 512);		/* 512 arbitrary -- LJW */
    lParam = MAKELONG(0,wSel);
    envp_global->reg.sp -= HANDLE_86 + INT_86 + LP_86;
    PUTWORD((LPBYTE)envp_global->reg.sp+6,(WORD)hdc);
    PUTDWORD((LPBYTE)envp_global->reg.sp+2,lParam);
    PUTWORD((LPBYTE)envp_global->reg.sp,(WORD)cch);
    invoke_binary();
    if (wSel)
	FreeSelector(wSel);
    return (int)envp_global->reg.ax;
}

void CALLBACK
hsw_lineddaproc(int xPos, int yPos, LPARAM lpData)
{
    envp_global->reg.sp -= 2*INT_86 + LP_86;
    PUTWORD((LPBYTE)envp_global->reg.sp+6,(short)xPos);
    PUTWORD((LPBYTE)envp_global->reg.sp+4,(short)yPos);
    PUTDWORD((LPBYTE)envp_global->reg.sp,lpData);
    invoke_binary();
}

int CALLBACK
hsw_mfenumproc(HDC hdc, HANDLETABLE *lpht, METARECORD *lpmr,
		int cObj, LPARAM lParam)
{
    WORD wSelRec = 0, wSelHandle = 0;
    DWORD size;
    LPWORD lpRec = (LPWORD)lpmr;

    envp_global->reg.sp -= HANDLE_86 + 2*LP_86 + INT_86 + LONG_86;

    PUTWORD((LPBYTE)envp_global->reg.sp+14,hdc);

    if (cObj && lpht) {
	wSelHandle = ASSIGNSEL((LPBYTE)lpht,cObj*HANDLE_86);
	PUTDWORD((LPBYTE)envp_global->reg.sp+10,MAKELP(wSelHandle,0));
    }
    else
	PUTDWORD((LPBYTE)envp_global->reg.sp+10,0L);

    if (lpRec) {
	size = GETWORD(&lpRec[RDSIZE]) * sizeof(WORD);
	wSelRec = ASSIGNSEL(lpRec, size);
	PUTDWORD((LPBYTE)envp_global->reg.sp+6,MAKELP(wSelRec,0));
    }
    else
	PUTDWORD((LPBYTE)envp_global->reg.sp+6,0L);

    PUTWORD((LPBYTE)envp_global->reg.sp+4,(WORD)cObj);

    PUTDWORD((LPBYTE)envp_global->reg.sp,lParam);

    invoke_binary();

    if (wSelRec)
	FreeSelector(wSelRec);
    if (wSelHandle)
	FreeSelector(wSelHandle);

    return (int)envp_global->reg.ax;
}

/* OLE-specific procedures */
DWORD CALLBACK
hsw_oleserverrelease(LPARAM lParam)
{
    LPOLESERVER lpServer = (LPOLESERVER)lParam;

    envp_global->reg.sp -= DWORD_86;

#ifdef	LATER
    convert OLESERVER struct and OLESERVERVTBL struct
#endif

    PUTDWORD((LPBYTE)envp_global->reg.sp,lpServer);
    invoke_binary();

    return MAKELONG(envp_global->reg.ax,envp_global->reg.dx);
}
