/*    
	Net.c	2.4
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
#include "Net.h"
#include "Log.h"

UINT WINAPI
WNetAddConnection(LPSTR lpszNetPath, LPSTR lpszPassword, LPSTR lpszLocalName)
{
    LOGSTR((LF_LOG,"WNetAddConnection: STUB local %s\n", lpszLocalName));
    return WN_NET_ERROR;
}

UINT WNetCancelConnection(LPSTR lpszName, BOOL fForce)
{
    LOGSTR((LF_LOG,"WNetCancelConnect: STUB local %s\n", lpszName));
    return WN_NET_ERROR;
}

UINT WINAPI
WNetGetConnection(LPSTR lpszLocalName, LPSTR lpszRemoteName,
		  UINT *lpcbRemoteName)
{
    LOGSTR((LF_LOG,"WNetGetConnection: STUB local %s\n",lpszLocalName));
    return WN_NET_ERROR;
}

UINT WINAPI
WNetGetCaps(UINT uiMask)
{
    LOGSTR((LF_API,"WNetGetCaps: STUB index %x\n",uiMask));
    switch (uiMask) {
	case WNNC_SPEC_VERSION:
	    return (UINT)(LOWORD(GetVersion()));
	case WNNC_NET_TYPE:
	    return WNNC_NET_None;
	case WNNC_DRIVER_VERSION:
	case WNNC_USER:
	case WNNC_CONNECTION:
	case WNNC_PRINTING:
	case WNNC_DEVMODE:
	case WNNC_ERROR:
	case WNNC_BROWSE:
	default:
	    return 0;
    }
}

WORD WINAPI
WNetGetUser(LPSTR lpszName, LPWORD lpwLen)
{
    LOGSTR((LF_API,"WNetGetUser: STUB\n"));
    return WN_NOT_SUPPORTED;
}
