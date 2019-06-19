/*    
	Kernel.c	2.54
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
#include <stdlib.h>
#include <string.h>

#include "windows.h"

#include "WinDefs.h"
#include "kerndef.h"
#include "Resources.h"
#include "Module.h"
#include "Kernel.h"
#include "Driver.h"
#include "Log.h"
#include "WinMalloc.h"

/******************************************************************************/

LPQUEUE AllocQueue(int);

/* external functions */
extern BOOL MenuMouseMove(LPMSG);
extern BOOL TWIN_ForwardButtonEvent(LPMSG);
extern BOOL HaveDisabledAncestor(HWND);

/*****************************************************************************/

LPOBJHEAD lpModuleHead = (LPOBJHEAD)NULL;
LPQUEUE  SystemQueue;

extern MODULEINFO *lpModuleTable[];

static BYTE bModIndex;

/*****************/
/* API functions */
/*****************/

LONG WINAPI
SetSwapAreaSize(UINT uiSize)
{
    APISTR((LF_APISTUB,"SetSwapAreaSize(UINT=%x)\n", uiSize));
    return MAKELONG((uiSize)?uiSize:0x8000,0xc000);
}

BOOL WINAPI
DefineHandleTable(WORD wOffset)
{
    APISTR((LF_APISTUB,"DefineHandleTable(WORD=%x)\n", wOffset));
    return TRUE;
}

UINT WINAPI
RegisterWindowMessage(LPCSTR lpszMessage)
{
    UINT rc;
    APISTR((LF_APICALL,"RegisterWindowMessage(LPCSTR=%s)\n", lpszMessage));
    rc = GlobalAddAtom(lpszMessage);
    APISTR((LF_APIRET,"RegisterWindowMessage: returns UINT %x\n",rc));
    return rc;
}

FARPROC WINAPI
MakeProcInstance(FARPROC lpproc, HINSTANCE hInstance)
{
    APISTR((LF_APICALL,"MakeProcInstance(FARPROC=%x,HINSTANCE=%x)\n", 
	lpproc,hInstance));
    APISTR((LF_APIRET,"MakeProcInstance: returns FARPROC %x\n",lpproc));
    return lpproc;	
}

void WINAPI
FreeProcInstance(FARPROC lpproc)
{
    APISTR((LF_APICALL,"FreeProcInstance(FARPROC=%x)\n", lpproc));
    APISTR((LF_APIRET,"FreeProcInstance: returns void\n"));
    return;
}

BOOL  WINAPI
SetMessageQueue(int cMsg)
{
    LPTASKINFO lpTaskInfo;
    LPQUEUE    lpQueue;
    BOOL       bResult;

    if (!(lpTaskInfo = GETTASKINFO(GetCurrentTask()))) {
	return FALSE;
    }
    if(lpTaskInfo->lpQueue)
    	WinFree((char *)(lpTaskInfo->lpQueue));

    lpQueue = AllocQueue(cMsg);
    bResult = (BOOL) ((lpTaskInfo->lpQueue = lpQueue) != 0);
    RELEASETASKINFO(lpTaskInfo);
    return bResult;
}

BOOL WINAPI
IsBadStringPtr(const void *lpsz, UINT cchMax)
{
    APISTR((LF_APISTUB,"IsBadStringPtr(void * %p,UINT=%d)\n", lpsz, cchMax));
    return FALSE;
}

BOOL WINAPI
IsBadReadPtr(const void *lp, UINT cb)
{
    APISTR((LF_APISTUB,"IsBadReadPtr(void * %p,UINT=%d)\n", lp, cb));
    return FALSE;
}

BOOL WINAPI
IsBadWritePtr(void *lp, UINT cb)
{
    APISTR((LF_APISTUB,"IsBadWritePtr(void * %p,UINT=%d)\n", lp, cb));
    return FALSE;
}

