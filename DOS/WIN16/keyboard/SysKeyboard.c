/*    
	SysKeyboard.c	1.6
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

#include "Log.h"
#include "Driver.h"

int WINAPI
GetKeyboardType(int fnKeyboard)
{
    return (int)DRVCALL_KEYBOARD(PKH_GETTYPE,fnKeyboard,0,0);
}

int WINAPI
GetKBCodePage(void)
{
    return (int)DRVCALL_KEYBOARD(PKH_CODEPAGE,0,0,0);
}

UINT WINAPI
MapVirtualKey(UINT uKeyCode, UINT fuMapType)
{
    UINT retcode;

    retcode = (UINT)DRVCALL_KEYBOARD(PKH_MAPKEY,uKeyCode,fuMapType,0);

    APISTR((LF_API,"MapVirtualKey: %x type %x returned %x\n",
	uKeyCode,fuMapType,retcode));

    return retcode;
}

int WINAPI
ToAscii(UINT uVirtKey, UINT uScanCode, BYTE *lpbKeyState,
	DWORD *lpdwTransKey, UINT fuState)
{
    LPSTR lpKeyBuffer = (LPSTR)lpdwTransKey;

    LOGSTR((LF_API,"ToAscii STUB: (%x,%x,%x,%x,%x)\n",
	uVirtKey,uScanCode,lpbKeyState,lpdwTransKey,fuState));

    /* virtual key is ascii, but need to account for shift keys... */
    *lpKeyBuffer = uVirtKey;

    return 1;
}

int WINAPI
GetKeyNameText(LONG lParam, LPSTR lpszBuffer, int cbMaxKey)
{
    if (!lpszBuffer || cbMaxKey <= 0)
	return 0;

    (void)DRVCALL_KEYBOARD(PKH_KEYTEXT,lParam,cbMaxKey,lpszBuffer);

    return strlen(lpszBuffer);
}

