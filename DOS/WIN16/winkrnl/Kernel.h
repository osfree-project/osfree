/********************************************************************

	@(#)Kernel.h	2.18    Kernel object definitions.
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
 
#ifndef Kernel__h
#define Kernel__h


#include <setjmp.h>
#include "ObjEngine.h"
#include "BinTypes.h"

/* ----- KERNEL OBJECTS -----*/

typedef struct tagMEMORYINFO
{
        OBJHEAD	ObjHead;	/* generic object header */
	struct  tagMEMORYINFO *lpNext;
	HMODULE hModule;	/* owning module */
	HTASK	hTask;		/* owning task */
	WORD	wFlags;		/* flags passed/set on handle */
	WORD	wType;		/* MT_ kind */
	DWORD	dwSize;		/* global object size */
	LPSTR   lpCore;		/* this points to native data */
	HANDLE	hMemory;	/* this is the memory handle  */
	WORD	wRefCount;	/* reference count */
	LPSTR	lpData;		/* points to resource data in 86-format */
	DWORD	dwBinSize;	/* size of object in lpData	*/
	WORD	wIndex;		/* resource type */

} MEMORYINFO;
typedef MEMORYINFO *LPMEMORYINFO;

typedef MEMORYINFO HANDLEINFO;
typedef HANDLEINFO *LPHANDLEINFO;

#define HSWMC_DEST_MASK             0x0003
#define HSWMC_DEST_WINDOW_PROC      0x0000
#define HSWMC_DEST_CALLWNDPROC      0x0002
#define HSWMC_DEST_FILTER           0x0001
#define HSWMC_DEST_DEF_FRAME	    0x0003
#define HSWMC_NATMSG                0x4000
#define HSWMC_BINMSG                0x2000
#define HSWMC_SIMSTACK              0x0400
#define HSWMC_ALLOC                 0x0200

typedef struct tagWMCONVERT {
	LPARAM  lParam;
	WPARAM  wParam;
	UINT    uMsg;
	LONGPROC targ;          /* only set if not HSWMC_HOOK_WNDPROC */
	int     hook_code;      /* only set if not HSWMC_HOOK_NONE  */
	WPARAM  hook_wParam;    /* only set if HSWMC_HOOK_CALLWNDPROC */
} WMCONVERT;

/* Task queue entry */
typedef struct tagQUEUEMSG
{
    struct tagQUEUEMSG  *lpNextMsg;		/* ptr to next */
    MSG			 Message;		/* message data */
    WMCONVERT		wmc;			/* WMCONVERT message */
} QUEUEMSG;

typedef QUEUEMSG *LPQUEUEMSG;

typedef struct tagQUEUE
{
    WORD	wQueueFlags;
    LPQUEUEMSG	lpQueueFree;		/* pointer to free message list */
    LPQUEUEMSG	lpQueueNext;		/* pointer to message in queue */
} QUEUE;

typedef QUEUE *LPQUEUE;

#define TASK_READY		0x0001
#define TASK_DELETE_PENDING	0x0002

/* Task information structure */
typedef struct tagTASKINFO
{
    OBJHEAD	ObjHead;		/* generic object header */
    HANDLE	hInst;			/* instance handle */
    WORD	wTDBSelector;		/* selector of TDB segment */
    LPSTR	lpCmdLine;		/* command line */
    LPQUEUE	lpQueue;		/* application message queue */
    WORD	wStateFlags;		/* task state flags */
    UINT	uiHeapSize;		/* initial heap size */
    UINT	uiStackSize;		/* initial stack size */
    UINT	uiAutodataSize;		/* initial bss size */
    WORD	wDGROUP;		/* the dgroup selector for this task */

    ENV	       *GlobalEnvp;		/* envp_global at last switch */
    DWORD	ThreadData;		/* system specific thread data */
    
} TASKINFO;

typedef TASKINFO *LPTASKINFO;

/* Intertask Sendmessage structure */
typedef struct tagITSMINFO ITSMINFO;

struct tagITSMINFO
{
    ITSMINFO   *lpPrev;
    HTASK	hSendingTask;
    HTASK	hReceivingTask;
    MSG		msg;
    LRESULT	lResult;
    BOOL	bSendReceived;
    BOOL	bSendCompleted;
};    

/* Global Variables */
extern BOOL bTaskingEnabled;

/* Prototypes */

HMODULE CreateModuleInstance(LPSTR);
void FreeModuleInstance(HMODULE);
HTASK InitializeNativeTask(LPSTR);
HTASK CreateTask(void);
void InitializeTask(HTASK hTask, ENV *envp, HINSTANCE hInst,
		    UINT heap_size, UINT stack_size, UINT bss_size);
HTASK ScheduleTask(void);
void ReadyTask(HTASK);
void DeleteTask(HTASK);
BOOL CreateDataInstance(HINSTANCE, HMODULE, HTASK);
BOOL ActivateTask(HTASK);
HMODULE GetModuleFromInstance(HINSTANCE);
HTASK GetTaskFromInstance(HINSTANCE);
HINSTANCE GetInstanceFromModule(HMODULE);
BOOL InternalYield(void);
void InternalDirectedYield(HTASK);
void WINAPI Sleep(DWORD);

BOOL QueueAddMsg(HTASK, WORD,LPMSG);
BOOL QueueGetMsg(LPQUEUE, LPMSG, HWND, UINT, UINT, UINT);
LPQUEUE QueueGetPtr(HTASK);
void QueueSetFlags(HTASK, WORD);
void QueueClearFlags(HTASK, WORD);

#endif /* Kernel__h */
