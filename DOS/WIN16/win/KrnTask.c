/*    
	KrnTask.c	1.45
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

#include "kerndef.h"
#include "Resources.h"
#include "Module.h"
#include "Kernel.h"
#include "Driver.h"
#include "Log.h"
#include "WinConfig.h"
#include "KrnTask.h"
#include "invoke_binary.h"
#include "LoadDLL.h"
#include "WindowCreate.h"
#include "Classes.h"
#include "Messages.h"

BOOL   bTaskingEnabled = TRUE;

static  LPOBJHEAD lpTaskHead = (LPOBJHEAD)NULL;
static  HANDLE    hCurrentTask = 0;
static  HANDLE	  hNativeTask = 0;
static  HANDLE	  hShellTask = 0;

extern LPQUEUE    SystemQueue;
extern ENV       *envp_global;
extern BOOL TWIN_InDriverWait;

extern LPQUEUE AllocQueue(int);
extern void TWIN_ReadyAllTasks(void);

#define	DEFAULT_QUEUE_SIZE	8
#define TASK_STACK_SIZE		0x100000

HINSTANCE FindPreviousInstance(HMODULE hModule, HTASK hTask);

void TWIN_NotShellTask()
{
    LPTASKINFO lpTask;
    LPTASKINFO lpNext;
    
    for (lpTask = (LPTASKINFO) lpTaskHead; 
	 lpTask;
	 lpTask = lpNext)
    {
	lpNext = (LPTASKINFO) lpTask->ObjHead.lpObjNext;
	
	if (lpTask->ObjHead.hObj != hCurrentTask)
	{
	    hShellTask = lpTask->ObjHead.hObj;
	}
    }
}

/* TaskMain is the first function executed by a created task.  It takes    */
/* care of setting up and invoking the binary application. When the binary */
/* task exits, then this task is deleted.                                  */
static void TaskMain()
{
    invoke_binary();
    DeleteTask(GetCurrentTask());
}

UINT
GetNumTasks(void)
{
    LPTASKINFO lpTask;
    int nTasks;
    
    nTasks = 0;

    for (lpTask = (LPTASKINFO) lpTaskHead; 
	 lpTask;
	 lpTask = (LPTASKINFO) lpTask->ObjHead.lpObjNext)
    {
	nTasks++;
    }

    return nTasks;
}

HTASK WINAPI
GetCurrentTask(void)
{
    APISTR((LF_APICALL,"GetCurrentTask()\n"));
    APISTR((LF_APIRET,"GetCurrentTask: returns HTASK %x\n",hCurrentTask));
    return hCurrentTask;
}

HTASK WINAPI
GetNativeTask(void)
{
    LOGSTR((LF_APICALL,"GetNativeTask()\n"));
    LOGSTR((LF_APIRET,"GetNativeTask: returns HTASK %x\n",hNativeTask));
    return hNativeTask;
}

BOOL WINAPI
IsTask(HTASK hTask)
{
    LPTASKINFO lpti;
    BOOL bR;
    
    APISTR((LF_APICALL,"IsTask(HTASK=%x)\n",hTask));
    lpti = GETTASKINFO(hTask);
    bR = lpti ? TRUE : FALSE;
    if (bR)
      RELEASETASKINFO(lpti);
    APISTR((LF_APIRET,"IsTask: returns BOOL %d\n",bR));
    return bR;
}

BOOL
TWIN_IsLiveTask(HTASK hTask)
{
    LPTASKINFO lpTask = GETTASKINFO(hTask);

    BOOL bR = (lpTask && !(lpTask->wStateFlags & TASK_DELETE_PENDING))
       ? TRUE : FALSE;

    if (lpTask)
      RELEASETASKINFO(lpTask);
    
    return bR;
}

BOOL WINAPI
ActivateTask(HTASK hTask)
{
    hCurrentTask = hTask;
#ifdef	LATER
    /* Task activation stuff */
#endif
    return(TRUE);
}

