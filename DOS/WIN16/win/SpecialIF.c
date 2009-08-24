/*    
	SpecialIF.c	2.57
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
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "windows.h"
/*
#include "shellapi.h"
#include "toolhelp.h"
*/
#include "ddeml.h"
#define __MMSYSTEM_H__
#define __SHELLAPI_H__
#define __TOOLHELP_H__
#include "Willows.h"

#include "WinDefs.h"
#include "kerndef.h"
#include "Endian.h"
#include "Log.h"
#include "Kernel.h"
#include "Resources.h"
#include "DPMI.h"
#include "BinTypes.h"
#include "dos.h"
#include "mfs_config.h"
#include "Dialog.h"
#include "Net.h"
#include "ConvertRes.h"

/* external variables */
extern DSCR *LDT;

#define CARRY_FLAG	1

/* external functions */
extern LPBYTE ExtractDialog(HINSTANCE, LPBYTE);
extern void CleanupDialog(LPBYTE);
extern LPBYTE ExtractMenuTemplate(LPBYTE *);
extern void CleanupMenuTemplate(LPBYTE);
extern LPBYTE hswr_menu_bin_to_nat(LPBYTE);
extern HMENU TWIN_LoadMenuIndirect(LPBYTE);
extern void GetLOGFONT(LPLOGFONT,LPBYTE);
extern void GetBIHeader(BITMAPINFOHEADER *,LPBYTE);
extern UINT GetPSPSelector();
extern BINADDR make_binary_thunk(LPDWORD,DWORD);
extern DWORD make_native_thunk(DWORD,DWORD);
extern LPWORD hsw_ConvertWordArray(LPBYTE,int);
extern LPINT  hsw_ConvertArrayToInt(LPBYTE,int);
extern LPWORD hsw_ConvertArrayToWord(LPBYTE,int);
extern ERRORCODE *error_code(UINT);

/* internal functions */
static LPLONG ReadFormatString(LPSTR, BYTE *);
static BITMAPINFO *GetBitmapInfo(LPBYTE, UINT);
static HGLOBAL GetPackedDIB(HGLOBAL);
static void GetMODULEENTRY(LPBYTE, MODULEENTRY *);

void IT_SENDMESSAGE (ENV *,LONGPROC);

/*
 * Data area used for MakeProcInstance()
 */
WORD wProcInstanceSelector = 0;
BYTE *lpbProcInstanceMemory = NULL;
BYTE *lpbProcInstanceLast = NULL;
BYTE baProcInstanceTemplate[8] = 
{ 
    0xb8, 0, 0, 	/* MOV AX,<instance selector> */
    0xea, 0, 0, 0, 0	/* JMP FAR <exported function>    */
};

static LPLONG
ReadFormatString(LPSTR lpString,BYTE *lpSP)
{
	static LPSTR lpFormatChars = "cdiulxsX";
	static long *ap = NULL;
	static int nCount = 10;
	short	ival;
	char 	c;
	int 	i = 0;

	if (!ap) {
		ap = (long *)WinMalloc(nCount*sizeof(long));
		if (ap == NULL)
			return NULL;
	}
	while (*lpString) {
		while ((c=*lpString++) && (c != '%'));
		if (!c) break;
		if (*lpString != '%') {
			while (!strchr(lpFormatChars,(int)(c=*lpString++)));
			if (c == 'l') {
				if (*lpString == 's') {
				    ap[i++] = (long)GetAddress(
					GETWORD(lpSP+2),GETWORD(lpSP));
				    lpString++;
				}
				else
				    ap[i++] = (long)GETDWORD(lpSP);
				lpSP += 4;
			}
			else {
			    if (c == 's') {
				ap[i++] = (long)GetAddress(
					GETWORD(lpSP+2),GETWORD(lpSP));
				lpSP += 4;
			    }
			    else if (c == 'u') {	/* don't sign extend */
				ap[i++] = GETWORD(lpSP);
				lpSP += 2;
			    }
			    else {
				ival = (short) GETWORD(lpSP);
				ap[i++] = (long) ival;	/* force conversion */
				lpSP += 2;
			    }
			}
			if (i >= nCount) {
				nCount += 10;
				ap = (long *)WinRealloc
					((LPSTR)ap,nCount * sizeof(long));
				if (ap == NULL)
					return NULL;
			}
		}
		else
		        lpString++;
	}
	return (ap);
}

void
IT_LP1LP (ENV *envp,LONGPROC f)		/* Ansi functions */
{
	DWORD retcode;
	LPSTR lpString;
	WORD sel,offset;
	short delta;

	sel = GETWORD(SP+6);
	offset = GETWORD(SP+4);
	lpString = (LPSTR)GetAddress(sel,offset);
	retcode = (f)(lpString);
	delta = (short)((DWORD)lpString - retcode);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = offset - delta;
	envp->reg.dx = sel;
}

