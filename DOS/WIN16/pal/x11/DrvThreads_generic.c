/********************************************************************

	@(#)DrvThreads_generic.c	1.9 Unix thread subsystem .
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
#include <unistd.h>

#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "windows.h"
#include "Log.h"
#include "DrvThreads.h"
#include "Driver.h"

typedef void (*THREADENTRYFUNC)();
typedef void (*DESTRUCTORFUNC)();

#define TASK_STACK_SIZE		0x100000

/**********************************************************************
 * Structure that holds thread specific info.
 */
typedef struct drv_thread_s
{
    jmp_buf thread_context;
    void **psd;
    int n_keys_allocated;
} drv_thread_t;

/**********************************************************************
 * Miscellaneous state information.
 */
static drv_thread_t *cur_thread_p;
static drv_thread_t main_thread;
static int n_keys_used = 0;

/**********************************************************************
 * static storage used while we switch stacks in initialization of threads
 */
static jmp_buf *saved_new_thread;
static jmp_buf saved_old_thread;
static THREADENTRYFUNC func_save;

/**********************************************************************
 * Forward declarations
 */

/**********************************************************************
 * 
 */

void TWIN_InitStack(volatile int sp, THREADENTRYFUNC func,
		    jmp_buf *new_thread)
{
    /* This is the "multi-function" version. */
    
    /* put task pointers in storage that is not on the stack */
    saved_new_thread = new_thread;
    func_save = func;
    
    /* save current context */
    if (!setjmp(saved_old_thread))
    {
	/*
 	 * Call assembly routine to set the stack pointer.
	 * This call will not return, as it calls TWIN_InitStackEnd,
	 * which will return to the just set context via a longjmp.
	 */
	TWIN_SwitchStackAssem(sp);
    }
}

void TWIN_InitStackEnd()
{
    /* We are called right after the stack has been switched. */
    /* Save the newly created context. */
    if (!setjmp((*saved_new_thread)))
    {
	/* return to original context */
	longjmp(saved_old_thread, 1);
    }

    /* We won't get here until we reach the first task yield that */
    /* schedules this task.  The call will never return.          */
    (*func_save)();
}

DWORD
TWIN_DrvCreateThread(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    DWORD dwParam = (DWORD)dwParam1;
    drv_thread_t *lpContext;
    void *stack;
    
    lpContext = (drv_thread_t *) WinMalloc(sizeof(drv_thread_t));
    if (!lpContext)
	return (DWORD) NULL;

    stack = (void *) WinMalloc(TASK_STACK_SIZE);

    TWIN_InitStack(((int) stack + TASK_STACK_SIZE - 16) & ~0xf, 
		   (THREADENTRYFUNC) dwParam, &lpContext->thread_context);

    return (DWORD) lpContext;
}

DWORD
TWIN_DrvFreeThread(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    return 1;
}

DWORD
TWIN_DrvYieldToThread(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
    DWORD dwParam1 = (DWORD)dwParm1;
    DWORD dwParam2 = (DWORD)dwParm2;
    drv_thread_t *old_thread = (drv_thread_t *) dwParam1;
    drv_thread_t *new_thread = (drv_thread_t *) dwParam2;
    
    if (dwParam1 == dwParam2 || !dwParam1 || !dwParam2)
	return 1;

    /* Save the context of the current task. */
    if (!setjmp((old_thread->thread_context)))
    {
	/* If we get here, then setjmp was called normally, and we */
	/* haven't switched processes, yet.                        */
	
	/* remember which thread is running. */
	cur_thread_p = new_thread;

	/* switch to it. */
	longjmp((new_thread->thread_context), 1);

	/* We never get to here */
    }

    return 1;
}

DWORD 
TWIN_DrvCreatePSDKey(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    DWORD key = n_keys_used++;
    
    return key;
}

DWORD
TWIN_DrvDeletePSDKey(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    return 0;
}

DWORD
TWIN_DrvSetPSD(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
}

DWORD
TWIN_DrvGetPSD(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
}

DWORD
TWIN_DrvCanDoThreads(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return (DWORD)1;
}

DWORD
TWIN_DrvGetMainThread(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return (DWORD)&main_thread;
}

