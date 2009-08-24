/*    
	Hook.c	2.5
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
#include "Log.h"
#include "Hook.h"
#include "kerndef.h"

HOOKSTRUCT *lpHookList[12];

#ifdef STRICT
HHOOK WINAPI
#else
HOOKPROC WINAPI
#endif
SetWindowsHook(int idHook, HOOKPROC hkprc)
{
    APISTR((LF_API,"SetWindowsHook(id:%x,proc=%x)\n",idHook,hkprc));

    return (HOOKPROC)SetWindowsHookEx(idHook,hkprc,(HINSTANCE)0,
			GetCurrentTask());
}

HHOOK WINAPI
SetWindowsHookEx(int idHook, HOOKPROC hkprc, HINSTANCE hInstance,
		HTASK hTask)
{
    LPHOOKSTRUCT lpHookStruct;

    APISTR((LF_API,
	"SetWindowsHookEx: id %x proc %x inst %x task %x\n",
	idHook, hkprc, hInstance, hTask));

    if (!hkprc)
	return (HHOOK)0;

    lpHookStruct = (LPHOOKSTRUCT)WinMalloc(sizeof(HOOKSTRUCT));
    lpHookStruct->lpfnHookProc = hkprc;
    lpHookStruct->idHook = idHook;
    lpHookStruct->hInst = hInstance;
    lpHookStruct->hTask = hTask;
    lpHookStruct->lpHookNext = lpHookList[idHook+1];
    lpHookList[idHook+1] = lpHookStruct;
    return (HHOOK)lpHookStruct;
}

BOOL
UnhookWindowsHook(int idHook, HOOKPROC hkprc)
{
    LPHOOKSTRUCT lpHookStruct;

    APISTR((LF_API,"UnhookWindowsHook(idHook:%x,proc:%x)\n",
				idHook, hkprc));
    for (lpHookStruct = lpHookList[idHook+1]; lpHookStruct;
		lpHookStruct = lpHookStruct->lpHookNext)
	if (lpHookStruct->lpfnHookProc == hkprc)
	    break;
    if (lpHookStruct == NULL)
	return FALSE;

    return UnhookWindowsHookEx((HHOOK)lpHookStruct);
}

BOOL WINAPI
UnhookWindowsHookEx(HHOOK hHook)
{
    LPHOOKSTRUCT lpHookStruct;
    int idHook;

    APISTR((LF_API,"UnhookWindowsHookEx: hHook %x\n",hHook));

    idHook = ((LPHOOKSTRUCT)hHook)->idHook;
    for(lpHookStruct = lpHookList[idHook+1];lpHookStruct; 
                lpHookStruct = lpHookStruct->lpHookNext)
	if (lpHookStruct->lpHookNext == (LPHOOKSTRUCT)hHook)
	    break;
    if (lpHookStruct == NULL) {
	if (lpHookList[idHook+1] == (LPHOOKSTRUCT)hHook) 
	    lpHookList[idHook+1] = NULL;
	else
	    return FALSE;
    }
    else 
	lpHookStruct->lpHookNext = ((LPHOOKSTRUCT)hHook)->lpHookNext;
    WinFree((LPSTR)hHook);
    return TRUE;
}

LRESULT WINAPI
#ifdef STRICT
DefHookProc(int nCode, WPARAM wParam, LPARAM lParam, HHOOK *lphhook)
#else
DefHookProc(int nCode, WPARAM wParam, LPARAM lParam, HOOKPROC *lphhook)
#endif
{
    APISTR((LF_API,"DefHookProc: hHook %x\n",*lphhook));
    return (DWORD)CallNextHookEx((HHOOK)(*(HHOOK *)lphhook),
			nCode,wParam,lParam);

}

LRESULT WINAPI
CallNextHookEx(HHOOK hHook, int nCode, WPARAM wParam, LPARAM lParam)
{
    LPHOOKSTRUCT lpHookStruct;
    LRESULT      rc;

    APISTR((LF_APICALL,"CallNextHookEx: hHook %x\n",hHook));

    lpHookStruct = (LPHOOKSTRUCT)hHook;
    if (!lpHookStruct)
	return (LRESULT)NULL;

    while (lpHookStruct->lpHookNext) {
	if (lpHookStruct->lpHookNext->hTask == GetCurrentTask()) {
	    rc = lpHookStruct->lpHookNext->lpfnHookProc(nCode,wParam,lParam); 
    	    APISTR((LF_APIRET,"CallNextHookEx: returns LRESULT %x\n",rc));
	    return rc;
	}
	lpHookStruct = lpHookStruct->lpHookNext;
    }
    APISTR((LF_APIRET,"CallNextHookEx: returns LRESULT %x\n",NULL));
    return (LRESULT)NULL;
}

BOOL WINAPI
CallMsgFilter(LPMSG lpmsg, int nCode)
{
    LPHOOKSTRUCT lpMsgFilter,lpSysMsgFilter;
    LRESULT rc = 0;
    
    APISTR((LF_API,"CallMsgFilter(hwnd=%x,msg=%x,code:%x)\n",
	lpmsg->hwnd,lpmsg->message,nCode));

    if ((lpSysMsgFilter = lpHookList[WH_SYSMSGFILTER+1])) {
	if ((rc = lpSysMsgFilter->lpfnHookProc(nCode,0,(LPARAM)lpmsg)))
	    return TRUE;
    }
    if ((lpMsgFilter = lpHookList[WH_MSGFILTER+1])) {
	if (lpMsgFilter->hTask == GetWindowTask(lpmsg->hwnd))
	    rc = lpMsgFilter->lpfnHookProc(nCode,0,(LPARAM)lpmsg);
	return (rc)?TRUE:FALSE;
    }
    return FALSE;
}
