/*
	@(#)DrvHook.c	1.3
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
#include "Log.h"

#include "DrvHook.h"

DWORD
PrivateDriverHook(DWORD dwCode, LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
    switch (HIWORD(dwCode)) {
	case DSUB_INITDRIVER:
	case DSUB_INITSYSTEM:
	    return (DWORD)PrivateInitDriver(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);

	case DSUB_REGIONS:
	    return (DWORD)PrivateRegionsHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);

	case DSUB_IMAGES:
	    return (DWORD)PrivateImageHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);

	case DSUB_GRAPHICS:
	    return (DWORD)PrivateGraphicsHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);

	case DSUB_DC:
	    return (DWORD)PrivateDCHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);

	case DSUB_TEXT:
	    return (DWORD)PrivateTextHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);

	case DSUB_WINDOWS:
	    return (DWORD)PrivateWindowsHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);

	case DSUB_IPC:
	    return (DWORD)PrivateIPCHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);

	case DSUB_CURSORS:
	    return (DWORD)PrivateCursorHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);

	case DSUB_KEYBOARD:
	    return (DWORD)PrivateKeyboardHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);

	case DSUB_COLORS:
	    return (DWORD)PrivateColorsHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);

	case DSUB_EVENTS:
	    return (DWORD)PrivateEventHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);

	case DSUB_WINSOCK:
	    return (DWORD)PrivateWinSockHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);
				
	case DSUB_CONFIG:
	    return (DWORD)PrivateConfigurationHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);
				
	case DSUB_TIME:
	    return (DWORD)PrivateTimeHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);
				
	case DSUB_LICENSE:
	    return (DWORD)PrivateLicenseHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);
	
	case DSUB_PRINTER:
	    return (DWORD)PrivatePrinterHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);
			
	case DSUB_FILES:
	    return (DWORD)PrivateFilesHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);

	case DSUB_MEMORY:
		return (DWORD)PrivateMemoryHook(LOWORD(dwCode),
				dwParm1,dwParm2,lpStruct);
				
	default:
	    return 0;
    }
}

