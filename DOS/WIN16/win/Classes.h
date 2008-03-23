/********************************************************************

	@(#)Classes.h	2.14 *   Class Management API definitions.
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

********************************************************************/
 

#ifndef Classes__h
#define Classes__h

#define	SYSLOCAL	0
#define	SYSGLOBAL	1
#define	APPLOCAL	2
#define	APPGLOBAL	3

typedef	struct	tagCLASSINFO {
    struct tagCLASSINFO	*lpClassNext;	/* ptr to next class */
    struct tagCLASSINFO	*lpClassPrev;	/* ptr to prev class */
    UINT	wClassType;		/* class type */
    UINT	style;			/* class style */
    WNDPROC	lpfnWndProc;		/* class procedure */
    WNDPROC	lpfnBinToNat;		/* BINTONAT conversion procedure */
    WNDPROC	lpfnNatToBin;		/* NATTOBIN conversion procedure */
    int		cbClsExtra;		/* class extra bytes */
    int		cbWndExtra;		/* window extra bytes */
    HANDLE	hModule;		/* class module instance handle */
    HICON	hIcon;			/* class icon resource handle */
    HCURSOR	hCursor;		/* class cursor resource handle */
    HBRUSH	hbrBackground;		/* class backgr. brush handle */
    HDC		hDC;			/* class DC handle */
    ATOM	atmClassName;		/* class name atom */
    int		nUseCount;		/* Usage count  - private */
    LPSTR	lpMenuName;		/* menu name string */
    LPSTR	lpClsExtra;		/* ptr to class extra bytes */
    HICON	hIconSm;		/* (WIN32) small class icon */
} CLASSINFO;
typedef CLASSINFO	*PCLASSINFO;
typedef CLASSINFO NEAR	*NPCLASSINFO;
typedef CLASSINFO FAR	*LPCLASSINFO;

extern WNDPROC lpfnDefaultNatToBin;
extern WNDPROC lpfnDefaultBinToNat;

LRESULT TrackPopupWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT IconTitleWndProc(HWND, UINT, WPARAM, LPARAM);

typedef struct tagCLASSDATA {
	WNDCLASSEX	WndClassEx;
	WNDPROC		lpfnNatToBin;
	WNDPROC		lpfnBinToNat;
	UINT		uiIndex;	/* index in the global atom lookup */
} CLASSDATA;
typedef CLASSDATA	*PCLASSDATA;
typedef CLASSDATA NEAR	*NPCLASSDATA;
typedef CLASSDATA FAR	*LPCLASSDATA;

extern ATOM atmGlobalLookup[];
extern CLASSDATA SystemGlobalClasses[];

/* Prototypes for exported routines */
BOOL InitClasses();
HCLASS32 FindClass(LPCSTR, HINSTANCE);
LPSTR GetClassMenuName(HCLASS32);
UINT InternalGetClassName(HCLASS32, LPSTR, int);
void LockClass(HCLASS32, BOOL);
void InternalGetClassInfoEx(HCLASS32, LPWNDCLASSEX);
void TWIN_UnregisterModuleClasses(HMODULE);
BOOL TWIN_InternalUnregisterClass(LPCLASSINFO);

#endif /* Classes__h */
