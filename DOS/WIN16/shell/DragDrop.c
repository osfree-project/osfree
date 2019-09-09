/*
    
	DragDrop.c	1.4
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
/*
#include "shellapi.h"
*/
#define __SHELLAPI_H__
#include "Willows.h"

#include "Log.h"
#include "DragDrop.h"

void WINAPI
DragAcceptFiles(HWND hWnd, BOOL fAccept)
{
    DWORD dwExStyle;

    APISTR((LF_APICALL,"DragAcceptFiles(HWND=%x,BOOL=%d)\n",
	hWnd,fAccept));

    if (!IsWindow(hWnd)) {
        APISTR((LF_APIRET,"DragAcceptFiles: returns void\n"));
	return;
    }

    dwExStyle = GetWindowExStyle(hWnd);

    if (fAccept)
	dwExStyle |= WS_EX_ACCEPTFILES;
    else
	dwExStyle &= ~WS_EX_ACCEPTFILES;

    SetWindowLong(hWnd,GWL_EXSTYLE,dwExStyle);
    APISTR((LF_APIRET,"DragAcceptFiles: returns void\n"));
}

UINT WINAPI
DragQueryFile(HDROP hDrop, UINT iFile, LPSTR lpszFile, UINT cb)
{
    APISTR((LF_APISTUB,"DragQueryFile(HDROP=%x,UINT=%x,LPSTR=%s,UINT=%d)\n",
	hDrop,iFile,lpszFile?lpszFile:"NULL",cb));

    return 0;
}

BOOL WINAPI
DragQueryPoint(HDROP hDrop, POINT *lppt)
{
#ifdef	LATER
    HGLOBAL hGlobal = (HGLOBAL)hDrop;
    LPDRAGINFO lpDragInfo;

    APISTR((LF_APICALL,"DragQueryPoint(HDROP=%x,POINT=%x)\n",
	hDrop,lppt));

    if (NULL == (lpDragInfo = (LPDRAGINFO)GlobalLock(hGlobal))) {
        APISTR((LF_APICALL,"DragQueryPoint: returns BOOL FALSE\n"));
	return FALSE;
    }

    lppt->x = lpDragInfo->x;
    lppt->y = lpDragInfo->y;

    GlobalUnlock(hGlobal);

    APISTR((LF_APICALL,"DragQueryPoint: returns BOOL TRUE\n"));
    return TRUE;
#else
    APISTR((LF_APISTUB,"DragQueryPoint(HDROP=%x,POINT=%x)\n",
	hDrop,lppt));
    return FALSE;
#endif
}

void WINAPI
DragFinish(HDROP hDrop)
{
    HGLOBAL hGlobal = (HGLOBAL)hDrop;

    APISTR((LF_APICALL,"DragFinish(HDROP=%x)\n",hDrop));

    GlobalFree(hGlobal);

    APISTR((LF_APIRET,"DragFinish: returns void\n"));
}