HTASK
InitializeNativeTask(LPSTR lpszCmdLine)
{
    LPTASKINFO lpTaskTemp;
    HTASK hTemp;

    if (!DRVCALL_SYSTEM(PSSH_CANDOTHREADS, 0, 0, 0))
	bTaskingEnabled = FALSE;
    else
    {
	/* default is enabled, twinrc can only really turn it off */
	bTaskingEnabled = GetTwinInt(WCP_TASKING);
    }

    /* Create task structure */
    if (!(lpTaskTemp = CREATEHTASK(hTemp))) {
	return 0;
    }

    if (SystemQueue == 0)
	SystemQueue = AllocQueue(20*DEFAULT_QUEUE_SIZE);

    if ((lpTaskTemp->lpQueue = AllocQueue(DEFAULT_QUEUE_SIZE)) == 0) {
        RELEASETASKINFO(lpTaskTemp);
	FREEHTASK(hTemp);
	return 0;
    }

    lpTaskTemp->wStateFlags |= TASK_READY;

    lpTaskTemp->ObjHead.lpObjNext = (LPOBJHEAD)lpTaskHead;
    lpTaskTemp->ThreadData = DRVCALL_SYSTEM(PSSH_GETMAINTHREAD, 0, 0, 0);
    lpTaskHead = (LPOBJHEAD)lpTaskTemp;

    hCurrentTask = hTemp;
    hNativeTask = hTemp;
    hShellTask = hTemp;
    RELEASETASKINFO(lpTaskTemp);
    return hTemp;
}

BOOL
IsNativeTask()
{
    return hCurrentTask == hNativeTask;
}

HTASK
CreateTask()
{
    LPTASKINFO lpTaskTemp;
    HTASK hTemp;

    if (!bTaskingEnabled)
	return hCurrentTask;

    /* Create task structure */
    if (!(lpTaskTemp = CREATEHTASK(hTemp))) 
    {
	return 0;
    }

    if ((lpTaskTemp->lpQueue = AllocQueue(DEFAULT_QUEUE_SIZE)) == 0) 
    {
        RELEASETASKINFO(lpTaskTemp);
	FREEHTASK(hTemp);
	return 0;
    }

    lpTaskTemp->hInst = 0;
    lpTaskTemp->wTDBSelector = 0;
    lpTaskTemp->lpCmdLine = NULL;
    lpTaskTemp->wStateFlags = 0;
    lpTaskTemp->GlobalEnvp = (ENV*)WinMalloc(sizeof(*lpTaskTemp->GlobalEnvp));
    memset((LPSTR)lpTaskTemp->GlobalEnvp, 0, sizeof(*lpTaskTemp->GlobalEnvp));

    lpTaskTemp->ObjHead.lpObjNext = (LPOBJHEAD)lpTaskHead;
    lpTaskHead = (LPOBJHEAD)lpTaskTemp;

    RELEASETASKINFO(hpTaskTemp);
    return hTemp;
}

BOOL
TWIN_InitializeAnotherNativeTask(HTASK hTask, DWORD entry_point)
{
    LPTASKINFO lpTask;
    LPTASKINFO lpCurrentTask;
    
    if (!bTaskingEnabled)
	return 0;

    lpTask = GETTASKINFO(hTask);
    lpCurrentTask = GETTASKINFO(hCurrentTask);
    
    if (!lpTask || !lpCurrentTask) {
      if (lpTask)
	RELEASETASKINFO(lpTask);
      if (lpCurrentTask)
	RELEASETASKINFO(lpCurrentTask);
      return 0;
    }

    lpTask->hInst = GlobalAlloc(0,0);
    CreateDataInstance(lpTask->hInst, 
		       GetModuleFromInstance(lpCurrentTask->hInst), 
		       hTask);
    lpTask->wStateFlags |= TASK_READY;
    TWIN_CreateENV(lpTask);
    lpTask->uiHeapSize = 0;
    lpTask->uiStackSize = 0;
    lpTask->uiAutodataSize = 0;
    lpTask->wDGROUP = 0;

    lpTask->ThreadData = DRVCALL_SYSTEM(PSSH_CREATETHREAD, entry_point, 0, 0);

    RELEASETASKINFO(lpTask);
    RELEASETASKINFO(lpCurrentTask);

    return 1;
}