VOID WINAPI
Sleep(DWORD sleeptime)
{
    APISTR((LF_APICALL, "Sleep(DWORD=%d)\n",sleeptime));

    DRVCALL_SYSTEM(PSSH_SLEEP,sleeptime,0,0);

    APISTR((LF_APIRET, "Sleep: returns void\n"));
    return;
}

/* Supplemental functions */

HMODULE
CreateModuleInstance(LPSTR lpszModuleName)
{
    LPMODULEINFO lpModuleTemp;
    HMODULE hModule;

    if (!(lpModuleTemp = CREATEHMODULE(hModule))) {
	return (HMODULE)0;
    }

    lpModuleTemp->lpModuleName = WinMalloc(strlen(lpszModuleName)+1);
    strcpy(lpModuleTemp->lpModuleName, lpszModuleName);

    lpModuleTemp->atmModuleName = AddAtom(lpszModuleName);
    lpModuleTable[++bModIndex] = lpModuleTemp;
    lpModuleTemp->bModIndex = bModIndex;

    lpModuleTemp->ObjHead.lpObjNext = (LPOBJHEAD)lpModuleHead;
    lpModuleHead = (LPOBJHEAD)lpModuleTemp;
    RELEASEMODULEINFO(lpModuleTemp);
    return hModule;
}

void
FreeModuleInstance(HMODULE hModule)
{
    LPMODULEINFO lpModuleInfo, lpModuleTemp;

    if (!(lpModuleInfo = GETMODULEINFO(hModule))) {
	return;
    }
    DeleteAtom(lpModuleInfo->atmModuleName);
    WinFree(lpModuleInfo->lpModuleName);
    if (bModIndex == lpModuleInfo->bModIndex) bModIndex--;
    lpModuleTable[lpModuleInfo->bModIndex] = (LPMODULEINFO)0;

    if (lpModuleInfo == (LPMODULEINFO)lpModuleHead)
	lpModuleHead = (LPOBJHEAD)lpModuleInfo->ObjHead.lpObjNext;
    else {
	for (lpModuleTemp = (LPMODULEINFO)lpModuleHead;
	     lpModuleTemp &&
		(LPMODULEINFO)lpModuleTemp->ObjHead.lpObjNext != lpModuleInfo;
	     lpModuleTemp = (LPMODULEINFO)lpModuleTemp->ObjHead.lpObjNext);
	if (lpModuleTemp)
	    lpModuleTemp->ObjHead.lpObjNext = lpModuleInfo->ObjHead.lpObjNext;
    }
    RELEASEMODULEINFO(lpModuleInfo);
    FREEHMODULE(hModule);
}

int WINAPI
GetModuleUsage(HINSTANCE hInstance)
{
    return 1;
}

BOOL
CreateDataInstance(HINSTANCE hInst, HMODULE hModule, HTASK hTask)
{
    MEMORYINFO *lpMemory;

    if (!(lpMemory = GETHANDLEINFO(hInst)))
	return FALSE;

    lpMemory->hModule = hModule;
    lpMemory->hTask = hTask;

    RELEASEHANDLEINFO(lpMemory);
    return TRUE;
}

HMODULE
GetModuleFromInstance(HINSTANCE hInstance)
{
    MEMORYINFO *lpMemory;
    HANDLE hResult;

    if (hInstance == 0)
    {
        HTASK t;

        t = GetCurrentTask();
        return GetModuleFromInstance((GETTASKINFO(t))->hInst); 
    }

    if (!(lpMemory = GETHANDLEINFO(hInstance))) {
        MODULEINFO *lpModule = GETMODULEINFO(hInstance);
        BOOL bGotModule =  lpModule ? TRUE : FALSE;
	RELEASEMODULEINFO(lpModule);
	if (!bGotModule)
	    return (HMODULE)0;
	else {
	    RELEASEHANDLEINFO(lpMemory);
	    return (HMODULE)hInstance;
	}
    }
    hResult = lpMemory->hModule;
    RELEASEHANDLEINFO(lpMemory);
    return hResult;
}

