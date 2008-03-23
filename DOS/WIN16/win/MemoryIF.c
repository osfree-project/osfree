/*    
	MemoryIF.c	2.27
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
/*
#include "toolhelp.h"
*/
#define __TOOLHELP_H__
#include "Willows.h"

#include "kerndef.h"
#include "Endian.h"
#include "Log.h"
#include "Kernel.h"
#include "DPMI.h"
#include "BinTypes.h"
#include "Resources.h"
#include "Module.h"
#include "LoadEXE.h"
#include "dos.h"
#include "make_thunk.h"

#include <stdio.h>
#include <string.h>


extern DSCR *LDT;
extern ENV *envp_global;
extern WORD Sel86Flags[];
extern BOOL WINAPI TWIN_LocalInit(UINT uSegment, UINT uStartAddr, UINT uEndAddr);
extern HLOCAL WINAPI TWIN_LocalAlloc(UINT fuAllocFlags, UINT fuAlloc);
extern HLOCAL WINAPI TWIN_LocalReAlloc(HLOCAL hloc, UINT fuNewSize, UINT fuFlags);
extern UINT TWIN_AllocSelector(DWORD,UINT);
extern UINT TWIN_ReallocSelector(UINT,DWORD,UINT);

void IT_LOCALALLOC (ENV *,LONGPROC);
void IT_LOCALREALLOC (ENV *,LONGPROC);
void IT_LOCALINIT (ENV *,LONGPROC);
void IT_GLOBALALLOC (ENV *,LONGPROC);
void IT_GLOBALREALLOC (ENV *,LONGPROC);
void IT_GLOBALLOCK (ENV *,LONGPROC);
void IT_GLOBALUNLOCK (ENV *,LONGPROC);
void IT_GLOBALFREE (ENV *,LONGPROC);
void IT_GLOBALSIZE (ENV *,LONGPROC);
void IT_GLOBALLRU (ENV *,LONGPROC);
void IT_GLOBALNOTIFY (ENV *,LONGPROC);
void IT_LOADRESOURCE (ENV *,LONGPROC);
void IT_LOCKRESOURCE (ENV *,LONGPROC);
void IT_FREERESOURCE (ENV *,LONGPROC);
void IT_ACCESSRESOURCE (ENV *,LONGPROC);
void IT_SIZEOFRESOURCE (ENV *,LONGPROC);
void IT_GETCLIPBDATA (ENV *,LONGPROC);
void IT_SETCLIPBDATA (ENV *,LONGPROC);
void _86_GlobalHandle (ENV *,LONGPROC);
void _86_GetCodeHandle (ENV *,LONGPROC);
void _86_GlobalPageUnlock (ENV *,LONGPROC);
void _86_LocalNotify (ENV *,LONGPROC);
void _86_GetCurrentPDB (ENV *,LONGPROC);
void IT_LPINITT0 (ENV *,LONGPROC);
void IT_GLOBALENTRYHANDLE (ENV *,LONGPROC);

extern LPBYTE ConvertResourceToBin(LPBYTE,WORD);
extern CreateDataInstance(HINSTANCE,HMODULE,HTASK);

static int AccessResourceBin(HINSTANCE, HRSRC);
static void ZapSegRegs(ENV *,WORD);

/* IsBadPtr should be declared in a header file */
/* so it is accessible to ModTable.c as well. */
BOOL IsBadPtr(const void *, UINT);

