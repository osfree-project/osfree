/*    
	GdiTTEngine.c	2.4
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
#include "windows.h"
#include "Log.h"

WORD
EngineRealizeFont(LPLOGFONT lplf, LPVOID lpTextxForm, LPVOID lpdfEngine)
/* lpdfEngine	- LPFONTINFO (ddk) */
{
	return 0;
}


int
EngineEnumerateFont(LPSTR lszFace,
		int (FAR PASCAL *lpfn)(LPLOGFONT,LPTEXTMETRIC,int,LPSTR),
		DWORD dw)
{
	return 0;
}


void
EngineDeleteFont(LPVOID lpFontInfo)
{
	return;
}


long
EngineGetGlyphBmp(HDC hDC, LPVOID lpdf, int ch, int iSome, LPSTR lpstr,
		DWORD dwBufSize, LPVOID lpBitmapMetrics)
{
	return 0;
}


void
EngineGetCharWidth(LPVOID lpFontInfo, int i1, int i2, WORD *rgwWidth)
{
	return;
}


void
EngineSetFontContext(LPVOID lpdf, int iEsc)
{
	return;
}

DWORD WINAPI
GetFontData(HDC hDC, DWORD dw, DWORD dwOffset, LPVOID lpBuf, DWORD dwNumBytes)
{
	return 0;
}
