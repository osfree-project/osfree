/*    
	DPMI.c	2.14
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
#include <string.h>
#include "windows.h"
#include "Log.h"
#include "DPMI.h"

#define LDTSTEP 0x10

DSCR *LDT;		/* this is our descriptor table */
int nLDTSize;
extern WORD wGlobalBase;
static void ExpandLDT();

UINT TWIN_AllocSelector(DWORD,UINT);
UINT TWIN_ReallocSelector(UINT,DWORD,UINT);

WORD Sel86Flags[] = { 
	0,		/* illegal		*/
	ST_CODE16,	/* TRANSFER_CODE16	*/
	ST_DATA16,	/* TRANSFER_DATA	*/
	ST_CODE32,	/* TRANSFER_CODE32	*/
	ST_DATA32,	/* TRANSFER_DATA32	*/
	ST_CODE16,	/* TRANSFER_CALLBACK	*/
	ST_CODE32,	/* TRANSFER_NATIVE	*/
	ST_CODE16,	/* TRANSFER_BINARY	*/
	ST_CODE16	/* TRANSFER_RETURN	*/
};

UINT
FreeSelector(UINT uSel)
{
    SetPhysicalAddress((WORD)uSel,NULL);
    SetSelectorHandle((WORD)uSel,0);
    DPMI_Notify(DN_FREE,(WORD)uSel);
    return (UINT) NULL;
}

WORD
AssignSelector(LPBYTE lpStruct, WORD wSel, BYTE bSegType, DWORD dwSize)
{
	int n = 0;
	WORD wSelFlags;

	if (!wSel) {
	    while((++n < nLDTSize) && GetPhysicalAddress(n<<3));
	    if (n == nLDTSize)
		ExpandLDT();
	    if (n == wGlobalBase)
		wGlobalBase++;
	    wSel = (n<<3) | 0x7;
	}

	if (lpStruct)
	    SetPhysicalAddress(wSel,lpStruct);

	SetSelectorLimit(wSel,dwSize);
	SetSelectorType(wSel,bSegType);
	wSelFlags = Sel86Flags[bSegType];

	if (lpStruct == NULL)
	    wSelFlags &= ~DF_PRESENT;

	SetSelectorFlags(wSel,wSelFlags);
	DPMI_Notify(DN_ASSIGN,wSel|7);
	return wSel;
}

WORD
AssignSelRange(int nNumSelectors)
{
	int i,n = wGlobalBase;

	while ((n + nNumSelectors) >= nLDTSize) 
	    ExpandLDT();
	wGlobalBase += nNumSelectors;
	for (i=0; i<nNumSelectors; i++)
	    SetPhysicalAddress((i+n)<<3,(LPBYTE)(-1));
	return n;
}

UINT
TWIN_AllocSelector(DWORD dwSize, UINT uFlags)
{
    HGLOBAL	hGlobal;
    LPBYTE	lpData;
    DWORD	dwSelSize;
    int		nNumSel,i;
    UINT	uSel;
    WORD	wIndex;

    hGlobal = GlobalAlloc(uFlags, dwSize);
    if (hGlobal == 0)
	return FALSE;

    lpData = (LPBYTE)GlobalLock(hGlobal);
    if (dwSize > 0x10000) {     /* huge memory object */
	nNumSel = (dwSize + 0xffff) / 0x10000;
	wIndex = AssignSelRange(nNumSel);
	for (i = 0, dwSelSize = dwSize;
		 i < nNumSel; dwSelSize -= 0x10000, i++) {
	    uSel = (wIndex+i)<<3;
	    AssignSelector(lpData+(i*0x10000),uSel,
			TRANSFER_DATA,dwSelSize);
	    SetSelectorHandle(uSel,hGlobal);
	}
	uSel = wIndex<<3|7;
    }
    else {
	uSel = AssignSelector(lpData,0,TRANSFER_DATA,dwSize);
	SetSelectorHandle(uSel,hGlobal);
    }
    return uSel;
}

