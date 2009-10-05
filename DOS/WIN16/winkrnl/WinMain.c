/*    
	WinMain.c	1.17
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
#include "windowsx.h"
#include "commdlg.h"

#include "Log.h"
#include "kerndef.h"
#include "Resources.h"
#include "Module.h"

/* This is to determine whether this object file has been linked in */
BOOL bIsXWin = TRUE;

extern LPSTR strpbrkr(LPCSTR,LPCSTR);
extern HANDLE InternalLoadLibrary(WORD,LPSTR,LPARAM);
extern short WINAPI WSysGetExe(HWND, LPSTR, int);


extern int __argc;
extern char **__argv;

int
WinMain
(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, int nCmdShow)
{
    int    rc;
    LPSTR  lpszFileName=0,lpszTemp;
    char	szFile[256];

    APISTR((LF_APICALL,"WinMain(HINSTANCE=%x,HINSTANCE=%x,LPSTR=%s,int=%x)\n",
	hInstance,hPrev,lpCmdLine,nCmdShow));
	
    memset ( szFile, 0, sizeof ( szFile ) );

    if (__argc > 1) 
	{
	    lpszFileName = WinMalloc(lstrlen(__argv[1])+5);
	    lstrcpy(lpszFileName,__argv[1]);
	    lpszTemp = strpbrkr(lpszFileName,"/");
	    lpszTemp = (lpszTemp)?lpszTemp+1:lpszFileName;
	    if (!strchr(lpszTemp,'.'))
		lstrcat(lpszFileName,".exe");
	}
    else {
	lpszFileName = szFile;
	if ( WSysGetExe ( 0, lpszFileName, sizeof ( szFile ) ) != IDOK )
		return ( 0 );
    }

    rc = InternalLoadLibrary(ILL_BINARY|ILL_APPL|ILL_FILENAME|ILL_NOFORK,
					lpCmdLine,(LPARAM)lpszFileName);

    APISTR((LF_APIRET,"WinMain: returns int %d\n",rc));
    return 0;
}