void
InitializeTask(HTASK hTask, ENV *envp, HINSTANCE hInst,
		    UINT heap_size, UINT stack_size, UINT bss_size)
{
    LPTASKINFO lpTask;
    LPTASKINFO lpCurrentTask;
    
    lpTask = GETTASKINFO(hTask);
    lpCurrentTask = GETTASKINFO(hCurrentTask);
    
    if (!lpTask || !lpCurrentTask) {
      if (lpTask)
	RELEASETASKINFO(lpTask);
      if (lpCurrentTask)
	RELEASETASKINFO(lpCurrentTask);
      return;
    }

    lpTask->hInst = hInst;
    lpTask->wStateFlags |= TASK_READY;
    lpTask->GlobalEnvp = envp;
    lpTask->uiHeapSize = heap_size;
    lpTask->uiStackSize = stack_size;
    lpTask->uiAutodataSize = bss_size;
    lpTask->wDGROUP = envp->reg.ds;

    lpTask->ThreadData = DRVCALL_SYSTEM(PSSH_CREATETHREAD, (DWORD) TaskMain, 
					0, 0);
    RELEASETASKINFO(lpTask);
    RELEASETASKINFO(lpCurrentTask);
}

BOOL
TWIN_FreeTaskWindow(HWND hwnd, LPARAM lParam)
{
    TWIN_InternalDestroyWindow(hwnd);
    return TRUE;
}

void
FreeTask(HTASK hTask)
{
    LPTASKINFO lpTaskFree;
    LPTASKINFO lpTaskPrev;
    HMODULE hModule;

    /* Never free the current task */
    if (hTask == hCurrentTask)
    {
	TWIN_ReadyAllTasks();
	return;
    }
        
    lpTaskFree = GETTASKINFO(hTask);
    if (!lpTaskFree)
	return;

    /* Destroy all windows owned by this task... */
    EnumTaskWindows(hTask, (WNDENUMPROC)TWIN_FreeTaskWindow, 0);

    /* Free the PSP and TDB for this task */

    /* Free envp chain for this task */

    /* Free all segments used by this task */

    /* Is this the last instance of this module? */
    hModule = GetModuleFromInstance(lpTaskFree->hInst);
    if (!FindPreviousInstance(hModule, hTask))
    {
	/* Unregister all classes if this is last instance of module */
	TWIN_UnregisterModuleClasses(hModule);

	/* If the last instance of a module, then free the module */
    }

    if (lpTaskFree->lpQueue)
	WinFree((LPSTR) lpTaskFree->lpQueue);

    if (lpTaskHead == (LPOBJHEAD) lpTaskFree)
	lpTaskHead = (LPOBJHEAD) lpTaskFree->ObjHead.lpObjNext;
    else
    {
	for (lpTaskPrev = (LPTASKINFO) lpTaskHead; 
	     lpTaskPrev && 
	     (LPTASKINFO) lpTaskPrev->ObjHead.lpObjNext != lpTaskFree; 
	     lpTaskPrev = (LPTASKINFO) lpTaskPrev->ObjHead.lpObjNext)
	{
	    /* searching */
	}

	if (lpTaskPrev)
	    lpTaskPrev->ObjHead.lpObjNext = lpTaskFree->ObjHead.lpObjNext;
    }

    DRVCALL_SYSTEM(PSSH_FREETHREAD, (DWORD) lpTaskFree->ThreadData, 0, 0);
    RELEASETASKINFO(lpTaskFree);
    FREEHTASK(hTask);
}

void
ReadyTask(HTASK hTask)
{
    LPTASKINFO lpTask;
    
    if(hTask) {
	    lpTask = GETTASKINFO(hTask);
	    if (!lpTask)
		return;

	    lpTask->wStateFlags |= TASK_READY;
	    RELEASETASKINFO(lpTask);
    }
}    

void
TWIN_ReadyAllTasks()
{
    LPTASKINFO lpTask;
    LPTASKINFO lpNext;
    
    for (lpTask = (LPTASKINFO) lpTaskHead; 
	 lpTask;
	 lpTask = lpNext)
    {
	lpNext = (LPTASKINFO) lpTask->ObjHead.lpObjNext;

	lpTask->wStateFlags |= TASK_READY;
    }
}

