/*    
	KrnFileIO.c	1.12
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

#include "Log.h"
#include "mfs_config.h"
#include "FileSys.h"

DWORD WINAPI
FileCdr(FARPROC lpfnCdrProc)
{
    APISTR((LF_APISTUB,"FileCdr(FARPROC=%x)\n",lpfnCdrProc));
    return (DWORD)TRUE;
}

int WINAPI
OpenFile(LPCSTR lpszFileName,OFSTRUCT *lpofs,UINT wFunction)
{
    int   fd;
    const char *lpstr = lpszFileName;

    /*
     *  The "lpstr" variable is needed as a workaround to a bug
     *  in the SunOS "acc" compiler, which treats the LPCSTR typedef
     *  as type "const char * const" rather than the requested
     *  "const char *".
     */

    APISTR((LF_APICALL,"OpenFile(LPCSTR=%s,OFSTRUCT=%p,UINT=%x)\n",
	lpszFileName,lpofs,wFunction));

    if ((wFunction & OF_REOPEN)) {
	lpstr = lpofs->szPathName;
    }
    if (!lpstr) {
           APISTR((LF_APIFAIL,"OpenFile: returns int HFILE_ERROR\n"));
           return HFILE_ERROR;
    }

    if(wFunction & OF_PARSE) {
	memset((LPSTR)lpofs,0,sizeof(OFSTRUCT));
	MFS_ALTPATH(lpofs->szPathName, lpstr);
        APISTR((LF_APIFAIL,"OpenFile: returns int 0\n"));
	return 0;
    }

    lpofs->fFixedDisk = 1;

    if(wFunction & OF_DELETE) {
	fd = _ldelete(lpstr);
	if(fd == 0) {
          APISTR((LF_APIRET,"OpenFile: returns int 1\n"));
	  return 1;
	} else {
          APISTR((LF_APIFAIL,"OpenFile: returns int HFILE_ERROR\n"));
	  return HFILE_ERROR;
	}
    }

    if(wFunction & OF_CREATE)
	fd = _lcreat(lpstr, 0);
    else
	fd = OpenEngine(lpstr,lpofs,wFunction);		

#ifdef LATER
    if(fd < 0) {
	if((wFunction & (OF_PROMPT|OF_CREATE)) == OF_PROMPT) {
	    printf("prompting user for file %s cancel...\n",
					wFunction&OF_CANCEL?"w/":"w/o ");
	}
    }
    if((wFunction & (OF_READ|OF_VERIFY)) == (OF_READ|OF_VERIFY)) {
	printf("checking for date/time...\n");
    }
#endif

    if(fd < 0) {
    	APISTR((LF_APIFAIL,"OpenFile: returns int %d\n",fd));
	return fd;	
    }

    if(wFunction & OF_EXIST)
	_lclose(fd);

    APISTR((LF_APIRET,"OpenFile: returns int %d\n",fd));
    return fd;
}