void
IT_LOCALALLOC (ENV *envp,LONGPROC f)	/* TWIN_LocalAlloc */
{
	DWORD retcode;

	if (envp != envp_global)
	    envp_global = envp;
	retcode = TWIN_LocalAlloc((UINT)GETWORD(SP+6),
		(UINT)GETWORD(SP+4));
	envp->reg.sp += 2*UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_LOCALREALLOC (ENV *envp,LONGPROC f)	/* TWIN_LocalReAlloc */
{
	DWORD retcode;

	retcode = TWIN_LocalReAlloc((HANDLE)GETWORD(SP+8),
			(UINT)GETWORD(SP+6),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + 2*UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_LOCALINIT (ENV *envp,LONGPROC f)	/* TWIN_LocalInit */
{
    DWORD retcode;
	UINT seg;

	seg = (UINT)GETWORD(SP+8);
	if (!seg)
	    seg = envp->reg.ds;
	retcode = TWIN_LocalInit(seg, (UINT)GETWORD(SP+6),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += 3*UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GLOBALALLOC (ENV *envp,LONGPROC f)	/* GlobalAlloc */
{
	UINT uSel,flags;
	DWORD dwSize;
	DWORD dwMemSize;
	HGLOBAL hGlobal;

	dwSize = GETDWORD(SP+4);
	flags = GETWORD(SP+8);
	dwMemSize=dwSize;
	dwSize = max(32,(dwSize + 31) & 0xffffffe0);

	uSel = TWIN_AllocSelector(dwSize,flags);

	if (uSel) {
	    hGlobal = GetSelectorHandle(uSel);
	    CreateDataInstance((HINSTANCE)hGlobal,(HMODULE)0,
			GetCurrentTask());

	    if (flags & GMEM_MOVEABLE) {
		GlobalUnlock(hGlobal);
		uSel &= 0xfffe;
	    }
	}
	envp->reg.sp += UINT_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(uSel);
	envp->reg.dx = 0;
}

void
IT_GLOBALREALLOC (ENV *envp,LONGPROC f)		/* GlobalReAlloc */
{
    UINT uiFlags, uSel;
    HGLOBAL hGlobal;
    LPBYTE  lpSeg;
    WORD wSP=0,wBP=0;
    DWORD dwNewSize;

    uSel = GETWORD(SP+10) | 1;
    uiFlags = (UINT)GETWORD(SP+4);
    dwNewSize = GETDWORD(SP+6);


    if (dwNewSize != 0) {
#ifdef LATER
	dwNewSize = max(dwNewSize,32);
	dwNewSize = (dwNewSize + 31) & 0xffffffe0;
#endif
        
	lpSeg = GetPhysicalAddress(uSel);
	if (uSel == envp->reg.ss) {
	    wSP = (WORD)(SP - lpSeg);
	    wBP = (WORD)(BP - lpSeg);
	}
	uSel = TWIN_ReallocSelector(uSel,dwNewSize,uiFlags);
	hGlobal = GetSelectorHandle(uSel);
	if (uSel == envp->reg.ss) {
	    envp->reg.sp = (REGISTER)(GetPhysicalAddress(uSel) + wSP);
	    envp->reg.bp = (REGISTER)(GetPhysicalAddress(uSel) + wBP);
	}
	if (uiFlags & GMEM_MOVEABLE) {
	    GlobalUnlock(hGlobal);
	    uSel &= 0xfffe;
	}
    } else {
	/* check if we are discarding, size 0,... */
	hGlobal = GetSelectorHandle(uSel);
	(void) GlobalReAlloc(hGlobal,0,uiFlags);
    }

    envp->reg.sp += HANDLE_86 + UINT_86 + DWORD_86 + RET_86;
    envp->reg.ax = LOWORD(uSel);
    envp->reg.dx = 0;
}

void
IT_GLOBALLOCK (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	WORD wSel;
	HGLOBAL hGlobal=0;
	HMODULE hModule;
	MODULEINFO *modinfo = NULL;
	UINT	    memflags;

	wSel = GETWORD(SP+4) | 1;
	/* This is extra work that should be eliminated */
	if (!(wSel & OM_MASK)) {		/* can be hInstance */
	    hGlobal = GetSelectorHandle(wSel);
	    memflags = GlobalFlags(hGlobal);
	    retcode = (DWORD)GlobalLock(hGlobal);
	}
	else {
	    if ((hModule = GetModuleFromInstance(wSel))) {
		modinfo = GETMODULEINFO(hModule);
		if ((wSel = GetDataSelectorFromInstance(wSel))) {
	    	    hGlobal = GetSelectorHandle(wSel);
		    retcode = (DWORD)GlobalLock(hGlobal);
		}
		else 
		    retcode = (DWORD) NULL;
	    }
	    else
		retcode = (DWORD) NULL;
	}
			
	envp->reg.sp += HANDLE_86 + RET_86;
	if (retcode)
		retcode = (DWORD)MAKELP(wSel,0);
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
	if (modinfo != NULL)
	  RELEASEMODULEINFO(modinfo);
}

void
IT_GLOBALUNLOCK (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	WORD wSel;
	HGLOBAL hGlobal;

	wSel = GETWORD(SP+4) | 1;
	hGlobal = GetSelectorHandle(wSel);
				
	retcode = (f)(hGlobal);
	envp->reg.sp += HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GLOBALFREE (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	WORD wSel;
	HGLOBAL hGlobal;

	wSel = GETWORD(SP+4) | 1;

	if (wSel == 1)
	{
		envp->reg.sp += HANDLE_86 + RET_86;
		envp->reg.ax = GETWORD(SP+4);
		envp->reg.dx = 0;
		return;
	}

	hGlobal = GetSelectorHandle(wSel);
				
	ZapSegRegs(envp,wSel);

        /* for HUGE objects */
	while ((GetSelectorHandle((UINT)(wSel-8))==hGlobal)) 
	    wSel -= 8;

	while (GetSelectorHandle(wSel) == hGlobal) {
	    FreeSelector(wSel);
	    wSel += 8;
	}
	retcode = (f)(hGlobal);
	envp->reg.sp += HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
_86_GlobalHandle (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	WORD wSel;
	HGLOBAL hGlobal;
	MEMORYINFO *lpMemory = NULL;

	wSel = GETWORD(SP+4);
	hGlobal = GetSelectorHandle(wSel);
	if (hGlobal && (lpMemory = GETHANDLEINFO(hGlobal))) {
	    if (!(lpMemory->wFlags & GMEM_MOVEABLE))
		wSel |= 1;
	    else
		wSel &= 0xfffe;
	}
	else
	    wSel &= 0xfffe;
	retcode = MAKELONG(wSel,wSel | 1);
	envp->reg.sp += WORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
	if (lpMemory != NULL)
	  RELEASEHANDLEINFO(lpMemory);
}

void
_86_GetCodeHandle (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	WORD wSel;

	wSel = HIWORD(GETDWORD(SP+4));

	if (GetSelectorFlags(wSel) & DF_CODE)
	    retcode = MAKELONG(wSel & 0xfffe,wSel | 1);
	else	/* data segment */
	    retcode = 0;
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GLOBALSIZE (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	WORD wSel;
	HGLOBAL hGlobal;

	wSel = GETWORD(SP+4);
	hGlobal = GetSelectorHandle(wSel);

	if(hGlobal == wSel) {
		retcode = GlobalSize(hGlobal);
		if(retcode == 0)
			retcode = 762;
	} else
		retcode = GetSelectorLimit(wSel);
		
	envp->reg.sp += HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GLOBALLRU (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	HGLOBAL hGlobal;
	WORD wSel;

	wSel = GETWORD(SP+4);
	hGlobal = GetSelectorHandle(wSel);
		
	retcode = (f)(hGlobal);
	envp->reg.sp += HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = LOWORD(retcode);
}

void
IT_GLOBALNOTIFY (ENV *envp,LONGPROC f)
{
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = 0;
	envp->reg.dx = 0;
}

void
_86_LocalNotify (ENV *envp,LONGPROC f)
{
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = 0;
	envp->reg.dx = 0;
}

void
IT_LOADRESOURCE (ENV *envp,LONGPROC f)
{
	HGLOBAL	hGlobal;
	WORD wSel = 0;
	WORD wIndex;
	MEMORYINFO *lpMemory = NULL;
	int i,nNumSel;
	DWORD dwSelSize;
	HINSTANCE hInst;

	hInst = GetSelectorHandle(GETWORD(SP+6));
	hGlobal = LoadResource(hInst, (HRSRC)GETWORD(SP+4));
	if ((lpMemory = GETHANDLEINFO(hGlobal))) {
	    if (lpMemory->lpData == NULL) 
	        lpMemory->lpData = (LPSTR)ConvertResourceToBin
			((LPBYTE)lpMemory->lpCore,lpMemory->wIndex);
	    if (lpMemory->lpData) {
		nNumSel = (lpMemory->dwBinSize + 0xffff) / 0x10000;
		wIndex = AssignSelRange(nNumSel);
		for (i = 0, dwSelSize = lpMemory->dwBinSize;
		     i < nNumSel; dwSelSize -= 0x10000, i++) {
		    wSel = (wIndex+i)<<3;
		    AssignSelector((LPBYTE)lpMemory->lpData + 4 + (i*0x10000),
				wSel,TRANSFER_DATA,dwSelSize);
		    SetSelectorHandle(wSel,hGlobal);
		}
		wSel = wIndex << 3 | 7;
	    }
	}
	envp->reg.ax = wSel & 0xfffe;
	envp->reg.dx = 0;
	envp->reg.sp += 2*HANDLE_86 + RET_86;
	if (lpMemory != NULL) 
	  RELEASEHANDLEINFO(lpMemory);
}

extern LPBYTE ConvertResourceToBin(LPBYTE,WORD);

void
IT_LOCKRESOURCE (ENV *envp,LONGPROC f)
{
	DWORD retcode = 0L;
	WORD wSel;
	HGLOBAL	hGlobal;

	wSel = GETWORD(SP+4);
	hGlobal = GetSelectorHandle(wSel);
	if ((retcode = (DWORD)GlobalLock(hGlobal)))
	    retcode = (DWORD)MAKELP(wSel | 1,0);
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
	envp->reg.sp += HANDLE_86 + RET_86;
}

void
IT_FREERESOURCE (ENV *envp,LONGPROC f)
{
	WORD wSel;
	HGLOBAL hGlobal;

	wSel = GETWORD(SP+4) | 1;
	hGlobal = GetSelectorHandle(wSel);
	GlobalUnlock(hGlobal);

				
	ZapSegRegs(envp,wSel);
	FreeSelector(wSel);
#ifdef LATER
	envp->reg.ax = GlobalFree(hGlobal);
#else
	envp->reg.ax = 0;
#endif
	envp->reg.dx = 0;
	envp->reg.sp += HANDLE_86 + RET_86;
}

static int
AccessResourceBin(HINSTANCE hInst, HRSRC hRes)
{
    NAMEINFO *lpResInfo;
    MODULEINFO *modinfo = NULL;
    HFILE fd;
    HMODULE hModule;
    MEMORYINFO *lpMemory = NULL;
    OFSTRUCT of;
    char lpszFileName[_MAX_PATH];

    if (!(lpMemory = GETHANDLEINFO(hRes)))
	return 0;
    if (!(lpResInfo = (LPNAMEINFO)lpMemory->lpCore))
	return 0;
    if (!(modinfo = GETMODULEINFO(hInst))) {
	hModule = GetModuleFromInstance(hInst);
	if (!(modinfo = GETMODULEINFO(hModule))) {
	    RELEASEHANDLEINFO(lpMemory);
	    return 0;
	}
    }

    xdoscall(XDOS_GETDOSNAME,0,
	(void *) lpszFileName,(void *) modinfo->lpFileName);

    if ((fd = (int)OpenFile(lpszFileName,&of,OF_READ)) == HFILE_ERROR
		|| !lpResInfo->rcsoffset)
	return AccessResource(hInst,hRes);

    _llseek(fd,lpResInfo->rcsoffset,0);

    RELEASEHANDLEINFO(lpMemory);
    RELEASEMODULEINFO(modinfo);
    return (int)fd;
}

void
IT_ACCESSRESOURCE (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (DWORD)AccessResourceBin(
			(HINSTANCE)GetSelectorHandle(GETWORD(SP+6)),
			(HRSRC)GETWORD(SP+4));

	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
	envp->reg.sp += 2*HANDLE_86 + RET_86;
}

void
IT_SIZEOFRESOURCE (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	HINSTANCE hInst;
	HRSRC hrsrc;
	NAMEINFO *lpni = NULL;
	MEMORYINFO *lpMemory;

	hInst = (HINSTANCE)GetSelectorHandle(GETWORD(SP+6));
	hrsrc = (HRSRC)GETWORD(SP+4);
	lpMemory = GETHANDLEINFO(hrsrc);
	if (lpMemory)
	    lpni = (NAMEINFO *)lpMemory->lpCore;
	if (lpni) {
	    if (lpni->hGlobal) {
		lpMemory = GETHANDLEINFO(lpni->hGlobal);
		if (lpMemory && lpMemory->wType)
		    retcode = lpMemory->dwBinSize;
		else
		    retcode = lpni->rcslength;
	    }
	    else
		retcode = lpni->rcslength;
	}
	else
	    retcode = 0L;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
	envp->reg.sp += 2*HANDLE_86 + RET_86;
	if (lpMemory)
	  RELEASEHANDLEINFO(lpMemory);
}

void
IT_GETCLIPBDATA (ENV *envp,LONGPROC f)
{
	WORD wSel;
	HGLOBAL hGlobal;
	MEMORYINFO *lpMemory;
	UINT uiFormat = GETWORD(SP+4);

	hGlobal = (DWORD)(f)(uiFormat);
	if (uiFormat == CF_BITMAP)
           envp->reg.ax = hGlobal;
        else {   
	     if (hGlobal) {
	        lpMemory = GETHANDLEINFO(hGlobal);
	        if (lpMemory) {
	        	wSel = AssignSelector((LPBYTE)lpMemory->lpCore,0,
			TRANSFER_DATA,lpMemory->dwSize);
	        	SetSelectorHandle(wSel,hGlobal);
	        	envp->reg.ax = wSel & 0xfffe;
	        }
	        else 
	        	envp->reg.ax = 0;
	     }
	     else
	         envp->reg.ax = 0;
        }
	envp->reg.dx = 0;
	envp->reg.sp += UINT_86 + RET_86;
	if (lpMemory)
	  RELEASEHANDLEINFO(lpMemory);
}

void
IT_SETCLIPBDATA (ENV *envp,LONGPROC f)
{
	WORD wSel;
	HGLOBAL hGlobal;
	MEMORYINFO *lpMemory;
	UINT uiFormat;

	uiFormat = GETWORD(SP+6);

	switch (uiFormat) {
	    case CF_BITMAP:
		hGlobal = (HGLOBAL)(f)(uiFormat,GETWORD(SP+4));
		envp->reg.ax = hGlobal;
		break;
	    case CF_TEXT:
	    default:
		wSel = GETWORD(SP+4);
		hGlobal = (HGLOBAL)(f)(uiFormat,GetSelectorHandle(wSel));
		if (hGlobal) {
		    lpMemory = GETHANDLEINFO(hGlobal);
		    if (lpMemory) {
			wSel = AssignSelector((LPBYTE)lpMemory->lpCore,0,
				TRANSFER_DATA,lpMemory->dwSize);
			SetSelectorHandle(wSel,hGlobal);
			envp->reg.ax = wSel & 0xfffe;
		    }
		    else
			envp->reg.ax = 0;
		}
		else
		    envp->reg.ax = 0;
		break;
	}

	envp->reg.dx = 0;
	envp->reg.sp += UINT_86 + HANDLE_86 + RET_86;
	if (lpMemory)
	  RELEASEHANDLEINFO(lpMemory);
}

void
_86_PrestoChangoSelector (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	WORD wSelSource, wSelDest;

	wSelSource = GETWORD(SP+6);
	wSelDest = GETWORD(SP+4);
	if (GetPhysicalAddress(wSelSource) && GetPhysicalAddress(wSelDest)) {
	    if (wSelSource != wSelDest)
		CopySelector(wSelDest,wSelSource);
	    SetSelectorType(wSelDest,
		(GetSelectorType(wSelSource) == TRANSFER_CODE16)?
		TRANSFER_DATA : TRANSFER_CODE16);
	    DPMI_Notify(DN_MODIFY,wSelDest);
	    retcode = wSelDest;
	}
	else
	    retcode = 0L;
	envp->reg.sp += 2*UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
}

void
_86_AllocCStoDSAlias (ENV *envp,LONGPROC f)
{
    WORD wCodeSel,wDataSel;

    wCodeSel = GETWORD(SP+4);
    if (GetSelectorType(wCodeSel) == (LONG)TRANSFER_CODE16) {
	wDataSel = AssignSelector(GetPhysicalAddress(wCodeSel),0,
		TRANSFER_DATA,GetSelectorLimit(wCodeSel));
	SetSelectorHandle(wDataSel,GetSelectorHandle(wCodeSel));
    }
    else wDataSel = 0;
    envp->reg.sp += WORD_86 + RET_86;
    envp->reg.ax = wDataSel;
    envp->reg.dx = 0;
}

void
_86_AllocDStoCSAlias (ENV *envp,LONGPROC f)
{
    WORD wCodeSel,wDataSel;

    wDataSel = GETWORD(SP+4);
    if (GetSelectorType(wDataSel) == (LONG)TRANSFER_DATA) {
	wCodeSel = AssignSelector(GetPhysicalAddress(wDataSel),0,
		TRANSFER_CODE16,GetSelectorLimit(wDataSel));
	SetSelectorHandle(wCodeSel,GetSelectorHandle(wDataSel));
    }
    else wCodeSel = 0;
    envp->reg.sp += WORD_86 + RET_86;
    envp->reg.ax = wCodeSel;
    envp->reg.dx = 0;
}

void
_86_AllocSelector (ENV *envp,LONGPROC f)
{
	WORD wSel,wNewSel;

	wSel = GETWORD(SP+4);
	if (!wSel)
	    wNewSel = ASSIGNSEL(-1, 0);
	else
	    wNewSel = AssignSelector(GetPhysicalAddress(wSel),0,
		GetSelectorType(wSel),GetSelectorLimit(wSel));
	envp->reg.ax = wNewSel;
	envp->reg.dx = 0;
	envp->reg.sp += WORD_86 + RET_86;
}

void
_86_AllocSelectorArray (ENV *envp,LONGPROC f)
{
	WORD wNum;
	WORD wIndex;
	int n;

	wNum = GETWORD(SP+4);
	if (wNum && (wIndex = AssignSelRange(wNum))) {
	    for (n = 0; n < (int)wNum; n++)
		AssignSelector((LPBYTE)-1,(wIndex+n)<<3,
			TRANSFER_DATA,0);
	    envp->reg.ax = ((wIndex << 3) | 7) & 0xffff;
	}
	else
	    envp->reg.ax = 0;
	envp->reg.dx = 0;
	envp->reg.sp += WORD_86 + RET_86;
}

void
_86_FreeSelector (ENV *envp,LONGPROC f)
{
	WORD wSel;

	wSel = GETWORD(SP+4);
	ZapSegRegs(envp,wSel);
	FreeSelector(wSel);
	envp->reg.ax = 0;
	envp->reg.dx = 0;
	envp->reg.sp += WORD_86 + RET_86;
}

void
_86_GetSelectorBase (ENV *envp,LONGPROC f)
{
    UINT uSel;
    DWORD dwResult;

    uSel = (UINT)GETWORD(SP+4);
    dwResult = (DWORD)GetPhysicalAddress(uSel);
    envp->reg.ax = LOWORD(dwResult);   
    envp->reg.dx = HIWORD(dwResult);   
    envp->reg.sp += UINT_86 + RET_86;
}

void
_86_SetSelectorBase (ENV *envp,LONGPROC f)
{
    UINT uSel;
    DWORD dwBase;

    uSel = (UINT)GETWORD(SP+8);
    dwBase = GETDWORD(SP+4);
    SetPhysicalAddress(uSel,dwBase);
    envp->reg.ax = uSel;
    envp->reg.dx = 0;
    envp->reg.sp += UINT_86 + DWORD_86 + RET_86;
}

void
_86_GetSelectorLimit (ENV *envp,LONGPROC f)
{
    UINT uSel;
    DWORD dwResult;

    uSel = (UINT)GETWORD(SP+4);
    dwResult = GetSelectorLimit(uSel);
    envp->reg.ax = LOWORD(dwResult);   
    envp->reg.dx = HIWORD(dwResult);   
    envp->reg.sp += UINT_86 + RET_86;
}

void
_86_SetSelectorLimit (ENV *envp,LONGPROC f)
{
    UINT uSel;
    DWORD dwLimit;

    uSel = (UINT)GETWORD(SP+8);
    dwLimit = GETDWORD(SP+4);
    SetSelectorLimit(uSel,dwLimit);
    envp->reg.ax = 0;
    envp->reg.dx = 0;
    envp->reg.sp += UINT_86 + DWORD_86 + RET_86;
}

void
_86_SelectorAccessRights (ENV *envp,LONGPROC f)
{
    UINT uSel;
    WORD wOp, wRights;

    uSel = (UINT)GETWORD(SP+8);
    wOp = GETWORD(SP+6);
    if (wOp == 0)	/* AR_GET */
	envp->reg.ax = GetSelectorFlags(uSel);
    else {
	wRights = GETWORD(SP+4);
	SetSelectorFlags(uSel,wRights);
	DPMI_Notify(DN_MODIFY,uSel);
	envp->reg.ax = 0;
    }
    envp->reg.dx = 0;
    envp->reg.sp += UINT_86 + 2*WORD_86 + RET_86;
}

void
_86_GlobalDosAlloc (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	WORD wSel=0;
	LPBYTE lpData;
	DWORD dwSize;

	dwSize = GETDWORD(SP+4);
	retcode = GlobalAlloc(GMEM_FIXED, dwSize);
	if (retcode) {
		lpData = (LPBYTE)GlobalLock(retcode);
		wSel = AssignSelector(lpData,0,TRANSFER_DATA,dwSize);
		SetSelectorHandle(wSel,(HGLOBAL)retcode);
	}
	envp->reg.sp += DWORD_86 + RET_86;
	envp->reg.ax = envp->reg.dx = wSel;
}

void
_86_GlobalDosFree (ENV *envp,LONGPROC f)
{
	IT_GLOBALFREE(envp, (LONGPROC)&GlobalFree);
}

void
IT_WINEXEC (ENV *envp,LONGPROC f)
{
	char buf[0x100];
	LPSTR lpCmdLine,lpTemp;
	LPSTR lpString;
	DWORD retcode;

	lpCmdLine = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	if (!lpCmdLine || !strlen(lpCmdLine))
	    retcode = 0;
	else {
	    lpString = (LPSTR)WinStrdup(lpCmdLine);
	    lpTemp = strchr(lpString,' ');
	    if (lpTemp) 
		*lpTemp = '\0';
    	    xdoscall(XDOS_GETALTNAME,0,(void *) buf,(void *) lpString);
	    if (strlen(buf) == 0)
		strcpy(buf,lpString);
	    if (lpTemp) {
		strcat(buf," ");
		strcat(buf,++lpTemp);
	    }
	    retcode = (DWORD)WinExec(buf,(UINT)GETWORD(SP+4));
	    WinFree(lpString);
	}
	if (retcode > 32)
	    retcode = GetDataSelectorFromInstance(retcode);
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
	envp->reg.sp += LP_86 + UINT_86 + RET_86;
}

void
_86_GlobalPageUnlock (ENV *envp,LONGPROC f)
{
    DWORD retcode;
    WORD wSel;
    HGLOBAL hGlobal;

    wSel = GETWORD(SP+4);
    hGlobal = GetSelectorHandle(wSel);
		
    retcode = GlobalFlags(hGlobal);
    GlobalUnlock(hGlobal);
    envp->reg.sp += HANDLE_86 + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}

void
_86_GlobalUnfix (ENV *envp,LONGPROC f)
{
    DWORD retcode;
    WORD wSel;
    HGLOBAL hGlobal;

    wSel = GETWORD(SP+4);
    hGlobal = GetSelectorHandle(wSel);
		
    retcode = GlobalUnlock(hGlobal);
    envp->reg.sp += HANDLE_86 + RET_86;
}
   

void
IT_PLAYMETAFILE(ENV *envp,LONGPROC f)
{
	HDC hDC;
	WORD wSel;
	HGLOBAL hGlobal;

	hDC = GETWORD(SP+6);
	wSel = GETWORD(SP+4);
	hGlobal = GetSelectorHandle(wSel);
	envp->reg.ax = PlayMetaFile(hDC, hGlobal);
	envp->reg.sp += 2 * HANDLE_86 + RET_86;
}

void
IT_COPYMETAFILE(ENV *envp,LONGPROC f)
{
	char buf[0x100];
	WORD wSel;
	HGLOBAL hGlobal;
	LPSTR lpszFile;
	LPBYTE lpData;

	wSel = GETWORD(SP+8);
	hGlobal = GetSelectorHandle(wSel);
	lpszFile = GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	if (lpszFile && strchr(lpszFile,'\\')) {
    	    xdoscall(XDOS_GETALTNAME,0,(void *) buf,(void *) lpszFile); lpszFile = buf;
	}

	hGlobal = CopyMetaFile(hGlobal,lpszFile);
	if (hGlobal) {
	    lpData = (LPBYTE)GlobalLock(hGlobal);
	    wSel = AssignSelector(lpData,0,TRANSFER_DATA,
				GlobalSize(hGlobal));
	    SetSelectorHandle(wSel,hGlobal);
	    GlobalUnlock(hGlobal);
	}
	else
	    wSel = 0;
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = wSel & 0xfffe;
	envp->reg.dx = 0;
}

void
IT_CLOSEMF (ENV *envp,LONGPROC f)
{
	DWORD retcode = 0;
	WORD wSel;
	LPBYTE lpData;
	HMETAFILE hMF;

	hMF = CloseMetaFile(GETWORD(SP+4));
	if (hMF) {
		lpData = (LPBYTE)GlobalLock((HGLOBAL)hMF);
		wSel = AssignSelector(lpData,0,TRANSFER_DATA,
				GlobalSize((HGLOBAL)hMF));
		SetSelectorHandle(wSel,(HGLOBAL)hMF);
		GlobalUnlock((HGLOBAL)hMF);
		retcode = (DWORD)wSel & 0xfffe;
	}
	envp->reg.sp += HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_GETMF (ENV *envp,LONGPROC f)
{
	DWORD retcode = 0;
	WORD wSel;
	LPBYTE lpData;
	LPSTR lpString;
	HMETAFILE hMF;

	lpString = GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	hMF = GetMetaFile(lpString);
	if (hMF) {
		lpData = (LPBYTE)GlobalLock((HGLOBAL)hMF);
		wSel = AssignSelector(lpData,0,TRANSFER_DATA,
				GlobalSize((HGLOBAL)hMF));
		SetSelectorHandle(wSel,(HGLOBAL)hMF);
		GlobalUnlock((HGLOBAL)hMF);
		retcode = (DWORD)wSel & 0xfffe;
	}
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_ENUMMETAFILE (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	WORD wSel;
	DWORD dwProc;

	wSel = GETWORD(SP+12);
	if ((dwProc = GETDWORD(SP+8)))
	    dwProc = (DWORD)make_native_thunk(dwProc,(DWORD)hsw_mfenumproc);
	retcode = (f)(GETWORD(SP+14),
		GetSelectorHandle(wSel),
		dwProc,
		GETDWORD(SP+4));
	envp->reg.sp += 2*HANDLE_86 + LP_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SETMETAFILEBITS(ENV *envp, LONGPROC f)
{
	envp->reg.ax = GETWORD(SP+4);
	envp->reg.sp += HANDLE_86 + RET_86;
}


void
IT_LOADACCEL (ENV *envp,LONGPROC f)
{
	LPSTR lpString;
	HGLOBAL	hGlobal;
	WORD wSel = 0;
	MEMORYINFO *lpMemory;
	HINSTANCE hInst;

	lpString = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	hInst = GetSelectorHandle(GETWORD(SP+8));
	hGlobal = (f)((HANDLE) hInst,lpString);
	if (hGlobal && (lpMemory = GETHANDLEINFO(hGlobal))) {
	    if (lpMemory->lpData == NULL) 
		lpMemory->lpData = (LPSTR)ConvertResourceToBin
			((LPBYTE)lpMemory->lpCore,lpMemory->wIndex);
	    if (lpMemory->lpData) {
		wSel = AssignSelector((LPBYTE)lpMemory->lpData + 4,0,
			TRANSFER_DATA,lpMemory->dwBinSize);
		SetSelectorHandle(wSel,hGlobal);
	    }
	}
	
	envp->reg.ax = wSel & 0xfffe;
	envp->reg.dx = 0;
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	if (lpMemory)
	  RELEASEHANDLEINFO(lpMemory);
}

void
IT_LOCKSEGMENT (ENV *envp,LONGPROC f)
{
	UINT uSegment;

	uSegment = (UINT)GETWORD(SP+4);
	if (uSegment == 0xffff)
	    uSegment = (UINT)envp->reg.ds;
#ifdef LATER
	increment segment count
#endif
	envp->reg.sp += UINT_86 + RET_86;
	envp->reg.ax = uSegment;
	envp->reg.dx = 0;
}

void 
IT_ISBADPTR (ENV *envp,LONGPROC f)
{
        DWORD retcode;
	LPSTR lpString;

        lpString = MAKELP(GETWORD(SP+8),GETWORD(SP+6));
        retcode = (f)(lpString,(UINT)GETWORD(SP+4));
        envp->reg.sp += UINT_86 + LP_86 + RET_86;
        envp->reg.ax = LOWORD(retcode);
        envp->reg.dx = HIWORD(retcode);
}

void 
IT_ISBADHUGEPTR (ENV *envp,LONGPROC f)
{
        DWORD retcode;
	LPSTR lpString;

        lpString = MAKELP(GETWORD(SP+10),GETWORD(SP+8));
        retcode = (f)(lpString,GETDWORD(SP+4));
        envp->reg.sp += LONG_86 + LP_86 + RET_86;
        envp->reg.ax = LOWORD(retcode);
        envp->reg.dx = HIWORD(retcode);
}

void
IT_ISBADCODEPTR (ENV *envp,LONGPROC f)
{
        DWORD retcode;
	LPSTR lpString;

	lpString = MAKELP(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)(lpString,0);
        envp->reg.sp += LP_86 + RET_86;
        envp->reg.ax = LOWORD(retcode);
        envp->reg.dx = HIWORD(retcode);
}

BOOL
IsBadHugePtr(const void *lp, DWORD cb)
{
#ifdef	LATER
	/* add real logic here */
#endif
    if (cb < 65536L)
	return IsBadPtr(lp,(UINT)cb);
    MessageBox((HWND)0,"IsBadHugePtr: really huge object!",
	"Unimplemented feature",MB_OK|MB_ICONSTOP);
    return FALSE;
}

BOOL
IsBadPtr ( const void *lp, UINT cb)
{
#ifdef	LATER
	/* add real logic with DPMI for different flavors of IsBad* */
#endif
	WORD wSel;
	LPBYTE lpData;
	
	if (cb == 0)
	    return FALSE;

	wSel = HIWORD(lp);
	lpData = GetPhysicalAddress(wSel);
	if (lpData == NULL || lpData == (LPBYTE)(-1))
	    return TRUE;
	if (GetSelectorLimit(wSel) < cb)
	    return TRUE;
	return FALSE;
}

void
IT_GLOBALENTRYHANDLE (ENV *envp,LONGPROC f)
{
    DWORD retcode,dwSize;
    UINT uSel,uIndex;
    LPBYTE lpData;
    GLOBALENTRY ge;
    LPMODULEINFO lpModInfo;
    extern MODULEINFO *lpModuleTable[];

    lpData = (LPBYTE)GetAddress(GETWORD(SP+8), GETWORD(SP+6));
    dwSize = GETDWORD(lpData);
    if (dwSize != (3*DWORD_86 + HANDLE_86 + 3*WORD_86 + INT_86 +
		HANDLE_86 + 2*WORD_86 + 2*DWORD_86))
	retcode = 0;
    else {
	ge.dwSize = sizeof(GLOBALENTRY);
	uSel = (UINT)GETWORD(SP+4);
	if ((retcode = (DWORD)GlobalEntryHandle(&ge,GetSelectorHandle(uSel)))) {
	    PUTDWORD(lpData+4,ge.dwAddress);
	    PUTDWORD(lpData+8,ge.dwBlockSize);
	    PUTWORD(lpData+12,ge.hBlock);
	    PUTWORD(lpData+14,ge.wcLock);
	    PUTWORD(lpData+16,ge.wcPageLock);
	    PUTWORD(lpData+18,ge.wFlags);
	    PUTWORD(lpData+20,ge.wHeapPresent);
#ifdef	LATER
	    PUTWORD(lpData+22,ge.hOwner);
#else
	    /* this is a kludge to run DELPHI */
	    if (!(uIndex = (UINT)GetModuleIndex(uSel)))
		/* this is an index for KERNEL (maybe!) */
		uIndex = 1;
	    lpModInfo = lpModuleTable[uIndex];
	    PUTWORD(lpData+22,lpModInfo->ObjHead.hObj);
#endif
	    PUTWORD(lpData+24,ge.wType);
	    PUTWORD(lpData+26,ge.wData);
	    PUTDWORD(lpData+28,ge.dwNext);
	    PUTDWORD(lpData+32,ge.dwNextAlt);
	}
    }

    envp->reg.sp += LP_86 + HANDLE_86 + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}

static void
ZapSegRegs(ENV *envp, WORD wSel)
{
    ENV *tmp_env = envp;

    do {
	if (wSel == (LOWORD(tmp_env->reg.es) | 1))
		tmp_env->reg.es = 0;
	if (wSel == (LOWORD(tmp_env->reg.gs) | 1))
		tmp_env->reg.gs = 0;
	if (wSel == (LOWORD(tmp_env->reg.fs) | 1))
		tmp_env->reg.fs = 0;
    } while ((tmp_env = (ENV *)tmp_env->prev_env) != 0);
}