void
DeleteTask(HTASK hTask)
{
    LPTASKINFO lpTask;

    if (!hTask)
	hTask = hCurrentTask;

#ifdef LATER
    if (hTask == hShellTask)
	ExitWindows(0, 0);
#endif
    
    lpTask = GETTASKINFO(hTask);
    if (!lpTask)
	return;

    lpTask->wStateFlags |= TASK_DELETE_PENDING;
    lpTask->wStateFlags &= ~TASK_READY;

    /* If we are deleting ourselves, then we don't ever want to return    */
    /* from this function call.  So, we just sit in a loop yielding the   */
    /* processor to other tasks.  Calling Yield() once should do it, but  */
    /* it is very important that even a bug in Yield() doesn't let us     */
    /* return.                                                            */
    if (hTask == GetCurrentTask())
    {
	if (!bTaskingEnabled)
	    FatalExit(0);

	while (TWIN_SendMessagePendingForTask(hTask))
	    ReplyMessage(0);
	
	while (1)
	{
	    InternalYield();
	    lpTask->wStateFlags |= TASK_DELETE_PENDING;
	    lpTask->wStateFlags &= ~TASK_READY;
	}
    }
    RELEASETASKINFO(lpTask);
}

HTASK
ScheduleTask()
{
    LPQUEUEMSG lpQueueMsg;
    LPTASKINFO lpTask;
    LPTASKINFO lpNext;
    LPTASKINFO lpReadyTask = NULL;
    BOOL bBeforeCurrent = TRUE;
    int iLiveTasks = 0;

    if (bTaskingEnabled)
    {
	/*
	 * Before scheduling, process all events on the driver message
	 * queue.  This will assure us that all events are in the standard
	 * message queues before we try to decide which tasks are ready to
	 * run.
	 */
	TWIN_InDriverWait = TRUE;
	DriverWaitEvent(TRUE);
	TWIN_InDriverWait = FALSE;

	/*
	 * Examine the system message queue and ready any tasks that have
	 * messages waiting in that queue.
	 */
	for (lpQueueMsg = SystemQueue->lpQueueNext;
	     lpQueueMsg;
	     lpQueueMsg = (LPQUEUEMSG) lpQueueMsg->lpNextMsg)
	{
	    HWND hwnd = lpQueueMsg->Message.hwnd;
	    
	    if (hwnd)
		ReadyTask(GetWindowTask(hwnd));
	}
    
	/* We have chosen a very simple scheduling algorithm.  It is  */
	/* a round-robin scheme with all tasks having equal priority. */
	/* We look at three things to determine whether a process is  */
	/* ready to run:					      */
	/*	1. Is the ready flag set?			      */
	/*	2. Do we need a paint or other "flagged" message?     */
	/*	3. Are there any messages in the queue?		      */
	   
	for (lpTask = (LPTASKINFO) lpTaskHead; 
	     lpTask;
	     lpTask = lpNext)
	{
	    lpNext = (LPTASKINFO) lpTask->ObjHead.lpObjNext;
	    
	    /* Since we are looking through the tasks anyway, let's   */
	    /* delete processes that are pending deletion.            */
	    if (lpTask->wStateFlags & TASK_DELETE_PENDING)
	    {
		FreeTask(lpTask->ObjHead.hObj);
		continue;
	    }

	    /* This task is still alive.  Update the count.           */
	    iLiveTasks++;

	    /* Apply the test to see if process is ready.             */
	    if ((!bBeforeCurrent || !lpReadyTask) &&
		((lpTask->wStateFlags & TASK_READY) ||
		 lpTask->lpQueue->wQueueFlags ||
		 lpTask->lpQueue->lpQueueNext))
	    {
		lpReadyTask = lpTask;
		if (!bBeforeCurrent)
		    break;
	    }

	    if (lpTask->ObjHead.hObj == hCurrentTask)
		bBeforeCurrent = FALSE;
	}

	if (lpReadyTask)
	    return lpReadyTask->ObjHead.hObj;

	else if (!iLiveTasks)
	    FatalExit(0);
    }
    
    return hCurrentTask;
}