HTASK
GetTaskFromInstance(HINSTANCE hInstance)
{
    LPMEMORYINFO lpMemory;
    HANDLE hTask;
    extern HTASK GetNativeTask(void);

    if (hInstance == 0)
	return GetNativeTask();

    if (!(lpMemory = GETHANDLEINFO(hInstance)))
	return (HTASK)0;

    hTask = lpMemory->hTask;
    RELEASEHANDLEINFO(lpMemory);
    return hTask;
}

HINSTANCE
GetInstanceFromModule(HMODULE hModule)
{
    LPMODULEINFO lpModuleInfo;
    HANDLE hInst;
    if (hModule == 0)
	return (HINSTANCE)0;

    if (!(lpModuleInfo = GETMODULEINFO(hModule))) {
        LPHANDLEINFO lpHandleInfo = GETHANDLEINFO(hModule);
	BOOL bHandle = lpHandleInfo ? TRUE : FALSE;
	RELEASEHANDLEINFO(lpHandleInfo);
	if (!bHandle) {
	    return (HINSTANCE)NULL;
	}
	else
	    return (HINSTANCE)hModule;
    }
    
    hInst = lpModuleInfo->hInst;
    RELEASEMODULEINFO(lpModuleInfo);
    return hInst;
}

LPQUEUE
AllocQueue(int cMsg)
{
     LPQUEUE    lpQueue;
     LPQUEUEMSG lpQueueMsg;
     int nCnt;

    nCnt = sizeof(QUEUE);
    nCnt += sizeof(QUEUEMSG) * cMsg;

    if ((lpQueue = (LPQUEUE) WinMalloc(nCnt))) {

	    /* initialize queue flags and pointers */
	    lpQueue->wQueueFlags = 0;
	    lpQueue->lpQueueFree = (LPQUEUEMSG)(lpQueue+1);
	    lpQueue->lpQueueNext = 0;

	    /* initialize next pointers in the free list*/
	    for (nCnt = 0,lpQueueMsg = lpQueue->lpQueueFree;
			nCnt < (cMsg-1); 
			nCnt++, lpQueueMsg++)

		lpQueueMsg->lpNextMsg = lpQueueMsg + 1;

	    /* set the last one to NULL */
	    lpQueueMsg->lpNextMsg = 0;
    }
    
    return lpQueue;
}

BOOL WINAPI
GetInputState(void)
{
	LPQUEUE    lpQueue;
	lpQueue = SystemQueue;	

	return (lpQueue->lpQueueNext)?TRUE:FALSE;
}

DWORD WINAPI
GetQueueStatus(UINT fuFlags)
{
    APISTR((LF_APISTUB,"GetQueueStatus(UINT=%x)\n",fuFlags));
    return 0L;
}

/* we should use the HardwareEnabled flag when adding mouse or
 * key messages to the system queue. Currently it is ignored.
 */

static BOOL SystemQueueEnabled = TRUE;

BOOL WINAPI
EnableHardwareInput(BOOL fEnableInput)
{
	BOOL	last;
	last    	   = SystemQueueEnabled;
	SystemQueueEnabled = fEnableInput;
	return last;
}

/*
 * QueueAddMsg:
 *	message == 0	insert message at front 
 *	message == -1	append to message queue
 *	message == msg  append after message
 */

