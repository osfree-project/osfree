/*    
	EditMemBin.c	1.3
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
#include "BinTypes.h"
#include "DPMI.h"
#include "Kernel.h"
#include "Resources.h"
#include "Module.h"
#include "Edit.h"

extern ENV *envp_global;
LRESULT EditMemoryBin(HWND,UINT,WPARAM,LPARAM);
extern EDITMEMORYPROC lpfnEditMemBin;

HLOCAL TWIN_LocalFree(HLOCAL);
HLOCAL TWIN_LocalAlloc(UINT, UINT);
char NEAR * TWIN_LocalLock(HLOCAL);
BOOL TWIN_LocalUnlock(HLOCAL);
HLOCAL TWIN_LocalReAlloc(HLOCAL, UINT, UINT);
UINT TWIN_LocalSize(HLOCAL);

LRESULT
EditMemoryBin(HWND hWnd, UINT uAction, WPARAM wParam, LPARAM lParam)
{
    LPEDIT lp;
    DWORD dwSize;
    HANDLE hHandle;
    LRESULT res;
    REGISTER SaveDS;

    if (!(lp = (LPEDIT)GetWindowLong(hWnd,SWD_LPSTR)) || !lp->uSelDS) 
	return 0L;

    dwSize = (DWORD)lParam;
    hHandle = (HANDLE)wParam;
    SaveDS = envp_global->reg.ds;
    envp_global->reg.ds = lp->uSelDS;
    switch (uAction) {
	case EMA_ALLOC:
	    res = (LRESULT)TWIN_LocalAlloc(LMEM_MOVEABLE,(UINT)dwSize);
	    break;
	case EMA_LOCK:
	    res = (DWORD)TWIN_LocalLock(hHandle) + 
			(DWORD)GetPhysicalAddress(envp_global->reg.ds);
	    break;
	case EMA_UNLOCK:
	    res = (LRESULT)TWIN_LocalUnlock(hHandle);
	    break;
	case EMA_FREE:
	    res = (LRESULT)TWIN_LocalFree(hHandle);
	    break;
	case EMA_REALLOC:
	    res = (LRESULT)TWIN_LocalReAlloc(hHandle,dwSize,LMEM_MOVEABLE);
	    break;
	case EMA_SIZE:
	    res = (LRESULT)TWIN_LocalSize(hHandle);
	    break;
	default:
	    res = 0;
	    break;
    }
    envp_global->reg.ds = SaveDS;
    return res;
}
