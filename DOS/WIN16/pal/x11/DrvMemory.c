/********************************************************************

 	@(#)DrvMemory.c	1.5  Memory Management Module
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

********************************************************************/
 
#include "Driver.h"
#define SYSERR_MEMORY 4

DWORD DrvMemoryTab(void);

void TWIN_SystemError(int,int,int,LPARAM);

static DWORD
DrvMemoryAllocMem(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	LPVOID lpMem;

	lpMem = (LPVOID)WinMalloc(dwParam1);
        if (lpMem == NULL)
                TWIN_SystemError(0,SYSERR_MEMORY,3,dwParam1);
        return (DWORD)lpMem;
}

static DWORD 
DrvMemoryReallocMem(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
 	LPVOID lpMem;

	lpMem = (LPVOID)WinRealloc((LPSTR)lpStruct,dwParam1);
	if (lpMem == NULL)
		TWIN_SystemError(0,SYSERR_MEMORY,4,dwParam1);
	return (DWORD)lpMem;
}

static DWORD
DrvMemoryFreeMem(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	WinFree((LPSTR)lpStruct);
	return 1L;
}

static DWORD
DrvMemoryDoNothing(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 0L;
}

static TWINDRVSUBPROC DrvMemoryEntryTab[] = {
	DrvMemoryDoNothing,
	DrvMemoryDoNothing,
	DrvMemoryDoNothing,
	DrvMemoryAllocMem,
	DrvMemoryFreeMem,
	DrvMemoryReallocMem
};

DWORD
DrvMemoryTab(void)
{
#if defined(TWIN_RUNTIME_DRVTAB)
        DrvMemoryEntryTab[0] = DrvMemoryDoNothing;
        DrvMemoryEntryTab[1] = DrvMemoryDoNothing;
        DrvMemoryEntryTab[2] = DrvMemoryDoNothing;
        DrvMemoryEntryTab[3] = DrvMemoryAllocMem;
        DrvMemoryEntryTab[4] = DrvMemoryFreeMem;
        DrvMemoryEntryTab[5] = DrvMemoryReallocMem;
#endif
	return (DWORD)DrvMemoryEntryTab;
}
