/********************************************************************

	@(#)DrvThreads_X386.c	1.9
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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "windows.h"
#include "Log.h"
#include "DrvThreads.h"

typedef void (*THREADENTRYFUNC)();

#define TASK_STACK_SIZE		0x100000

static jmp_buf main_thread;

/* static storage used while we switch stacks */
static jmp_buf *saved_new_thread;
static jmp_buf saved_old_thread;
static THREADENTRYFUNC func_save;

/*
 *  For systems which allow some form of the "asm" directive,
 *  do the swap stack using in-line assembly code in a single
 *  routine.  For other systems, call out to an assembly
 *  module, which will call back to a followup routine to
 *  complete the operations, for a total of three routines
 *  rather than one.
 *
 */
void TWIN_InitStack(volatile int sp, THREADENTRYFUNC func,
		    jmp_buf *new_thread)
{
    /* This is the "single function" version. */
    
    /* put task pointers in storage that is not on the stack */
    saved_new_thread = new_thread;
    func_save = func;
    
    /* save current context */
    if (!setjmp(saved_old_thread))
    {
	asm ("mov  8(%ebp),%eax");
	asm ("mov  %eax, %esp");
	asm ("mov  %eax, %ebp");

	/* save newly created context */
	if (!setjmp((*saved_new_thread)))
	{
	    /* return to original context */
	    longjmp(saved_old_thread, 1);
	}

	/* We won't get here until we reach the first task yield that */
	/* schedules this task.  The call will never return.          */
	(*func_save)();
    }    
}

DWORD
TWIN_DrvCreateThread(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    DWORD dwParam = (DWORD)dwParam1;
    jmp_buf *lpContext;
    void *stack;
    
    lpContext = (jmp_buf *) WinMalloc(sizeof(jmp_buf));
    if (!lpContext)
	return (DWORD) NULL;

    stack = (void *) WinMalloc(TASK_STACK_SIZE);

    TWIN_InitStack(((int) stack + TASK_STACK_SIZE - 16) & ~0xf, 
		   (THREADENTRYFUNC) dwParam, lpContext);

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
    jmp_buf *old_thread = (jmp_buf *) dwParam1;
    jmp_buf *new_thread = (jmp_buf *) dwParam2;
    
    if (dwParam1 == dwParam2 || !dwParam1 || !dwParam2)
	return 1;

    /* Save the context of the current task. */
    if (!setjmp((*old_thread)))
    {
	/* If we get here, then setjmp was called normally, and we */
	/* haven't switched processes, yet.                        */
	longjmp((*new_thread), 1);

	/* We never get to here */
    }

    return 1;
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
      

DWORD
TWIN_DrvCreatePSDKey(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 0L;
}

DWORD
TWIN_DrvDeletePSDKey(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 0L;
}

DWORD
TWIN_DrvSetPSD(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 0L;
}

DWORD 
TWIN_DrvGetPSD(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 0L;
}
