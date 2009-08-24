/*    
	EditMem.c	1.6
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
#include "Resources.h"
#include "Module.h"
#include "Edit.h"

HMODULE GetModuleFromInstance(HINSTANCE);

EDITMEMORYPROC lpfnEditMemBin = 0;

LRESULT
EditMemoryAPI(HWND hWnd, UINT uAction, WPARAM wParam, LPARAM lParam)
{
    LPEDIT lp;
    HINSTANCE hInst;
    HMODULE hModule;
    MODULEINFO *modinfo = NULL;
    DWORD dwSize;
    HANDLE hHandle;
    LRESULT res;

    if (!(lp = GetLPEdit(hWnd)))
	return (LRESULT)0;
    if (!lp->uSelDS) {
	hInst = (HINSTANCE)GetWindowInstance(hWnd);
	if (hInst) {
	    hModule = GetModuleFromInstance(hInst);
	    modinfo = (MODULEINFO *)GETMODULEINFO(hModule);
	    if (modinfo && modinfo->lpDLL)
		lp->uSelDS = (UINT)modinfo->wDGROUP;
	}
    }
    RELEASEMODULEINFO(modinfo);
    if (lp->uSelDS && lpfnEditMemBin)
        return lpfnEditMemBin(hWnd,uAction,wParam,lParam);

    dwSize = (DWORD)lParam;
    hHandle = (HANDLE)wParam;
    switch (uAction) {
	case EMA_ALLOC:
	    res = (LRESULT)GlobalAlloc(GMEM_MOVEABLE,dwSize);
	    break;
	case EMA_LOCK:
	    res = (LRESULT)GlobalLock(hHandle);
	    break;
	case EMA_UNLOCK:
	    res = (LRESULT)GlobalUnlock(hHandle);
	    break;
	case EMA_FREE:
	    res = (LRESULT)GlobalFree(hHandle);
	    break;
	case EMA_REALLOC:
	    res = (LRESULT)GlobalReAlloc(hHandle,dwSize,GMEM_MOVEABLE);
	    break;
	case EMA_SIZE:
	    res = (LRESULT)GlobalSize(hHandle);
	    break;
	default:
	    res = 0;
	    break;
    }
    return res;
}
