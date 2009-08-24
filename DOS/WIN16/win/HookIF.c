/*    
	HookIF.c	2.12
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

#include "kerndef.h"
#include "Endian.h"
#include "Log.h"
#include "Kernel.h"
#include "DPMI.h"
#include "BinTypes.h"
#include "Hook.h"
#include "make_thunk.h"

#include <stdio.h>
#include <stdlib.h>

extern DSCR *LDT;

static LPARAM GetHookParam(int,int,LPARAM);

void
IT_SETWINDOWSHOOK (ENV *envp,LONGPROC f) 
{
	DWORD retcode;
	DWORD dwProc;
	HOOKPROC hookproc = 0;
	int idHook;

	dwProc = GETDWORD(SP+4);
	idHook = (int)((short)GETWORD(SP+8));
	switch (idHook) {
	    case WH_MSGFILTER:
	    case WH_SYSMSGFILTER:
		hookproc = (HOOKPROC)hsw_msgfilterproc;
		break;
	    case WH_KEYBOARD:
		hookproc = (HOOKPROC)hsw_keyboardproc;
		break;
	    case WH_MOUSE:
		hookproc = (HOOKPROC)hsw_mouseproc;
		break;
	    case WH_CALLWNDPROC:
		hookproc = (HOOKPROC)hsw_callwndproc;
		break;
	    case WH_CBT:
		hookproc = (HOOKPROC)hsw_cbtproc;
		break;
	    case WH_JOURNALRECORD:
	    case WH_JOURNALPLAYBACK:
	    case WH_GETMESSAGE:
	    case WH_HARDWARE:
	    case WH_DEBUG:
	    case WH_SHELL:
		hookproc = (HOOKPROC)hsw_hookproc;
		break;
	    default:
		break;
	}
	if (dwProc && hookproc)
	    dwProc = (DWORD)make_native_thunk(dwProc,(DWORD)hookproc);
	else
	    dwProc = 0L;
	retcode = (f)(idHook,dwProc);
	envp->reg.sp += INT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_SETWINDOWSHOOKEX (ENV *envp,LONGPROC f) 
{
	DWORD retcode;
	DWORD dwProc;
	int idHook;
	HOOKPROC hookproc=NULL;
	HTASK hTask;

	dwProc = GETDWORD(SP+8);
	idHook = (int)((short)GETWORD(SP+12));
	switch (idHook) {
	    case WH_MSGFILTER:
	    case WH_SYSMSGFILTER:
		hookproc = (HOOKPROC)hsw_msgfilterproc;
		break;
	    case WH_KEYBOARD:
		hookproc = (HOOKPROC)hsw_keyboardproc;
		break;
	    case WH_MOUSE:
		hookproc = (HOOKPROC)hsw_mouseproc;
		break;
	    case WH_CALLWNDPROC:
		hookproc = (HOOKPROC)hsw_callwndproc;
		break;
	    case WH_CBT:
		hookproc = (HOOKPROC)hsw_cbtproc;
		break;
	    case WH_JOURNALRECORD:
	    case WH_JOURNALPLAYBACK:
	    case WH_GETMESSAGE:
	    case WH_HARDWARE:
	    case WH_DEBUG:
	    case WH_SHELL:
		hookproc = (HOOKPROC)hsw_hookproc;
		break;
	    default:
		break;
	}
	if (dwProc && hookproc)
	    dwProc = (DWORD)make_native_thunk(dwProc,(DWORD)hookproc);
	else
	    dwProc = 0L;
	hTask = GetSelectorHandle(GETWORD(SP+4));
	retcode = SetWindowsHookEx(idHook,
			(HOOKPROC)dwProc,
			(HINSTANCE) GetSelectorHandle(GETWORD(SP+6)),
			hTask);
	envp->reg.sp += INT_86 + LP_86 + 2*HANDLE_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_UNHOOKWHOOK (ENV *envp,LONGPROC f) 
{
	DWORD retcode;
	DWORD dwProc;

	dwProc = GETDWORD(SP+4);
	dwProc = (dwProc)?(DWORD)make_native_thunk(dwProc,(DWORD)hsw_hookproc):0L;
	retcode = (f)((int)((short)GETWORD(SP+8)),dwProc);
	envp->reg.sp += INT_86 + LP_86 + RET_86;
	envp->reg.ax = LOWORD(retcode);
	envp->reg.dx = HIWORD(retcode);
}

void
IT_DEFHOOKPROC(ENV *envp,LONGPROC f)
{
    DWORD retcode;
    LPBYTE lpStruct;
    LPHOOKSTRUCT lpHookStruct;
    WPARAM wParam;
    LPARAM lParam;
    int idHook,code;

    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+6),GETWORD(SP+4));
    if (HIWORD(lpStruct))
	lpHookStruct = (LPHOOKSTRUCT)GETDWORD(lpStruct);
    if (HIWORD(lpHookStruct))
	idHook = lpHookStruct->idHook;	/* hook type */
    else
	idHook = -1;

    code = (int)((short)GETWORD(SP+14));
    wParam = (WPARAM)GETWORD(SP+12);
    lParam = GETDWORD(SP+8);
    lParam = GetHookParam(idHook,code,lParam);
    retcode = (f)(code,
			(UINT)wParam,
			lParam,
			(HHOOK *)&lpHookStruct);
    envp->reg.sp += INT_86 + UINT_86 + DWORD_86 + LP_86 + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}

