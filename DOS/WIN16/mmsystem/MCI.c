/*    
	MCI.c	1.2
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
/*
#include "mmsystem.h"
*/
#define __MMSYSTEM_H__ 
#include "Willows.h"
#include "Log.h"

#ifndef MCIERROR
#define MCIERROR DWORD
#endif

#ifndef MCIDEVICEID
#define MCIDEVICEID UINT
#endif

MCIERROR WINAPI
mciSendCommand(MCIDEVICEID IDDevice, UINT uMsg, DWORD dwCmd, DWORD dwParam)
{
    APISTR((LF_APISTUB,
	"mciSendCommand(MCIDEVICEID=%x,UINT=%x,DWORD=%x,DWORD=%x)\n",
	IDDevice, uMsg, dwCmd, dwParam));
    return 0;
}

MCIERROR WINAPI
mciSendString(LPCTSTR lpszCommand, LPTSTR lpszReturnString, UINT cchReturn, HANDLE hwndCallback)
{
    APISTR((LF_APISTUB,
	"mciSendString(LPCTSTR=%s,LPTSTR=%s,UINT=%x,HANDLE=%x)\n",
	lpszCommand, lpszReturnString, cchReturn, hwndCallback));
    return 0;
}

MCIDEVICEID WINAPI
mciGetDeviceID(LPCTSTR lpszDevice)
{
    APISTR((LF_APISTUB,"mciGetDeviceID(LPCTSTR=%s)",lpszDevice));
    return 0;
}