void
IT_1LPBMc (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	BITMAP bm;

	lpStruct = (LPBYTE)GetAddress
		(GETWORD(SP+6),GETWORD(SP+4));
	bm.bmType = (int)((short)GETWORD(lpStruct));
	bm.bmWidth = (int)((short)GETWORD(lpStruct+2));
	bm.bmHeight = (int)((short)GETWORD(lpStruct+4));
	bm.bmWidthBytes = (int)((short)GETWORD(lpStruct+6));
	memcpy((LPSTR)&bm.bmPlanes,(LPSTR)lpStruct+8,2);
	bm.bmBits = (LPBYTE)GetAddress
		(GETWORD(lpStruct+12),GETWORD(lpStruct+10));
	retcode = (f)(&bm);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CRBRUSHIND (ENV *envp,LONGPROC f)	/* CreateBrushIndirect */
{
	DWORD retcode;
	LPBYTE lpStruct;
	LOGBRUSH lb;
	HGLOBAL hGlobal;
	UINT uSel;

	lpStruct = (LPBYTE)GetAddress
		(GETWORD(SP+6),GETWORD(SP+4));
	lb.lbStyle = (UINT)GETWORD(lpStruct);
	lb.lbColor = GETDWORD(lpStruct+2);
	lb.lbHatch = (int)((short)GETWORD(lpStruct+6));
	if (lb.lbStyle == BS_DIBPATTERN) {
	    uSel = (UINT)lb.lbHatch;
	    if (!(hGlobal = GetPackedDIB((HGLOBAL)GetSelectorHandle(uSel))))
		retcode = 0;
	    else {
		lb.lbHatch = (int)hGlobal;
		retcode = CreateBrushIndirect(&lb);
		GlobalFree(hGlobal);
	    }
	}
	else
	    retcode = CreateBrushIndirect(&lb);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LPLFc (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	LOGFONT lf;

	lpStruct = (LPBYTE)GetAddress
		(GETWORD(SP+6),GETWORD(SP+4));
	GetLOGFONT(&lf,lpStruct);
	retcode = (f)(&lf);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LPLPc (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	LOGPEN lp;

	lpStruct = (LPBYTE)GetAddress
		(GETWORD(SP+6),GETWORD(SP+4));
	lp.lopnStyle = (UINT)GETWORD(lpStruct);
	GetPOINT(lp.lopnWidth,lpStruct+2);
	lp.lopnColor = (COLORREF)GETDWORD(lpStruct+6);
	retcode = (f)(&lp);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CREATEPAL (ENV *envp,LONGPROC f)	/* CreatePalette */
{
	DWORD retcode;
	LPBYTE lpStruct;
	LPLOGPALETTE lpLogPal;
	WORD palNumEntries;

	lpStruct = (LPBYTE)GetAddress
		(GETWORD(SP+6),GETWORD(SP+4));
	palNumEntries = GETWORD(lpStruct+2);
	lpLogPal = (LPLOGPALETTE)WinMalloc(sizeof(LOGPALETTE)+
			palNumEntries*sizeof(PALETTEENTRY));
	lpLogPal->palVersion = GETWORD(lpStruct);
	lpLogPal->palNumEntries = palNumEntries;
	memcpy((LPSTR)&lpLogPal->palPalEntry,(LPSTR)lpStruct+4,palNumEntries*4);
	retcode = CreatePalette(lpLogPal);
	WinFree((LPSTR)lpLogPal);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_LP2LP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;
	WORD sel,offset;
	short delta;

	sel = GETWORD(SP+10);
	offset = GETWORD(SP+8);
	lpString1 = (LPSTR)GetAddress(sel,offset);
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)(lpString1, lpString2);
	delta = (short)(retcode - (DWORD)lpString1);
	envp->reg.sp += 2*LP_86 + RET_86;
	envp->reg.ax = offset + delta;
	envp->reg.dx = sel;
}

void
IT_1LPUI1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpArray;
	UINT *lpUint;
	int count;

	lpArray = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	count = (int)((short)GETWORD(SP+4));
	lpUint = (UINT *)hsw_ConvertArrayToInt(lpArray,count);
	retcode = (f)(lpArray,count);
	envp->reg.sp += INT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPPS (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	PAINTSTRUCT ps;
	LPBYTE lpStruct;

#ifdef	SEVEREDEBUG
	memset((LPSTR)&ps,'\0',sizeof(PAINTSTRUCT));
#endif

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)((HANDLE)GETWORD(SP+8),&ps);
	PUTWORD(lpStruct,(WORD)ps.hdc);
	PUTWORD(lpStruct+2,(WORD)ps.fErase);
	PUTWORD(lpStruct+4,(WORD)ps.rcPaint.left);
	PUTWORD(lpStruct+6,(WORD)ps.rcPaint.top);
	PUTWORD(lpStruct+8,(WORD)ps.rcPaint.right);
	PUTWORD(lpStruct+10,(WORD)ps.rcPaint.bottom);
	PUTWORD(lpStruct+12,(WORD)ps.fRestore); 
	PUTWORD(lpStruct+14,(WORD)ps.fIncUpdate);
	memcpy((LPSTR)lpStruct+16,(LPSTR)ps.rgbReserved,16);
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPPSc (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	PAINTSTRUCT ps;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	ps.hdc = (HDC)GETWORD(lpStruct);
	ps.fErase = (BOOL)GETWORD(lpStruct+2);
	ps.rcPaint.left = GETWORD(lpStruct+4);
	ps.rcPaint.top = GETWORD(lpStruct+6);
	ps.rcPaint.right = GETWORD(lpStruct+8);
	ps.rcPaint.bottom = GETWORD(lpStruct+10);
	ps.fRestore = (BOOL)GETWORD(lpStruct+12);
	ps.fIncUpdate = (BOOL)GETWORD(lpStruct+14);
	memcpy((LPSTR)ps.rgbReserved,(LPSTR)lpStruct+16,16);
	retcode = (f)((HANDLE)GETWORD(SP+8),&ps);
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPTM (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	TEXTMETRIC tm;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)((HANDLE)GETWORD(SP+8),&tm);
	PUTWORD(lpStruct,(WORD)tm.tmHeight);
	PUTWORD(lpStruct+2,(WORD)tm.tmAscent);
	PUTWORD(lpStruct+4,(WORD)tm.tmDescent);
	PUTWORD(lpStruct+6,(WORD)tm.tmInternalLeading);
	PUTWORD(lpStruct+8,(WORD)tm.tmExternalLeading);
	PUTWORD(lpStruct+10,(WORD)tm.tmAveCharWidth);
	PUTWORD(lpStruct+12,(WORD)tm.tmMaxCharWidth);
	PUTWORD(lpStruct+14,(WORD)tm.tmWeight);
	memcpy((LPSTR)lpStruct+16,(LPSTR)&tm.tmItalic,9);
	PUTWORD(lpStruct+25,(WORD)tm.tmOverhang);
	PUTWORD(lpStruct+27,(WORD)tm.tmDigitizedAspectX);
	PUTWORD(lpStruct+29,(WORD)tm.tmDigitizedAspectY);
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPWP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	WINDOWPLACEMENT wp;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	wp.length = (UINT)sizeof(WINDOWPLACEMENT);
	retcode = (f)((HANDLE)GETWORD(SP+8),&wp);
	PUTWORD(lpStruct+2,(WORD)wp.flags); 
	PUTWORD(lpStruct+4,(WORD)wp.showCmd);
	PutPOINT(lpStruct+6,wp.ptMinPosition);
	PutPOINT(lpStruct+10,wp.ptMaxPosition);
	RECT_TO_86(lpStruct+14,wp.rcNormalPosition);
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPWPc (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	WINDOWPLACEMENT wp;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	wp.length = (UINT)GETWORD(lpStruct);
	wp.flags = (UINT)GETWORD(lpStruct+2); 
	wp.showCmd = (UINT)GETWORD(lpStruct+4);
	GetPOINT(wp.ptMinPosition,lpStruct+6);
	GetPOINT(wp.ptMaxPosition,lpStruct+10);
	RECT_TO_C(wp.rcNormalPosition,lpStruct+14);
	retcode = (f)((HANDLE)GETWORD(SP+8),&wp);
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_ENUMPROPS (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	DWORD dwProc;

	dwProc = GETDWORD(SP+4);
	dwProc = (dwProc)?make_native_thunk(dwProc,(DWORD)hsw_enumprops):0L;
	retcode = (f)((HANDLE)GETWORD(SP+8),dwProc);
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_ENUMWINDOWS (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	DWORD dwProc;

	dwProc = GETDWORD(SP+8);
	dwProc = (dwProc)?make_native_thunk(dwProc,(DWORD)hsw_wndenumproc):0L;
	retcode = EnumWindows((WNDENUMPROC)dwProc,(LPARAM)GETDWORD(SP+4));
	envp->reg.sp += LONG_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETMODULEFNAME (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;
	char lpUNIXName[_MAX_PATH];
	char lpDOSName[_MAX_PATH];
	int nLength;
	WORD sel;
	HINSTANCE hInst;

	sel = GETWORD(SP+8);
	nLength = (int)((short)GETWORD(SP+4));
	lpString = (LPSTR)GetAddress(sel,GETWORD(SP+6));
	hInst = GetSelectorHandle(GETWORD(SP+10));
	retcode = (f)((HANDLE)hInst,
			lpUNIXName,_MAX_PATH);
	if (retcode) {
            xdoscall(XDOS_GETDOSPATH,0,(void *) lpDOSName,(void *) lpUNIXName);
	    strncpy(lpString, lpDOSName, min(nLength,( int)strlen(lpDOSName)+1));
	    retcode = strlen(lpString);
	}
	else
	    *lpString = '\0'; 
	envp->reg.sp += HANDLE_86 + LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = sel;
}

void
IT_ENUMCHILDWIN (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	DWORD dwProc;

	dwProc = GETDWORD(SP+8);
	dwProc = (dwProc)?make_native_thunk(dwProc,(DWORD)hsw_wndenumproc):0L;
	retcode = EnumChildWindows((HWND)GETWORD(SP+12),
			(WNDENUMPROC)dwProc,
			(LPARAM)GETDWORD(SP+4));
	envp->reg.sp += HANDLE_86 + LP_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_ENUMTASKWIN (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	DWORD dwProc;
	HTASK hTask;

	dwProc = GETDWORD(SP+8);
	dwProc = (dwProc)?make_native_thunk(dwProc,(DWORD)hsw_wndenumproc):0L;
	hTask = GetSelectorHandle(GETWORD(SP+12));
	retcode = EnumTaskWindows(hTask,
			(WNDENUMPROC)dwProc,
			(LPARAM)GETDWORD(SP+4));
	envp->reg.sp += HANDLE_86 + LP_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_ENUMFONTS (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	DWORD dwProc;
	LPSTR lpString;

	dwProc = GETDWORD(SP+8);
	dwProc = (dwProc)?make_native_thunk(dwProc,(DWORD)hsw_fontenumproc):0L;
	lpString = (LPSTR)GetAddress(GETWORD(SP+14),
			GETWORD(SP+12));
	retcode = (f)((HANDLE)GETWORD(SP+16),
			lpString,
			dwProc,
			GETDWORD(SP+4));
	envp->reg.sp += HANDLE_86 + 2*LP_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETCLASSINFO (ENV *envp,LONGPROC f)	/* GetClassInfo */
{
	DWORD retcode,dwTemp;
	LPBYTE lpStruct;
	LPSTR lpString;
	WNDCLASS wc;
	HINSTANCE hInst;

	lpString = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	wc.style = GETWORD(lpStruct);
	wc.cbClsExtra = (int)GETWORD(lpStruct+6);
	wc.cbWndExtra = (int)GETWORD(lpStruct+8);
	wc.hInstance = (HINSTANCE)GETWORD(lpStruct+10);
	wc.hIcon = (HICON)GETWORD(lpStruct+12);
	wc.hCursor = (HCURSOR)GETWORD(lpStruct+14);
	wc.hbrBackground = (HBRUSH)GETWORD(lpStruct+16);
	hInst = GetSelectorHandle(GETWORD(SP+12));
	retcode = GetClassInfo((HANDLE) hInst,
			lpString,
			&wc);
	if (retcode) {
	    PUTWORD(lpStruct,(WORD)wc.style);
	    dwTemp = make_binary_thunk((LPDWORD)wc.lpfnWndProc,
					(DWORD)IT_SENDMESSAGE);
	    PUTDWORD(lpStruct+2,dwTemp);
	    PUTWORD(lpStruct+6,(WORD)wc.cbClsExtra);
	    PUTWORD(lpStruct+8,(WORD)wc.cbWndExtra);
	    PUTWORD(lpStruct+10,
		    (WORD)GetDataSelectorFromInstance(wc.hInstance));
	    PUTWORD(lpStruct+12,(WORD)wc.hIcon);
	    PUTWORD(lpStruct+14,(WORD)wc.hCursor);
	    PUTWORD(lpStruct+16,(WORD)wc.hbrBackground);
	    /* put zero in menuname field, copy initial
		class name to classname field */
	    PUTDWORD(lpStruct+18,0L);
	    dwTemp = GETDWORD(SP+8);
	    PUTDWORD(lpStruct+22,dwTemp);
	}
	envp->reg.sp += HANDLE_86 + 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_INSERTMENU (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	UINT uFlags;
	LPSTR lpString;

	uFlags = GETWORD(SP+10);
	if (!(uFlags & (MF_BITMAP | MF_OWNERDRAW))) {
	    lpString = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	    retcode = (f)((HANDLE)GETWORD(SP+14),
			(UINT)GETWORD(SP+12),
			uFlags,
			(UINT)GETWORD(SP+8),
			lpString);
	}
	else
	    retcode = (f)((HANDLE)GETWORD(SP+14),
			(UINT)GETWORD(SP+12),
			uFlags,
			(UINT)GETWORD(SP+8),
			GETDWORD(SP+4));
	envp->reg.sp += HANDLE_86 + 3*UINT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_APPENDMENU (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	UINT uFlags;
	LPSTR lpString;

	uFlags = GETWORD(SP+10);
	if (!(uFlags & (MF_BITMAP | MF_OWNERDRAW))) {
	    lpString = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	    retcode = (f)((HANDLE)GETWORD(SP+12),
			uFlags,
			(UINT)GETWORD(SP+8),
			lpString);
	}
	else
	    retcode = (f)((HANDLE)GETWORD(SP+12),
			uFlags,
			(UINT)GETWORD(SP+8),
			GETDWORD(SP+4));
	envp->reg.sp += HANDLE_86 + 2*UINT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2H1LPP1UI (ENV *envp,LONGPROC f)		/* MapWindowPoints */
{
	DWORD retcode;
	UINT count;
	LPBYTE lpStruct;
	static POINT *lppt = NULL;
	static UINT nMaxCount = 0;
	RECT rcRect;
	POINT pt;
	int i;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	count = (UINT)GETWORD(SP+4);
	if (count == 1) {
		GetPOINT(pt,lpStruct);
		retcode = f((HANDLE)GETWORD(SP+12),
			(HANDLE)GETWORD(SP+10),
			&pt,
			count);
		PutPOINT(lpStruct,pt);
	}
	else if (count == 2) {
		RECT_TO_C(rcRect,lpStruct);
		retcode = f((HANDLE)GETWORD(SP+12),
			(HANDLE)GETWORD(SP+10),
			&rcRect,
			count);
		RECT_TO_86(lpStruct,rcRect);
	}
	else {
		if (count > nMaxCount) {
			if (!lppt)
				lppt = (LPPOINT)WinMalloc(count*sizeof(POINT));
			else
				lppt = (LPPOINT)WinRealloc((LPSTR)lppt,count*sizeof(POINT));
			nMaxCount = count;
		}
		for (i = 0; i < (int)count; i++) 
			GetPOINT((*(LPPOINT)(lppt+i)),lpStruct+(2*INT_86+i));
		retcode = f((HANDLE)GETWORD(SP+12),
			(HANDLE)GETWORD(SP+10),
			lppt,
			count);
		for (i = 0; i < (int)count; i++)
			PutPOINT(lpStruct+(2*INT_86*i),(*(LPPOINT)(lppt+i)));
	}
	envp->reg.sp += 2*HANDLE_86 + LP_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_QUERYSENDMSG (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	MSG msg;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)((HANDLE)NULL,
			(HANDLE)NULL,
			(HANDLE)NULL,
			&msg);
	PUTWORD(lpStruct,(HANDLE)msg.hwnd);
	PUTWORD(lpStruct+2,(UINT)msg.message);
	PUTWORD(lpStruct+4,(WPARAM)msg.wParam);
	PUTDWORD(lpStruct+6,(LPARAM)msg.lParam);
	PUTDWORD(lpStruct+10,(DWORD)msg.time);
	PutPOINT(lpStruct+14,msg.pt);
	envp->reg.sp += 3*HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETMESSAGE (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	MSG msg;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	retcode = (f)(&msg,
			(HANDLE)GETWORD(SP+8),
			(UINT)GETWORD(SP+6),
			(UINT)GETWORD(SP+4));
	hsw_getmessage_nat_to_bin(lpStruct,&msg);
	retcode = (GETWORD(lpStruct+2) != WM_QUIT);
	envp->reg.sp += LP_86 + HANDLE_86 + 2*UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SETTIMER (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	DWORD   dwProc;

	dwProc = GETDWORD(SP+4);
	if (HIWORD(dwProc)) {
		dwProc = make_native_thunk(dwProc,(DWORD)hsw_timerproc);
	}
	retcode = (f)((HANDLE)GETWORD(SP+12),
			(UINT)GETWORD(SP+10),
			(UINT)GETWORD(SP+8),
			dwProc);
	envp->reg.sp += HANDLE_86 + 2*UINT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_ENUMOBJECTS (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	HDC hDC;
	WORD wObjType;
	DWORD dwProc;
	LPARAM lParam;

	lParam = (LPARAM)GETDWORD(SP+4);
	dwProc = GETDWORD(SP+8);
	wObjType = GETWORD(SP+12);
	hDC = (HDC)GETWORD(SP+14);

	if (dwProc) 
	{
	    if (wObjType == OBJ_BRUSH)
		dwProc = make_native_thunk(dwProc, (DWORD)hsw_enumbrushproc);
	    else if (wObjType == OBJ_PEN)
		dwProc = make_native_thunk(dwProc, (DWORD)hsw_enumpenproc);
	}

	retcode = (f) (hDC, wObjType, dwProc, lParam);

	envp->reg.sp += HANDLE_86 + INT_86 + LP_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_DIALOG (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;
	DWORD dwProc;
	HINSTANCE hInst;

	lpString = (LPSTR)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	dwProc = GETDWORD(SP+4);
	dwProc = (dwProc)?
		make_native_thunk(dwProc,(DWORD)hsw_common_nat_to_bin):0L;
	hInst = GetSelectorHandle(GETWORD(SP+14));
	retcode = (f)((HANDLE)hInst,
			lpString,
			(HANDLE)GETWORD(SP+8),
			dwProc);
	envp->reg.sp += 2*HANDLE_86 + 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_DLGPARAM (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	DWORD dwProc;
	LPSTR lpString;
	HINSTANCE hInst;

	lpString = (LPSTR)GetAddress(GETWORD(SP+16),GETWORD(SP+14));
	dwProc = GETDWORD(SP+8);
	dwProc = (dwProc)?make_native_thunk(dwProc,
					(DWORD)hsw_common_nat_to_bin):0L;
	hInst = GetSelectorHandle(GETWORD(SP+18));
	retcode = (f)((HANDLE)hInst,
			lpString,
			(HANDLE)GETWORD(SP+12),
			dwProc,
			(LONG)GETDWORD(SP+4));
	envp->reg.sp += 2*HANDLE_86 + 2*LP_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_LINEDDA (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	DWORD dwProc;

	dwProc = GETDWORD(SP+8);
	dwProc = (dwProc)?make_native_thunk(dwProc,(DWORD)hsw_lineddaproc):0L;
	retcode = (f)((int)((short)GETWORD(SP+18)),
			(int)((short)GETWORD(SP+16)),
			(int)((short)GETWORD(SP+14)),
			(int)((short)GETWORD(SP+12)),
			dwProc,
			(LONG)GETDWORD(SP+4));
	envp->reg.sp += 4*INT_86 + LP_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GRAYSTRING (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	DWORD dwProc;
	LPSTR lpString;

	dwProc = GETDWORD(SP+18);
	dwProc =
	    (dwProc)?make_native_thunk(dwProc,(DWORD)hsw_graystringproc):0L;
	lpString = (dwProc)?(LPSTR)GETDWORD(SP+14):
			    (LPSTR)GetAddress(GETWORD(SP+16),GETWORD(SP+14));
	retcode = (f)((HANDLE)GETWORD(SP+24),
			(HANDLE)GETWORD(SP+22),
			dwProc,
			lpString,
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += 2*HANDLE_86 + LP_86 + LONG_86 + 5*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

/* messaging APIs - special treatment */
void
IT_DISPATCHMSG (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	MSG msg;
	WMCONVERT *lpConvert;
	WMCONVERT wmc;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	msg.message = (UINT)(int)GETSHORT(lpStruct+2);
	msg.hwnd = (HANDLE)GETWORD(lpStruct);
	msg.time = (DWORD)GETWORD(lpStruct+10);
	GetPOINT(msg.pt,lpStruct+14);
	if (msg.message == WM_CONVERT) {
	    if (!(msg.wParam & HSWMC_NATMSG))
		FatalAppExit(0,"non-native WM_CONVERT in DISPATCHMSG\n");
	    else {
		lpConvert = (WMCONVERT *)GETDWORD(lpStruct+6);
		msg.message = lpConvert->uMsg;
		msg.wParam = lpConvert->wParam;
		msg.lParam = lpConvert->lParam;
	    }
	}
	else {
	    if ((msg.message != WM_TIMER) && 
		(msg.message != WM_SYSTIMER) &&
		(msg.message != WM_MOUSEMOVE)) {
		    wmc.uMsg = msg.message;
		    wmc.wParam = (WPARAM)GETWORD(lpStruct+4);
		    wmc.lParam = (LPARAM)GETDWORD(lpStruct+6);
		    wmc.targ = (LONGPROC)NULL;
		    msg.wParam = HSWMC_BINMSG;
		    msg.message = WM_CONVERT;
		    msg.lParam = (LPARAM)&wmc;
	    }
	    else {
		msg.wParam = (WPARAM)GETWORD(lpStruct+4);
		msg.lParam = (LPARAM)GETDWORD(lpStruct+6);
	    }
	}
	retcode = (f)(&msg);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_TRANSLATEMSG (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	MSG msg;

	lpStruct = (LPBYTE)GetAddress
		(GETWORD(SP+6),GETWORD(SP+4));
	msg.message = (UINT)GETWORD(lpStruct+2);
	if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) {
		msg.hwnd = (HANDLE)GETWORD(lpStruct);
		msg.wParam = GETWORD(lpStruct+4);
		msg.lParam = GETDWORD(lpStruct+6);
		msg.time = GETDWORD(lpStruct+10);
		msg.pt.x = GETWORD(lpStruct+14);
		msg.pt.y = GETWORD(lpStruct+16);
		retcode = (f)(&msg);
	}
	else
		retcode = 0;
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_ISDIALOGMSG (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	MSG msg;
	WMCONVERT *lpConvert;
	WMCONVERT wmc;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	msg.message = (UINT)GETWORD(lpStruct+2);
	msg.hwnd = (HANDLE)GETWORD(lpStruct);
	msg.time = (DWORD)GETWORD(lpStruct+10);
	GetPOINT(msg.pt,lpStruct+14);
	if ((msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) ||
	    (msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST)) {
		msg.wParam = (WPARAM)GETWORD(lpStruct+4);
		msg.lParam = (LPARAM)GETDWORD(lpStruct+6);
	}
	else {
	    if (msg.message == WM_CONVERT) {
		if (!(msg.wParam & HSWMC_NATMSG))
		    FatalAppExit(0,"non-native WM_CONVERT in ISDIALOGMSG\n");
		else {
		    lpConvert = (WMCONVERT *)GETDWORD(lpStruct+6);
		    msg.message = lpConvert->uMsg;
		    msg.wParam = lpConvert->wParam;
		    msg.lParam = lpConvert->lParam;
		}
	    }
	    else {
		wmc.uMsg = msg.message;
		wmc.wParam = (WPARAM)GETWORD(lpStruct+4);
		wmc.lParam = (LPARAM)GETDWORD(lpStruct+6);
		wmc.targ = (LONGPROC)NULL;
		msg.wParam = HSWMC_BINMSG;
		msg.message = WM_CONVERT;
		msg.lParam = (LPARAM)&wmc;
	    }
	}
	retcode = (f)((HANDLE)GETWORD(SP+8),&msg);
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_TRANSLATEMDISYSACCEL (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	MSG msg;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	msg.message = (UINT)GETWORD(lpStruct+2);
	if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) {
		msg.hwnd = (HANDLE)GETWORD(lpStruct);
		msg.wParam = (WPARAM)GETWORD(lpStruct+4);
		msg.lParam = (LPARAM)GETDWORD(lpStruct+6);
		msg.time = (DWORD)GETWORD(lpStruct+10);
		GetPOINT(msg.pt,lpStruct+14);
		retcode = (f)((HANDLE)GETWORD(SP+8),&msg);
		PUTWORD(lpStruct,(HANDLE)msg.hwnd);
		PUTWORD(lpStruct+2,(UINT)msg.message);
		PUTWORD(lpStruct+4,(WPARAM)msg.wParam);
		PUTDWORD(lpStruct+6,(LPARAM)msg.lParam);
		PUTDWORD(lpStruct+10,(DWORD)msg.time);
		PutPOINT(lpStruct+14,msg.pt);
	}
	else
		retcode = 0;
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CALLMSGFILTER (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	MSG msg;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	msg.hwnd = (HANDLE)GETWORD(lpStruct);
	msg.message = (UINT)GETWORD(lpStruct+2);
	msg.wParam = (WPARAM)GETWORD(lpStruct+4);
	msg.lParam = (LPARAM)GETDWORD(lpStruct+6);
	msg.time = (DWORD)GETWORD(lpStruct+10);
	GetPOINT(msg.pt,lpStruct+14);
	retcode = (f)(&msg,(int)((short)GETWORD(SP+4)));
	PUTWORD(lpStruct,(HANDLE)msg.hwnd);
	PUTWORD(lpStruct+2,(UINT)msg.message);
	PUTWORD(lpStruct+4,(WPARAM)msg.wParam);
	PUTDWORD(lpStruct+6,(LPARAM)msg.lParam);
	PUTDWORD(lpStruct+10,(DWORD)msg.time);
	PutPOINT(lpStruct+14,msg.pt);
	envp->reg.sp += INT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_TRANSLATEACCEL (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	HGLOBAL hAccTable;
	MSG msg;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	msg.message = (UINT)GETWORD(lpStruct+2);
	if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST) {
		msg.hwnd = (HANDLE)GETWORD(lpStruct);
		msg.wParam = (WPARAM)GETWORD(lpStruct+4);
		msg.lParam = (LPARAM)GETDWORD(lpStruct+6);
		msg.time = (DWORD)GETWORD(lpStruct+10);
		GetPOINT(msg.pt,lpStruct+14);
		hAccTable = GetSelectorHandle(GETWORD(SP+8));
		retcode = (f)((HANDLE)GETWORD(SP+10),
			hAccTable,
			&msg);
		PUTWORD(lpStruct,(HANDLE)msg.hwnd);
		PUTWORD(lpStruct+2,(UINT)msg.message);
		PUTWORD(lpStruct+4,(WPARAM)msg.wParam);
		PUTDWORD(lpStruct+6,(LPARAM)msg.lParam);
		PUTDWORD(lpStruct+10,(DWORD)msg.time);
		PutPOINT(lpStruct+14,msg.pt);
	}
	else
		retcode = 0;
	envp->reg.sp += 2*HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SENDMESSAGE (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	WMCONVERT wmc;

#ifdef	SEVEREDEBUG
	memset((LPSTR)&wmc,'\0',sizeof(WMCONVERT));
#endif

	wmc.uMsg = (UINT)GETWORD(SP+10);
	if (wmc.uMsg == WM_CONVERT)
	    FatalAppExit(0,"IT_SENDMESSAGE should not get WM_CONVERT!");
	wmc.wParam = (WORD)GETWORD(SP+8);
	wmc.lParam = GETDWORD(SP+4);
	wmc.targ = f;
	retcode = (f)((HWND)GETWORD(SP+12),WM_CONVERT,
			HSWMC_BINMSG,(DWORD)&wmc);
	envp->reg.sp += HANDLE_86 + UINT_86 + WORD_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_POSTAPPMSG (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	WMCONVERT wmc;
	HTASK hTask;

#ifdef	SEVEREDEBUG
	memset((LPSTR)&wmc,'\0',sizeof(WMCONVERT));
#endif

	wmc.uMsg = (UINT)GETWORD(SP+10);
	if (wmc.uMsg == WM_CONVERT)
	    FatalAppExit(0,"IT_POSTAPPMSG should not get WM_CONVERT!");
	wmc.wParam = (WORD)GETWORD(SP+8);
	wmc.lParam = GETDWORD(SP+4);
	wmc.targ = f;
	hTask = GetSelectorHandle(GETWORD(SP+12));
	retcode = (f)(hTask,WM_CONVERT,HSWMC_BINMSG,(DWORD)&wmc);
	envp->reg.sp += HANDLE_86 + UINT_86 + WORD_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SENDDLGITEMMSG (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	WMCONVERT wmc;

#ifdef	SEVEREDEBUG
	memset((LPSTR)&wmc,'\0',sizeof(WMCONVERT));
#endif

	wmc.uMsg = (UINT)GETWORD(SP+10);
	if (wmc.uMsg == WM_CONVERT)
	    FatalAppExit(0,"IT_SENDMESSAGE should not get WM_CONVERT!");
	wmc.wParam = (WORD)GETWORD(SP+8);
	wmc.lParam = GETDWORD(SP+4);
	wmc.targ = (LONGPROC)0;
	retcode = (f)((HWND)GETWORD(SP+14),
			(int)((short)GETWORD(SP+12)),
			WM_CONVERT,
			HSWMC_BINMSG,
			(DWORD)&wmc);
	envp->reg.sp += HANDLE_86 + INT_86 + UINT_86 + WORD_86
		+ LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_PEEKMESSAGE (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	MSG msg;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	if ((retcode = (f)(&msg,
			(HANDLE)GETWORD(SP+10),
			(UINT)GETWORD(SP+8),
			(UINT)GETWORD(SP+6),
			(UINT)GETWORD(SP+4)))) {
	hsw_getmessage_nat_to_bin(lpStruct,&msg);
    }
	envp->reg.sp += LP_86 + HANDLE_86 + 3*UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

/* end of messaging stuff */

void
IT_CALLWINPROC (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	WMCONVERT wmc;
	HWND hWnd;
	DWORD dwConvert;
#ifdef	STRICT
	WNDPROC lpFunc;
#else
	FARPROC lpFunc;
#endif

#ifdef	SEVEREDEBUG
	memset((LPSTR)&wmc,'\0',sizeof(WMCONVERT));
#endif

	wmc.targ = (LONGPROC)NULL;
	hWnd = GETWORD(SP+12);
	dwConvert =  GetClassLong(hWnd,GCL_NATTOBIN);
	lpFunc =
#ifdef	STRICT
		(WNDPROC)
#else
		(FARPROC)
#endif
			make_native_thunk(GETDWORD(SP+14),dwConvert);
	wmc.uMsg = (UINT)GETWORD(SP+10);
	wmc.wParam = (WORD)GETWORD(SP+8);
	wmc.lParam = (LONG)GETDWORD(SP+4);
	retcode = CallWindowProc(lpFunc,
			hWnd,
			WM_CONVERT,
			HSWMC_DEST_WINDOW_PROC | HSWMC_BINMSG,
			(DWORD)&wmc);
	envp->reg.sp += HANDLE_86 + LP_86 + UINT_86 + WORD_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CREATEDIBM (ENV *envp,LONGPROC f)	/* CreateDIBitmap */
{
	DWORD retcode;
	LPSTR lpString;
	LPBYTE lpStruct;
	BITMAPINFO *lpbmi;
	BITMAPINFOHEADER bmih;
	UINT fuColorUse;

	lpString = (LPSTR)GetAddress
			(GETWORD(SP+12),GETWORD(SP+10));
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+8),GETWORD(SP+6));
	fuColorUse = (UINT)GETWORD(SP+4);
	lpbmi = GetBitmapInfo(lpStruct,fuColorUse);
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+20),GETWORD(SP+18));
	GetBIHeader(&bmih,lpStruct);
	retcode = CreateDIBitmap((HANDLE)GETWORD(SP+22),
			&bmih,
			GETDWORD(SP+14),
			lpString,
			lpbmi,
			fuColorUse);
	WinFree((LPSTR)lpbmi);
	envp->reg.sp += HANDLE_86 + 3*LP_86 + UINT_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CREATEDIBPB (ENV *envp,LONGPROC f)	/* CreateDIBPatternBrush */
{
	DWORD retcode;
	HGLOBAL hGlobal,hBinGlobal;
	UINT uSel;

	uSel = (UINT)GETWORD(SP+6);
	hBinGlobal = GetSelectorHandle(uSel);
	if (!(hGlobal = GetPackedDIB(hBinGlobal)))
	    retcode = 0;
	else {
	    retcode = CreateDIBPatternBrush(hGlobal,GETWORD(SP+4));
	    GlobalFree(hGlobal);
	}
	envp->reg.sp += HANDLE_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CREATEDLGIN (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	DWORD dwProc;
	HINSTANCE hInst;
	LPBYTE lpStruct;

	dwProc = GETDWORD(SP+4);
	dwProc = (dwProc)?make_native_thunk(dwProc,
					(DWORD)hsw_common_nat_to_bin):0L;
	hInst = (HINSTANCE)GetSelectorHandle(GETWORD(SP+14));
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+12),GETWORD(SP+10));
	lpStruct = ExtractDialog(hInst,lpStruct);
	retcode = InternalCreateDialogIndirectParam(hInst,
			lpStruct,
			(HANDLE)GETWORD(SP+8),
			(DLGPROC)dwProc,0L);
	CleanupDialog(lpStruct);
	envp->reg.sp += 2*HANDLE_86 + 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CREATEDLGIP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	DWORD dwProc;
	HINSTANCE hInst;
	LPBYTE lpStruct;

	dwProc = GETDWORD(SP+8);
	dwProc = (dwProc)?make_native_thunk(dwProc,
					(DWORD)hsw_common_nat_to_bin):0L;
	hInst = (HINSTANCE)GetSelectorHandle(GETWORD(SP+18));
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+16),GETWORD(SP+14));
	lpStruct = ExtractDialog(hInst,lpStruct);
	retcode = InternalCreateDialogIndirectParam(hInst,
			lpStruct,
			(HANDLE)GETWORD(SP+12),
			(DLGPROC)dwProc,
			(LPARAM)GETDWORD(SP+4));
	CleanupDialog(lpStruct);
	envp->reg.sp += 2*HANDLE_86 + 3*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_DLGBOXIN (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	DWORD dwProc;
	HINSTANCE hInst;
	LPBYTE lpStruct;
	DIALOGDATA *lpDlg;
	WORD wSel;
	HWND hWnd;
	HWND hDlg;

	dwProc = GETDWORD(SP+4);
	dwProc = (dwProc)?make_native_thunk(dwProc,
					(DWORD)hsw_common_nat_to_bin):0L;
	hInst = (HINSTANCE)GetSelectorHandle(GETWORD(SP+12));
	wSel = (HGLOBAL)GETWORD(SP+10);
	hWnd = (HANDLE)GETWORD(SP+8);
	lpStruct = GetPhysicalAddress(wSel);

	lpDlg = (DIALOGDATA *)ExtractDialog(hInst,lpStruct);
	/* modal dialogs are always visible */
	lpDlg->lStyle |= WS_VISIBLE;

	hDlg = InternalCreateDialogIndirectParam(hInst,
		lpDlg,
		hWnd,
		(DLGPROC)dwProc,
		0);
	CleanupDialog((LPBYTE)lpDlg);

	if (hDlg == 0)
	    retcode = -1L;
	else {
	    if (hWnd)
		EnableWindow(hWnd,FALSE);
	    retcode = InternalDialog(hDlg);
	}

	envp->reg.sp += 3*HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_DLGBOXIP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	DWORD dwProc;
	HINSTANCE hInst;
	LPBYTE lpStruct;
	DIALOGDATA *lpDlg;
	WORD wSel;
	HWND hWnd;
	HWND hDlg;

	dwProc = GETDWORD(SP+8);
	dwProc = (dwProc)?make_native_thunk(dwProc,
					(DWORD)hsw_common_nat_to_bin):0L;
	hInst = (HINSTANCE)GetSelectorHandle(GETWORD(SP+16));
	wSel = (HGLOBAL)GETWORD(SP+14);
	hWnd = (HWND)GETWORD(SP+12);
	lpStruct = GetPhysicalAddress(wSel);

	lpDlg = (DIALOGDATA *)ExtractDialog(hInst,lpStruct);
	/* modal dialogs are always visible */
	lpDlg->lStyle |= WS_VISIBLE;

	hDlg = InternalCreateDialogIndirectParam(hInst,
		lpDlg,
		hWnd,
		(DLGPROC)dwProc,
		GETDWORD(SP+4));
	CleanupDialog((LPBYTE)lpDlg);

	if (hDlg == 0)
	    retcode = -1L;
	else {
	    if (hWnd)
		EnableWindow(hWnd,FALSE);
	    retcode = InternalDialog(hDlg);
	}

	envp->reg.sp += 3*HANDLE_86 + 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CREATEFONT (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress
			(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)((int)((short)GETWORD(SP+32)),
			(int)((short)GETWORD(SP+30)),
			(int)((short)GETWORD(SP+28)),
			(int)((short)GETWORD(SP+26)),
			(int)((short)GETWORD(SP+24)),
			(BYTE)GETWORD(SP+22),
			(BYTE)GETWORD(SP+20),
			(BYTE)GETWORD(SP+18),
			(BYTE)GETWORD(SP+16),
			(BYTE)GETWORD(SP+14),
			(BYTE)GETWORD(SP+12),
			(BYTE)GETWORD(SP+10),
			(BYTE)GETWORD(SP+8),
			lpString);
	envp->reg.sp += 5*INT_86 + 8*WORD_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CREATEWIN (ENV *envp,LONGPROC f)	/* CreateWindow */
{
	DWORD retcode;
	LPSTR lpString1,lpString2;
	LPBYTE lpStruct;
	WORD w1,w2,w3,w4;
	CLIENTCREATESTRUCT ccs;
	BOOL fMDIClient = FALSE;
	WNDCLASS ClassInfo;
	HINSTANCE hInst;
	DWORD dwTemp;

	lpString1 = (LPSTR)GetAddress
			(GETWORD(SP+32),GETWORD(SP+30));
	lpString2 = (LPSTR)GetAddress
			(GETWORD(SP+28),GETWORD(SP+26));

	hInst = (HINSTANCE)GetSelectorHandle(GETWORD(SP+8));
	dwTemp = (DWORD)hInst;
	if (!(dwTemp & OM_MASK)) {	/* not a handle, maybe a selector */
	    LPMEMORYINFO lpMemory;
	    hInst = (HINSTANCE)GetSelectorHandle(dwTemp);
	    if (!(lpMemory = GETHANDLEINFO(hInst))) {	/* ho handle behind, fail... */
		envp->reg.sp += 3*HANDLE_86 + DWORD_86 +
				4*INT_86 + 3*LP_86 + RET_86;
		envp->reg.ax = 0;
		envp->reg.dx = 0;
		return;
	    }
	    else
	        RELEASEHANDLEINFO(lpMemory);
	}

	if (!strcasecmp(lpString1,"MDICLIENT")) 
	    fMDIClient = TRUE;
	else {
	    if (GetClassInfo(hInst,lpString1,&ClassInfo))
		if (ClassInfo.lpfnWndProc == &DefMDICLIENTProc) 
		    fMDIClient = TRUE;
	}
	if (fMDIClient) {
	    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	    ccs.hWindowMenu = (HMENU)GETWORD(lpStruct);
	    ccs.idFirstChild = (UINT)((short)GETWORD(lpStruct+2));
	    lpStruct = (LPBYTE)&ccs;
	}
	else
	    lpStruct = (LPBYTE)GETDWORD(SP+4);
	w1 = GETWORD(SP+20);
	w2 = GETWORD(SP+18);
	w3 = GETWORD(SP+16);
	w4 = GETWORD(SP+14);
	retcode = CreateWindow(lpString1,
			lpString2,
			(DWORD)GETDWORD(SP+22),
			(w1 == CW_USEDEFAULT16)?
				(int)CW_USEDEFAULT:(int)((short)w1),
			(w2 == CW_USEDEFAULT16)?
				(int)CW_USEDEFAULT:(int)((short)w2),
			(w3 == CW_USEDEFAULT16)?
				(int)CW_USEDEFAULT:(int)((short)w3),
			(w4 == CW_USEDEFAULT16)?
				(int)CW_USEDEFAULT:(int)((short)w4),
			(HANDLE)GETWORD(SP+12),
			(HANDLE)GETWORD(SP+10),
			hInst,
			(LPVOID)lpStruct);
	envp->reg.sp += 3*HANDLE_86 + DWORD_86 + 4*INT_86 + 3*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CREATEWINEX (ENV *envp,LONGPROC f)	/* CreateWindowEx */
{
	DWORD retcode;
	LPSTR lpString1,lpString2;
	LPBYTE lpStruct;
	WORD w1,w2,w3,w4;
	CLIENTCREATESTRUCT ccs;
	BOOL fMDIClient = FALSE;
	WNDCLASS ClassInfo;
	HINSTANCE hInst;

	lpString1 = (LPSTR)GetAddress
			(GETWORD(SP+32),GETWORD(SP+30));
	lpString2 = (LPSTR)GetAddress
			(GETWORD(SP+28),GETWORD(SP+26));

	hInst = (HINSTANCE)GetSelectorHandle(GETWORD(SP+8));

	if (!strcasecmp(lpString1,"MDICLIENT")) 
	    fMDIClient = TRUE;
	else {
	    if (GetClassInfo(hInst,lpString1,&ClassInfo))
		if (ClassInfo.lpfnWndProc == &DefMDICLIENTProc) 
		    fMDIClient = TRUE;
	}
	if (fMDIClient) {
	    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	    ccs.hWindowMenu = (HMENU)GETWORD(lpStruct);
	    ccs.idFirstChild = (UINT)((short)GETWORD(lpStruct+2));
	    lpStruct = (LPBYTE)&ccs;
	}
	else
	    lpStruct = (LPBYTE)GETDWORD(SP+4);
	w1 = GETWORD(SP+20);
	w2 = GETWORD(SP+18);
	w3 = GETWORD(SP+16);
	w4 = GETWORD(SP+14);
	retcode = (DWORD)CreateWindowEx((DWORD)GETDWORD(SP+34),
			lpString1,
			lpString2,
			GETDWORD(SP+22),
			(w1 == CW_USEDEFAULT16)?
				(int)CW_USEDEFAULT:(int)((short)w1),
			(w2 == CW_USEDEFAULT16)?
				(int)CW_USEDEFAULT:(int)((short)w2),
			(w3 == CW_USEDEFAULT16)?
				(int)CW_USEDEFAULT:(int)((short)w3),
			(w4 == CW_USEDEFAULT16)?
				(int)CW_USEDEFAULT:(int)((short)w4),
			(HANDLE)GETWORD(SP+12),
			(HANDLE)GETWORD(SP+10),
			(HANDLE)hInst,
			(LPVOID)lpStruct);
	envp->reg.sp += 3*HANDLE_86 + 2*DWORD_86 + 4*INT_86 + 3*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_DRAWTEXT (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;
	LPSTR lpString;

	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+8),GETWORD(SP+6));
	lpString = (LPSTR)GetAddress
			(GETWORD(SP+14),GETWORD(SP+12));
	RECT_TO_C(rcRect,lpStruct);
	retcode = (f)((HANDLE)GETWORD(SP+16),
			lpString,
			(int)((short)GETWORD(SP+10)),
			&rcRect,
			(UINT)GETWORD(SP+4));
	RECT_TO_86(lpStruct,rcRect);
	envp->reg.sp += 2*LP_86 + HANDLE_86 + INT_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_EXTTEXTOUT (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	LPSTR lpString;
	RECT rcRect;
	RECT *lpRect = (RECT *)NULL;
	int cChars;

	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+16),GETWORD(SP+14));
	lpString = (LPSTR)GetAddress
			(GETWORD(SP+12),GETWORD(SP+10));
	if (lpStruct) {
		RECT_TO_C(rcRect,lpStruct);
		lpRect = &rcRect;
	}
	lpStruct = (LPBYTE)GetAddress
		(GETWORD(SP+6),GETWORD(SP+4));
	cChars = (int)((short)GETWORD(SP+8));
	if (lpStruct)
		lpStruct = (LPBYTE)hsw_ConvertArrayToInt(lpStruct,cChars);
	retcode = (f)((HANDLE)GETWORD(SP+24),
			(int)((short)GETWORD(SP+22)),
			(int)((short)GETWORD(SP+20)),
			(UINT)GETWORD(SP+18),
			lpRect,
			lpString,
			(UINT)cChars,
			lpStruct);
	envp->reg.sp += HANDLE_86 + 3*LP_86 + 2*INT_86 + 2*UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETCHARWID (ENV *envp,LONGPROC f)	/* GetCharWidth */
{
	DWORD retcode;
	UINT uFirst,uLast;
	LPBYTE lpArray;
	LPBYTE lpStruct;
	int nLength;

	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+6),GETWORD(SP+4));
	uFirst = (UINT)GETWORD(SP+10);
	uLast  = (UINT)GETWORD(SP+8);
	nLength = uLast - uFirst + 1;
	lpArray = (LPBYTE)WinMalloc(nLength*sizeof(int));
	retcode = GetCharWidth((HANDLE)GETWORD(SP+12),
			uFirst, uLast, (int *)lpArray);
	memcpy((LPSTR)lpStruct,
		(LPSTR)hsw_ConvertArrayToWord(lpArray,nLength),
		nLength*2);
	WinFree((LPSTR)lpArray);
	envp->reg.sp += HANDLE_86 + LP_86 + 2*UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETCHARABC (ENV *envp,LONGPROC f)	/* GetCharABCWidths */
{
	DWORD retcode;
	UINT uFirst,uLast;
	LPABC lpArray;
	LPBYTE lpStruct;
	int i,nLength;

	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+6),GETWORD(SP+4));
	uFirst = (UINT)GETWORD(SP+10);
	uLast  = (UINT)GETWORD(SP+8);
	nLength = uLast - uFirst + 1;
	lpArray = (LPABC)WinMalloc(nLength*sizeof(ABC));
	retcode = GetCharABCWidths((HANDLE)GETWORD(SP+12),
			uFirst, uLast, (LPABC)lpArray);
	if (retcode) {
	    for (i = 0; i < nLength; i++) {
		PUTWORD(lpStruct,(WORD)lpArray[i].abcA);
		PUTWORD(lpStruct+2,(WORD)lpArray[i].abcB);
		PUTWORD(lpStruct+4,(WORD)lpArray[i].abcC);
		lpStruct += 6;
	    }
	}
	WinFree((LPSTR)lpArray);
	envp->reg.sp += HANDLE_86 + LP_86 + 2*UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETKERNPAIRS (ENV *envp,LONGPROC f)	/* GetKerningPairs */
{
	DWORD retcode;
	int cPairs,i;
	KERNINGPAIR *lpKrnPair;
	LPBYTE lpStruct;

	cPairs = (int)((short)GETWORD(SP+8));
	if (!(lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4))))
	    retcode = GetKerningPairs((HDC)GETWORD(SP+10),cPairs,0);
	else {
	    if (cPairs == 0)
		retcode = 0;
	    else {
		lpKrnPair = (KERNINGPAIR *)
			WinMalloc(sizeof(KERNINGPAIR)*cPairs);
		retcode = GetKerningPairs((HDC)GETWORD(SP+10),cPairs,lpKrnPair);
		if (retcode)
		    for (i = 0; i < retcode; i++) {
			PUTWORD(lpStruct,lpKrnPair[i].wFirst);
			PUTWORD(lpStruct+2,lpKrnPair[i].wSecond);
			PUTWORD(lpStruct+4,LOWORD(lpKrnPair[i].iKernAmount));
			lpStruct += 2*WORD_86 + INT_86;
		    }
		WinFree((LPSTR)lpKrnPair);
	    }
	}
	envp->reg.sp += HANDLE_86 + LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SCROLLDC (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect1,rcRect2,rcRect3;
	RECT *lpRect1 = NULL;
	RECT *lpRect2 = NULL;
	RECT *lpRect3 = NULL;

	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+16),GETWORD(SP+14));
	if (lpStruct) {
	    RECT_TO_C(rcRect1,lpStruct);
	    lpRect1 = &rcRect1;
	}
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+12),GETWORD(SP+10));
	if (lpStruct) {
	    RECT_TO_C(rcRect2,lpStruct);
	    lpRect2 = &rcRect2;
	}
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+6),GETWORD(SP+4));
	if (lpStruct) 
	    lpRect3 = &rcRect3;
	retcode = (f)((HANDLE)GETWORD(SP+22),
		(int)((short)GETWORD(SP+20)),
		(int)((short)GETWORD(SP+18)),
		lpRect1,
		lpRect2,
		(HANDLE)GETWORD(SP+8),
		lpRect3);
	if (lpStruct)
	    RECT_TO_86(lpStruct,rcRect3);
	envp->reg.sp += 2*HANDLE_86 + 3*LP_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SCROLLWINEX (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect1,rcRect2,rcRect3;
	RECT *lpRect1,*lpRect2,*lpRect3;

	lpRect1 = lpRect2 = lpRect3 = (RECT *)NULL;
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+18),GETWORD(SP+16));
	if (lpStruct) {
		RECT_TO_C(rcRect1,lpStruct);
		lpRect1 = &rcRect1;
	}
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+14),GETWORD(SP+12));
	if (lpStruct) {
		RECT_TO_C(rcRect2,lpStruct);
		lpRect2 = &rcRect2;
	}
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+8),GETWORD(SP+6));
	if (lpStruct) 
		lpRect3 = &rcRect3;
	retcode = (f)((HANDLE)GETWORD(SP+24),
			(int)((short)GETWORD(SP+22)),
			(int)((short)GETWORD(SP+20)),
			lpRect1,
			lpRect2,
			(HANDLE)GETWORD(SP+10),
			lpRect3,
			(UINT)GETWORD(SP+4));
	if (lpRect3)
		RECT_TO_86(lpStruct,rcRect3);
	envp->reg.sp += 2*HANDLE_86 + 3*LP_86 + 2*INT_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_STRETCHDIBITS (ENV *envp,LONGPROC f)	/* StretchDIBits */
{
	DWORD retcode;
	LPSTR lpString;
	LPBYTE lpStruct;
	BITMAPINFO *lpbmi;
	UINT fuColorUse;

	lpString = (LPSTR)GetAddress
			(GETWORD(SP+16),GETWORD(SP+14));
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+12),GETWORD(SP+10));
	fuColorUse = (UINT)GETWORD(SP+8);
	lpbmi = GetBitmapInfo(lpStruct,fuColorUse);
	retcode = StretchDIBits((HANDLE)GETWORD(SP+34),
			(int)((short)GETWORD(SP+32)),
			(int)((short)GETWORD(SP+30)),
			(int)((short)GETWORD(SP+28)),
			(int)((short)GETWORD(SP+26)),
			(int)((short)GETWORD(SP+24)),
			(int)((short)GETWORD(SP+22)),
			(int)((short)GETWORD(SP+20)),
			(int)((short)GETWORD(SP+18)),
			lpString,
			lpbmi,
			fuColorUse,
			GETDWORD(SP+4));

	WinFree((LPSTR)lpbmi);
	envp->reg.sp += HANDLE_86 + 2*LP_86 + 8*INT_86 +
			UINT_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_TEXTOUT (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)((HANDLE)GETWORD(SP+14),
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			lpString,
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + 3*INT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_TABTEXTOUT (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;
	LPBYTE lpStruct;
	int cTabStops;

	lpString = (LPSTR)GetAddress
			(GETWORD(SP+16),GETWORD(SP+14));
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+8),GETWORD(SP+6));
	cTabStops = (int)((short)GETWORD(SP+10));
	if (lpStruct)
		lpStruct = (LPBYTE)hsw_ConvertArrayToInt(lpStruct,cTabStops);
	retcode = (f)((HANDLE)GETWORD(SP+22),
			(int)((short)GETWORD(SP+20)),
			(int)((short)GETWORD(SP+18)),
			lpString,
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			lpStruct,
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + 5*INT_86 + 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETTABBEDTEXTEXT (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;
	LPBYTE lpStruct;
	int cTabStops;

	lpString = (LPSTR)GetAddress
			(GETWORD(SP+14),GETWORD(SP+12));
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+6),GETWORD(SP+4));
	cTabStops = (int)((short)GETWORD(SP+8));
	if (lpStruct)
		lpStruct = (LPBYTE)hsw_ConvertArrayToInt(lpStruct,cTabStops);
	retcode = (f)((HANDLE)GETWORD(SP+16),
			lpString,
			(int)((short)GETWORD(SP+10)),
			lpStruct);
	envp->reg.sp += HANDLE_86 + 2*INT_86 + 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SETDIBTD (ENV *envp,LONGPROC f)	/* SetDIBitsToDevice */
{
	DWORD retcode;
	LPSTR lpString;
	LPBYTE lpStruct;
	BITMAPINFO *lpbmi;
	UINT fuColorUse;

	lpString = (LPSTR)GetAddress
			(GETWORD(SP+12),GETWORD(SP+10));
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+8),GETWORD(SP+6));
	fuColorUse = (UINT)GETWORD(SP+4);
	lpbmi = GetBitmapInfo(lpStruct,fuColorUse);
	retcode = SetDIBitsToDevice((HANDLE)GETWORD(SP+30),
			(int)((short)GETWORD(SP+28)),
			(int)((short)GETWORD(SP+26)),
			(int)((short)GETWORD(SP+24)),
			(int)((short)GETWORD(SP+22)),
			(int)((short)GETWORD(SP+20)),
			(int)((short)GETWORD(SP+18)),
			(UINT)GETWORD(SP+16),
			(UINT)GETWORD(SP+14),
			lpString,
			lpbmi,
			fuColorUse);
	WinFree((LPSTR)lpbmi);
	envp->reg.sp += HANDLE_86 + 6*INT_86 + 3*UINT_86 + 2*LP_86 + RET_86;	
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SETDIBITS (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;
	LPBYTE lpStruct;
	BITMAPINFO *lpbmi;
	UINT fuColorUse;

	lpString = (LPSTR)GetAddress
			(GETWORD(SP+12),GETWORD(SP+10));
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+8),GETWORD(SP+6));
	fuColorUse = (UINT)GETWORD(SP+4);
	lpbmi = GetBitmapInfo(lpStruct,fuColorUse);
	retcode = (f)((HANDLE)GETWORD(SP+20),
			(HANDLE)GETWORD(SP+18),
			(UINT)GETWORD(SP+16),
			(UINT)GETWORD(SP+14),
			lpString,
			lpbmi,
			fuColorUse);
	WinFree((LPSTR)lpbmi);
	envp->reg.sp += 2*HANDLE_86 + 3*UINT_86 + 2*LP_86 + RET_86;		
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETDIBITS (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;
	LPBYTE lpStruct;
	BITMAPINFO *lpbmi;
	WORD wRGBCount;
	UINT fuColorUse;

	lpString = (LPSTR)GetAddress
			(GETWORD(SP+12),GETWORD(SP+10));
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+8),GETWORD(SP+6));
	fuColorUse = (UINT)GETWORD(SP+4);
	lpbmi = GetBitmapInfo(lpStruct,fuColorUse);
	retcode = (f)((HANDLE)GETWORD(SP+20),
			(HANDLE)GETWORD(SP+18),
			(UINT)GETWORD(SP+16),
			(UINT)GETWORD(SP+14),
			lpString,
			lpbmi,
			fuColorUse);

	if (lpbmi->bmiHeader.biClrUsed)
            wRGBCount = lpbmi->bmiHeader.biClrUsed;
	else 
	    wRGBCount = 1 << lpbmi->bmiHeader.biBitCount;
	memcpy((LPSTR)lpStruct+GETDWORD(lpStruct),(LPSTR)&lpbmi->bmiColors[0],
		sizeof(RGBQUAD)*wRGBCount);

	WinFree((LPSTR)lpbmi);
	envp->reg.sp += 2*HANDLE_86 + 3*UINT_86 + 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_REGCLASS (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	DWORD dwProc;
	WNDCLASS wc;

	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+6),GETWORD(SP+4));
	wc.style = (UINT)GETWORD(lpStruct);
	dwProc = GETDWORD(lpStruct+2);
	dwProc = (dwProc)?make_native_thunk(dwProc,
				(DWORD)hsw_common_nat_to_bin):0L;
	wc.lpfnWndProc = (WNDPROC)dwProc;
	wc.cbClsExtra = (int)GETWORD(lpStruct+6);
	wc.cbWndExtra = (int)GETWORD(lpStruct+8);
	wc.hInstance = (HANDLE)GetSelectorHandle(GETWORD(lpStruct+10));
	wc.hIcon = (HANDLE)GETWORD(lpStruct+12);
	wc.hCursor = (HANDLE)GETWORD(lpStruct+14);
	wc.hbrBackground = (HANDLE)GETWORD(lpStruct+16);
	wc.lpszMenuName = (LPSTR)GetAddress
			(GETWORD(lpStruct+20),GETWORD(lpStruct+18));
	wc.lpszClassName = (LPSTR)GetAddress
			(GETWORD(lpStruct+24),GETWORD(lpStruct+22));
	retcode = (f)((LPWNDCLASS)&wc);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
_86_DOS3Call (ENV *envp,LONGPROC f)
{
	envp->reg.sp -= WORD_86;
	LOGSTR((LF_APICALL,"DOS3CALL(ax=%x,..)\n",envp->reg.ax));	
	INTERRUPT(0x21, (ENV *)&envp->reg);
	LOGSTR((LF_APIRET,"DOS3CALL(ax=%x,..)\n",envp->reg.ax));	
}

void
_86_NetBIOSCall (ENV *envp,LONGPROC f)
{
	envp->reg.sp -= WORD_86;
	APISTR((LF_APICALL,"NetBIOSCALL(ax=%x,..)\n",envp->reg.ax));	
	INTERRUPT(0x5c, (ENV *)&envp->reg);
	APISTR((LF_APIRET,"NetBIOSCALL(ax=%x,..)\n",envp->reg.ax));	
}

LPBYTE TWIN_HashProcInst(WORD instance_segment, 
			 WORD proc_segment, WORD proc_offset)
{
    LPBYTE lpb;
    unsigned int hash_idx;
    unsigned int starting_idx;
    
    /*
     * Do we need to allocate the instance selector?
     */
    if (!wProcInstanceSelector)
    {
	HGLOBAL hMem;
	
	hMem = GlobalAlloc(GMEM_ZEROINIT, 0x10000);
	if (!hMem)
	    return NULL;
	
	lpbProcInstanceMemory = (BYTE*) GlobalLock(hMem);
	wProcInstanceSelector = AssignSelector(lpbProcInstanceMemory,
					       0, TRANSFER_CODE16, 
					       0x10000);
	SetSelectorHandle(wProcInstanceSelector, hMem);

	lpbProcInstanceLast = lpbProcInstanceMemory;
    }
	
    hash_idx = (((proc_offset << 8) & 0xff00) + ((proc_offset >> 8) & 0x00ff) +
		((proc_segment << 5) & 0xff00) + 
		((proc_segment >> 13) & 0x00ff) +
		((instance_segment << 5) & 0xff00) + 
		((instance_segment >> 13) & 0x00ff));
    hash_idx = (hash_idx << 3) & 0xfff8;
    
    lpb = lpbProcInstanceMemory + hash_idx;
    starting_idx = hash_idx;

    while ((lpb[1] != (instance_segment & 0xff)) ||
	   (lpb[2] != ((instance_segment >> 8) & 0xff)) ||
	   (lpb[4] != (proc_offset & 0xff)) ||
	   (lpb[5] != ((proc_offset >> 8) & 0xff)) ||
	   (lpb[6] != (proc_segment & 0xff)) ||
	   (lpb[7] != ((proc_segment >> 8) & 0xff)))
    {
	if (lpb[0] != baProcInstanceTemplate[0] ||
	    lpb[3] != baProcInstanceTemplate[3])
	    break;

	hash_idx += 8;
	if (hash_idx > 0x10000)
	    hash_idx = 0;

	if (hash_idx == starting_idx)
	    return NULL;

	lpb = lpbProcInstanceMemory + hash_idx;
    }

    lpb[1] = instance_segment & 0xff;
    lpb[2] = (instance_segment >> 8) & 0xff;
    lpb[4] = proc_offset & 0xff;
    lpb[5] = (proc_offset >> 8) & 0xff;
    lpb[6] = proc_segment & 0xff;
    lpb[7] = (proc_segment >> 8) & 0xff;
    lpb[0] = baProcInstanceTemplate[0];
    lpb[3] = baProcInstanceTemplate[3];

    return lpb;
}

void
IT_MAKEPRIN (ENV *envp,LONGPROC f)
{
	DWORD func_addr;
	HANDLE hInst;
	WORD data_sel;
	BYTE *lpb;

	/*
	 * Get the arguments off of the stack.
	 */
	func_addr = GETDWORD(SP+6);
	hInst = GetSelectorHandle(GETWORD(SP+4));
	
	/*
	 * Determine the data selector for this task
	 */
	data_sel = GetDataSelectorFromInstance(hInst);
	
	lpb = TWIN_HashProcInst(data_sel, 
				HIWORD(func_addr), LOWORD(func_addr));
	if (!lpb)
	    goto fail;

	envp->reg.sp += LP_86 + HANDLE_86 + RET_86;
	envp->reg.ax = (lpb - lpbProcInstanceMemory);
	envp->reg.dx = wProcInstanceSelector;

	return;
	
    fail:
	envp->reg.sp += LP_86 + HANDLE_86 + RET_86;
	envp->reg.ax = 0;
	envp->reg.dx = 0;

	return;
}

void
IT_FREEPRIN (ENV *envp,LONGPROC f)
{
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = 0;
	envp->reg.dx = 0;
}

void
_86_GetInstanceData (ENV *envp,LONGPROC f)
{
    HINSTANCE hInst;
    WORD wOffset;
    int nCnt;
    LPSTR src, dst;

    hInst = GetSelectorHandle(GETWORD(SP+8));
    wOffset = GETWORD(SP+6);
    nCnt = (int)(short)GETWORD(SP+4);

    dst = GetAddress(envp->reg.ds, wOffset);
    src = (LPSTR) GlobalLock(hInst) + wOffset;

    memcpy(dst, src, nCnt);

    envp->reg.sp += HANDLE_86 + WORD_86 + INT_86 + RET_86;
    envp->reg.ax = nCnt;
    envp->reg.dx = 0;
}

void
IT_GWLONG (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	HANDLE hwnd;
	int nOffset;

	hwnd = GETWORD(SP+6);
	nOffset = (int)((short)GETWORD(SP+4));
	retcode = (f)(hwnd,nOffset);
	if (retcode && ((TestWF(hwnd,WFDIALOGWINDOW) &&
			(nOffset == DWL_DLGPROC)) ||
	    		(nOffset == GWL_WNDPROC))) {
	    retcode = make_binary_thunk((LPDWORD)retcode,(DWORD)IT_SENDMESSAGE);
	}
	envp->reg.sp += HANDLE_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SWLONG (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	HANDLE hwnd;
	int nOffset;
	FARPROC lpfnNativeThunk,lpfnNatToBin;
	DWORD dwProc;

	hwnd = (HANDLE)GETWORD(SP+10);
	nOffset = (int)((short)GETWORD(SP+8));
	if ((TestWF(hwnd,WFDIALOGWINDOW) && nOffset == DWL_DLGPROC) ||
	    (nOffset == GWL_WNDPROC)) {
	    if (!(lpfnNatToBin = (FARPROC)GetClassLong(hwnd,GCL_NATTOBIN)))
	        retcode = 0;
	    else {
	        dwProc = GETDWORD(SP+4);
	        lpfnNativeThunk = (dwProc)?(FARPROC)make_native_thunk
		    (dwProc,(DWORD)lpfnNatToBin):0L;
	        retcode = SetWindowLong(hwnd,
			nOffset,
			(DWORD)lpfnNativeThunk);
	        retcode = make_binary_thunk((LPDWORD)retcode,
					(DWORD)IT_SENDMESSAGE);
	    }
	}
	else
		retcode = (f)(hwnd,
			nOffset,
			GETDWORD(SP+4));
	envp->reg.sp += HANDLE_86 + INT_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GCWORD (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	int nOffset;

	nOffset = (int)((short)GETWORD(SP+4));
#ifdef	TWIN32
	if (nOffset == GCL_HBRBACKGROUND ||
	    nOffset == GCL_HCURSOR ||
	    nOffset == GCL_HICON ||
	    nOffset == GCL_HMODULE ||
	    nOffset == GCL_CBWNDEXTRA ||
	    nOffset == GCL_CBCLSEXTRA ||
	    nOffset == GCL_STYLE)
	    retcode = GetClassLong((HWND)GETWORD(SP+6),nOffset);
	else
#endif
	    retcode = GetClassWord((HWND)GETWORD(SP+6),nOffset);
	if (nOffset == 
#ifndef	TWIN32
		GCW_STYLE
#else
		GCL_STYLE
#endif
			)
	    retcode &= ~CS_SYSTEMGLOBAL;
	else if (nOffset ==
#ifndef	TWIN32
			GCW_HMODULE
#else
			GCL_HMODULE
#endif
					)
	    retcode = GetDataSelectorFromInstance(retcode);
	envp->reg.sp += HANDLE_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void IT_SCWORD (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	int nOffset;
	WORD wVal;

	nOffset = (int)((short)GETWORD(SP+6));
	wVal = (WORD)GETWORD(SP+4);
	if (nOffset ==
#ifndef	TWIN32
			GCW_HMODULE
#else
			GCL_HMODULE
#endif
					)
		wVal = GetModuleFromInstance(GetSelectorHandle(wVal));
#ifdef	TWIN32
	if (nOffset == GCL_HBRBACKGROUND ||
	    nOffset == GCL_HCURSOR ||
	    nOffset == GCL_HICON ||
	    nOffset == GCL_HMODULE ||
	    nOffset == GCL_CBWNDEXTRA ||
	    nOffset == GCL_CBCLSEXTRA ||
	    nOffset == GCL_STYLE)
	    retcode = SetClassLong((HWND)GETWORD(SP+8), nOffset, wVal);
	else
#endif
	    retcode = SetClassWord((HWND)GETWORD(SP+8), nOffset, wVal);
	envp->reg.sp += HANDLE_86 + INT_86 + WORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GCLONG (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	int nOffset;
	HWND hWnd;

	hWnd = GETWORD(SP+6);
	nOffset = (int)((short)GETWORD(SP+4));
	if ((retcode = GetClassLong(hWnd,nOffset)) &&
	        (nOffset == GCL_WNDPROC)) {
	    retcode = make_binary_thunk((LPDWORD)retcode,
					(DWORD)IT_SENDMESSAGE);
	}
	envp->reg.sp += HANDLE_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SCLONG (ENV *envp,LONGPROC f)
{
	DWORD retcode,dwProc;
	int nOffset;
	HWND hwnd;
	FARPROC lpfnNativeThunk,lpfnNatToBin;

	hwnd = (HWND)GETWORD(SP+10);
	nOffset = (int)((short)GETWORD(SP+8));

	if (nOffset == GCL_WNDPROC) {
	    if (!(lpfnNatToBin = (FARPROC)GetClassLong(hwnd,GCL_NATTOBIN)))
	        retcode = 0;
	    else {
	        dwProc = GETDWORD(SP+4);
	        lpfnNativeThunk = (dwProc)?(FARPROC)make_native_thunk
			(dwProc,(DWORD)lpfnNatToBin):0L;
	        retcode = SetClassLong(hwnd,
			nOffset,
			(DWORD)lpfnNativeThunk);
	        retcode = make_binary_thunk((LPDWORD)retcode,
					(DWORD)IT_SENDMESSAGE);
	    }
	}
	else
		retcode = SetClassLong(hwnd,
			nOffset,
			GETDWORD(SP+4));
	envp->reg.sp += HANDLE_86 + INT_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPPA1I (ENV *envp,LONGPROC f)		/* DPtoLP, LPtoDP */
{
	DWORD retcode;
	int cPoints;
	LPBYTE lpStruct,lpArray;

	cPoints = GETWORD(SP+4);
	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+8),GETWORD(SP+6));
	lpArray = (LPBYTE)hsw_ConvertArrayToInt(lpStruct,cPoints*2);
	retcode = (f)((HANDLE)GETWORD(SP+10),
			lpArray,
			cPoints);
	lpArray = (LPBYTE)hsw_ConvertArrayToWord(lpArray,cPoints*2);
	memcpy((LPSTR)lpStruct,(LPSTR)lpArray,cPoints*4);
	envp->reg.sp += HANDLE_86 + LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_POLY (ENV *envp,LONGPROC f)	/* Polygon/Polyline */
{
	DWORD retcode;
	LPBYTE lpStruct;
	int nCount;

	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+8),GETWORD(SP+6));
	nCount = (int)((short)GETWORD(SP+4));
	retcode = (DWORD)(f)((HDC)GETWORD(SP+10),
		(LPPOINT)hsw_ConvertArrayToInt(lpStruct,nCount*2),
		nCount);
	envp->reg.sp += HANDLE_86 + LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_POLYPOLY (ENV *envp,LONGPROC f)	/* PolyPolygon */
{
	DWORD retcode;
	LPBYTE lpStruct;
	LPINT lpPoly,lpCounts;
	LPPOINT lppt;
	int nCount,n,nVertices;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	nCount = (int)((short)GETWORD(SP+4));
	lpPoly = hsw_ConvertArrayToInt((LPBYTE)lpStruct,nCount);
	lpCounts = (LPINT)WinMalloc(nCount*sizeof(int));
	memcpy((LPSTR)lpCounts,(LPSTR)lpPoly,nCount*sizeof(int));
	for (n=0,nVertices = 0;n<nCount;n++)
	    nVertices += lpCounts[n];

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	lppt = (LPPOINT)hsw_ConvertArrayToInt(lpStruct,nVertices*2);

	retcode = (DWORD)PolyPolygon((HDC)GETWORD(SP+14),
				(const POINT *)lppt,
				lpCounts,nCount);

	WinFree((LPSTR)lpCounts);

	envp->reg.sp += HANDLE_86 + 2*LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CREATEPOLYRGN (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	int nCount;
	int *lpInt;

	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+10),GETWORD(SP+8));
	nCount = (int)((short)GETWORD(SP+6));
	lpInt = hsw_ConvertArrayToInt(lpStruct,2*nCount);
	retcode = CreatePolygonRgn((const POINT *)lpInt,
			nCount,
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += LP_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CREATEPOLYPOLYRGN (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct,lpCounts;
	int nCount,nPointCount,i;
	LPINT lpPolyCount,lpPoints,lpInts;

	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+14),GETWORD(SP+12));
	lpCounts = (LPBYTE)GetAddress
			(GETWORD(SP+10),GETWORD(SP+8));
	nCount = (int)((short)GETWORD(SP+6));
	lpInts = hsw_ConvertArrayToInt(lpCounts,nCount);
	lpPolyCount = (LPINT)WinMalloc(nCount*sizeof(int));
	memcpy((LPSTR)lpPolyCount,(LPSTR)lpInts,nCount*sizeof(int));
	for (i = 0, nPointCount = 0; i < nCount; i++)
	    nPointCount += lpPolyCount[i];
	lpPoints = hsw_ConvertArrayToInt(lpStruct,2*nPointCount);
	retcode = CreatePolyPolygonRgn((const POINT *)lpPoints,
			(const int *)lpPolyCount,
			nCount,
			(int)((short)GETWORD(SP+4)));
	WinFree((LPSTR)lpPolyCount);
	envp->reg.sp += 2*LP_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_WSPRINTF (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;
	LPSTR lpData;

	lpString1 = (LPSTR)GetAddress
				(GETWORD(SP+6),GETWORD(SP+4));
	lpString2 = (LPSTR)GetAddress
				(GETWORD(SP+10),GETWORD(SP+8));
	lpData = (LPSTR)ReadFormatString(lpString2,(BYTE *)(SP+12));
#if defined(NETWARE)
	vsprintf(lpString1,lpString2, (va_list) lpData);
#else
	vsprintf(lpString1,lpString2, lpData);
#endif
	retcode = strlen(lpString1);
	envp->reg.sp += RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_WVSPRINTF (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;
	LPBYTE lpStruct;
	LPSTR lpData;

	lpString1 = (LPSTR)GetAddress
				(GETWORD(SP+14),GETWORD(SP+12));
	lpString2 = (LPSTR)GetAddress
				(GETWORD(SP+10),GETWORD(SP+8));
	lpStruct = (LPBYTE)GetAddress
				(GETWORD(SP+6),GETWORD(SP+4));
	lpData = (LPSTR)ReadFormatString(lpString2,lpStruct);
#if defined(NETWARE)
	vsprintf(lpString1,lpString2, (va_list) lpData);
#else
	vsprintf(lpString1,lpString2, lpData);
#endif
	retcode = strlen(lpString1);
	envp->reg.sp += 3*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_OPENFILE (ENV *envp,LONGPROC f)
{
	DWORD	 retcode;
	LPSTR	 lpString;
	LPBYTE	 lpStruct;
	OFSTRUCT of;
	ERRORCODE *err;

#define	OFS_MAXPATHNAME	128

	lpString = (LPSTR)GetAddress
				(GETWORD(SP+12),GETWORD(SP+10));
	lpStruct = (LPBYTE)GetAddress
				(GETWORD(SP+8),GETWORD(SP+6));
	of.cBytes = *lpStruct;
	of.fFixedDisk = *(lpStruct+1);
	of.nErrCode = GETWORD(lpStruct+2);
	memcpy((LPSTR)of.szPathName,(LPSTR)lpStruct+8,OFS_MAXPATHNAME);

	retcode = OpenFile(lpString, &of, (UINT)GETWORD(SP+4));

	*lpStruct = of.cBytes;
	*(lpStruct+1) = of.fFixedDisk;
	PUTWORD(lpStruct+2,of.nErrCode);
	if (retcode != HFILE_ERROR)
	    MFS_DOSPATH((LPSTR)(lpStruct+8), of.szPathName);
	else {
	    err = error_code(0); 
	    PUTWORD(lpStruct+2,(WORD)err->ec_extended);
	}
	envp->reg.sp += 2*LP_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_LOPEN (ENV *envp, LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = _lopen(lpString,(int)GETWORD(SP+4));
	envp->reg.sp += LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}
	

#define	BIN_BRUSH_SIZE		8
#define	BIN_PEN_SIZE		10
#define	BIN_BITMAP_SIZE		14
#define	BIN_FONT_SIZE		50

void
IT_GETOBJECT (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	HGDIOBJ handle;
	LPOBJHEAD lpObjInfo;
	LPBYTE lpStruct;
	WORD objType;
	LOGBRUSH lb;
	LOGPEN lp;
	LOGFONT lf;
	BITMAP bm;
	UINT uiPalCount;
	int cbBuffer, cbCount;

	lpStruct = (LPBYTE)GetAddress
				(GETWORD(SP+6),GETWORD(SP+4));
	handle = (HGDIOBJ)GETWORD(SP+10);
	cbBuffer = (int)((short)GETWORD(SP+8));
	retcode = (DWORD)0;
	if ((lpObjInfo = GETGDIINFO(handle))) {
		objType= GET_OBJECT_TYPE(lpObjInfo);
		switch(objType) {
			case OT_BRUSH:
				retcode = GetObject
					((HGDIOBJ)GETWORD(SP+10),
					sizeof(LOGBRUSH), &lb);
				if ( !retcode )
					break;
				retcode = BIN_BRUSH_SIZE;
				cbCount = 0;
				if (cbCount >= cbBuffer)
					break;
				PUTWORD(lpStruct,(WORD)lb.lbStyle);
				cbCount += 2;
				if (cbCount >= cbBuffer)
					break;
				PUTDWORD(lpStruct+2,(DWORD)lb.lbColor);
				cbCount += 4;
				if (cbCount >= cbBuffer)
					break;
				PUTWORD(lpStruct+6,(WORD)lb.lbHatch);
				break;
			case OT_PEN:
				retcode = GetObject
					((HGDIOBJ)GETWORD(SP+10),
					sizeof(LOGPEN), &lp);
				if ( !retcode )
					break;
				retcode = BIN_PEN_SIZE;
				cbCount = 0;
				if (cbCount >= cbBuffer)
					break;
				PUTWORD(lpStruct,(WORD)lp.lopnStyle);
				cbCount += 2;
				if (cbCount >= cbBuffer)
					break;
				PutPOINT(lpStruct+2,lp.lopnWidth);
				cbCount += 4;
				if (cbCount >= cbBuffer)
					break;
				PUTDWORD(lpStruct+6,lp.lopnColor);
				break;
			case OT_FONT:
				retcode = GetObject
					((HGDIOBJ)GETWORD(SP+10),
					sizeof(LOGFONT), &lf);
				if ( !retcode )
					break;
				retcode = BIN_FONT_SIZE;
				PutLOGFONT(lpStruct,&lf,cbBuffer);
				break;
			case OT_BITMAP:
				retcode = GetObject
					((HGDIOBJ)GETWORD(SP+10),
					sizeof(BITMAP), &bm);
				if ( !retcode )
					break;
				retcode = BIN_BITMAP_SIZE;
				cbCount = 0;
				if (cbCount >= cbBuffer)
					break;
				PUTWORD(lpStruct,(WORD)bm.bmType);
				cbCount += 2;
				if (cbCount >= cbBuffer)
					break;
				PUTWORD(lpStruct+2,(WORD)bm.bmWidth);
				cbCount += 2;
				if (cbCount >= cbBuffer)
					break;
				PUTWORD(lpStruct+4,(WORD)bm.bmHeight);
				cbCount += 2;
				if (cbCount >= cbBuffer)
					break;
				PUTWORD(lpStruct+6,(WORD)bm.bmWidthBytes);
				cbCount += 2;
				if (cbCount >= cbBuffer)
					break;
				*(lpStruct+8) = bm.bmPlanes;
				cbCount++;
				if (cbCount >= cbBuffer)
					break;
				*(lpStruct+9) = bm.bmBitsPixel;
				cbCount++;
				if (cbCount >= cbBuffer)
					break;
				PUTDWORD(lpStruct+10,(DWORD)bm.bmBits);
				break;
			case OT_PALETTE:
				retcode = GetObject
					((HGDIOBJ)GETWORD(SP+10),
					sizeof(UINT), (LPVOID)&uiPalCount);
				PUTWORD(lpStruct,uiPalCount);
				break;
			default:
				break;
		}
		RELEASEGDIINFO(lpObjInfo);
	}
	envp->reg.sp += HANDLE_86 + INT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_PLAYMETAREC (ENV *envp,LONGPROC f)
{

	(f)((HDC)GETWORD(SP+14),
		(HGLOBAL *)GetAddress(GETWORD(SP+12),GETWORD(SP+10)),
		(METARECORD *)GetAddress(GETWORD(SP+8),GETWORD(SP+6)),
		(UINT)GETWORD(SP+4));

	envp->reg.sp += HANDLE_86 + UINT_86 + 2*LP_86 + RET_86;
}

void
_86_Win87Em (ENV *envp,LONGPROC f)
{
	switch(LOWORD(envp->reg.bx)) {
	    case 0:	/* Initialize the FP emulator */
	    case 1:	/* Reset the FP emulator */
	    case 2:	/* Stop the FP emulator */
	    case 3:	/* Set coprocessor error exception handler */
		break;
	    case 10:	/* Retrieve FP stack count */
		envp->reg.ax = 0;
		break;
	    case 11:	/* Query coprocessor presence */
		envp->reg.ax = 1;
		break;
	}
	envp->reg.flags &= ~CARRY_FLAG;
	envp->reg.sp += RET_86;
}

void
IT_INQSYSTEM (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)(GETWORD(SP+6),
			GETWORD(SP+4),
			0);
	envp->reg.sp += 2*WORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SETSYSCOLORS (ENV *envp,LONGPROC f)
{
	int nElements,i;
	LPBYTE lpStruct1,lpStruct2;
	LPINT lpInts;
	COLORREF *lpColors;

	nElements = (int)((short)GETWORD(SP+12));
	lpStruct1 = (LPBYTE)GetAddress
			(GETWORD(SP+10),GETWORD(SP+8));
	lpStruct2 = (LPBYTE)GetAddress
			(GETWORD(SP+6),GETWORD(SP+4));
	lpColors = (COLORREF *)WinMalloc(nElements*sizeof(COLORREF));
	lpInts = hsw_ConvertArrayToInt(lpStruct1,nElements);
	for(i=0;i<nElements;i++)
		*(lpColors+i) = GETDWORD(lpStruct2 + i*DWORD_86);
	SetSysColors(nElements,lpInts,lpColors);
	WinFree((LPSTR)lpColors);
	envp->reg.sp += INT_86 + 2*LP_86 + RET_86;
}

void
IT_GETRCAPS (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	RASTERIZER_STATUS rs;
	LPBYTE lpStruct;

	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (DWORD)(f)(&rs,(int)((short)GETWORD(SP+4)));
	PUTWORD(lpStruct,3*INT_86);
	PUTWORD(lpStruct+2,(WORD)rs.wFlags);
	PUTWORD(lpStruct+4,(WORD)rs.nLanguageID);
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
	envp->reg.sp += LP_86 + INT_86 + RET_86;
}

static BITMAPINFO *
GetBitmapInfo(LPBYTE lpData, UINT fuColorUse)
{
    WORD wRGBCount;
    RGBQUAD *rgb_quad;
    /* DIB_PAL_COLORS - the palette index is DWORD/WORD depending on whether
     * you are using Win32 or not.  The original code accesses the palette
     * index via UINT - which is the correct way to do it (according to
     * Microsoft) since UINT is supposed to be 32/16 bit depending on the
     * target platform.
     * PROBLEM - even when building a 16-bit library, we always define UINT
     * to be unsigned int (so UINT will never be 16-bits in size).
     * POSSIBLE SOLUTION - in windows.h, use #ifdef TWIN32 to typedef UINT to
     * be unsigned int or unsigned short.
     * PROBLEM WITH POSSIBLE SOLUTION - why risk breaking the library?
     * What if other code depends on UINT being > 16 bits (even for a 16-bit
     * library)?
     * THE SOLUTION - for this function only, use DWORD for TWIN32 else use
     * WORD.
     */
#if 0	/* original code - won't work until UINT definition is fixed */
    UINT *lpUint;
#elif defined(TWIN32)
    DWORD *lpDword;
#else
    WORD *lpWord;
#endif
    BITMAPINFOHEADER bmiHeader;
    BITMAPINFO *lpbmi;
    int i;

    if (lpData == NULL)
	return (BITMAPINFO *)NULL;
    GetBIHeader(&bmiHeader,lpData);
    lpData += bmiHeader.biSize;
    if (bmiHeader.biClrUsed)
	wRGBCount = bmiHeader.biClrUsed;
    else {
	wRGBCount = bmiHeader.biBitCount;
	wRGBCount = (wRGBCount==1)?2:(wRGBCount==4)?16:(wRGBCount==8)?256:0;
    }
    lpbmi = (BITMAPINFO *)WinMalloc
	    (sizeof(BITMAPINFOHEADER) + wRGBCount *
#if 0	/* original code - won't work until UINT definition is fixed */
	    ((fuColorUse == DIB_RGB_COLORS) ? sizeof(RGBQUAD) : sizeof(UINT)));
#elif defined(TWIN32)
	    ((fuColorUse == DIB_RGB_COLORS) ? sizeof(RGBQUAD) : sizeof(DWORD)));
#else
	    ((fuColorUse == DIB_RGB_COLORS) ? sizeof(RGBQUAD) : sizeof(WORD)));
#endif
    lpbmi->bmiHeader = bmiHeader;
    if (wRGBCount) {
	if (fuColorUse == DIB_RGB_COLORS) {
	    rgb_quad = &lpbmi->bmiColors[0];
	    for (i=0; i < (int)wRGBCount; i++) {
		rgb_quad[i].rgbBlue = *lpData++;
		rgb_quad[i].rgbGreen = *lpData++;
		rgb_quad[i].rgbRed = *lpData++;
		if (bmiHeader.biSize == 40)
		    rgb_quad[i].rgbReserved = *lpData++;
		else
		    rgb_quad[i].rgbReserved = 0;
	    }
	}
	else {	/* DIB_PAL_COLORS */
#if 0	/* original code - won't work until UINT definition is fixed */
	    lpUint = (UINT *)&lpbmi->bmiColors[0];
#elif defined(TWIN32)
	    lpDword = (DWORD *)&lpbmi->bmiColors[0];
#else
	    lpWord = (WORD *)&lpbmi->bmiColors[0];
#endif
	    for (i=0; i < (int)wRGBCount; i++) {
#if 0	/* original code - won't work until UINT definition is fixed */
		lpUint[i] = (UINT)GETWORD(lpData);
#elif defined(TWIN32)
		lpDword[i] = (DWORD)GETWORD(lpData);
#else
		lpWord[i] = (WORD)GETWORD(lpData);
#endif
		lpData += 2;
	    }
	}
    }
    return lpbmi;
}

static HGLOBAL
GetPackedDIB(HGLOBAL hGlobalBin)
{
    LPBYTE lpStruct,lpNative;
    BITMAPINFOHEADER bmiHeader;
    RGBQUAD *rgb_quad;
    WORD wRGBCount;
    HGLOBAL hGlobal;
    int i;

    if (!(lpStruct = (LPBYTE)GlobalLock(hGlobalBin)))
	return 0;
    GetBIHeader(&bmiHeader,lpStruct);
    lpStruct += bmiHeader.biSize;  
    if (bmiHeader.biClrUsed)
	wRGBCount = bmiHeader.biClrUsed;
    else {
	wRGBCount = bmiHeader.biBitCount;
	wRGBCount = (wRGBCount==1)?2:(wRGBCount==4)?16:(wRGBCount==8)?256:0;
    }
    hGlobal = GlobalAlloc(GHND,
			sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD)*wRGBCount +
			bmiHeader.biSizeImage);
    lpNative = (LPBYTE)GlobalLock(hGlobal);
    *(BITMAPINFOHEADER *)lpNative = bmiHeader;
    lpNative += sizeof(BITMAPINFOHEADER);
    if (wRGBCount) {
	rgb_quad = (RGBQUAD *)WinMalloc(sizeof(RGBQUAD) * wRGBCount);
	for (i=0; i < (int)wRGBCount; i++) {
	    rgb_quad[i].rgbBlue = *lpStruct++;
	    rgb_quad[i].rgbGreen = *lpStruct++;
	    rgb_quad[i].rgbRed = *lpStruct++;
	    if (bmiHeader.biSize == 40)
		rgb_quad[i].rgbReserved = *lpStruct++;
	    else
		rgb_quad[i].rgbReserved = 0;
	}
	memcpy((LPSTR)lpNative,(LPSTR)rgb_quad,sizeof(RGBQUAD)*wRGBCount);
	WinFree((LPSTR)rgb_quad);
    }
    lpNative += sizeof(RGBQUAD)*wRGBCount;
    memcpy((LPSTR)lpNative,(LPSTR)lpStruct,bmiHeader.biSizeImage);
    GlobalUnlock(hGlobalBin);
    GlobalUnlock(hGlobal);
    return hGlobal;
}

void
_86_GetFreeSystemResources (ENV *envp,LONGPROC f)
{
	envp->reg.ax = 0x50;	/* % for free space for system resource */
	envp->reg.dx = 0;
	envp->reg.sp += UINT_86 + RET_86;
}

void
_86_GetHeapSpaces (ENV *envp,LONGPROC f)
{
	envp->reg.dx = 0xfff0;
	envp->reg.ax = 0xff00;
	envp->reg.sp += HANDLE_86 + RET_86;
}

void
_86_SystemHeapInfo (ENV *envp,LONGPROC f)
{
	LPBYTE lpStruct;
	HINSTANCE hInst;
	DWORD dwSize;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	dwSize = DWORD_86 + 2*HANDLE_86 + 2*WORD_86;
	PUTDWORD(lpStruct,dwSize);
	PUTWORD(lpStruct+4,0x50);	/* % free space in USER heap */
	PUTWORD(lpStruct+6,0x50);	/* % free space in GDI heap */
	hInst = GetInstanceFromModule(GetModuleHandle("USER"));
	PUTWORD(lpStruct+8,
		(WORD)GetDataSelectorFromInstance(hInst)); /* USER's DGROUP */
	hInst = GetInstanceFromModule(GetModuleHandle("GDI"));
	PUTWORD(lpStruct+10,
		(WORD)GetDataSelectorFromInstance(hInst)); /* GDI's DGROUP */
	envp->reg.ax = 1;
	envp->reg.sp += LP_86 + RET_86;
}

void
_86_GetAppCompatFlags (ENV *envp,LONGPROC f)
{
	envp->reg.dx = 0;
	envp->reg.ax = 0;
	envp->reg.sp += HANDLE_86 + RET_86;
}

void
IT_LOADMENUINDIRECT (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	LPBYTE lpMenuTemplate;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	lpMenuTemplate = ExtractMenuTemplate(&lpStruct);
	retcode = (DWORD)(f)(lpMenuTemplate);
	CleanupMenuTemplate(lpMenuTemplate);
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
	envp->reg.sp += LP_86 + RET_86;
}

void
IT_DEFFRAMEPROC (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	WMCONVERT wmc;

#ifdef	SEVEREDEBUG
	memset((LPSTR)&wmc,'\0',sizeof(WMCONVERT));
#endif

	wmc.uMsg = (UINT)GETWORD(SP+10);
	wmc.hook_wParam = GETWORD(SP+12);
	if (wmc.uMsg == WM_CONVERT)
	    FatalAppExit(0,"IT_DEFFRAMEPROC should not get WM_CONVERT!");
	wmc.wParam = (WORD)GETWORD(SP+8);
	wmc.lParam = GETDWORD(SP+4);
	wmc.targ = f;
	retcode = (f)((HWND)GETWORD(SP+14),
			(HWND)GETWORD(SP+12),
			WM_CONVERT,
			HSWMC_BINMSG | HSWMC_DEST_DEF_FRAME,(DWORD)&wmc);
	envp->reg.sp += 2*HANDLE_86 + UINT_86 + WORD_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}


void
IT_GETDOSENV (ENV *envp,LONGPROC f)
{
    LPBYTE lpStruct;
    UINT uSel;

    uSel = GetPSPSelector();
    lpStruct = GetPhysicalAddress(uSel);
    envp->reg.ax = 0;
    envp->reg.dx = GETWORD(lpStruct+0x2c);
    envp->reg.sp += RET_86;
}

void
IT_1D1LP1LPD (ENV *envp,LONGPROC f)	/* RegCreate/OpenKey */
{
	DWORD retcode;
	HKEY hkRes;
	LPSTR lpString;
	LPVOID lpStruct;

	lpString = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	lpStruct = (LPVOID)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)(GETDWORD(SP+12),
			lpString,
			&hkRes);
	PUTDWORD(lpStruct,hkRes);
	envp->reg.sp += DWORD_86 + 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1D2LP1LPD (ENV *envp,LONGPROC f)	/* RegQueryValue */
{
	DWORD retcode;
	LONG cb;
	LPSTR lpString1, lpString2;
	LPVOID lpStruct;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	lpStruct = (LPVOID)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	cb = GETDWORD(lpStruct);
	retcode = (f)(GETDWORD(SP+16),
			lpString1,
			lpString2,
			&cb);
	PUTDWORD(lpStruct,cb);
	envp->reg.sp += DWORD_86 + 3*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1D1LP1D1LP1D (ENV *envp,LONGPROC f)	/* RegSetValue */
{
	DWORD retcode;
	LPSTR lpString1, lpString2;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+18),GETWORD(SP+16));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	retcode = (f)(GETDWORD(SP+20),
			lpString1,
			GETDWORD(SP+12),
			lpString2,
			GETDWORD(SP+4));
	envp->reg.sp += 3*DWORD_86 + 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2D1LP1D (ENV *envp,LONGPROC f)     /* RegEnumKey */
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	retcode = (f)(GETDWORD(SP+16),
			GETDWORD(SP+12),
			lpString,
			GETDWORD(SP+4));
	envp->reg.sp += 3*DWORD_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SETPALENTRIES (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPVOID lpStruct;

	lpStruct = (LPVOID)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)((HPALETTE)GETWORD(SP+12),
			(UINT)GETWORD(SP+10),
			(UINT)GETWORD(SP+8),
			(const PALETTEENTRY *)lpStruct);
	envp->reg.sp += HANDLE_86 + 2*UINT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETPALENTRIES (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPVOID lpStruct;

	lpStruct = (LPVOID)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)((HGDIOBJ)GETWORD(SP+12),
			(UINT)GETWORD(SP+10),
			(UINT)GETWORD(SP+8),
			(PALETTEENTRY *)lpStruct);
	envp->reg.sp += HANDLE_86 + 2*UINT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETTEMPFN (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
#ifdef TWIN32
	retcode = GetTempFileName16((BYTE)GETWORD(SP+14),
			lpString1, (UINT)GETWORD(SP+8), lpString2);
#else
	retcode = GetTempFileName((BYTE)GETWORD(SP+14),
			lpString1, (UINT)GETWORD(SP+8), lpString2);
#endif
	envp->reg.sp += 2*LP_86 + WORD_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_LOADLIBRARY (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;
	char FileName[_MAX_PATH];

	lpString = (LPSTR)GetAddress
		(GETWORD(SP+6),GETWORD(SP+4));
    	xdoscall(XDOS_GETALTNAME,0,(void *) FileName,(void *) lpString);
	retcode = LoadLibrary(FileName);
	if (retcode > 32)
		retcode = GetDataSelectorFromInstance(retcode);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETPRIVATEPRINT (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2,lpString3;
	char FileName[_MAX_PATH];

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+16),GETWORD(SP+14));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	lpString3 = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
    	xdoscall(XDOS_GETALTNAME,0,(void *) FileName,(void *) lpString3);
	retcode = (f)(lpString1, 
			lpString2,
			GETWORD(SP+8),
			FileName);
	envp->reg.sp += 3*LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETPRIVATEPRSTR (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2,lpString3,lpString4,lpString5;
	char FileName[_MAX_PATH];

   	lpString1 = (LPSTR)GetAddress
      				(GETWORD(SP+24),GETWORD(SP+22));
  	lpString2 = (LPSTR)GetAddress
      				(GETWORD(SP+20),GETWORD(SP+18));
  	lpString3 = (LPSTR)GetAddress
            		(GETWORD(SP+16),GETWORD(SP+14));
  	lpString4 = (LPSTR)GetAddress
            		(GETWORD(SP+12),GETWORD(SP+10));
  	lpString5 = (LPSTR)GetAddress
            		(GETWORD(SP+6),GETWORD(SP+4));
    	xdoscall(XDOS_GETALTNAME,0,(void *) FileName,(void *) lpString5);
  	retcode = (f)(lpString1, 
                 	lpString2,
                 	lpString3,
                 	lpString4,
                 	(int)((short)GETWORD(SP+8)),
                 	FileName);
  	envp->reg.sp += 5*LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_WRITEPRIVPRSTR (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2,lpString3,lpString4;
	char FileName[_MAX_PATH];

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+18),GETWORD(SP+16));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	lpString3 = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	lpString4 = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
    	xdoscall(XDOS_GETALTNAME,0,(void *) FileName,(void *) lpString4);
	retcode = (f)(lpString1, 
			lpString2,
			lpString3,
			FileName);
	envp->reg.sp += 4*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SYSPARAMINFO (ENV *envp,LONGPROC f)
{
	DWORD retcode=0;
	UINT uAction;
	LPBYTE lpStruct;
	LPVOID lpvParam;
	BOOL bParam;
	int nParam;
	WORD wParam;
	LOGFONT lf;

	uAction = (UINT)GETWORD(SP+12);
	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6));

	switch(uAction) {
	    case SPI_GETBEEP:			/* lpvParam points to BOOL */
	    case SPI_GETFASTTASKSWITCH:
	    case SPI_GETICONTITLEWRAP:
	    case SPI_GETMENUDROPALIGNMENT:
	    case SPI_GETSCREENSAVEACTIVE:
		retcode = (f)(uAction,
			(UINT)GETWORD(SP+10),
			(LPVOID)&bParam,
			(UINT)GETWORD(SP+4));
		if (retcode)
		    PUTWORD(lpStruct,(WORD)bParam);
		break;
	    case SPI_GETBORDER:			/* lpvParam points to int */
	    case SPI_GETGRIDGRANULARITY:
	    case SPI_GETKEYBOARDDELAY:
	    case SPI_GETSCREENSAVETIMEOUT:
		retcode = (f)(uAction,
			(UINT)GETWORD(SP+10),
			(LPVOID)&nParam,
			(UINT)GETWORD(SP+4));
		if (retcode)
		    PUTWORD(lpStruct,(WORD)nParam);
		break;
	    case SPI_GETICONTITLELOGFONT:	/* lpvParam -> LOGFONT */
		retcode = (f)(uAction,
			(UINT)GETWORD(SP+10),
			(LPVOID)&lf,
			(UINT)GETWORD(SP+4));
		if (retcode)
		    PutLOGFONT(lpStruct,&lf,GETWORD(SP+10));
		break;
	    case SPI_SETICONTITLELOGFONT:
		GetLOGFONT(&lf,lpStruct);
		retcode = (f)(uAction,
			(UINT)GETWORD(SP+10),
			(LPVOID)&lf,
			(UINT)GETWORD(SP+4));
		break;
	    case SPI_GETKEYBOARDSPEED:		/* lpvParam -> WORD */
		lpvParam = (LPVOID)&wParam;
		if (retcode)
		    PUTWORD(lpStruct,wParam);
		break;
	    case SPI_LANGDRIVER:		/* lpvParam -> string */
	    case SPI_SETDESKWALLPAPER:
	    case SPI_SETDESKPATTERN:
		retcode = (f)(uAction,
			(UINT)GETWORD(SP+10),
			(LPVOID)lpStruct,
			(UINT)GETWORD(SP+4));
		break;
	    case SPI_SETBEEP:
	    case SPI_SETBORDER:
	    case SPI_SETDOUBLECLKHEIGHT:
	    case SPI_SETDOUBLECLICKTIME:
	    case SPI_SETDOUBLECLKWIDTH:
	    case SPI_SETFASTTASKSWITCH:
	    case SPI_SETGRIDGRANULARITY:
	    case SPI_SETICONTITLEWRAP:
	    case SPI_SETKEYBOARDDELAY:
	    case SPI_SETKEYBOARDSPEED:
	    case SPI_SETMENUDROPALIGNMENT:
	    case SPI_SETMOUSEBUTTONSWAP:
	    case SPI_SETSCREENSAVEACTIVE:
	    case SPI_SETSCREENSAVETIMEOUT:
		retcode = (f)(uAction,
			(UINT)GETWORD(SP+10),
			(LPVOID)NULL,
			(UINT)GETWORD(SP+4));
		break;
	    case SPI_ICONHORIZONTALSPACING:
	    case SPI_ICONVERTICALSPACING:
		retcode = (f)(uAction,
			(UINT)GETWORD(SP+10),
			(lpStruct)?(LPVOID)&nParam:(LPVOID)NULL,
			(UINT)GETWORD(SP+4));
		if (retcode && lpStruct)
		    PUTWORD(lpStruct,(WORD)nParam);    
		break;
#ifdef LATER
	    case SPI_GETMOUSE:
	    case SPI_SETMOUSE:
		break;
#endif
	    default:
		break;
	}
	envp->reg.sp += 3*UINT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_ENUMPENPROC (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LOGPEN lp;
	LPBYTE lpStruct;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	if (lpStruct) {
	    lp.lopnStyle = (UINT)GETWORD(lpStruct);
	    GetPOINT(lp.lopnWidth,lpStruct+2);
	    lp.lopnColor = (COLORREF)GETDWORD(lpStruct+6);
	    retcode = (f)(&lp,GETDWORD(SP+4));
	}
	else
	    retcode = (f)(NULL,GETDWORD(SP+4));
	envp->reg.sp += 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
}

void
IT_ENUMBRUSHPROC (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LOGBRUSH lb;
	LPBYTE lpStruct;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	if (lpStruct) {
	    lb.lbStyle = (UINT)GETWORD(lpStruct);
	    lb.lbColor = GETDWORD(lpStruct+2);
	    lb.lbHatch = (int)((short)GETWORD(lpStruct+6));
	    retcode = (f)(&lb,GETDWORD(SP+4));
	}
	else
	    retcode = (f)(NULL,GETDWORD(SP+4));
	envp->reg.sp += 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
}
	
void
IT_ENUMFONTPROC (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LOGFONT lf;
	LOGFONT *lplf = 0;
	TEXTMETRIC tm;
	TEXTMETRIC *lptm = 0;
	LPBYTE lpStruct;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+16),GETWORD(SP+14));
	if (lpStruct) {
	    lplf = &lf;
	    GetLOGFONT(lplf,lpStruct);
	}
	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	if (lpStruct) {
	    lptm = &tm;
	    tm.tmHeight = (int)(short)GETWORD(lpStruct);
	    tm.tmAscent = (int)(short)GETWORD(lpStruct+2);
	    tm.tmDescent = (int)(short)GETWORD(lpStruct+4);
	    tm.tmInternalLeading = (int)(short)GETWORD(lpStruct+6);
	    tm.tmExternalLeading = (int)(short)GETWORD(lpStruct+8);
	    tm.tmAveCharWidth = (int)(short)GETWORD(lpStruct+10);
	    tm.tmMaxCharWidth = (int)(short)GETWORD(lpStruct+12);
	    tm.tmWeight = (int)(short)GETWORD(lpStruct+14);
	    tm.tmItalic = *(lpStruct+16);
	    tm.tmUnderlined = *(lpStruct+17);
	    tm.tmStruckOut = *(lpStruct+18);
	    tm.tmFirstChar = *(lpStruct+19);
	    tm.tmLastChar = *(lpStruct+20);
	    tm.tmDefaultChar = *(lpStruct+21);
	    tm.tmBreakChar = *(lpStruct+22);
	    tm.tmPitchAndFamily = *(lpStruct+23);
	    tm.tmCharSet = *(lpStruct+24);
	    tm.tmOverhang = (int)(short)GETWORD(lpStruct+25);
	    tm.tmDigitizedAspectX = (int)(short)GETWORD(lpStruct+27);
	    tm.tmDigitizedAspectY = (int)(short)GETWORD(lpStruct+29);
	}
	retcode = (f)(lplf,lptm,
			GETWORD(SP+8),
			GETDWORD(SP+4));
	envp->reg.sp += 3*LP_86 + WORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
}

void
IT_GETDIR (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;
	UINT cbBuffer;
	char buf[_MAX_PATH];
	char Path[_MAX_PATH];

	lpString = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	cbBuffer = (UINT)GETWORD(SP+4);
	retcode = (f)(buf,_MAX_PATH);
	if (retcode) {
            xdoscall(XDOS_GETDOSPATH,0,(void *) Path,(void *) buf);
	    strncpy(lpString,Path,min(cbBuffer,strlen(Path)+1));
	    retcode = strlen(lpString);
	}
	envp->reg.sp += UINT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_WINFROMPT (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	POINT pt;

	GetPOINT(pt,SP+4);
	retcode = WindowFromPoint(pt);
	envp->reg.sp += 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CHWINFROMPT (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	POINT pt;

	GetPOINT(pt,SP+4);
	retcode = ChildWindowFromPoint((HANDLE)GETWORD(SP+8),pt);
	envp->reg.sp += HANDLE_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_PTINRECT (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;
	POINT pt;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	RECT_TO_C(rcRect,lpStruct);
	GetPOINT(pt,SP+4);
	retcode = PtInRect(&rcRect,pt);
	envp->reg.sp += LP_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_NOTIFYREG (ENV *envp,LONGPROC f)	/* NotifyRegister */
{
    DWORD retcode, dwProc;
    HTASK hTask;

    dwProc = GETDWORD(SP+6);
#ifdef	LATER
    dwProc = (dwProc)?make_native_thunk(dwProc,(DWORD)hsw_notifyreg):0L;
#endif
    hTask = GetSelectorHandle(GETWORD(SP+10));
    retcode = NotifyRegister(hTask,
			(LPFNNOTIFYCALLBACK)dwProc,
			GETWORD(SP+4));
    envp->reg.sp += HANDLE_86 + LP_86 + WORD_86 + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}

void
IT_INTERRUPTREG (ENV *envp,LONGPROC f)	/* InterruptRegister */
{
    DWORD retcode, dwProc;
    HTASK hTask;

    dwProc = GETDWORD(SP+4);
#ifdef	LATER
    dwProc = (dwProc)?make_native_thunk(dwProc,(DWORD)hsw_interruptreg):0L;
#endif
    hTask = GetSelectorHandle(GETWORD(SP+8));
    retcode = InterruptRegister(hTask,(FARPROC)dwProc);
    envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}

void
IT_MEMMANINFO (ENV *envp,LONGPROC f)	/* MemManInfo */
{
    DWORD retcode,dwCount,dwBuffer;
    LPBYTE lpStruct;
    MEMMANINFO mmi;
    int i;

    if (!(lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4))))
	retcode = 0;
    else {
	mmi.dwSize = sizeof(MEMMANINFO);
	retcode = MemManInfo(&mmi);
	if (retcode) {
	    dwBuffer = GETDWORD(lpStruct);
	    lpStruct += DWORD_86;
	    for (i = 0, dwCount = 2*DWORD_86;
		 i < 9;
		 dwCount += DWORD_86, i++, lpStruct += 4) {
		if (dwCount >= dwBuffer)
		    break;
		PUTDWORD(lpStruct,*((LPDWORD)&mmi+1+i));
	    }
	    if ((dwCount - DWORD_86 + WORD_86) < dwBuffer)
		PUTWORD(lpStruct,mmi.wPageSize);
	}
    }
    envp->reg.sp += LP_86 + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}

void
IT_TIMERCOUNT (ENV *envp,LONGPROC f)
{
    DWORD   retcode;
    LPBYTE lpStruct;
    TIMERINFO ti;

    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
    if (lpStruct) {
	ti.dwSize = GETDWORD(lpStruct);
	retcode = (f)(&ti);
	if (retcode) {
	    PUTDWORD(lpStruct+4,ti.dwmsSinceStart);
	    PUTDWORD(lpStruct+8,ti.dwmsThisVM);
	}
    }
    else
	retcode = (f)(NULL);

    envp->reg.sp += LP_86 + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}

void
IT_LREADWR (ENV *envp,LONGPROC f)
{
	DWORD   retcode;
	HANDLE  handle;
	LPSTR	lpAddress;
	UINT	count;

	handle = (HANDLE)GETWORD(SP+10);
	lpAddress = (char *)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	count = (UINT)GETWORD(SP+4);

	retcode = (f)(handle,lpAddress,count);

	envp->reg.sp += HANDLE_86 + LP_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_HREADWR (ENV *envp,LONGPROC f)
{
	DWORD   retcode;
	HANDLE  handle;
	LPSTR	lpAddress;
	LONG	count;

	handle = (HANDLE)GETWORD(SP+12);
	lpAddress = (char *)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	count = (LONG)GETDWORD(SP+4);

	retcode = (f)(handle,lpAddress,count);

	envp->reg.sp += HANDLE_86 + LP_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

static UINT _86_WNetGetConnection(LPSTR,LPSTR,UINT *);

void
IT_WNETGETCONN(ENV *envp,LONGPROC f)
{
	UINT ui;
	LPSTR lpString1,lpString2;
	LPBYTE lpStr;
	DWORD retcode;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	lpStr = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	ui = (UINT)GETWORD(lpStr);
	retcode = (DWORD)_86_WNetGetConnection(lpString1,lpString2,&ui);
	PUTWORD(lpStr,(WORD)ui);
	envp->reg.sp += 3*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_WNETGETUSER(ENV *envp,LONGPROC f)	/* WNetGetUser */
{
	LPSTR lpString;
	LPBYTE lpData;
	WORD wLen;
	DWORD retcode;

	lpString = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	lpData = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	wLen = GETWORD(lpData);
	retcode = WNetGetUser(lpString,&wLen);
	envp->reg.sp += 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

static UINT
_86_WNetGetConnection(LPSTR lpszLocalName, LPSTR lpszRemoteName,
		  UINT *lpcbRemoteName)
{
    char netname[_MAX_PATH];
    char drivename[5];
    UINT cbmax;

    cbmax = *lpcbRemoteName;
    if (!strcmp(lpszLocalName,"C:") || !strcmp(lpszLocalName,"D:")) {
	strcpy(drivename,lpszLocalName);
	strcat(drivename,"\\");
    	xdoscall(XDOS_GETALTNAME,0,(void *) netname,(void *) drivename);
	strncpy(lpszRemoteName,netname,min(strlen(netname)+1,cbmax));
	*lpcbRemoteName = min(strlen(netname),cbmax);
	return WN_SUCCESS;
    }
    else if (!strcmp(lpszLocalName,"LPT1:")) {
	strcpy(lpszRemoteName,lpszLocalName);
	*lpcbRemoteName = strlen("LPT1:");
	return WN_SUCCESS;
    }
    else {
	*lpcbRemoteName = 0;
	return WN_NET_ERROR;
    }
}

void
IT_1LPME(ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	MODULEENTRY me;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	me.wNext = GETWORD(lpStruct + DWORD_86 + MAX_MODULE_NAME + 1 +
			HANDLE_86 + WORD_86 + MAX_PATH + 1);
	me.dwSize = GETDWORD(lpStruct);
	retcode = (f)(&me);
	if (retcode)
	    GetMODULEENTRY(lpStruct,&me);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

static void
GetMODULEENTRY(LPBYTE lpData, MODULEENTRY *lpme)
{
	LPBYTE ptr = lpData;

	PUTDWORD(ptr,lpme->dwSize);
	ptr += DWORD_86;
	memcpy((LPSTR)ptr,(LPSTR)&lpme->szModule[0],MAX_MODULE_NAME+1);
	ptr += MAX_MODULE_NAME+1;
	PUTWORD(ptr,lpme->hModule);
	ptr += HANDLE_86;
	PUTWORD(ptr,lpme->wcUsage);
	ptr += WORD_86;
	memcpy((LPSTR)ptr,(LPSTR)&lpme->szExePath[0],MAX_PATH+1);
	ptr += MAX_PATH+1;
	PUTWORD(ptr,lpme->wNext);
}

void
IT_GETCURTASK(ENV *envp,LONGPROC f)
{
    HTASK hTask;
    LPTASKINFO lpTaskInfo;

    hTask = GetCurrentTask();
    if ((lpTaskInfo = GETTASKINFO(hTask))) {
	envp->reg.ax = lpTaskInfo->wTDBSelector;
	RELEASETASKINFO(lpTaskInfo);
    }
    else
	envp->reg.ax = 0;
    envp->reg.sp += RET_86;
}

void
IT_GETWINTASK(ENV *envp,LONGPROC f)
{
    HTASK hTask;
    LPTASKINFO lpTaskInfo;

    hTask = GetWindowTask((HWND)GETWORD(SP+4));
    if ((lpTaskInfo = GETTASKINFO(hTask))) {
	envp->reg.ax = lpTaskInfo->wTDBSelector;
	RELEASETASKINFO(lpTaskInfo);
    }
    else
	envp->reg.ax = 0;
    envp->reg.sp += HANDLE_86 + RET_86;
}

void
IT_1HTASK(ENV *envp,LONGPROC f)
{
    HTASK hTask;
    DWORD retcode;

    hTask = GetSelectorHandle(GETWORD(SP+4));
    retcode = (f)(hTask);
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
    envp->reg.sp += HANDLE_86 + RET_86;
}

void
IT_BITBLT (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+22),
			(int)((short)GETWORD(SP+20)),
			(int)((short)GETWORD(SP+18)),
			(int)((short)GETWORD(SP+16)),
			(int)((short)GETWORD(SP+14)),
			(HANDLE)GETWORD(SP+12),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			GETDWORD(SP+4));
	envp->reg.sp += 2*HANDLE_86 + 6*INT_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_STRETCHBLT (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+26),
			(int)((short)GETWORD(SP+24)),
			(int)((short)GETWORD(SP+22)),
			(int)((short)GETWORD(SP+20)),
			(int)((short)GETWORD(SP+18)),
			(HANDLE)GETWORD(SP+16),
			(int)((short)GETWORD(SP+14)),
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			GETDWORD(SP+4));
	envp->reg.sp += 2*HANDLE_86 + 8*INT_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_DLGDIRLIST (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	retcode = (f)((HANDLE)GETWORD(SP+14),
			lpString,
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + LP_86 + 2*INT_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETMENUSTRING (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	retcode = (f)((HANDLE)GETWORD(SP+14),
			(UINT)GETWORD(SP+12),
			lpString,
			(int)((short)GETWORD(SP+6)),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + 2*UINT_86 + LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_LOADMENUIND (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpData, lpMenu;

	lpData = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	lpMenu = hswr_menu_bin_to_nat(lpData);
	retcode = (DWORD)TWIN_LoadMenuIndirect(lpMenu);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_WINHELP (ENV *envp, LONGPROC f)
{
    LPSTR lpszHelpFile;
    HWND hwnd;
    UINT fuCommand;
    DWORD dwData;
    DWORD retcode;
    MULTIKEYHELP *mkhp;
    HELPWININFO hwi;
    BYTE *bp;
    
    hwnd = GETWORD(SP+14);
    lpszHelpFile = (LPSTR) GetAddress(GETWORD(SP+12),GETWORD(SP+10));
    fuCommand = GETWORD(SP+8);
    dwData = GETDWORD(SP+4);

    switch (fuCommand)
    {
      case HELP_KEY:
      case HELP_PARTIALKEY:
      case HELP_COMMAND:
	dwData = (DWORD) GetAddress(HIWORD(dwData), LOWORD(dwData));
	break;

      case HELP_MULTIKEY:
	bp = (BYTE *) GetAddress(HIWORD(dwData), LOWORD(dwData));
	mkhp = (MULTIKEYHELP *) WinMalloc(sizeof(*mkhp) + GETWORD(bp));
	mkhp->mkSize = GETWORD(bp);
	mkhp->mkKeylist = bp[2];
	strcpy((char*) mkhp->szKeyphrase, (char*) bp + 3);
	dwData = (DWORD) mkhp;
	break;

      case HELP_SETWINPOS:
	bp = (BYTE *) GetAddress(HIWORD(dwData), LOWORD(dwData));
	hwi.wStructSize = GETWORD(bp);
	hwi.x = GETWORD(bp+2);
	hwi.y = GETWORD(bp+4);
	hwi.dx = GETWORD(bp+6);
	hwi.dy = GETWORD(bp+8);
	hwi.wMax = GETWORD(bp+10);
        hwi.rgchMember[0] = *(bp+12);
        hwi.rgchMember[1] = *(bp+13);
	dwData = (DWORD) &hwi;
	break;
    }

    retcode = WinHelp(hwnd, lpszHelpFile, fuCommand, dwData);

    envp->reg.sp += HANDLE_86 + LP_86 + UINT_86 + DWORD_86 + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}

void
IT_GETWINDOWWORD (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	int nOffset;

	nOffset = (int) ((short) GETWORD(SP+4));
#ifdef	TWIN32
	if (nOffset == GWL_HINSTANCE ||
	    nOffset == GWL_HWNDPARENT ||
	    nOffset == GWL_ID)
	    retcode = GetWindowLong((HWND)GETWORD(SP+6), nOffset);
	else
#endif
	    retcode = GetWindowWord((HWND)GETWORD(SP+6), nOffset);
	if (nOffset ==
#ifndef	TWIN32
		GWW_HINSTANCE
#else
		GWL_HINSTANCE
#endif
				)
		retcode = GetDataSelectorFromInstance(retcode);
	
	envp->reg.sp += HANDLE_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SETWINDOWWORD (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	int nOffset;
	WORD wVal;

	nOffset = (int) ((short) GETWORD(SP+6));
	wVal = (WORD)GETWORD(SP+4);
	if (nOffset ==
#ifndef	TWIN32
		GWW_HINSTANCE
#else
		GWL_HINSTANCE
#endif
				)
		wVal = GetSelectorHandle(wVal);
	
#ifdef	TWIN32
	if (nOffset == GWL_HINSTANCE ||
	    nOffset == GWL_HWNDPARENT ||
	    nOffset == GWL_ID)
	    retcode = SetWindowLong((HWND)GETWORD(SP+8), nOffset, wVal);
	else
#endif
	    retcode = SetWindowWord((HWND)GETWORD(SP+8), nOffset, wVal);
	envp->reg.sp += HANDLE_86 + INT_86 + WORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}


void
IT_FINDEXECUTABLE (ENV *envp,LONGPROC f)
{
	LPSTR lpszFile;
	LPSTR lpszDir;
	LPSTR lpResult;
	DWORD retcode;
	WORD sel,offset;

	sel = GETWORD(SP+6);
	offset = GETWORD(SP+4);
	lpResult = (LPSTR)GetAddress(sel,offset);

	sel = GETWORD(SP+10);
	offset = GETWORD(SP+8);
	lpszDir = (LPSTR)GetAddress(sel,offset);

	sel = GETWORD(SP+14);
	offset = GETWORD(SP+12);
	lpszFile = (LPSTR)GetAddress(sel,offset);

        retcode = (f)(lpszFile,lpszDir,lpResult);

	envp->reg.sp += 3*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_CALCCHILDSCROLL (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	HWND  hwnd;
	WORD  wValue;

	hwnd = GETWORD(SP+6);
	wValue =   GETWORD(SP+4);

        retcode = (f)(hwnd,wValue);

	envp->reg.sp += 2*WORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

int
CalcChildScroll(HWND hWnd,WORD value)
{
	int rc;
	rc = 0;
	printf("calcchildscroll: %x %x\n",hWnd,value);	
	ShowWindow(hWnd,SW_SHOW);
	UpdateWindow(hWnd);
	return rc;
}

