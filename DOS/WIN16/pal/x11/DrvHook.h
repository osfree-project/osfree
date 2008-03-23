/*
	@(#)DrvHook.h	1.5
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
 
#ifndef DrvHook__h
#define DrvHook__h

#include "windows.h"

#include "Driver.h"

DWORD PrivateDriverHook(DWORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateSystemHook(DWORD, LPARAM, LPARAM, LPVOID);

DWORD PrivateRegionsHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateImageHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateGraphicsHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateDCHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateTextHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateWindowsHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateCommHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateIPCHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateCursorHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateKeyboardHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateColorsHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateEventHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateInitDriver(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateWinSockHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivatePrinterHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateConfigurationHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateTimeHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateLicenseHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateFilesHook(WORD, LPARAM, LPARAM, LPVOID);
DWORD PrivateMemoryHook(WORD, LPARAM, LPARAM, LPVOID);

#endif /* DrvHook__h */
