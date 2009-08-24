/********************************************************************

	@(#)DrvThreads_nothreads.c	1.2
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
 
#include "windows.h"
#include "Log.h"
#include "DrvThreads.h"
#include "Driver.h"

DWORD
TWIN_DrvCreateThread(LPARAM dwParam1, LPARAM  dwParam2, LPVOID lpStruct)
{
	return 0L;
}

DWORD 
TWIN_DrvFreeThread(LPARAM dwParam1, LPARAM  dwParam2, LPVOID lpStruct)
{
        return 0L;
}

DWORD 
TWIN_DrvYieldToThread(LPARAM dwParam1, LPARAM  dwParam2, LPVOID lpStruct)
{
        return 0L;
}

DWORD 
TWIN_DrvCreatePSDKey(LPARAM dwParam1, LPARAM  dwParam2, LPVOID lpStruct)
{
        return 0L;
}

DWORD 
TWIN_DrvDeletePSDKey(LPARAM dwParam1, LPARAM  dwParam2, LPVOID lpStruct)
{
        return 0L;
}

DWORD 
TWIN_DrvSetPSD(LPARAM dwParam1, LPARAM  dwParam2, LPVOID lpStruct)
{
        return 0L;
}

DWORD 
TWIN_DrvGetPSD(LPARAM dwParam1, LPARAM  dwParam2, LPVOID lpStruct)
{
        return 0L;
}

DWORD 
TWIN_DrvCanDoThreads(LPARAM dwParam1, LPARAM  dwParam2, LPVOID lpStruct)
{
        return 0L;
}

DWORD 
TWIN_DrvGetMainThread(LPARAM dwParam1, LPARAM  dwParam2, LPVOID lpStruct)
{
        return 0L;
}

