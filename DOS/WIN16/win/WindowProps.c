/*    
	WindowProps.c	2.7
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

#include "KrnAtoms.h"
#include "WinDefs.h"
#include "Log.h"
 
#define	PROPMAN_SETPROP		0
#define	PROPMAN_GETPROP		1
#define	PROPMAN_REMOVEPROP	2
#define	PROPMAN_ENUMPROPS	3

static ATOMTABLE PropTable;

static DWORD PropMan(int, HWND, ATOM, BOOL, LONG);

static DWORD
PropMan(int iCmd, HWND hWnd, ATOM atom, BOOL bOriginalAtom, LONG hData)
{
	HWND32		hWnd32;
	LPPROPITEM	lpPropItem;
	LPPROPITEM	*lpPrevAddr=NULL;
	LPSTR		lpStr;
	BOOL		rc;
	char		szStr[128];

	if ( !(hWnd32 = GETHWND32(hWnd)) )
		return 0;

	switch ( iCmd ) {
	case PROPMAN_SETPROP:
		if ( !hWnd32->lpPropertyList )
			/* Initialize lpPrevAddr in case property */
			/* list is empty.                         */
			lpPrevAddr = &hWnd32->lpPropertyList;

		for ( lpPropItem = hWnd32->lpPropertyList; lpPropItem;
					lpPropItem = lpPropItem->lpNextProp ) {
			if ((lpPropItem->bOriginalAtom == bOriginalAtom) &&
			    (lpPropItem->atmProp == atom)) {
				/* Modify existing property */
				lpPropItem->hHandle = hData;
				RELEASEWININFO(hWnd32);
				return TRUE;
			}
			lpPrevAddr = &lpPropItem->lpNextProp;
		}

		/* Add new item into the list */
		*lpPrevAddr = (LPPROPITEM)WinMalloc(sizeof(PROPITEM));
		(*lpPrevAddr)->lpNextProp = 0;
		(*lpPrevAddr)->atmProp    = atom;
		(*lpPrevAddr)->hHandle    = hData;
		(*lpPrevAddr)->bOriginalAtom = bOriginalAtom;
		RELEASEWININFO(hWnd32);
		return TRUE;

	case PROPMAN_GETPROP:
		for ( lpPropItem = hWnd32->lpPropertyList; lpPropItem;
					lpPropItem = lpPropItem->lpNextProp ) {
			if ((lpPropItem->bOriginalAtom == bOriginalAtom) &&
			    (lpPropItem->atmProp == atom))
			{
			    	RELEASEWININFO(hWnd32);
			    	return lpPropItem->hHandle;
			}
		}
		RELEASEWININFO(hWnd32);
		return 0;

	case PROPMAN_REMOVEPROP:
		for ( lpPropItem = hWnd32->lpPropertyList; lpPropItem;
					lpPropItem = lpPropItem->lpNextProp ) {
			if ((lpPropItem->bOriginalAtom == bOriginalAtom) &&
			    (lpPropItem->atmProp == atom)) {
			    	RELEASEWININFO(hWnd32);
				return (DWORD)lpPropItem;
			}
		}
		RELEASEWININFO(hWnd32);
		return 0;
	case PROPMAN_ENUMPROPS:
		rc = -1;
		for ( lpPropItem = hWnd32->lpPropertyList; lpPropItem;
					lpPropItem = lpPropItem->lpNextProp ) {

			/* Send either atom or ptr to string, depending */
			/* on what has been passed to SetProp().        */
			if ( lpPropItem->bOriginalAtom )
				lpStr = (LPSTR)(DWORD)lpPropItem->atmProp;
			else {
				GetAtomName(lpPropItem->atmProp, szStr, 
					sizeof(szStr));
				lpStr = szStr;
			}

			/* Call EnumProc */
			rc = ((PROPENUMPROC)hData)
					(hWnd, lpStr, lpPropItem->hHandle);
		}
		RELEASEWININFO(hWnd32);
		return (DWORD)rc;	/* -1, if there is no property list */
	}

	RELEASEWININFO(hWnd32);
	return 0;
}

BOOL WINAPI
SetProp(HWND hWnd, LPCSTR lpString, HANDLE hData)
{   
	ATOM		atom;
	BOOL		rc, bOriginalAtom;
	    
	if ( HIWORD(lpString) ) {
		atom = AddAtomEx(&PropTable,lpString);
		bOriginalAtom = FALSE;
	}
	else {
		atom = (ATOM)((DWORD)lpString);
		bOriginalAtom = TRUE;
	}

	rc = (BOOL)PropMan(PROPMAN_SETPROP, hWnd, atom, bOriginalAtom, hData);
	APISTR((LF_API, "SetProp(hWnd=0x%x, lpStr=0x%x, hData=0x%x) returns 0x%x\n",
		hWnd, lpString, hData,rc));
	return rc;
}   
 
HANDLE WINAPI
GetProp(HWND hWnd, LPCSTR lpString)
{   
	ATOM		atom;
	HANDLE		rch = 0;

	if ((HIWORD(lpString))) {
		if ((atom = FindAtomEx(&PropTable,lpString)))
		    rch = (HANDLE)PropMan(PROPMAN_GETPROP, hWnd, atom, 
					FALSE, 0);
		else
		    rch = 0;
	}
	else
		rch = (HANDLE)PropMan(PROPMAN_GETPROP, hWnd,
			(ATOM)(DWORD)lpString,
			TRUE, 0);
	APISTR((LF_API, "GetProp(hWnd=0x%x, lpStr=0x%x) returns 0x%x\n",
		hWnd, lpString, rch));
	return rch;
}

HANDLE WINAPI
RemoveProp(HWND hWnd, LPCSTR lpString)
{   
	LPPROPITEM	lpPropItem;
	HANDLE		rch;
	ATOM		atom;
	BOOL		bOriginalAtom;
    
	if ( HIWORD(lpString) ) {
		if (!(atom = FindAtomEx(&PropTable,lpString)))
		    return 0;
		bOriginalAtom = FALSE;
	}
	else {
		atom = (ATOM)((DWORD)lpString);
		bOriginalAtom = TRUE;
	}

	lpPropItem = (LPPROPITEM)PropMan(PROPMAN_REMOVEPROP, hWnd, atom, 
					bOriginalAtom, 0);

	if ( lpPropItem ) {
		rch = lpPropItem->hHandle;
		if (!bOriginalAtom)
		    DeleteAtomEx(&PropTable,lpPropItem->atmProp);
	}
	else
	   rch = 0;

	APISTR((LF_API, "RemoveProp(hWnd=0x%x, lpStr=0x%x) returns 0x%x\n",
		hWnd, lpString,rch));
	return rch;
}      

int WINAPI
EnumProps(HWND hWnd, PROPENUMPROC lpEnumProc)
{
        return (int)PropMan(PROPMAN_ENUMPROPS, hWnd, 0, 0, (LONG)lpEnumProc);
}