UINT
TWIN_ReallocSelector(UINT uSel, DWORD dwNewSize, UINT uFlags)
{
    HGLOBAL hGlobal;
    DWORD dwOldSize,dwSelSize;
    LPBYTE lpData;
    WORD wIndex;
    UINT uSeln;
    int nOldNumSel, nNewNumSel, i;
    BOOL bNew = FALSE, bNextFree;

    hGlobal = GetSelectorHandle(uSel);
    dwOldSize = GlobalSize(hGlobal);
    GlobalUnlock(hGlobal);
    if (!(hGlobal = GlobalReAlloc(hGlobal,dwNewSize,uFlags)))
	return 0;

    lpData = (LPBYTE)GlobalLock(hGlobal);

    if (dwOldSize > 0x10000)
	nOldNumSel = (dwOldSize + 0xffff) / 0x10000;
    else
	nOldNumSel = 1;
    if (dwNewSize > 0x10000)
	nNewNumSel = (dwNewSize + 0xffff) / 0x10000;
    else
	nNewNumSel = 1;
    if (nNewNumSel < nOldNumSel) {
	for (i = nNewNumSel, uSeln = uSel; i < nOldNumSel; i++, uSeln += 8)
	    FreeSelector(uSeln);
    }
    else if (nNewNumSel > nOldNumSel) {
	bNextFree = TRUE;
	for (i = nOldNumSel, uSeln = uSel; i < nNewNumSel; i++, uSeln += 8)
	    if (GetPhysicalAddress(uSeln) != (LPBYTE)-1) {
		bNextFree = FALSE;
		break;
	    }
	if (!bNextFree) {
	    for (i = 0, uSeln = uSel; i < nOldNumSel; i++, uSeln += 8)
		FreeSelector(uSeln);
	    wIndex = AssignSelRange(nNewNumSel);
	    uSel = (wIndex <<3)|7;
	    bNew = TRUE;
	}
    }
    for (i=0, uSeln = uSel, dwSelSize = dwNewSize;
			i < nNewNumSel;
			i++, uSeln += 8, dwSelSize -= 0x10000) {
	if (bNew)
	    AssignSelector(lpData+(i*0x10000),uSeln,
			TRANSFER_DATA,dwSelSize);
	else {
	    SetPhysicalAddress(uSeln,lpData+i*0x10000);
	    SetSelectorLimit(uSeln,dwSelSize);
	    DPMI_Notify(DN_MODIFY,uSeln);
	}
	SetSelectorHandle(uSeln,hGlobal);
    }
    return uSel;
}

LPSTR
GetAddress(WORD hiword,WORD loword)
{
	if (hiword && ((WORD)(hiword >> 3) <= (WORD)nLDTSize))
		return ((LPSTR)GetSelectorAddress(hiword) + loword);
	else
		return (LPSTR)(MAKELONG(loword,hiword));
}

WORD
MakeSegment(DWORD dwSize, WORD wType)
{
    HGLOBAL hGlobal;
    WORD wSel;
    LPBYTE lpData;

    hGlobal = GlobalAlloc(GHND,dwSize);
    lpData = (LPBYTE)GlobalLock(hGlobal);
    wSel = AssignSelector(lpData,0,wType,dwSize);
    SetSelectorHandle(wSel,hGlobal);
    return wSel;
}

static void
ExpandLDT()
{
	int i;

	i = nLDTSize;
	nLDTSize += LDTSTEP; 
	LDT = (DSCR *)WinRealloc((LPSTR)LDT,sizeof(DSCR)*nLDTSize);

	if (!LDT)
	    FatalAppExit(0,"Cannot realloc LDT"); 

	memset(&LDT[i],'\0',LDTSTEP*sizeof(DSCR));
}

DWORD
GetSelectorBase(UINT uSelector)
{
    return (DWORD)GetPhysicalAddress(uSelector);
}

int
GetAHSHIFT(void)
{
    return 3;
}

int
GetAHINCR(void)
{
    return 8;
}

