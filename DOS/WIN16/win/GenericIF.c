/*    
	GenericIF.c	2.15
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

#include "windows.h"

#include "kerndef.h"
#include "Endian.h"
#include "Log.h"
#include "BinTypes.h"
#include "Resources.h"
#include "Module.h"
#include "DPMI.h"

extern MODULEINFO *lpModuleTable[];
extern void debuggerbreak(void);

int
Trap (ENV *envp,LONGPROC f)
{
	unsigned int module,ordinal;
	char *name;
    	char buf[0x40];
	BINADDR binaddr;
	MODULEINFO *modinfo;
	int rc;

	binaddr = envp->trans_addr;
	ordinal = (unsigned int)LOWORD(binaddr)>>3;
	module = (unsigned int)GetModuleIndex(HIWORD(binaddr));
	modinfo = lpModuleTable[module];
	if (modinfo && modinfo->lpEntryTable) {
	    name = modinfo->lpEntryTable[ordinal].name;
	    sprintf(buf,"TRAP: mod %s, ord %x\nAPI:  %s",
			modinfo->lpModuleName,ordinal,name);
	}
	else {
	    name = "???";
	    if (modinfo)
		sprintf(buf,"TRAP: mod %s, ord %x\nAPI:  %s",
			modinfo->lpModuleName,ordinal,name);
	    else
		sprintf(buf,"TRAP: mod %x, ord %x\nAPI:  %s",
			module,ordinal,name);
	}

	rc = MessageBox((HWND)NULL,buf,
		"Unimplemented API",
		MB_ABORTRETRYIGNORE|MB_ICONEXCLAMATION);

	switch(rc) {
		case IDABORT:
#if !defined(TWIN_INTERP_NOT_SUPPORTED)
			debuggerbreak();
#endif
			break;
			
		case IDIGNORE:
			break;

		case IDRETRY:
			FatalExit(0);
			break;
	}
	return rc;
}

void
IT_V (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)();
	envp->reg.sp += RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1B (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode =  (f)((BYTE)(GETWORD(SP+4)));
	envp->reg.sp += WORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((UINT)(GETWORD(SP+4)));
	envp->reg.sp += UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	HANDLE h;

	h = (HANDLE)(GETWORD(SP+4));
	if (!(h & OM_MASK))
		h = GetSelectorHandle(h);

	retcode = (f)(h);
	envp->reg.sp += HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((int)((short)GETWORD(SP+4)));
	envp->reg.sp += INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1D (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)(GETDWORD(SP+4));
	envp->reg.sp += DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}
void
IT_1LPSTR (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;
	WORD	sel,off;
	
	sel = GETWORD(SP+6);
	off = GETWORD(SP+4);

	lpString = (LPSTR)GetAddress(sel,off);
	if(HIWORD(lpString) == 0) {
		lpString = (LPSTR)GetAddress(envp->reg.ds,off);
	}

	if(HIWORD(lpString))
		retcode = (f)(lpString);
	else	{
		retcode = 0;
	}

	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}


void
IT_1LP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress
		(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)(lpString);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LPP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	POINT pt;

	lpStruct = (LPBYTE)GetAddress
		(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)(&pt);
	PutPOINT(lpStruct,pt);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LPR (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;

	lpStruct = (LPBYTE)GetAddress
		(GETWORD(SP+6),GETWORD(SP+4));
	RECT_TO_C(rcRect,lpStruct);
	retcode = (f)(&rcRect);
	RECT_TO_86(lpStruct,rcRect);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LPRc (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;

	lpStruct = (LPBYTE)GetAddress
		(GETWORD(SP+6),GETWORD(SP+4));
	if (lpStruct) {
		RECT_TO_C(rcRect,lpStruct);
		retcode = (f)(&rcRect);
	}
	else
		retcode = (f)((LPRECT)NULL);
	envp->reg.sp += LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}


void
IT_2H (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+6),(HANDLE)GETWORD(SP+4));
	envp->reg.sp += 2*HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((int)((short)GETWORD(SP+6)),
		(int)((short)GETWORD(SP+4)));
	envp->reg.sp += 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1W (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+6),
		GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+6),
		(UINT)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1UI1H (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	retcode = (f)((UINT)GETWORD(SP+6),
		(HANDLE)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+6),
		(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((UINT)GETWORD(SP+6),
		(UINT)GETWORD(SP+4));
	envp->reg.sp += 2*UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1D1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)(GETDWORD(SP+6),
		(UINT)GETWORD(SP+4));
	envp->reg.sp += UINT_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1UI1D (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((UINT)GETWORD(SP+8),
		GETDWORD(SP+4));
	envp->reg.sp += UINT_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2D (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)(GETDWORD(SP+8),
		GETDWORD(SP+4));
	envp->reg.sp +=  2*DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1I1D (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((int)((short)GETWORD(SP+8)),
		GETDWORD(SP+4));
	envp->reg.sp += INT_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1D (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+8),
		GETDWORD(SP+4));
	envp->reg.sp += HANDLE_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2LP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)(lpString1, lpString2);
	envp->reg.sp += 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LP1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)(lpString,(UINT)GETWORD(SP+4));
	envp->reg.sp += UINT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LP1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)(lpString,(int)((short)GETWORD(SP+4)));
	envp->reg.sp += INT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1UI1LP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)((UINT)GETWORD(SP+8),lpString);
	envp->reg.sp += UINT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;
	HANDLE h;

	h = (HANDLE)(GETWORD(SP+8));
	if (!(h & OM_MASK))
		h = GetSelectorHandle(h);

	lpString = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)(h,lpString);
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LP1H (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;
	HANDLE h;

	h = (HANDLE)(GETWORD(SP+4));
	if (!(h & OM_MASK))
		h = GetSelectorHandle(h);

	lpString = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)(lpString,(HANDLE)h);
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1D1LP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)(GETDWORD(SP+8),lpString);
	envp->reg.sp += DWORD_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	POINT pt;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	if (lpStruct) {
		GetPOINT(pt,lpStruct);
		retcode = (f)((HANDLE)GETWORD(SP+8),&pt);
		PutPOINT(lpStruct,pt);
	}
	else
		retcode = (f)((HANDLE)GETWORD(SP+8),(LPPOINT)NULL);
	envp->reg.sp += LP_86 + HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPSZ (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	SIZE sz;
	LPBYTE lpStruct;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	if (lpStruct) {
		retcode = (f)((HANDLE)GETWORD(SP+8),&sz);
		PUTWORD(lpStruct,(WORD)sz.cx);
		PUTWORD(lpStruct+2,(WORD)sz.cy);
	}
	else
		retcode = (f)((HANDLE)GETWORD(SP+8),(SIZE *)NULL);
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPR (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	RECT rcRect;
	LPBYTE lpStruct;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	if (lpStruct) {
		RECT_TO_C(rcRect,lpStruct);
		retcode = (f)((HANDLE)GETWORD(SP+8),&rcRect);
		RECT_TO_86(lpStruct,rcRect);
	}
	else
		retcode = (f)((HANDLE)GETWORD(SP+8),(LPRECT)NULL);
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPRc (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	if (lpStruct) {
	    RECT_TO_C(rcRect,lpStruct);
		retcode = (f)((HANDLE)GETWORD(SP+8),&rcRect);
	}
	else
		retcode = (f)((HANDLE)GETWORD(SP+8),(LPRECT)NULL);
	envp->reg.sp += HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2LPRc (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect1,rcRect2;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	RECT_TO_C(rcRect2,lpStruct);
	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	RECT_TO_C(rcRect1,lpStruct);
	retcode = (f)(&rcRect1,&rcRect2);
	envp->reg.sp += 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LPR1LPRc (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct1,lpStruct2;
	RECT rcRect1,rcRect2;

	lpStruct1 = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	lpStruct2 = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	if (lpStruct1 && lpStruct2) {
		RECT_TO_C(rcRect2,lpStruct1);
		retcode = (f)(&rcRect1,&rcRect2);
		RECT_TO_86(lpStruct2,rcRect1);
	} else
		retcode = 0;
	envp->reg.sp += 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_3H (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+8),
			(HANDLE)GETWORD(SP+6),
			(HANDLE)GETWORD(SP+4));
	envp->reg.sp += 3*HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_3UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((UINT)GETWORD(SP+8),
			(UINT)GETWORD(SP+6),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += 3*UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_3I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += 3*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H2UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+8),
			(UINT)GETWORD(SP+6),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + 2*UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H2I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+8),
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2H1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+8),
			(HANDLE)GETWORD(SP+6),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += 2*HANDLE_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2H1D (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+10),
			(HANDLE)GETWORD(SP+8),
			GETDWORD(SP+4));
	envp->reg.sp += 2*HANDLE_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1I1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+8),
			(int)((short)GETWORD(SP+6)),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + INT_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1I1W (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+8),
			(int)((short)GETWORD(SP+6)),
			(WORD)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + INT_86 + WORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1D1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+10),
			GETDWORD(SP+6),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + UINT_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1L1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+10),
			GETDWORD(SP+6),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + INT_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2I1D (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),		
			GETDWORD(SP+4));
	envp->reg.sp += 2*INT_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H2L (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+12),
			GETDWORD(SP+8),
			GETDWORD(SP+4));
	envp->reg.sp += HANDLE_86 + 2*DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_3D (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)(GETDWORD(SP+12),
		GETDWORD(SP+8),
		GETDWORD(SP+4));
	envp->reg.sp += 3*DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2D1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)(GETDWORD(SP+10),
		GETDWORD(SP+6),
		(UINT)GETWORD(SP+4));
	envp->reg.sp += UINT_86 + 2*DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_3LP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2, lpString3;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	lpString3 = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)(lpString1, lpString2, lpString3);
	envp->reg.sp += 3*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1UI1LP1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)((UINT)GETWORD(SP+10),
			lpString,
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += UINT_86 + LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LP1I1W (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	retcode = (f)(lpString,(int)((short)GETWORD(SP+6)),GETWORD(SP+4));
	envp->reg.sp += WORD_86 + INT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LP1H (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)((HANDLE)GETWORD(SP+10),
			lpString,
			(HANDLE)GETWORD(SP+4));
	envp->reg.sp += 2*HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LP1W (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)((HANDLE)GETWORD(SP+10),
			lpString,
			(WORD)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + LP_86 + WORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LP1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)((HANDLE)GETWORD(SP+10),
			lpString,
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1D1LP1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)(GETDWORD(SP+10),
			lpString,
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += DWORD_86 + LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1I1LP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)((HANDLE)GETWORD(SP+10),
			(int)((short)GETWORD(SP+8)),
			lpString);
	envp->reg.sp += HANDLE_86 + INT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2LP1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)(lpString1, 
			lpString2,
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += 2*LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2LP1L (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	retcode = (f)(lpString1, 
			lpString2,
			GETDWORD(SP+4));
	envp->reg.sp += 2*LP_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_3L1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)(GETDWORD(SP+14),
		GETDWORD(SP+10),
		GETDWORD(SP+6),
		(UINT)GETWORD(SP+4));
	envp->reg.sp += UINT_86 + 3*DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1B1LP1UI1LP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)((BYTE)GETWORD(SP+14),
			lpString1,
			(UINT)GETWORD(SP+8),
			lpString2);
	envp->reg.sp += 2*LP_86 + WORD_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2LP1I1LP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2,lpString3;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+16),GETWORD(SP+14));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	lpString3 = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)(lpString1, 
			lpString2,
			GETWORD(SP+8),
			lpString3);
	envp->reg.sp += 3*LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1L1LPB (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)((HANDLE)GETWORD(SP+12),
			(LONG)GETDWORD(SP+8),
			lpString);
	envp->reg.sp += HANDLE_86 + LONG_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPR1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	if (lpStruct) {
		retcode = (f)((HANDLE)GETWORD(SP+10),
			&rcRect,
			(int)((short)GETWORD(SP+4)));
		RECT_TO_86(lpStruct,rcRect);
	}
	else
		retcode = (f)((HANDLE)GETWORD(SP+10),
			(DWORD)NULL,
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPR1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	if (lpStruct) {
		retcode = (f)((HANDLE)GETWORD(SP+10),
			&rcRect,
			(int)((short)GETWORD(SP+4)));
		RECT_TO_86(lpStruct,rcRect);
	}
	else
		retcode = (f)((HANDLE)GETWORD(SP+10),
			(DWORD)NULL,
			(UINT)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + LP_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPRc1H (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	RECT_TO_C(rcRect,lpStruct);
	retcode = (f)((HANDLE)GETWORD(SP+10),
			&rcRect,
			(HANDLE)GETWORD(SP+4));
	envp->reg.sp += 2*HANDLE_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPRc1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;
	LPRECT lpRect;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	if (lpStruct) {
	    RECT_TO_C(rcRect,lpStruct);
		lpRect = &rcRect;
	}
	else lpRect = (RECT *)NULL;
	retcode = (f)((HANDLE)GETWORD(SP+10),
			(DWORD)lpRect,
			(int)(short)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPRc1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;
	RECT *lpRect;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	if (lpStruct) {
	    RECT_TO_C(rcRect,lpStruct);
		lpRect = &rcRect;
	}
	else lpRect = (RECT *)NULL;
	retcode = (f)((HANDLE)GETWORD(SP+10),
			(DWORD)lpRect,
			(UINT)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + LP_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LPR2LPRc (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect1,rcRect2,rcRect3;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	RECT_TO_C(rcRect3,lpStruct);
	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	RECT_TO_C(rcRect2,lpStruct);
	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	retcode = (f)(&rcRect1,
			&rcRect2,
			&rcRect3);
	RECT_TO_86(lpStruct,rcRect1);
	envp->reg.sp += 3*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LPR1D1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	RECT_TO_C(rcRect,lpStruct);
	retcode = (f)(&rcRect,
			GETDWORD(SP+6),
			(int)((short)GETWORD(SP+4)));
	RECT_TO_86(lpStruct,rcRect);
	envp->reg.sp += LP_86 + DWORD_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LPR2I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	RECT_TO_C(rcRect,lpStruct);
	retcode = (f)(&rcRect,
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	RECT_TO_86(lpStruct,rcRect);
	envp->reg.sp += LP_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_4I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += 4*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H3I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+10),
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + 3*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2H2UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+10),
			(HANDLE)GETWORD(SP+8),
			(UINT)GETWORD(SP+6),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += 2*HANDLE_86 + 2*UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2H2I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+10),
			(HANDLE)GETWORD(SP+8),
			(int)((short)GETWORD(SP+6)), 
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += 2*HANDLE_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_3H1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+10),
			(HANDLE)GETWORD(SP+8),
			(HANDLE)GETWORD(SP+6),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += 3*HANDLE_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H2I1H (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+10),
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(HANDLE)GETWORD(SP+4));
	envp->reg.sp += 2*HANDLE_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1I1UI1I (ENV *envp,LONGPROC f)		/* SetDlgItemInt() */
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+10),
			(int)((short)GETWORD(SP+8)),
			(UINT)GETWORD(SP+6),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + 2*INT_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H2I1D (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+12),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			GETDWORD(SP+4));
	envp->reg.sp += HANDLE_86 + 2*INT_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1I2LPI (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	int int1,int2;
	LPBYTE lpStr1,lpStr2;

	lpStr1 = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	lpStr2 = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)((HANDLE)GETWORD(SP+14),
			(int)((short)GETWORD(SP+12)),
			&int1,&int2);
	PUTWORD(lpStr1,int1);
	PUTWORD(lpStr2,int2);
	envp->reg.sp += HANDLE_86 + INT_86 + 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1UI2L (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+14),
			(UINT)GETWORD(SP+12),
			GETDWORD(SP+8),
			GETDWORD(SP+4));
	envp->reg.sp += HANDLE_86 + UINT_86 + 2*LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1I1LP1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)((HANDLE)GETWORD(SP+12),
			(int)((short)GETWORD(SP+10)),
			(DWORD)lpString, 
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + LP_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1I1LPI1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	BOOL bFlag;
	LPBYTE lpBool;

	lpBool = (LPBYTE)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)((HANDLE)GETWORD(SP+12),
			(int)((short)GETWORD(SP+10)),
			&bFlag,
			(int)((short)GETWORD(SP+4)));
	if (lpBool)
	    PUTWORD(lpBool,(WORD)bFlag);
	envp->reg.sp += HANDLE_86 + LP_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LP2I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	retcode = (f)((HANDLE)GETWORD(SP+12),
			(DWORD)lpString, 
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + LP_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LP2I1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	retcode = (f)((HANDLE)GETWORD(SP+14),
			(DWORD)lpString, 
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + LP_86 + 2*INT_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1UI1LP1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;
	HANDLE h;

	h = (HANDLE)(GETWORD(SP+12));
	if (!(h & OM_MASK))
		h = GetSelectorHandle(h);

	lpString = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)(h,
			(UINT)GETWORD(SP+10),
			(DWORD)lpString, 
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + UINT_86 + LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LP1UI1D (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	retcode = (f)((HANDLE)GETWORD(SP+14),
			lpString, 
			(UINT)GETWORD(SP+8),
			GETDWORD(SP+4));
	envp->reg.sp += HANDLE_86 + LP_86 + UINT_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H2LP1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)((HANDLE)GETWORD(SP+14),
			lpString1, 
			lpString2,
			(UINT)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + 2*LP_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2LP1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)(lpString1, 
			lpString2,
			(UINT)GETWORD(SP+4));
	envp->reg.sp += 2*LP_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2LP1UI1H (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	retcode = (f)(lpString1, 
			lpString2,
			(UINT)GETWORD(SP+6),
			(HANDLE)GETWORD(SP+4));
	envp->reg.sp += 2*LP_86 + UINT_86 + HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2LP1H (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)(lpString1, 
			lpString2,
			(HANDLE)GETWORD(SP+4));
	envp->reg.sp += 2*LP_86 + HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H2LP1H (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+8),GETWORD(SP+6));
	retcode = (f)((HANDLE)GETWORD(SP+14),
			lpString1,
			lpString2,
			(HANDLE)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + 2*LP_86 + HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H2LP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;
	HANDLE h;

	h = (HANDLE)(GETWORD(SP+12));
	if (!(h & OM_MASK))
		h = GetSelectorHandle(h);

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)(h, lpString1, lpString2);
	envp->reg.sp += HANDLE_86 + 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1UI3LP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2,lpString3;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	lpString3 = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)((UINT)GETWORD(SP+16),
			lpString1, 
			lpString2,
			lpString3);
	envp->reg.sp += UINT_86 + 3*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_4LP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2,lpString3,lpString4;

	lpString1 = (LPSTR)GetAddress(GETWORD(SP+18),GETWORD(SP+16));
	lpString2 = (LPSTR)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	lpString3 = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	lpString4 = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)(lpString1, 
			lpString2,
			lpString3,
			lpString4);
	envp->reg.sp += 4*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H2I1LPP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	POINT pt;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	if (lpStruct) {
		retcode = (f)((HANDLE)GETWORD(SP+12),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			&pt);
		PutPOINT(lpStruct,pt);
	}
	else
		retcode = (f)((HANDLE)GETWORD(SP+12),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			(DWORD)NULL);
	envp->reg.sp += HANDLE_86 + LP_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LP1I1LPSZ (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	LPSTR lpString;
	POINT pt;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	lpString = (LPSTR)GetAddress(GETWORD(SP+12),GETWORD(SP+10));
	retcode = (f)((HANDLE)GETWORD(SP+14),
			lpString,
			(int)((short)GETWORD(SP+8)),
			&pt);
	PutPOINT(lpStruct,pt);
	envp->reg.sp += HANDLE_86 + 2*LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPRc1H1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;
	RECT *lpRect;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	if (lpStruct) {
		RECT_TO_C(rcRect,lpStruct);
		lpRect = &rcRect;
	}
	else 
		lpRect = (RECT *)NULL;
	retcode = (f)((HANDLE)GETWORD(SP+12),
			(DWORD)lpRect,
			(HANDLE)GETWORD(SP+6),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += 2*HANDLE_86 + LP_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LPR1D1I1D (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+16),GETWORD(SP+14));
	RECT_TO_C(rcRect,lpStruct);
	retcode = (f)(&rcRect,
			GETDWORD(SP+10),
			(BOOL)GETWORD(SP+8),
			GETDWORD(SP+4));
	RECT_TO_86(lpStruct,rcRect);
	envp->reg.sp += LP_86 + 2*DWORD_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_5I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += 5*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H4I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+12),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + 4*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_3H2I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+12),
			(HANDLE)GETWORD(SP+10),
			(HANDLE)GETWORD(SP+8),
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += 3*HANDLE_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H2UI2H (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+12),
			(UINT)GETWORD(SP+10),
			(UINT)GETWORD(SP+8),
			(HANDLE)GETWORD(SP+6),
			(HANDLE)GETWORD(SP+4));
	envp->reg.sp += 3*HANDLE_86 + 2*UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H2I1D1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+14),
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			GETDWORD(SP+6),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + 2*INT_86 + DWORD_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2H1UI1W1L (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+14),
			(HANDLE)GETWORD(SP+12),
			(UINT)GETWORD(SP+10),
			(WORD)GETWORD(SP+8),
			GETDWORD(SP+4));
	envp->reg.sp += 2*HANDLE_86 + UINT_86 + WORD_86 + LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1D1H1UI2L (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)(GETDWORD(SP+16),
			(HANDLE)GETWORD(SP+14),
			(UINT)GETWORD(SP+12),
			GETDWORD(SP+8),
			GETDWORD(SP+4));
	envp->reg.sp += DWORD_86 + HANDLE_86 + UINT_86 + 2*LONG_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2I2UI1LPB (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)((int)((short)GETWORD(SP+14)),
			(int)((short)GETWORD(SP+12)),
			(UINT)GETWORD(SP+10),
			(UINT)GETWORD(SP+8),
			lpString);
	envp->reg.sp += 2*INT_86 + 2*UINT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1UI1LP2UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString;

	lpString = (LPSTR)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	retcode = (f)((HANDLE)GETWORD(SP+14),
			(UINT)GETWORD(SP+12),
			lpString,
			(UINT)GETWORD(SP+6),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += HANDLE_86 + 3*UINT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1LPR4I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	retcode = (f)(&rcRect,
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	RECT_TO_86(lpStruct,rcRect);
	envp->reg.sp += LP_86 + 4*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H2I2LPRc (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect1,rcRect2;
	RECT *lpRect1,*lpRect2;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
	if (lpStruct) {
		RECT_TO_C(rcRect2,lpStruct);
		lpRect2 = &rcRect2;
	}
	else
		lpRect2 = (RECT *)NULL;
	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+10),GETWORD(SP+8));
	if (lpStruct) {
		RECT_TO_C(rcRect1,lpStruct);
		lpRect1 = &rcRect1;
	}
	else
		lpRect1 = (RECT *)NULL;
	retcode = (f)((HANDLE)GETWORD(SP+16),
			(int)((short)GETWORD(SP+14)),
			(int)((short)GETWORD(SP+12)),
			lpRect1,
			lpRect2);
	envp->reg.sp += HANDLE_86 + 2*LP_86 + 2*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1LPRc2I1D (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;
	LPRECT lpRect;

	lpStruct = (LPBYTE)GetAddress(GETWORD(SP+14),GETWORD(SP+12));
	if (lpStruct) {
	    RECT_TO_C(rcRect,lpStruct);
		lpRect = &rcRect;
	}
	else lpRect = (RECT *)NULL;
	retcode = (f)((HANDLE)GETWORD(SP+16),
			(DWORD)lpRect,
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			GETDWORD(SP+4));
	envp->reg.sp += HANDLE_86 + LP_86 + 2*INT_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_6I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((int)((short)GETWORD(SP+14)),
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += 6*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H5I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+14),
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + 5*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H4I1D (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+16),
			(int)((short)GETWORD(SP+14)),
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			GETDWORD(SP+4));
	envp->reg.sp += HANDLE_86 + 4*INT_86 + DWORD_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_4LP1I (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2,lpString3,lpString4;

  	lpString1 = (LPSTR)GetAddress
            		(GETWORD(SP+20),GETWORD(SP+18));
   	lpString2 = (LPSTR)GetAddress
            		(GETWORD(SP+16),GETWORD(SP+14));
   	lpString3 = (LPSTR)GetAddress
            		(GETWORD(SP+12),GETWORD(SP+10));
   	lpString4 = (LPSTR)GetAddress
            		(GETWORD(SP+8),GETWORD(SP+6));
   	retcode = (f)(lpString1, 
           	lpString2,
           	lpString3,
           	lpString4,
           	(int)((short)GETWORD(SP+4)));
   	envp->reg.sp += 4*LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_4LP1I1LP (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2,lpString3,lpString4,lpString5;

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
  	retcode = (f)(lpString1, 
                 	lpString2,
                 	lpString3,
                 	lpString4,
                 	(int)((short)GETWORD(SP+8)),
                 	lpString5);
  	envp->reg.sp += 5*LP_86 + INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H2I1LPSZ (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	POINT pt;
	LPBYTE lpStruct;

	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+6),GETWORD(SP+4));
	if (lpStruct) {
		retcode = (f)((HANDLE)GETWORD(SP+12),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			&pt);
		PutPOINT(lpStruct,pt);
	}
	else 
		retcode = (f)((HANDLE)GETWORD(SP+12),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			(DWORD)NULL);
	envp->reg.sp += HANDLE_86 + 2*INT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H4I1LPSZ (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	POINT pt;
	LPBYTE lpStruct;

	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+6),GETWORD(SP+4));
	if (lpStruct) {
		retcode = (f)((HANDLE)GETWORD(SP+16),
			(int)((short)GETWORD(SP+14)),
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			&pt);
		PutPOINT(lpStruct,pt);
	}
	else 
		retcode = (f)((HANDLE)GETWORD(SP+16),
			(int)((short)GETWORD(SP+14)),
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			(DWORD)NULL);
	envp->reg.sp += HANDLE_86 + 4*INT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H6I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+16),
			(int)((short)GETWORD(SP+14)),
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + 6*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_2H4I1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+16),
			(HANDLE)GETWORD(SP+14),
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += 2*HANDLE_86 + 4*INT_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H2I2B2LPB (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;
	HANDLE h;

	h = (HANDLE)(GETWORD(SP+20));
	if (!(h & OM_MASK))
		h = GetSelectorHandle(h);

	lpString1 = (LPSTR)GetAddress
			(GETWORD(SP+10),GETWORD(SP+8));
	lpString2 = (LPSTR)GetAddress
			(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)(h,
			(int)((short)GETWORD(SP+18)),
			(int)((short)GETWORD(SP+16)),
			(BYTE)GETWORD(SP+14),
			(BYTE)GETWORD(SP+12),
			lpString1, 
			lpString2); 
	envp->reg.sp += HANDLE_86 + 2*INT_86 + 2*WORD_86 + 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H4I2LPB (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPSTR lpString1,lpString2;
	HANDLE h;

	h = (HANDLE)(GETWORD(SP+20));
	if (!(h & OM_MASK))
		h = GetSelectorHandle(h);

	lpString1 = (LPSTR)GetAddress
			(GETWORD(SP+10),GETWORD(SP+8));
	lpString2 = (LPSTR)GetAddress
			(GETWORD(SP+6),GETWORD(SP+4));
	retcode = (f)(h,
			(int)((short)GETWORD(SP+18)),
			(int)((short)GETWORD(SP+16)),
			(int)((short)GETWORD(SP+14)),
			(int)((short)GETWORD(SP+12)),
			lpString1, 
			lpString2); 
	envp->reg.sp += HANDLE_86 + 4*INT_86 + 2*LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H1UI3I1H1LPRc (ENV *envp,LONGPROC f)
{
	DWORD retcode;
	LPBYTE lpStruct;
	RECT rcRect;
	RECT *lpRect = (RECT *)NULL;

	lpStruct = (LPBYTE)GetAddress
			(GETWORD(SP+6),GETWORD(SP+4));
	if (lpStruct) {
		RECT_TO_C(rcRect,lpStruct);
		lpRect = &rcRect;
	}
	retcode = (f)((HANDLE)GETWORD(SP+18),
			(UINT)GETWORD(SP+16),
			(int)((short)GETWORD(SP+14)),
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			(HANDLE)GETWORD(SP+8),
			lpRect);
	envp->reg.sp += 2*HANDLE_86 + UINT_86 + 3*INT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_3H4I1UI (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+18),
			(HANDLE)GETWORD(SP+16),
			(HANDLE)GETWORD(SP+14),
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(UINT)GETWORD(SP+4));
	envp->reg.sp += 3*HANDLE_86 + 4*INT_86 + UINT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_1H8I (ENV *envp,LONGPROC f)
{
	DWORD retcode;

	retcode = (f)((HANDLE)GETWORD(SP+20),
			(int)((short)GETWORD(SP+18)),
			(int)((short)GETWORD(SP+16)),
			(int)((short)GETWORD(SP+14)),
			(int)((short)GETWORD(SP+12)),
			(int)((short)GETWORD(SP+10)),
			(int)((short)GETWORD(SP+8)),
			(int)((short)GETWORD(SP+6)),
			(int)((short)GETWORD(SP+4)));
	envp->reg.sp += HANDLE_86 + 8*INT_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SHELLEXECUTE(ENV *envp,LONGPROC f)
{
	DWORD   retcode;
	const char *lpop;
        const char *lpfile;
        const char *lpparam;
        const char *lpdir;
	HWND    hWnd;
	int	fs;


	hWnd = (HANDLE)GETWORD(SP+22);
	lpop = (LPSTR)GetAddress
		(GETWORD(SP+20),GETWORD(SP+18));
	lpfile = (LPSTR)GetAddress
		(GETWORD(SP+16),GETWORD(SP+14));
	lpparam = (LPSTR)GetAddress
		(GETWORD(SP+12),GETWORD(SP+10));
	lpdir = (LPSTR)GetAddress
		(GETWORD(SP+8),GETWORD(SP+6));
	fs = (int)((short)GETWORD(SP+4));

	retcode = (f)(hWnd,lpop,lpfile,lpparam,lpdir,fs);

	envp->reg.sp += HANDLE_86 + 4*LP_86 + UINT_86 + RET_86;

	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