BOOL 
QueueAddMsg(HTASK hTask, WORD Insert, LPMSG lpMsg)
{
    LPTASKINFO lpTaskInfo;
    LPQUEUE    lpQueue;
    LPQUEUEMSG lpQueueMsg,lpQueuePtr;

    if(hTask) {
	if (!(lpTaskInfo = GETTASKINFO(hTask))) {
	    return FALSE;
	}
    	lpQueue = lpTaskInfo->lpQueue;
    } else  {
#ifdef LATER
	    /* check if system queue enabled   */
	    /* if(SystemQueueEnabled == FALSE) */
	    /*   return FALSE;	 	       */
#endif
	lpQueue = SystemQueue;	
    }
    if(lpQueue == 0) {
	RELEASETASKINFO(lpTaskInfo);
	return FALSE;
    }

#ifdef	LATER
    /* accelerate mouse moves... */
    if (lpMsg->message == WM_MOUSEMOVE) {
	if (MenuMouseMove(lpMsg))
	    return TRUE;
    }
#endif

    /* see if any free messages */
    if ((lpQueueMsg = lpQueue->lpQueueFree) == 0) {
	RELEASETASKINFO(lpTaskInfo);      
	return FALSE;
    }

    /* copy the new message in */
    memcpy((LPSTR)&(lpQueueMsg->Message), (LPSTR)lpMsg, sizeof(MSG));
    if (lpQueueMsg->Message.message == WM_CONVERT)
	lpQueueMsg->wmc = *((WMCONVERT *)lpQueueMsg->Message.lParam);

    /* take message off the free list */
    lpQueue->lpQueueFree = lpQueueMsg->lpNextMsg;

    /* if no messages yet, then insert... */
    if(lpQueue->lpQueueNext == 0)
	Insert = 0;

    /* insert msg */
    if(Insert == 0) {
	/* set the next pointer of this message to the first message */
	lpQueueMsg->lpNextMsg = lpQueue->lpQueueNext;

	/* set the message queue to point to the new message */
	lpQueue->lpQueueNext = lpQueueMsg;
    } else {
	/* append msg to queue */
	/* find the message before this message */
	for(lpQueuePtr = lpQueue->lpQueueNext;
            lpQueuePtr->lpNextMsg;
	    lpQueuePtr = lpQueuePtr->lpNextMsg);
		
	/* set the last pointer to this one, and terminate this one */
	lpQueuePtr->lpNextMsg = lpQueueMsg;
	lpQueueMsg->lpNextMsg = 0;
    }
    RELEASETASKINFO(lpTaskInfo);
    return(TRUE);
}