void
IT_CALLNEXTHOOKEX(ENV *envp,LONGPROC f)
{
    DWORD retcode;
    LPHOOKSTRUCT lpHookStruct;
    int idHook=0, code;
    WPARAM wParam;
    LPARAM lParam;

    lpHookStruct = (LPHOOKSTRUCT)GETDWORD(SP+12);
    if (lpHookStruct)
	idHook = lpHookStruct->idHook;  /* hook type */
			
    code = (int)((short)GETWORD(SP+10));
    wParam = (WPARAM)GETWORD(SP+8);
    lParam = (LPARAM)GETDWORD(SP+4);
    if (lpHookStruct->lpHookNext)  /* otherwise no need to convert lParam */
	lParam = GetHookParam(idHook,code,lParam);
    retcode = (f)((HHOOK)lpHookStruct,
		code,
		wParam,
		lParam);
    envp->reg.sp += LONG_86 + INT_86 + WORD_86 + DWORD_86 + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}

static LPARAM
GetHookParam(int idHook, int code, LPARAM lParam)
{
    LPBYTE lpData,lpStruct;
    WORD w1,w2,w3,w4;
    static CREATESTRUCT cs;
    static CLIENTCREATESTRUCT ccs;
    static MSG msg;
    static MOUSEHOOKSTRUCT mhs;
    static CBT_CREATEWND cbtcs;

    switch (idHook) {

	case WH_MSGFILTER:	/* mouse and keyboard messages only */
	case WH_SYSMSGFILTER:
	case WH_GETMESSAGE:
	    lpData = (LPBYTE)GetAddress(HIWORD(lParam),LOWORD(lParam));
	    msg.hwnd = (HWND)GETWORD(lpData);
	    msg.message = (UINT)GETWORD(lpData+2);
	    msg.wParam = (WPARAM)GETWORD(lpData+4);
	    msg.lParam = (LPARAM)GETDWORD(lpData+6);
	    GetPOINT(msg.pt,lpData+10);
	    lParam = (LPARAM)&msg;
	    break;
	case WH_MOUSE:
	    lpData = (LPBYTE)GetAddress(HIWORD(lParam),LOWORD(lParam));
	    GetPOINT(mhs.pt,lpData);
	    mhs.hwnd = (HWND)GETWORD(lpData+4);
	    mhs.wHitTestCode = (UINT)GETWORD(lpData+6);
	    mhs.dwExtraInfo = GETDWORD(lpData+8);
	    lParam = (LPARAM)&mhs;
	    break;
	case WH_CBT:
	    lpData = (LPBYTE)GetAddress(HIWORD(lParam),LOWORD(lParam));
	    switch (code) {
		case HCBT_CREATEWND:
		    lpStruct = (LPBYTE)GetAddress(GETWORD(lpData+2),
				GETWORD(lpData));
		    cs.hInstance = (HANDLE) GetSelectorHandle(GETWORD(lpStruct+4));
		    cs.hMenu = (HANDLE)GETWORD(lpStruct+6);
		    cs.hwndParent = (HANDLE)GETWORD(lpStruct+8);
		    w1 = GETWORD(lpStruct+10);
		    w2 = GETWORD(lpStruct+12);
		    w3 = GETWORD(lpStruct+14);
		    w4 = GETWORD(lpStruct+16);
		    cs.cy = (w1 == CW_USEDEFAULT16)?
			(int)CW_USEDEFAULT:(int)((short)w1);
		    cs.cx = (w2 == CW_USEDEFAULT16)?
			(int)CW_USEDEFAULT:(int)((short)w2);
		    cs.y = (w3 == CW_USEDEFAULT16)?
			(int)CW_USEDEFAULT:(int)((short)w3);
		    cs.x = (w4 == CW_USEDEFAULT16)?
			(int)CW_USEDEFAULT:(int)((short)w4);
		    cs.style = GETDWORD(lpStruct+18);
		    cs.lpszName = (LPSTR)GetAddress(
				GETWORD(lpStruct+24),GETWORD(lpStruct+22));
		    cs.lpszClass = (LPSTR)GetAddress(
				GETWORD(lpStruct+28),GETWORD(lpStruct+26));
		    if ( HIWORD(cs.lpszClass) && 
                         (strcasecmp(cs.lpszClass,"MDICLIENT") == 0) ) {
			    lpStruct = (LPBYTE)GetAddress
				(GETWORD(lpStruct+2),GETWORD(lpStruct));
			    ccs.hWindowMenu = (HANDLE)GETWORD(lpStruct);
			    ccs.idFirstChild = (UINT)GETWORD(lpStruct+2);
			    cs.lpCreateParams = &ccs;
	    	    }
		    else
			cs.lpCreateParams = (LPBYTE)GETDWORD(lpStruct);
		    cbtcs.lpcs = &cs;
		    cbtcs.hwndInsertAfter = (HWND)GETWORD(lpData+4);
	    	    lParam = (LPARAM)(&cbtcs);
		    break; 
		default:
		    break;
	    }
	    break;
	case WH_JOURNALRECORD:
	case WH_JOURNALPLAYBACK:
	case WH_CALLWNDPROC:
	case WH_HARDWARE:
	case WH_DEBUG:
	case WH_SHELL:
	    break;

	case WH_KEYBOARD:
	default:
	    break;
    }
    return lParam;
}
