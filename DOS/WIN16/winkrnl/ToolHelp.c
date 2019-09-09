/*    
	ToolHelp.c	2.10
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
/*
#include "toolhelp.h"
*/
#define __TOOLHELP_H__
#include "Willows.h"

#include "kerndef.h"
#include "Kernel.h"
#include "Resources.h"
#include "Module.h"
#include "Log.h"

#include <stdio.h>
#include <string.h>


extern LPOBJHEAD lpModuleHead;

static BOOL FillModuleEntry(MODULEENTRY *,MODULEINFO *);

BOOL WINAPI
MemManInfo(MEMMANINFO *lpmmi)
{
    DWORD dwSize;

    LOGSTR((LF_APISTUB,"MemManInfo: \n"));

    if (lpmmi) {
	dwSize = 2*sizeof(DWORD);

	if (dwSize >= lpmmi->dwSize)
	    return FALSE;
	dwSize += sizeof(DWORD);
	lpmmi->dwLargestFreeBlock = 0;

	if (dwSize >= lpmmi->dwSize)
	    return TRUE;
	dwSize += sizeof(DWORD);
	lpmmi->dwMaxPagesAvailable = 0x1000;

	if (dwSize >= lpmmi->dwSize)
	    return TRUE;
	dwSize += sizeof(DWORD);
	lpmmi->dwMaxPagesLockable = 0;

	if (dwSize >= lpmmi->dwSize)
	    return TRUE;
	dwSize += sizeof(DWORD);
	lpmmi->dwTotalLinearSpace = 0;

	if (dwSize >= lpmmi->dwSize)
	    return TRUE;
	dwSize += sizeof(DWORD);
	lpmmi->dwTotalUnlockedPages = 0;

	if (dwSize >= lpmmi->dwSize)
	    return TRUE;
	dwSize += sizeof(DWORD);
	lpmmi->dwFreePages = 0;

	if (dwSize >= lpmmi->dwSize)
	    return TRUE;
	dwSize += sizeof(DWORD);
	lpmmi->dwTotalPages = 0;

	if (dwSize >= lpmmi->dwSize)
	    return TRUE;
	dwSize += sizeof(DWORD);
	lpmmi->dwFreeLinearSpace = 0;

	if (dwSize >= lpmmi->dwSize)
	    return TRUE;
	dwSize += sizeof(WORD);
	lpmmi->dwSwapFilePages = 0;

	if (dwSize >= lpmmi->dwSize)
	    return TRUE;
	lpmmi->wPageSize = 0x1000;
	return TRUE;
    }
    return FALSE;
}

BOOL WINAPI
NotifyRegister(HTASK hTask, LPFNNOTIFYCALLBACK lpfnCallback, WORD wFlags)
{
    LOGSTR((LF_APISTUB,"NotifyRegister: hTask %x callback %x flags %x\n",
	hTask,lpfnCallback,wFlags));
    return TRUE;
}

BOOL WINAPI
NotifyUnRegister(HTASK hTask)
{
    LOGSTR((LF_APISTUB,"NotifyUnRegister: hTask %x\n",hTask));
    return TRUE;
}

BOOL WINAPI
InterruptRegister(HTASK hTask, FARPROC lpfnIntCallback)
{
    LOGSTR((LF_APISTUB,"InterruptRegister: hTask %x callback %x\n",
	hTask,lpfnIntCallback));
    return TRUE;
}

BOOL WINAPI
InterruptUnRegister(HTASK hTask)
{
    LOGSTR((LF_APISTUB,"InterruptUnRegister: hTask %x\n",hTask));
    return TRUE;
}

BOOL WINAPI
ModuleFirst(MODULEENTRY *lpme)
{
    BOOL retcode;

    retcode = FillModuleEntry(lpme,(MODULEINFO *)lpModuleHead);
    LOGSTR((LF_LOG,"ModuleFirst: struct size %x, we need %x\n", 
	lpme->dwSize, (unsigned int) sizeof(MODULEENTRY)));

    return retcode;
}

BOOL WINAPI
ModuleNext(MODULEENTRY *lpme)
{
    BOOL retcode = 0;

    if (lpme && lpme->wNext) {
        LPMODULEINFO modinfo = GETMODULEINFO((lpme->wNext));
	retcode = FillModuleEntry(lpme,modinfo);
	RELEASEMODULEINFO(modinfo);
    }
    return retcode;
}

HMODULE WINAPI
ModuleFindHandle(MODULEENTRY *lpme,HMODULE hModule)
{
  return(TRUE);
}

HMODULE WINAPI
ModuleFindName(MODULEENTRY *lpme,LPCSTR lpszName)
{
  return(TRUE);
}

static BOOL
FillModuleEntry(MODULEENTRY *lpme,MODULEINFO *modinfo)
{
    OBJHEAD *lpObjNext;

    if (!modinfo || !lpme)
	return FALSE;
    lpme->dwSize = sizeof(MODULEENTRY);
    if (modinfo->lpModuleName)
	strcpy(&lpme->szModule[0],modinfo->lpModuleName);
    lpme->hModule = modinfo->ObjHead.hObj;
    lpme->wcUsage = 1;	/* TODO LATER -- support reference count */
    if (modinfo->lpFileName)
	strcpy(&lpme->szExePath[0],modinfo->lpFileName);
    if ((lpObjNext = (LPOBJHEAD)modinfo->ObjHead.lpObjNext))
	lpme->wNext = (WORD)lpObjNext->hObj;
    else
	lpme->wNext = 0;
    return TRUE;
}

BOOL WINAPI
GlobalEntryHandle(GLOBALENTRY *lpge, HGLOBAL hGlobal)
{
    if (!lpge)
	return FALSE;

    LOGSTR((LF_APISTUB,"GlobalEntryHandle: hGlobal %x\n",hGlobal));

    if (lpge->dwSize != sizeof(GLOBALENTRY))
	return FALSE;

    memset((LPSTR)lpge,'\0', sizeof(GLOBALENTRY));
    lpge->dwSize = sizeof(GLOBALENTRY);

#ifdef	LATER
    /* return all correct values */
#endif
    return TRUE;
}

BOOL WINAPI
TimerCount(TIMERINFO *lpti)
{
    if (!lpti)
	return FALSE;

    LOGSTR((LF_APISTUB,"TimerCount: \n"));

    return FALSE;
}