BOOL 
QueueGetMsg(LPQUEUE lpQueue, LPMSG lpMsg, HWND hWnd,
		UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wFlags)
{

#define	POOLSIZE	16
#define	POOLMASK	0x000f

    LPQUEUEMSG lpQueueMsg, lpQueuePtr, lpQueueNext;
    MSG msg;
    int	   nmsgs;
    int	   rc;
    static WMCONVERT ConvPool[POOLSIZE];
    static int nPoolIndex = 0;
    extern BOOL IsMouseOrKeyboardMsg(UINT);
    UINT cFlags;

    if(lpQueue == 0) {
	    lpQueue = SystemQueue;
    }

    /* search for a message meeting our filters... */
    for(lpQueueMsg = lpQueue->lpQueueNext,nmsgs=0,rc=0;
	lpQueueMsg;
	lpQueueMsg = lpQueueNext) {

	cFlags = wFlags;

	lpQueueNext = (LPQUEUEMSG)(lpQueueMsg->lpNextMsg);

	msg = lpQueueMsg->Message;

	if(msg.hwnd && !IsWindow(msg.hwnd)) {
		// we have an event for a non-existent window
		// don't even try to pass it on, instead flush
		// it from the queue, and get another...
		cFlags |= PM_REMOVE;
	} else {

	if ((lpQueue == SystemQueue) &&
	    (IsMouseOrKeyboardMsg(msg.message) == WM_MOUSEFIRST))
	    if (!TWIN_ForwardButtonEvent(&msg))
		continue;

	if (wMsgFilterMin) {
	    if (msg.message < wMsgFilterMin)
		continue;
	}

	if (wMsgFilterMax) {
	     if (msg.message > wMsgFilterMax)
		continue;
	}

	if (hWnd == HWND_BROADCAST) {
		if (msg.hwnd)
			continue;

	} else if (hWnd) {
		if (msg.hwnd &&
		    msg.hwnd != hWnd) {
	            if (!IsChild(hWnd, msg.hwnd))
			continue;
		}
	}
	}

	if(msg.hwnd && !IsWindow(msg.hwnd))
	    rc++;

	else {
	    if (msg.hwnd &&
		GetWindowTask(msg.hwnd) != GetCurrentTask())
	    {
		ReadyTask(GetWindowTask(msg.hwnd));
		continue;
	    }
	    
	    if (msg.hwnd &&
			HaveDisabledAncestor(msg.hwnd) && 
			lpQueue == SystemQueue) {
		if ((rc == 0) &&
			(msg.message != WM_MOUSEMOVE) &&
			(msg.message != WM_NCMOUSEMOVE))
		    MessageBeep(0);
		rc++;
	    }
	    else {
		rc = 0;
		memcpy((LPSTR)lpMsg,(LPSTR)&(msg), sizeof(MSG));
		if (lpMsg->message == WM_CONVERT) {
		    memcpy((LPSTR)&ConvPool[nPoolIndex],
			(LPSTR)&(lpQueueMsg->wmc),
			sizeof(WMCONVERT));
		    lpMsg->lParam = (LPARAM)&ConvPool[nPoolIndex];
		    lpMsg->wParam |= HSWMC_ALLOC;
		    nPoolIndex = (nPoolIndex+1) & POOLMASK;
		}
	    }
	}

	/* are we removing it? */
	if (cFlags & PM_REMOVE) {
		/* is it the first one in the queue */
		if (lpQueueMsg == lpQueue->lpQueueNext)
		    lpQueue->lpQueueNext = lpQueueMsg->lpNextMsg;
		else {
		    /* find the message before this one */
		    for(lpQueuePtr = lpQueue->lpQueueNext;
			    lpQueuePtr->lpNextMsg != lpQueueMsg;
			    lpQueuePtr = lpQueuePtr->lpNextMsg);

		/* set the next message of our previous to our next msg */
		    lpQueuePtr->lpNextMsg = lpQueueMsg->lpNextMsg;
		}

		/* add this message to the free list */
		lpQueueMsg->lpNextMsg = lpQueue->lpQueueFree;
		lpQueue->lpQueueFree = lpQueueMsg;
	}
	if(rc == 0) return(TRUE);
    }
    return FALSE;
}

LPQUEUE
QueueGetPtr(HTASK hTask)
{
	LPTASKINFO lpTaskInfo;
	LPQUEUE lpQueue;

    	if (!(lpTaskInfo = GETTASKINFO(hTask))) {
	    return (LPQUEUE)NULL;
	}
	else {
	    lpQueue = lpTaskInfo->lpQueue;
	    RELEASETASKINFO(lpTaskInfo);
	    return lpQueue;
	}
}

void 
QueueSetFlags(HTASK hTask, WORD wFlags)
{
    LPTASKINFO lpTaskInfo;

    if (!(lpTaskInfo = GETTASKINFO(hTask)))
	return;

    lpTaskInfo->lpQueue->wQueueFlags |= wFlags;
    RELEASETASKINFO(lpTaskInfo);
}

void
QueueClearFlags(HTASK hTask, WORD wFlags)
{
    LPTASKINFO lpTaskInfo;

    if (!(lpTaskInfo = GETTASKINFO(hTask)))
	return;

    lpTaskInfo->lpQueue->wQueueFlags &= ~wFlags;
    RELEASETASKINFO(lpTaskInfo);
}

DWORD
TWIN_InternalMemory(int nFunc, LPSTR ptr, UINT size)
{
    switch (nFunc) {
	case TLC_MEMALLOC:
	    return (DWORD)WinMalloc(size);

	case TLC_MEMREALLOC:
	    return (DWORD)WinRealloc(ptr, size);

	case TLC_MEMFREE:
	    WinFree(ptr);
	    return 0L;

	default:
	    return 0L;
    }
}

DWORD WINAPI
GetAppCompatFlags(HANDLE hTask)
{
	return 0;	/* Task module was built to target Windows 3.1 */
}

