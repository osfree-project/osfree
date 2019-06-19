/********************************************************************

	@(#)DrvSystem.c	1.43   Unix drivers system subsystem implementation.
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
 

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "platform.h"

#include "Driver.h"

#if !defined(NETWARE) 
#include <unistd.h>
#endif

#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "windows.h"
#include "Log.h"
#include "DrvThreads.h"

#define	SYSERR_MEMORY	4

extern void TWIN_SystemError(UINT,DWORD,DWORD,DWORD);
static DWORD DrvSystemAllocMem(LPARAM,LPARAM,LPVOID);
static DWORD DrvSystemReallocMem(LPARAM,LPARAM,LPVOID);
static DWORD DrvSystemFreeMem(LPARAM,LPARAM,LPVOID);
static DWORD DrvSystemMakeExeData(LPARAM,LPARAM,LPVOID);
static DWORD DrvSystemLoadLibrary(LPARAM,LPARAM,LPVOID);
static DWORD DrvSystemSleep(LPARAM,LPARAM,LPVOID);
static DWORD DrvSystemDoNothing(LPARAM,LPARAM,LPVOID);
DWORD DrvSystemTab(void);

static DWORD DrvMakeExeData(LPVOID,DWORD);
static int DrvLoadLibrary(char *);
static char * DrvGetShlibExt(void);
static DWORD DrvSleep(DWORD);

static TWINDRVSUBPROC DrvSystemEntryTab[] = {
	DrvSystemDoNothing,
	DrvSystemDoNothing,
	DrvSystemDoNothing,
	DrvSystemAllocMem,
	DrvSystemReallocMem,
	DrvSystemFreeMem,
	TWIN_DrvCreateThread,	
	TWIN_DrvFreeThread,
	TWIN_DrvYieldToThread,
	TWIN_DrvCanDoThreads,
	TWIN_DrvGetMainThread,
	DrvSystemMakeExeData,
	DrvSystemDoNothing,
	TWIN_DrvCreatePSDKey,
	TWIN_DrvDeletePSDKey,
	TWIN_DrvSetPSD,
	TWIN_DrvGetPSD,
	DrvSystemLoadLibrary,
	DrvSystemSleep
};

static DWORD
DrvMakeExeData(LPVOID lpData,DWORD dwLength)
{
#ifdef TWIN_RISC_CPU
    SYNC_CODE_DATA;
#endif

    return 1L;
}

/********************************************************************
*   DrvGetCommandLine
*
*	This is stubbed here for compatibility with the Mac version
********************************************************************/
int DrvGetCommandLine(int argc, char ***argv)
{
	return(argc);
}


#if defined(TWIN_HASDLFCN)
#include <dlfcn.h>
#endif

static int 
DrvLoadLibrary(char *lpszLibFileName)
{
    char      LibraryPath[256];
    char      LibraryFile[256];
    char      LibraryFullName[256];
    char      EntryName[128];
    void     *hSO;
    char      *s;

    /*
     *  Split the file into path and filename components.  The
     *  path might be null at this point.
     */
    s = strrchr (lpszLibFileName, '/');
    if (s == NULL)
    {
	/*
	 *  No path info, entire string is the filename.
	 */
	strcpy(LibraryPath, "");
	strcpy(LibraryFile, lpszLibFileName);
    }
    else
    {
	/*
	 *  Copy the info in as the path, and set the null terminator
	 *  immediately following the found '/'.  Set the filename
	 *  portion to be the part after the '/'.
	 */
	strcpy(LibraryPath, lpszLibFileName);
	LibraryPath[(s - (char *)lpszLibFileName) + 1] = 0;
	strcpy(LibraryFile, s+1);
    }

    /*
     *  Convert the filename to lowercase.  Probably not the
     *  optimal thing to do, but we need to handle getting uppercase
     *  module names at this point, so enforcing a "lowercase only"
     *  policy is a moderately ok way to do this.  Do not touch
     *  the case of the path, if it exists.  Only the filename itself
     *  is modified at this point.
     */
    strlwr(LibraryFile);

    /*
     *  Look for an extension.  We only munge the filename if the
     *  extension is .dll, or there is no extension.  Other names
     *  are left untouched.
     */
    s = strrchr (LibraryFile, '.');
    if (s)
    {
	if (strcmp(s, ".dll") == 0)
	{
	    /*
	     *  A .dll extension has been found, so remove it.  Set
	     *  the pointer to NULL to indicate that there is no longer
	     *  an extension.
	     */
	    *s = 0;
	    s = NULL;
	}
    }

    /*
     *  If there is still an extension on the file, then we do not
     *  munge the name --- revert to the original for the full name.
     *  Otherwise, we prepend "lib" to the filename, append "32" for
     *  the 32-bit version of the library, and append the system-specific
     *  shared-library extension.
     */
    if (s)
    {

	strcpy(LibraryFullName, lpszLibFileName);
	strcpy(EntryName, LibraryFile);
    }
    else
    {
	strcpy(LibraryFullName, LibraryPath);
	strcat(LibraryFullName, "lib");
	strcat(LibraryFullName, LibraryFile);
#ifdef TWIN32
	strcat(LibraryFullName, "32");
#endif
	strcat(LibraryFullName, DrvGetShlibExt());
	strcpy(EntryName, "TWIN_LibEntry_lib");
	strcat(EntryName, LibraryFile);
    }

    hSO = dlopen(LibraryFullName,RTLD_LAZY);
    if (hSO == 0)
    {
	return (FALSE);
    }

    /* LoadLibrary should continue to initialize  */
    /* and not try to load a filename.dll         */
    return TRUE;
}