void WINAPI
Yield(void)
{
    ReadyTask(hCurrentTask);
    InternalYield();
}

void    WINAPI 
DirectedYield(HTASK hTask)
{
    ReadyTask(hCurrentTask);
    InternalDirectedYield(hTask);
}

BOOL InternalYield()
{
    HTASK hTask;
    
    hTask = ScheduleTask();
    if (hTask == hCurrentTask)
	return FALSE;
    
    InternalDirectedYield(hTask);
    return TRUE;
}    

void
InternalDirectedYield(HTASK hTask)
{
    LPTASKINFO lpNewTask;
    LPTASKINFO lpOldTask;

    if (hTask == hCurrentTask)
	return;
    
    lpNewTask = GETTASKINFO(hTask);
    lpOldTask = GETTASKINFO(hCurrentTask);
    if (!lpNewTask || !lpOldTask) {
        if (lpNewTask) {
	  RELEASETASKINFO(lpNewTask);
	}
	if (lpOldTask) {
	  RELEASETASKINFO(lpOldTask);
	}
	return;
    }

    /* clear the ready flag on the target task */
    lpNewTask->wStateFlags &= ~TASK_READY;

    /* save global variables that are task specific */
    lpOldTask->GlobalEnvp = envp_global;

    /* set global variables from saved versions in target task */
    envp_global = lpNewTask->GlobalEnvp;
    
    /* set the current task to the target task */
    hCurrentTask = hTask;

    /* switch from old thread to target thread */
    DRVCALL_SYSTEM(PSSH_YIELDTOTHREAD, (DWORD) lpOldTask->ThreadData, 
		   (DWORD) lpNewTask->ThreadData, 0);
    RELEASETASKINFO(lpNewTask);
    RELEASETASKINFO(lpOldTask);

    /* Are there any SendMessages to process? */
    TWIN_ReceiveMessage(FALSE);
}

HINSTANCE FindPreviousInstance(HMODULE hModule, HTASK hTask)
{
    LPTASKINFO lpTask;
    LPTASKINFO lpNext;
    HMODULE hTaskModule;
    
    for (lpTask = (LPTASKINFO) lpTaskHead; 
	 lpTask;
	 lpTask = lpNext)
    {
	lpNext = (LPTASKINFO) lpTask->ObjHead.lpObjNext;
	
	if (lpTask->ObjHead.hObj != hTask && lpTask->hInst)
	{
	    hTaskModule = GetModuleFromInstance(lpTask->hInst);
	    if (hTaskModule && hTaskModule == hModule)
		return lpTask->hInst;
	}
    }

    return 0;
}

HINSTANCE
TWIN_GetInstanceFromTask(HTASK hTask)
{
    LPTASKINFO lpTask;
    HINSTANCE hResult;

    if (NULL == (lpTask = GETTASKINFO(hTask)))
	return (HINSTANCE)0;
    else {
        hResult = lpTask->hInst;
	RELEASETASKINFO(lpTask);
	return hResult;
    }
}

void
TWIN_LocalExpandStackFix(UINT uSegment, LPBYTE lpSegment, LPBYTE lpOldSegment)
{
    ENV *lpEnv;
    LPTASKINFO lpTask;
    LPTASKINFO lpNext;
    
    for (lpTask = (LPTASKINFO) lpTaskHead; lpTask; lpTask = lpNext)
    {
	lpNext = (LPTASKINFO) lpTask->ObjHead.lpObjNext;
	
	if (lpTask->ObjHead.hObj != hCurrentTask)
	    lpEnv = lpTask->GlobalEnvp;
	else
	    lpEnv = envp_global;
	
	for( ; lpEnv; lpEnv = (ENV *) lpEnv->prev_env) 
	{

	    if (lpEnv->reg.ss == uSegment) 
	    {
		lpEnv->reg.sp = 
		    (REGISTER) (lpSegment + (WORD) ((DWORD) lpEnv->reg.sp - 
						    (DWORD) lpOldSegment));
		lpEnv->reg.bp = 
		    (REGISTER) (lpSegment + (WORD) ((DWORD) lpEnv->reg.bp - 
						    (DWORD) lpOldSegment));
	    }
	}
    }
}
