/*    
	Ver.c	2.8
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


void
NewTrap(char *name)
{
	int	rc;
	char	buf[128];

	sprintf(buf, "Unimplemented API: %s\n\nPress OK to continue execution,\npress Cancel to quit application", name);
	rc = MessageBox((HWND)NULL, buf, "Trap", MB_ICONEXCLAMATION | MB_OKCANCEL);
	if ( rc == IDCANCEL )
		FatalExit(0);
}


DWORD WINAPI
GetFileResourceSize(LPCSTR lpstrFilename, LPCSTR lpstrResType,
      LPCSTR lpstrResID, DWORD FAR *lpdwFileOffset)
{
	NewTrap("GetFileResourceSize");
	return 0;
}


BOOL WINAPI
GetFileResource(LPCSTR lpstrFilename, LPCSTR lpstrResType,
      LPCSTR lpstrResID, DWORD dwFileOffset, DWORD dwResLen,
      void FAR* lpData)
{
	NewTrap("GetFileResource");
	return 0;
}

DWORD WINAPI
GetFileVersionInfoSize( LPSTR lptstrFilename, LPDWORD lpdwHandle )
{
	NewTrap("GetFileVersionInfoSize");
	return 0;
}

BOOL WINAPI
GetFileVersionInfo( LPSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData )
{
	NewTrap("GetFileVersionInfo");
	return 0;
}

DWORD WINAPI
VerFindFile(
        DWORD uFlags,
        LPSTR szFileName,
        LPSTR szWinDir,
        LPSTR szAppDir,
        LPSTR szCurDir,
        PUINT lpuCurDirLen,
        LPSTR szDestDir,
        PUINT lpuDestDirLen
        )
{
	NewTrap("VerFindFile");
	return 0;
}

DWORD WINAPI
VerInstallFile
(
  DWORD uFlags,
  LPSTR szSrcFileName,
  LPSTR szDestFileName,
  LPSTR szSrcDir,
  LPSTR szDestDir,
  LPSTR szCurDir,
  LPSTR szTmpFile,
  PUINT lpuTmpFileLen
)
{
	NewTrap("VerInstallFile");
	return 0;
}

DWORD WINAPI
VerLanguageName( DWORD wLang, LPSTR szLang, DWORD nSize )
{
	NewTrap("VerLanguageName");
	return 0;
}


BOOL WINAPI
VerQueryValue
(
  const LPVOID pBlock,
  LPSTR lpSubBlock,
  LPVOID * lplpBuffer,
  PUINT puLen
)
{
	NewTrap("VerQueryValue");
	return 0;
}

