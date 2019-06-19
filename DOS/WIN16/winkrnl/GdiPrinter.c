/*    
	GdiPrinter.c	1.6
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

#include "print.h"
#include "Log.h"
#include "GdiDC.h"
#include "DeviceData.h"

int	WINAPI
Escape(HDC hDC, int nEscape, int cbInput, LPCSTR lpszInData, void *lpvOutData)
{
    HDC32 hDC32;
    int result;
    LSDS_PARAMS argptr;

    APISTR((LF_API,"Escape: %x, hDC %x\n",nEscape,hDC));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.escape.cbInput = cbInput;
    argptr.lsde.escape.lpszInData = (LPCSTR)lpszInData;
    argptr.lsde.escape.lpvOutData = lpvOutData;

    result = (int)DC_OUTPUT(LSD_ESCAPE,hDC32,nEscape,&argptr);
    RELEASEDCINFO(hDC32);
    return result;
}

BOOL WINAPI
QueryAbort(HDC hDC, int reserved)
{
    APISTR((LF_APISTUB,"QueryAbort(HDC=%x,int=%d)\n",hDC,reserved));
    return FALSE;
}

int WINAPI
SetAbortProc(HDC hDC, ABORTPROC lpfnAbortProc)
{
    HDC32 hDC32;
    int result;
    LSDS_PARAMS argptr;

    APISTR((LF_API,"SetAbortProc: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.lpfnabort = lpfnAbortProc;

    result = (int)DC_OUTPUT(LSD_SETABORTPROC,hDC32,0,&argptr);
    RELEASEDCINFO(hDC32);
    return result;
}

int WINAPI
AbortDoc(HDC hDC)
{
    HDC32 hDC32;
    int result;

    APISTR((LF_API,"AbortDoc: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    result = (int)DC_OUTPUT(LSD_ABORTDOC,hDC32,0,0);
    RELEASEDCINFO(hDC32);
    return result;
}

int WINAPI
StartDoc(HDC hDC, DOCINFO *lpdi)
{
    HDC32 hDC32;
    int result;
    LSDS_PARAMS argptr;

    APISTR((LF_API,"StartDoc: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    argptr.lsde_validate.dwInvalidMask = 0;

    argptr.lsde.lpdocinfo = lpdi;

    result = (int)DC_OUTPUT(LSD_STARTDOC,hDC32,0,&argptr);
    RELEASEDCINFO(hDC32);
    return result;
}

int WINAPI
EndDoc(HDC hDC)
{
    HDC32 hDC32;
    int result;

    APISTR((LF_API,"EndDoc: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    result = (int)DC_OUTPUT(LSD_ENDDOC,hDC32,0,0);
    RELEASEDCINFO(hDC32);    
    return result;
}

int WINAPI
StartPage(HDC hDC)
{
    HDC32 hDC32;
    int result;

    APISTR((LF_API,"StartPage: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    result = (int)DC_OUTPUT(LSD_STARTPAGE,hDC32,0,0);
    RELEASEDCINFO(hDC32);
    return result;
}

int WINAPI
EndPage(HDC hDC)
{
    HDC32 hDC32;
    int result;

    APISTR((LF_API,"EndPage: hDC %x\n",hDC));

    ASSERT_HDC(hDC32,hDC,0);

    result = (int)DC_OUTPUT(LSD_ENDPAGE,hDC32,0,0);
    RELEASEDCINFO(hDC32);
    return result;
}
