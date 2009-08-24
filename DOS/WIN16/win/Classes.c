/*    
	Classes.c	2.22
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

#include "kerndef.h"
#include "Endian.h"
#include "KrnAtoms.h"
#include "Classes.h"
#include "Log.h"

static LPCLASSINFO lpClasses[] = {
	NULL,
	NULL,
	NULL,
	NULL,
};

static ATOMTABLE ClassTable;

/* external linkage */
extern HMODULE GetModuleFromInstance(HINSTANCE);
extern HINSTANCE GetInstanceFromModule(HMODULE);


/* Prototypes for internal static routines */

static LPCLASSINFO SearchClass(LPCLASSINFO, LPCSTR, HMODULE);
static LPCLASSINFO InternalRegisterClassEx(const WNDCLASSEX *);
static LONG ClassLongPrivate(int, LPCLASSINFO, int, DWORD);
static WORD ClassWordPrivate(int, LPCLASSINFO, int, WORD);

#define GLOBALAddAtomEx AddAtomEx
#define GLOBALDeleteAtomEx DeleteAtomEx
#define GLOBALGetAtomNameEx GetAtomNameEx

/* Initialization routine */

BOOL
InitClasses()
{
    CLASSDATA *lpClass;
    LPCLASSINFO lpClassInfo;
    HCURSOR hArrowCursor;

    hArrowCursor = LoadCursor(0, IDC_ARROW);
    for (lpClass = SystemGlobalClasses;
	lpClass->WndClassEx.lpszClassName;lpClass++) {
	if (!(lpClassInfo = InternalRegisterClassEx(&lpClass->WndClassEx)))
	    return FALSE;
	ClassLongPrivate(WND_SET,lpClassInfo,GCL_NATTOBIN,
		(DWORD)lpClass->lpfnNatToBin);
	ClassLongPrivate(WND_SET,lpClassInfo,GCL_BINTONAT,
		(DWORD)lpClass->lpfnBinToNat);
	atmGlobalLookup[lpClass->uiIndex] = lpClassInfo->atmClassName;
#ifndef	TWIN32
	ClassWordPrivate(WND_SET,lpClassInfo,GCW_HCURSOR,(WORD)
#else
	ClassLongPrivate(WND_SET,lpClassInfo,GCL_HCURSOR,(DWORD)
#endif
		((lpClass->uiIndex == LOOKUP_EDIT) ? LoadCursor(0,IDC_IBEAM) :
		hArrowCursor));
    }
    return TRUE;
}

/* API functions */

BOOL WINAPI
GetClassInfo(HINSTANCE hInstance, LPCSTR lpszClassName, LPWNDCLASS lpwc)
{
    WNDCLASSEX wcx;

    APISTR((LF_APICALL, "GetClassInfo(HINSTANCE=%x,LPCSTR=%s,LPWNDCLASS=%x)\n",
	hInstance, 
	HIWORD(lpszClassName) ? lpszClassName : "ATOM",
	lpwc));

    if (!GetClassInfoEx(hInstance, lpszClassName, &wcx)) {
    	APISTR((LF_APIFAIL, "GetClassInfo: returns BOOL FALSE\n"));
	return (FALSE);
    }

    lpwc->style = wcx.style;
    lpwc->lpfnWndProc = wcx.lpfnWndProc;
    lpwc->cbClsExtra = wcx.cbClsExtra;
    lpwc->cbWndExtra = wcx.cbWndExtra;
    lpwc->hInstance = wcx.hInstance;
    lpwc->hIcon = wcx.hIcon;
    lpwc->hCursor = wcx.hCursor;
    lpwc->hbrBackground = wcx.hbrBackground;
    lpwc->lpszMenuName = wcx.lpszMenuName;
    lpwc->lpszClassName = wcx.lpszClassName;

    APISTR((LF_APIRET, "GetClassInfo: returns BOOL TRUE\n"));
    return (TRUE);

}

void
InternalGetClassInfo(HCLASS32 hClass32, LPWNDCLASS lpwc)
{
	LPCLASSINFO lpClassInfo = (LPCLASSINFO)hClass32;

	if (!lpClassInfo)
		return;
	lpwc->style = lpClassInfo->style;
	lpwc->lpfnWndProc = lpClassInfo->lpfnWndProc;
	lpwc->cbClsExtra = lpClassInfo->cbClsExtra;
	lpwc->cbWndExtra = lpClassInfo->cbWndExtra;
	lpwc->hInstance = (lpClassInfo->hModule)?
		GetInstanceFromModule(lpClassInfo->hModule):0;
	lpwc->hIcon = lpClassInfo->hIcon;
	lpwc->hCursor = lpClassInfo->hCursor;
	lpwc->hbrBackground = lpClassInfo->hbrBackground;
	lpwc->lpszMenuName = (LPSTR)NULL;
	lpwc->lpszClassName = (LPSTR)NULL;
}

/* (WIN32) GetClassInfoEx ************************************************** */

BOOL WINAPI
GetClassInfoEx(HINSTANCE hInstance, LPCTSTR lpszClassName, LPWNDCLASSEX lpwcx)
{
    LPCLASSINFO ClassFound;
    HMODULE hModule;

    APISTR((LF_APICALL, 
	"GetClassInfoEx(HINSTANCE=%x,LPCTSTR=%s,LPWNDCLASSEX=%x)\n",
		hInstance, 
		HIWORD(lpszClassName) ? lpszClassName : "ATOM",
		lpwcx));

    if (!hInstance) {
	if (!(ClassFound =
		SearchClass(lpClasses[SYSGLOBAL],lpszClassName,(HMODULE)0)))
	    if (!(ClassFound =
		SearchClass(lpClasses[APPGLOBAL],lpszClassName,(HMODULE)0))) {
    		APISTR((LF_APIFAIL, "GetClassInfoEx: returns BOOL FALSE\n"));
	        return FALSE;
	    }
    }
    else {
	hModule = GetModuleFromInstance(hInstance);
	if (!(ClassFound =
		SearchClass(lpClasses[APPLOCAL],lpszClassName,hModule)))
	    if (!(ClassFound =
		SearchClass(lpClasses[APPGLOBAL], lpszClassName, hModule))) {
    		APISTR((LF_APIFAIL, "GetClassInfoEx: returns BOOL FALSE\n"));
		    return FALSE;
	    }
    }
    InternalGetClassInfoEx((HCLASS32)ClassFound, lpwcx);
    lpwcx->lpszClassName = (LPSTR)lpszClassName;
    lpwcx->style &= ~CS_SYSTEMGLOBAL;

    APISTR((LF_APIRET, "GetClassInfoEx: returns BOOL TRUE\n"));
    return TRUE;
}

void
InternalGetClassInfoEx(HCLASS32 hClass32, LPWNDCLASSEX lpwcx)
{
	LPCLASSINFO lpClassInfo = (LPCLASSINFO)hClass32;

	if (!lpClassInfo)
		return;
	lpwcx->cbSize = sizeof(WNDCLASSEX);
	lpwcx->style = lpClassInfo->style;
	lpwcx->lpfnWndProc = lpClassInfo->lpfnWndProc;
	lpwcx->cbClsExtra = lpClassInfo->cbClsExtra;
	lpwcx->cbWndExtra = lpClassInfo->cbWndExtra;
	lpwcx->hInstance = (lpClassInfo->hModule)?
		GetInstanceFromModule(lpClassInfo->hModule):0;
	lpwcx->hIcon = lpClassInfo->hIcon;
	lpwcx->hCursor = lpClassInfo->hCursor;
	lpwcx->hbrBackground = lpClassInfo->hbrBackground;
	lpwcx->lpszMenuName = (LPSTR)NULL;
	lpwcx->lpszClassName = (LPSTR)NULL;
	lpwcx->hIconSm = lpClassInfo->hIconSm;

}

/* (WIN32) END ************************************************************* */

ATOM WINAPI
RegisterClass(const WNDCLASS *lpwc)
{
    WNDCLASSEX wcx;
    ATOM atom;

    APISTR((LF_APICALL, "RegisterClass(WNDCLASS *=%x)\n", lpwc));

    wcx.cbSize = sizeof(WNDCLASSEX);
    wcx.style = lpwc->style;
    wcx.lpfnWndProc = lpwc->lpfnWndProc;
    wcx.cbClsExtra = lpwc->cbClsExtra;
    wcx.cbWndExtra = lpwc->cbWndExtra;
    wcx.hInstance = lpwc->hInstance;
    wcx.hIcon = lpwc->hIcon;
    wcx.hCursor = lpwc->hCursor;
    wcx.hbrBackground = lpwc->hbrBackground;
    wcx.lpszMenuName = lpwc->lpszMenuName;
    wcx.lpszClassName = lpwc->lpszClassName;
    wcx.hIconSm = (HICON)0;

    atom = RegisterClassEx(&wcx);
    APISTR((LF_APIRET, "RegisterClass: returns ATOM %x\n",atom));
    return atom;

}

/* (WIN32) RegisterClassEx ************************************************* */

ATOM WINAPI
RegisterClassEx(CONST WNDCLASSEX *lpwcx)
{
    LPCLASSINFO lpClassInfo;
    ATOM atmClass;

    APISTR((LF_APICALL, "RegisterClassEx(WNDCLASS *=%x)\n", lpwcx));
    lpClassInfo = InternalRegisterClassEx(lpwcx);
    atmClass = (lpClassInfo)?lpClassInfo->atmClassName:(ATOM)0;
    APISTR((LF_APIRET, "RegisterClassEx: returns ATOM %x\n",atmClass));
    return atmClass;

}

static LPCLASSINFO
InternalRegisterClassEx(const WNDCLASSEX *lpwcx)
{
    int Type;
    HMODULE hModule;
    LPCLASSINFO lpNewC, *lpClassType;
    int    size;

    if (lpwcx->hInstance) {
        if (!(hModule = GetModuleFromInstance(lpwcx->hInstance))) {
	    return 0;
        }
    }
    else
	hModule = 0;

    if (!lpwcx->lpszClassName)
	return 0;

    if (lpwcx->style & CS_SYSTEMGLOBAL)
	Type = SYSGLOBAL;
    else
	Type = (lpwcx->style & CS_GLOBALCLASS)?APPGLOBAL:APPLOCAL;
    lpClassType = &lpClasses[Type];

    if (*lpClassType) {
    	lpNewC = SearchClass(*lpClassType, lpwcx->lpszClassName, hModule);
        if (lpNewC) {
            return 0;
        }
    }

    lpNewC = (LPCLASSINFO)WinMalloc(sizeof(CLASSINFO));

    memset((LPSTR)lpNewC, 0, sizeof(CLASSINFO));

    if (lpwcx->cbClsExtra)
	lpNewC->lpClsExtra = (LPSTR)WinMalloc(lpwcx->cbClsExtra);
    else
	lpNewC->lpClsExtra = NULL;

    lpNewC->wClassType = Type;
    lpNewC->style = lpwcx->style;
    lpNewC->lpfnWndProc = lpwcx->lpfnWndProc;
    lpNewC->cbClsExtra = lpwcx->cbClsExtra;
    lpNewC->cbWndExtra = lpwcx->cbWndExtra;
    lpNewC->hModule = hModule;
    lpNewC->hIcon = lpwcx->hIcon;
    lpNewC->hCursor = lpwcx->hCursor;
    lpNewC->hbrBackground = lpwcx->hbrBackground;
    lpNewC->nUseCount = 0;
    lpNewC->hIconSm = lpwcx->hIconSm;
    if (!(lpwcx->style & CS_SYSTEMGLOBAL)) {
	lpNewC->lpfnNatToBin = lpfnDefaultNatToBin;
	lpNewC->lpfnBinToNat = lpfnDefaultBinToNat;
    }
    if (HIWORD((DWORD)(lpwcx->lpszMenuName))) {
	size = strlen(lpwcx->lpszMenuName)+1;
	lpNewC->lpMenuName = WinMalloc(size);
	strcpy(lpNewC->lpMenuName, lpwcx->lpszMenuName);
    } else
	lpNewC->lpMenuName = (LPSTR)lpwcx->lpszMenuName;

    if (!HIWORD((DWORD)(lpwcx->lpszClassName)))
	lpNewC->atmClassName =
	    (ATOM)(LOWORD((DWORD)lpwcx->lpszClassName));
    else
	lpNewC->atmClassName = (lpwcx->style & CS_GLOBALCLASS)?
	    GLOBALAddAtomEx(&ClassTable,lpwcx->lpszClassName):
	    AddAtomEx(&ClassTable,lpwcx->lpszClassName);

    if (lpNewC->cbClsExtra)
	memset(lpNewC->lpClsExtra, 0, lpNewC->cbClsExtra);

    /* link in the new class */
    lpNewC->lpClassNext = *lpClassType;
    if (*lpClassType) (*lpClassType)->lpClassPrev = lpNewC;
    *lpClassType = lpNewC;

    return lpNewC;
}

/* (WIN32) END ************************************************************* */

BOOL WINAPI
UnregisterClass(LPCSTR lpClassName, HINSTANCE hInstance)
{
    LPCLASSINFO lpClassFound;
    BOOL rc;

    APISTR((LF_APICALL,"UnregisterClass(LPCSTR=%s, HINSTANCE=%x)\n",
		HIWORD(lpClassName)?lpClassName:"ATOM", 
		hInstance));

    if (!(lpClassFound = (LPCLASSINFO)FindClass(lpClassName, hInstance))) {
    	APISTR((LF_APIFAIL,"UnregisterClass: returns BOOL FALSE\n"));
	return FALSE;
    }

    if ((lpClassFound->wClassType == SYSGLOBAL) ||
	(lpClassFound->nUseCount)) {
    	APISTR((LF_APIFAIL,"UnregisterClass: returns BOOL FALSE\n"));
	return FALSE;
    }


    rc =  TWIN_InternalUnregisterClass(lpClassFound);

    APISTR((LF_APIRET,"UnregisterClass: returns BOOL %d\n",rc));
    return rc;
}

BOOL TWIN_InternalUnregisterClass(LPCLASSINFO lpClassFound)
{
    if (lpClassFound->lpClsExtra)
	WinFree(lpClassFound->lpClsExtra);

    if (HIWORD((DWORD)(lpClassFound->lpMenuName)))
	WinFree(lpClassFound->lpMenuName);

    if (lpClassFound->style & CS_GLOBALCLASS)
	GLOBALDeleteAtomEx(&ClassTable,lpClassFound->atmClassName);
    else
	DeleteAtomEx(&ClassTable,lpClassFound->atmClassName);

    if (lpClassFound->lpClassPrev)
	lpClassFound->lpClassPrev->lpClassNext = lpClassFound->lpClassNext;
    if (lpClassFound->lpClassNext)
	lpClassFound->lpClassNext->lpClassPrev = lpClassFound->lpClassPrev;
    if (lpClasses[lpClassFound->wClassType] == lpClassFound)
	lpClasses[lpClassFound->wClassType] = lpClassFound->lpClassNext;

    WinFree((LPSTR)lpClassFound);

    return TRUE;
}

/* These are the supplemental routines */

HCLASS32
FindClass(LPCSTR lpClassName, HINSTANCE hInstance)
{
    LPCLASSINFO lpClassFound;
    HMODULE hModule;

    APISTR((LF_APICALL, "FindClass(LPCSTR=%p,HINSTANCE=%x)\n",
	HIWORD(lpClassName) ? lpClassName : "atom", hInstance));

    if (hInstance)
	hModule = GetModuleFromInstance(hInstance);
    else
	hModule = 0;
	lpClassFound =	SearchClass(lpClasses[APPLOCAL], lpClassName, hModule);

    if (lpClassFound) {
    	APISTR((LF_APIRET,"FindClass: returns HCLASS32 %d\n",lpClassFound));
	return (HCLASS32)lpClassFound;
    }
		
	lpClassFound =	SearchClass(lpClasses[APPGLOBAL],lpClassName,(HMODULE)0);
    if (lpClassFound) {
    	APISTR((LF_APIRET,"FindClass: returns HCLASS32 %d\n",lpClassFound));
        return (HCLASS32)lpClassFound;
    }

    lpClassFound =  SearchClass(lpClasses[SYSGLOBAL], lpClassName,(HMODULE)0);
    APISTR((LF_APIRET,"FindClass: returns HCLASS32 %d\n",lpClassFound));
    return (HCLASS32)lpClassFound;
}

static LPCLASSINFO
SearchClass(LPCLASSINFO lpClassType, LPCSTR lpClassStr,
			HMODULE hModule)
{
    LPCLASSINFO lpClass;
    char lpAtomString[80];
    char lpDialogString[80];
    LPSTR lpClassName;

    if (!hModule && !HIWORD((DWORD)lpClassStr) &&
	LOWORD((DWORD)lpClassStr) == LOWORD((DWORD)WC_DIALOG)) {
	strcpy(lpDialogString,TWIN_DIALOGCLASS);
	lpClassName = lpDialogString;
    }
    else
	lpClassName = (LPSTR)lpClassStr;

    for(lpClass = lpClassType;lpClass;lpClass = lpClass->lpClassNext) {
	if (!HIWORD((DWORD)lpClassName)) {
	    if (lpClass->atmClassName ==
		(ATOM)(LOWORD((DWORD)lpClassName)) &&
		(!hModule || (lpClass->hModule == hModule)))
		return lpClass;
	}
	else {
	    if (lpClass->style & CS_GLOBALCLASS)
		GLOBALGetAtomNameEx(&ClassTable,
			lpClass->atmClassName,lpAtomString,80);
	    else
		GetAtomNameEx(&ClassTable,
			lpClass->atmClassName,lpAtomString,80);
	    if((!strcasecmp(lpAtomString,lpClassName)) &&
		(!hModule || (lpClass->hModule == hModule)))
		return lpClass;
	}
    }
    return 0;
}

LPSTR
GetClassMenuName(HCLASS32 hClass32)
{
    return (hClass32)?((LPCLASSINFO)hClass32)->lpMenuName:(LPSTR)0;
}

UINT
InternalGetClassName(HCLASS32 hClass32, LPSTR lpClassName,
			int cchClassName)
{
    LPCLASSINFO lpClass;

    if (!hClass32)
	return 0;
    lpClass = (LPCLASSINFO)hClass32;
    return (lpClass->style & CS_GLOBALCLASS)?
	GLOBALGetAtomNameEx(&ClassTable,
		lpClass->atmClassName,lpClassName,cchClassName):
	GetAtomNameEx(&ClassTable,
		lpClass->atmClassName,lpClassName,cchClassName);
}

void
LockClass(HCLASS32 hClass32, BOOL bLock)
{
    if (bLock)
	((LPCLASSINFO)hClass32)->nUseCount++;
    else
	((LPCLASSINFO)hClass32)->nUseCount--;
}

LONG
GetClassHandleLong(HCLASS32 hClass32, int nIndex)
{
    return ClassLongPrivate(WND_GET,(LPCLASSINFO)hClass32,nIndex,0L);
}

WORD
GetClassHandleWord(HCLASS32 hClass32, int nIndex)
{
    return ClassWordPrivate(WND_GET,(LPCLASSINFO)hClass32,nIndex,0L);
}

LONG
SetClassHandleLong(HCLASS32 hClass32, int nIndex, LONG dwNewLong)
{
    return ClassLongPrivate(WND_SET,(LPCLASSINFO)hClass32,nIndex,dwNewLong);
}

WORD
SetClassHandleWord(HCLASS32 hClass32, int nIndex, WORD wNewWord)
{
    return ClassWordPrivate(WND_SET,(LPCLASSINFO)hClass32,nIndex,wNewWord);
}

static LONG
ClassLongPrivate(int Func, LPCLASSINFO lpWindowClass,
			int nIndex, DWORD dwNewLong)
{
    LPSTR ptr;
    DWORD dwTemp;

    if (nIndex < 0)
	switch(nIndex) {
#ifdef	TWIN32
	    case GCL_CBCLSEXTRA:
		dwTemp = (DWORD)(lpWindowClass->cbClsExtra);
		if (Func == WND_SET) {
		    if (dwTemp)
	 		lpWindowClass->lpClsExtra = WinRealloc
			((LPSTR)(lpWindowClass->lpClsExtra), dwNewLong);
		    else
                        lpWindowClass->lpClsExtra = WinMalloc(dwNewLong);
		    lpWindowClass->cbClsExtra = dwNewLong;
		}
		return dwTemp;
	    case GCL_CBWNDEXTRA:
		dwTemp = (DWORD)(lpWindowClass->cbWndExtra);
		if (Func == WND_SET)
		    lpWindowClass->cbWndExtra = dwNewLong;
		return(dwTemp);
	    case GCL_HBRBACKGROUND:
		dwTemp = (DWORD)(lpWindowClass->hbrBackground);
		if (Func == WND_SET)
		    lpWindowClass->hbrBackground = (HBRUSH)dwNewLong;
		return(dwTemp);
	    case GCL_HCURSOR:
		dwTemp = (DWORD)(lpWindowClass->hCursor);
		if (Func == WND_SET)
		    lpWindowClass->hCursor = (HCURSOR)dwNewLong;
		return(dwTemp);
	    case GCL_HICON:
		dwTemp = (DWORD)(lpWindowClass->hIcon);
		if (Func == WND_SET)
		    lpWindowClass->hIcon = (HICON)dwNewLong;
		return(dwTemp);
	    case GCL_HMODULE:
		dwTemp = (DWORD)(lpWindowClass->hModule);
		if (Func == WND_SET)
		    lpWindowClass->hModule = (HMODULE)dwNewLong;
		return(dwTemp);
	    case GCL_STYLE:
		dwTemp = (DWORD)(lpWindowClass->style);
		if (Func == WND_SET)
		    lpWindowClass->style = dwNewLong; /* ??? */
		return(dwTemp);
	    case GCL_HDC:
		dwTemp = (DWORD)(lpWindowClass->hDC);
		if (Func == WND_SET)
		    lpWindowClass->hDC = (HDC)dwNewLong;
		return(dwTemp);
#endif
	    case GCL_MENUNAME:
		dwTemp = (DWORD)(lpWindowClass->lpMenuName);
		if (Func == WND_SET)
		    lpWindowClass->lpMenuName = (LPSTR)dwNewLong;
		return(dwTemp);
	    case GCL_WNDPROC:
		dwTemp = (DWORD)(lpWindowClass->lpfnWndProc);
		if (Func == WND_SET)
		    lpWindowClass->lpfnWndProc = (WNDPROC)dwNewLong;
		return(dwTemp);
	    case GCL_BINTONAT:
		dwTemp = (DWORD)(lpWindowClass->lpfnBinToNat);
		if (Func == WND_SET)
		    lpWindowClass->lpfnBinToNat = (WNDPROC)dwNewLong;
		return(dwTemp);
	    case GCL_NATTOBIN:
		dwTemp = (DWORD)(lpWindowClass->lpfnNatToBin);
		if (Func == WND_SET)
		    lpWindowClass->lpfnNatToBin = (WNDPROC)dwNewLong;
		return(dwTemp);
	    default:
		return(0L);
	}
    else {
	ptr = (LPSTR)(lpWindowClass->lpClsExtra) + nIndex;
	dwTemp = GETDWORD(ptr);
	if (Func == WND_SET)
	    PUTDWORD(ptr,dwNewLong);
	return(dwTemp);
    }
}

static WORD
ClassWordPrivate(int Func, LPCLASSINFO lpWindowClass,
			int nIndex, WORD wNewWord)
{
    LPSTR ptr;
    WORD wTemp;

    if (nIndex < 0)
	switch(nIndex) {
#ifndef	TWIN32
	    case GCW_CBCLSEXTRA:
		wTemp = (WORD)(lpWindowClass->cbClsExtra);
		if (Func == WND_SET) {
		    if (wTemp)
	 		lpWindowClass->lpClsExtra = WinRealloc
			((LPSTR)(lpWindowClass->lpClsExtra), wNewWord);
		    else
                        lpWindowClass->lpClsExtra = WinMalloc(wNewWord);
		    lpWindowClass->cbClsExtra = wNewWord;
		}
		return wTemp;
	    case GCW_CBWNDEXTRA:
		wTemp = (WORD)(lpWindowClass->cbWndExtra);
		if (Func == WND_SET)
		    lpWindowClass->cbWndExtra = wNewWord;
		return(wTemp);
	    case GCW_HBRBACKGROUND:
		wTemp = (WORD)(lpWindowClass->hbrBackground);
		if (Func == WND_SET)
		    lpWindowClass->hbrBackground = (HBRUSH)wNewWord;
		return(wTemp);
	    case GCW_HCURSOR:
		wTemp = (WORD)(lpWindowClass->hCursor);
		if (Func == WND_SET)
		    lpWindowClass->hCursor = (HCURSOR)wNewWord;
		return(wTemp);
	    case GCW_HICON:
		wTemp = (WORD)(lpWindowClass->hIcon);
		if (Func == WND_SET)
		    lpWindowClass->hIcon = (HICON)wNewWord;
		return(wTemp);
	    case GCW_HMODULE:
		wTemp = (WORD)(lpWindowClass->hModule);
		if (Func == WND_SET)
		    lpWindowClass->hModule = (HMODULE)wNewWord;
		return(wTemp);
	    case GCW_STYLE:
		wTemp = (WORD)(lpWindowClass->style);
		if (Func == WND_SET)
		    lpWindowClass->style = wNewWord; /* ??? */
		return(wTemp);
	    case GCW_HDC:
		wTemp = (WORD)(lpWindowClass->hDC);
		if (Func == WND_SET)
		    lpWindowClass->hDC = (HDC)wNewWord;
		return(wTemp);
#endif
	    case GCW_ATOM:
		wTemp = (WORD)(lpWindowClass->atmClassName);
		if (Func == WND_SET)
		    lpWindowClass->atmClassName = (ATOM)wNewWord;
		return(wTemp);
	    default:
		return 0;
	}
    else {
	ptr = (LPSTR)(lpWindowClass->lpClsExtra) + nIndex;
	wTemp = GETWORD(ptr);
	if (Func == WND_SET)
	    PUTWORD(ptr,wNewWord);
	return(wTemp);
    }
}

BOOL
EnumClasses(UINT fuFlags, CLASSENUMPROC lpfnEnumProc, LPARAM lParam)
{
    LPCLASSINFO lpClass;
    WNDCLASS WndClass;

    if (fuFlags & ECF_SYSGLOBAL) {
	for (lpClass = lpClasses[SYSGLOBAL];
	     lpClass;
	     lpClass = lpClass->lpClassNext) {
	    InternalGetClassInfo((HCLASS32)lpClass,&WndClass);
	    WndClass.lpszClassName = (LPSTR)MAKEINTATOM(lpClass->atmClassName);
	    if (!lpfnEnumProc((HCLASS32)lpClass,&WndClass,lParam))
		return FALSE;
	}
    }

    if (fuFlags & ECF_APPGLOBAL) {
	for (lpClass = lpClasses[APPGLOBAL];
	     lpClass;
	     lpClass = lpClass->lpClassNext) {
	    InternalGetClassInfo((HCLASS32)lpClass,&WndClass);
	    WndClass.lpszClassName = (LPSTR)MAKEINTATOM(lpClass->atmClassName);
	    if (!lpfnEnumProc((HCLASS32)lpClass,&WndClass,lParam))
		return FALSE;
	}
    }

    if (fuFlags & ECF_APPLOCAL) {
	for (lpClass = lpClasses[APPLOCAL];
	     lpClass;
	     lpClass = lpClass->lpClassNext) {
	    InternalGetClassInfo((HCLASS32)lpClass,&WndClass);
	    WndClass.lpszClassName = (LPSTR)MAKEINTATOM(lpClass->atmClassName);
	    if (!lpfnEnumProc((HCLASS32)lpClass,&WndClass,lParam))
		return FALSE;
	}
    }

    return TRUE;
}

LPCLASSINFO TWIN_FindModuleClass(LPCLASSINFO lpClassType, HMODULE hModule)
{
    LPCLASSINFO lpClass;
    
    for (lpClass = lpClassType; lpClass; lpClass = lpClass->lpClassNext)
	if (lpClass->hModule == hModule)
	    return lpClass;
    
    return NULL;
}

void TWIN_UnregisterModuleClasses(HMODULE hModule)
{
    LPCLASSINFO lpClassFound;

    while ((lpClassFound = TWIN_FindModuleClass(lpClasses[APPLOCAL], 
						hModule)))
    {
	TWIN_InternalUnregisterClass(lpClassFound);
    }
    
    while ((lpClassFound = TWIN_FindModuleClass(lpClasses[APPGLOBAL], 
						hModule)))
    {
	TWIN_InternalUnregisterClass(lpClassFound);
    }
    
    while ((lpClassFound = TWIN_FindModuleClass(lpClasses[SYSGLOBAL], 
						hModule)))
    {
	TWIN_InternalUnregisterClass(lpClassFound);
    }
}

ATOM atmGlobalLookup[] = {
	0,				/* ROOTWClass */
	0,				/* BUTTON */
	0,				/* COMBOBOX */
	0,				/* EDIT */
	0,				/* LISTBOX */
	0,				/* COMBOLBOX */
	0,				/* MDICLIENT */
	0,				/* SCROLLBAR */
	0,				/* STATIC */
	0,				/* FRAMECLASS */
	0,				/* DIALOGCLASS */
	0,				/* TRACKPOPUP */
	0,				/* ICONTITLE */
	0,				/* MENULBOX */
	0,				/* SYSSCROLL */
};

CLASSDATA SystemGlobalClasses[] =
{
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL | CS_GLOBALCLASS,
			(WNDPROC)DefROOTProc,
			0,
			0,
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)0,
			NULL,
			"ROOTWClass",
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_ROOTWCLASS,
	},
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL|CS_GLOBALCLASS|CS_DBLCLKS|CS_PARENTDC,
			(WNDPROC)DefBUTTONProc,
			0,
			sizeof(WORD)+sizeof(HFONT)+sizeof(WORD),
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)0,
			NULL,
			"Button",
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_BUTTON,
	},
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL|CS_GLOBALCLASS|CS_DBLCLKS|CS_PARENTDC,
			(WNDPROC)DefCOMBOBOXProc,
			0,
			sizeof(LPVOID),
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)0,
			NULL,
			"ComboBox",
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_COMBOBOX,
	},
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL|CS_GLOBALCLASS|CS_DBLCLKS|CS_PARENTDC,
			(WNDPROC)DefEDITProc,
			0,
			sizeof(LPVOID),
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)0,
			NULL,
			"Edit",
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_EDIT,
	},
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL|CS_GLOBALCLASS|CS_DBLCLKS|CS_PARENTDC,
			(WNDPROC)DefLISTBOXProc,
			0,
			sizeof(HMENU)+sizeof(HWND)+sizeof(LPVOID)
				+sizeof(LPVOID),
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)0,
			NULL,
			"ListBox",
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_LISTBOX,
	},
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL|CS_GLOBALCLASS|CS_DBLCLKS|CS_SAVEBITS
				|CS_HREDRAW|CS_VREDRAW,
		    	(WNDPROC)DefCOMBOLBOXProc,
			0,
			sizeof(HMENU)+sizeof(HWND)+sizeof(LPVOID)
				+sizeof(LPVOID),
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)0,
			NULL,
			"ComboLBox",
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_COMBOLBOX,
	},
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL|CS_GLOBALCLASS|0,
			(WNDPROC)DefMDICLIENTProc,
			0,
			16+sizeof(LPVOID),  /* QuattroPro uses extra bytes */
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)(COLOR_APPWORKSPACE+1),
			NULL,
			"MDIClient",
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_MDICLIENT,
	},
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL|CS_GLOBALCLASS|CS_DBLCLKS|CS_PARENTDC
				|CS_HREDRAW|CS_VREDRAW,
		    	(WNDPROC)DefSCROLLBARProc,
			0,
			sizeof(LPVOID),
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)0,
			NULL,
			"ScrollBar",
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_SCROLLBAR,
	},
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL|CS_GLOBALCLASS|CS_DBLCLKS|CS_PARENTDC
				|CS_HREDRAW|CS_VREDRAW,
		    	(WNDPROC)DefSCROLLBARProc,
			0,
			sizeof(LPVOID),
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)0,
			NULL,
			TWIN_SYSSCROLLCLASS,
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_SYSSCROLL,
	},
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL|CS_GLOBALCLASS|CS_PARENTDC,
		    	(WNDPROC)DefSTATICProc,
			0,
			sizeof(HFONT)+sizeof(WORD),
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)0,
			NULL,
			"Static",
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_STATIC,
	},
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL|CS_GLOBALCLASS|0,
			(WNDPROC)FrameProc,
			0,
			sizeof(HMENU)+sizeof(HWND)+sizeof(LPVOID)
				+sizeof(LPVOID),
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)(COLOR_MENU+1),
			NULL,
			TWIN_FRAMECLASS,
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_FRAME,
	},
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL|CS_GLOBALCLASS|CS_SAVEBITS,
			(WNDPROC)DefDlgProc,
			0,
			DLGWINDOWEXTRA,
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)(COLOR_WINDOW+1),
			NULL,
			TWIN_DIALOGCLASS,
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_DIALOGCLASS,
	},
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL|CS_GLOBALCLASS|CS_DBLCLKS,
			(WNDPROC)MenuBarProc,
			0,
			sizeof(WORD) + sizeof(LPVOID),
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)0,
			NULL,
			"TRACKPOPUP",
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_TRACKPOPUP,
	},
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL|CS_GLOBALCLASS|CS_DBLCLKS,
			(WNDPROC)IconTitleWndProc,
			0,
			0,
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)0,
			NULL,
			"ICONTITLE",
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_ICONTITLE,
	},
	{
		{
			sizeof(WNDCLASSEX),
			CS_SYSTEMGLOBAL|CS_GLOBALCLASS|CS_SAVEBITS|CS_DBLCLKS,
			(WNDPROC)DefLISTBOXProc,
			0,
			sizeof(HMENU)+sizeof(HWND)+sizeof(LPVOID)
				+sizeof(LPVOID),
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)(COLOR_MENU+1),
			NULL,
			"MENULBOX",
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		LOOKUP_MENULBOX,
	},
	{
		{
			sizeof(WNDCLASSEX),
			0,
			NULL,
			0,
			0,
			(HINSTANCE)0,
			(HICON)0,
			(HCURSOR)0,
			(HBRUSH)0,
			NULL,
			NULL,		/* NULL class name terminates list */
			(HICON)0,
		},
		(WNDPROC)NULL,
		(WNDPROC)NULL,
		0,
	},
};
