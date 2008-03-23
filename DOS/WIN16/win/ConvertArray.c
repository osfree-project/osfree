/*    
	ConvertArray.c	2.3
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
#include "Endian.h"

/* exported functions */
LPWORD hsw_ConvertWordArray(LPBYTE,int);
LPINT  hsw_ConvertArrayToInt(LPBYTE, int);
LPWORD hsw_ConvertArrayToWord(LPBYTE,int);

LPWORD
hsw_ConvertWordArray(LPBYTE lpStr,int nCount)
{
	static int nArraySize;
	static WORD *lpArray;
	int n;

	if(nArraySize < nCount) {
	    if(lpArray)
		lpArray = (LPWORD)WinRealloc((LPSTR)lpArray,nCount*2);
	    else
		lpArray = (LPWORD)WinMalloc(nCount*2);
	    if(lpArray == 0)
		return 0;
	    nArraySize  = nCount;
	}
	for(n=0;n<nCount;n++) 
	    lpArray[n] = GETWORD(lpStr+2*n);

	return lpArray;
}

LPINT
hsw_ConvertArrayToInt(LPBYTE lpStr,int nCount)
{
	static int nArraySize;
	static LPINT lpArray;
	int n;

	if(nArraySize < nCount) {
	    if(lpArray)
		lpArray = (LPINT)WinRealloc((LPSTR)lpArray,
					nCount*sizeof(int));
	    else
		lpArray = (LPINT)WinMalloc(nCount*sizeof(int));
	    if(lpArray == 0)
		return 0;
	    nArraySize  = nCount;
	}
	for(n=0;n<nCount;n++) 
	    lpArray[n] = (int)GETSHORT(lpStr+2*n);

	return lpArray;
}

LPWORD
hsw_ConvertArrayToWord(LPBYTE lpStr,int nCount)
{
	static int nArraySize;
	static LPWORD lpArray;
	LPINT lpInt;
	int n;

	if(nArraySize < nCount) {
	    if(lpArray)
		lpArray = (LPWORD)WinRealloc((LPSTR)lpArray,
					nCount*sizeof(WORD));
	    else
		lpArray = (LPWORD)WinMalloc(nCount*sizeof(WORD));
	    if(lpArray == 0)
		return 0;
	    nArraySize  = nCount;
	}
	lpInt = (LPINT)lpStr;
	for(n=0;n<nCount;n++) 
	    PUTWORD(&lpArray[n],(WORD)lpInt[n]);

	return lpArray;
}