/* (WIN32) Kernel Error **************************************************** */

static DWORD dwLastError = 0;

DWORD	WINAPI
GetLastError(VOID)
{
	DWORD rc;
	APISTR((LF_APICALL, "GetLastError()\n"));

	rc = dwLastError;

	APISTR((LF_APIRET, "GetLastError: returns DWORD %x\n",rc));

	return rc;
}

VOID	WINAPI
SetLastError(DWORD dwError)
{
	APISTR((LF_APICALL, "SetLastError(DWORD=%x)\n",dwError));
	dwLastError = dwError;
	APISTR((LF_APIRET, "SetLastError: returns void\n"));
}

VOID	WINAPI
SetLastErrorEx(DWORD dwError, DWORD dwType)
{
	APISTR((LF_APICALL, "SetLastErrorEx(DWORD=%x,DWORD=%x)\n",
		dwError,dwType));

	dwLastError = dwError;

	APISTR((LF_APIRET, "SetLastErrorEx: returns void\n"));
}

/*****************************************************************************/

LPSTR
WinStrdup(LPCSTR szStringIn)
{
	char * szStringOut;
	short int Length;
	/* get length of string */
	Length = strlen( szStringIn );
	/* alloc space to hold dup */
	szStringOut = (char *) WinMalloc( Length + 1 );
	/* copy string */
	strcpy( szStringOut, szStringIn );
	/* return pointer to dup string */
	return ( szStringOut );
}

LPVOID WinCalloc( unsigned int units, unsigned int size )
{
	LPVOID *ptr;
	ptr = WinMalloc(units * size);
	if (*ptr)
		memset(ptr, 0, size);
	return ptr;
}

#ifdef WINMALLOC_CHECK

/* from here on, WinMalloc is WinMalloc, etc... */
#undef WINMALLOC_CHECK

#undef WinMalloc
#undef WinFree
#undef WinRealloc

#endif

LPVOID
WinMalloc(unsigned int size)
{
    DWORD dwSize = (size+3) & ~3;
    LPVOID lpCore = malloc(dwSize+32);
    return lpCore;
}

void 
WinFree(LPVOID ptr)
{
    LPVOID lpCore = ptr;
    free(lpCore);
}

LPVOID
WinRealloc(LPVOID ptr, unsigned int size)
{
   LPVOID lpCore;
   DWORD dwSize = (size+3) & ~3;

   if (!ptr) 
      return(WinMalloc(size));

    lpCore = realloc(ptr,dwSize);
    return lpCore;
}

/*****************************************************************************/

typedef struct _malloc_tag {
	struct _malloc_tag *next;
	char	*data;
	int     size;
	int	call;
	char 	*lparam;
	int	 wparam;
	int	 flag;
	int 	 handle;
} MALLOCINFO;

MALLOCINFO	*lpMallocInfo;

static int	totalalloc;
static int	totalfree;
static int	totalacalls;
static int	totalfcalls;

LPVOID
WinMallocCheck(unsigned int size, char *lparam,int wparam,int flag, int handle)
{
	LPVOID   lpMemory;
	MALLOCINFO *p;

        lpMemory =  WinMalloc(size);

        logstr(LF_DEBUG,"%s:%d: WinMallocCheck data=%x size=%x %x %x\n",
		lparam,wparam,
                lpMemory,size,
		flag,handle);

	p = (MALLOCINFO *) WinMalloc(sizeof(MALLOCINFO));
	memset(p,0,sizeof(MALLOCINFO));

	p->next = lpMallocInfo;
	p->data = lpMemory;
	p->size = size;

	p->lparam = lparam;
	p->wparam = wparam;
	p->flag   = flag;
	p->handle = handle; 
	p->call   = totalacalls;
	
	lpMallocInfo = p;
	totalacalls++;

	totalalloc += size;

        return lpMemory;
}