static char *
DrvGetShlibExt()
{
    /*
     *  Return a pointer to a string with the extension used on this
     *  platform for shared libraries.
     */
    return(".so");
}

static DWORD
DrvSleep(DWORD sleeptime)
{
    struct timeval tv;

    /*
     *  Need to sleep the given number of milliseconds.  We do that by
     *  using select() with no file descriptors, and an appropriate
     *  timeout.  This is not thread-safe, and ultimately should not
     *  be used when thread-safe behaviour is needed.
     */
    tv.tv_sec = sleeptime / 1000;
    tv.tv_usec = (sleeptime % 1000) * 1000;  /* Cvt residual msec to usec */

    select(0, NULL, NULL, NULL, &tv);
    return(0);
}

static DWORD
DrvSystemAllocMem(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	LPVOID lpMem = (LPVOID)WinMalloc(dwParam1);
	if (lpMem == NULL)
		TWIN_SystemError(0,SYSERR_MEMORY,3,dwParam1);
	return (DWORD)lpMem;
}

static DWORD
DrvSystemReallocMem(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	LPVOID lpMem = (LPVOID)WinRealloc((LPSTR)lpStruct,dwParam1);
	if (lpMem == NULL)
		TWIN_SystemError(0,SYSERR_MEMORY,4,dwParam1);
	return (DWORD)lpMem;
}

static DWORD
DrvSystemFreeMem(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	WinFree((LPSTR)lpStruct);
	return 1L;
}

static DWORD
DrvSystemMakeExeData(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return DrvMakeExeData((LPVOID)lpStruct,dwParam1);
}

static DWORD
DrvSystemLoadLibrary(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return DrvLoadLibrary((char *)lpStruct);
}

static DWORD
DrvSystemSleep(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return DrvSleep(dwParam1);
}

DWORD
DrvSystemTab(void)
{
#if defined(TWIN_RUNTIME_DRVTAB)
        DrvSystemEntryTab[0] = DrvSystemDoNothing;
        DrvSystemEntryTab[1] = DrvSystemDoNothing;
        DrvSystemEntryTab[2] = DrvSystemDoNothing;
        DrvSystemEntryTab[3] = DrvSystemAllocMem;
        DrvSystemEntryTab[4] = DrvSystemReallocMem;
        DrvSystemEntryTab[5] = DrvSystemFreeMem;
        DrvSystemEntryTab[6] = TWIN_DrvCreateThread;
        DrvSystemEntryTab[7] = TWIN_DrvFreeThread;
        DrvSystemEntryTab[8] = TWIN_DrvYieldToThread;
        DrvSystemEntryTab[9] = TWIN_DrvCanDoThreads;
        DrvSystemEntryTab[10] = TWIN_DrvGetMainThread;
        DrvSystemEntryTab[11] = DrvSystemMakeExeData;
        DrvSystemEntryTab[12] = DrvSystemDoNothing;
        DrvSystemEntryTab[13] = TWIN_DrvCreatePSDKey;
        DrvSystemEntryTab[14] = TWIN_DrvDeletePSDKey;
        DrvSystemEntryTab[15] = TWIN_DrvSetPSD;
        DrvSystemEntryTab[16] = TWIN_DrvGetPSD;
        DrvSystemEntryTab[17] = DrvSystemLoadLibrary;
        DrvSystemEntryTab[18] = DrvSystemSleep;
#endif
	return (DWORD)DrvSystemEntryTab;
}

static DWORD
DrvSystemDoNothing(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 1L;
}