void
WinFreeCheck(LPVOID ptr,char *lparam, int wparam)
{
	MALLOCINFO *p,*l;

	for(p=lpMallocInfo; p; p = p->next) {
		if(p->data == ptr) {
			if(lpMallocInfo == p) {
				lpMallocInfo = p->next;
			} else {
				for(l=lpMallocInfo; l; l = l->next) {
					if(l->next == p) {
						l->next = p->next;
					}
				}
			}
        		logstr(LF_DEBUG,"%s:%d: WinFreeCheck: data=%x from=%s:%d %x %x\n",
			    lparam,wparam,
                	    ptr,
			    p->lparam,p->wparam,
			    p->flag,p->handle);
			WinFree(p);

			totalfree += p->size;
			totalfcalls++;
			break;
		}
	}
	if(p == 0) {
		logstr(LF_DEBUG,"*** ERROR *** WinFreeCheck: freeing %x %s:%d\n",
			ptr, lparam,wparam);
	}
		WinFree(ptr);
}

LPVOID
WinReallocCheck(LPVOID ptr, unsigned int size, char *lparam, int wparam,int flag,int handle)
{
	LPVOID	lp;
	MALLOCINFO *p;

	for(p=lpMallocInfo; p; p = p->next) {
		if(p->data == ptr) {
			totalalloc -= p->size;
			break;
		}
	}

		lp = WinRealloc(ptr,size);

        logstr(LF_DEBUG,"%s:%d: WinReallocCheck: old data=%x new data=%x size=%x %x %x\n",
		lparam,wparam,
                ptr,lp,size,
		flag,handle);

	if(ptr == 0)
		return 0;

	if(lp) {
		p->size  = size;
	} else {
		p->size  = 0;
	}

	p->data  = lp;
	p->lparam = lparam;
	p->wparam = wparam;

	totalalloc += size;	
	return lp;
}

long
WinMallocInfo(int opcode, char *ptr,int flag, int handle)
{
	MALLOCINFO *p;
	int	total,n;

	if(lpMallocInfo == 0)
		return 0;

	switch(opcode) {
	case WMI_DUMP:
		logstr(LF_DEBUG,"WinMallocInfo Malloc Chain\n");

		total = 0;
		n     = 0;
		for(p=lpMallocInfo; p; p = p->next) {
			if (p->flag == 7)
			   continue;
			logstr(LF_DEBUG,"%d: %s:%d: #%d size=%d type=%x handle=%x\n",
				n++,
				p->lparam?p->lparam:"(void)",p->wparam,
				p->call,p->size,p->flag,p->handle);
			total += p->size;
			
		}

		logstr(LF_DEBUG,"WinMallocInfo Statistics\n");
		logstr(LF_DEBUG,"   calls alloc = %d size=%d\n",
			totalacalls,totalalloc);
		logstr(LF_DEBUG,"   calls free = %d size=%d\n",
			totalfcalls,totalfree);
		logstr(LF_DEBUG,"   remaining = %d delta=%d\n",
			total, totalalloc-totalfree);
		break;

        case WMI_STAT:
		logstr(handle,"%s memory = %d\n", ptr, totalalloc-totalfree);
		return totalalloc-totalfree;
	
	case WMI_TAG:
		for(p=lpMallocInfo; p; p = p->next) {
			if(p->data == ptr) {
				p->flag   = flag;	
				p->handle = handle;
				break;
			}
		}
		return 0;

	case WMI_CHECK:
		for(p=lpMallocInfo; p; p = p->next) {
			if(p->handle == handle) {
				logstr(LF_DEBUG,"WinMallocInfo: %s:%d %d %d\n",
					p->lparam, p->wparam,
					p->flag, p->handle);
				return 1;
			}
		}
		return 0;
		
	default:
		break;
	}
	return 0;
}


void TWIN_FreeAllMemory()	/* free all remaining memory as we exit windows .. */
{
	MALLOCINFO *p, *q;
	for(p=lpMallocInfo; p; p = q) {
		q	=	p->next;
		WinFree(p);
	}
}

