/*    
	KrnCatch.c	1.13
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
#include <stdarg.h>
#include <string.h>

#include "windows.h"
/*
#include "shellapi.h"
#include "toolhelp.h"
*/
#include "ddeml.h"

#include "kerndef.h"
#include "Endian.h"
#include "Log.h"
#include "Kernel.h"
#include "Resources.h"
#include "DPMI.h"
#include "BinTypes.h"
#include "dos.h"
#include "mfs_config.h"

extern ENV *envp_global;

#define MAX_ACTIVE_CATCHES	256

typedef struct tagCATCHMAP
{
    HTASK htask;
    ENV *envp;
    BOOL is_source_catch;

    /* Binary Catch() data */
    WORD binary_ss;
    WORD binary_sp;
    WORD binary_cs;
    WORD binary_ip;
    LPBYTE catch_buf;

    /* Source Catch() data */
    jmp_buf *jmpbuf;
    unsigned long stack_pos;
} CATCHMAP;

static CATCHMAP CatchMap[MAX_ACTIVE_CATCHES];
static int NextToTry;

static int TWIN_GetCatchId()
{
    int i;
    int idx;
    
    for (i = 0; i < MAX_ACTIVE_CATCHES; i++)
    {
	idx = (NextToTry + i) % MAX_ACTIVE_CATCHES;
	if (!CatchMap[idx].htask)
	{
	    CatchMap[idx].htask = GetCurrentTask();
	    NextToTry = (idx + 1) % MAX_ACTIVE_CATCHES;
	    return idx;
	}
    }

    exit(1);
}

void TWIN_FreeObsoleteCatches(ENV *envp)
{
    CATCHMAP *cm;
    HTASK this_task;
    int i;
    
    cm = CatchMap;
    this_task = GetCurrentTask();
    for (i = 0; i < MAX_ACTIVE_CATCHES; i++, cm++)
    {
	if (cm->htask == this_task && cm->envp == envp)
	{
	    cm->htask = 0;
	    cm->binary_ss = 0;
	    cm->binary_sp = 0;
	    cm->envp = NULL;
	    if (cm->is_source_catch)
	    {
		cm->is_source_catch = 0;
		WinFree(cm->jmpbuf);
		cm->jmpbuf = 0;
	    }
	}
    }
}

static int TWIN_CatchExists(LPBYTE lpCatchBuf, WORD binary_ss, WORD binary_sp, 
			    WORD binary_cs, WORD binary_ip)
{
    CATCHMAP *cm;
    HTASK this_task;
    int i;
    
    cm = CatchMap;
    this_task = GetCurrentTask();
    for (i = 0; i < MAX_ACTIVE_CATCHES; i++, cm++)
    {
	if (cm->htask == this_task &&
	    cm->catch_buf == lpCatchBuf &&
	    cm->binary_ss == binary_ss &&
	    cm->binary_sp == binary_sp &&
	    cm->binary_cs == binary_cs &&
	    cm->binary_ip == binary_ip)
	{
	    return i;
	}
    }

    return -1;
}

void
_86_Catch (ENV *envp,LONGPROC f)
{
    WORD binary_sp;
    WORD binary_bp;
    WORD binary_ip;
    WORD binary_cs;
    int catchid;
    LPBYTE lpCatchBuf;
    CATCHMAP *cm;

    lpCatchBuf = (LPBYTE) GetAddress(GETWORD(SP+6), GETWORD(SP+4));

    binary_sp = envp->reg.sp - (unsigned long)GetSelectorAddress(envp->reg.ss);
    binary_bp = envp->reg.bp - (unsigned long)GetSelectorAddress(envp->reg.ss);
    binary_ip = LOWORD(envp->return_addr);
    binary_cs = HIWORD(envp->return_addr);

    catchid = TWIN_CatchExists(lpCatchBuf, envp->reg.ss, binary_sp, 
				binary_cs, binary_ip);
    
    if (catchid < 0)
	catchid = TWIN_GetCatchId();

    cm = &CatchMap[catchid];
    cm->binary_ss = envp->reg.ss;
    cm->binary_sp = binary_sp;
    cm->binary_cs = binary_cs;
    cm->binary_ip = binary_ip;
    cm->catch_buf = lpCatchBuf;
    cm->envp = envp;
    cm->is_source_catch = 0;
    cm->jmpbuf = NULL;

    PUTWORD(lpCatchBuf+0, binary_cs);
    PUTWORD(lpCatchBuf+2, binary_ip);
    PUTWORD(lpCatchBuf+4, envp->reg.ds);
    PUTWORD(lpCatchBuf+6, envp->reg.ss);
    PUTWORD(lpCatchBuf+8, binary_sp);
    PUTWORD(lpCatchBuf+10, binary_bp);
    PUTWORD(lpCatchBuf+12, envp->reg.di);
    PUTWORD(lpCatchBuf+14, envp->reg.si);
    PUTWORD(lpCatchBuf+16, catchid);
	
    envp->is_catch = 1;
    envp->reg.sp += LP_86 + RET_86;
    envp->reg.ax = 0;
    envp->reg.dx = 0;
}

void
_86_Throw (ENV *envp,LONGPROC f)
{
    LPBYTE lpCatchBuf;
    int ErrValue;

    lpCatchBuf = (LPBYTE) GetAddress(GETWORD(SP+8), GETWORD(SP+6));
    ErrValue = GETWORD(SP+4);

    Throw((int *) lpCatchBuf, ErrValue);
}

static int stack_grows_positive(unsigned long prev_stack_addr)
{
    int stack_var;
    
    if ((unsigned long) &stack_var > prev_stack_addr)
	return 1;
    else
	return 0;
}

static void free_catch(CATCHMAP *cm)
{
    cm->htask = 0;
    cm->binary_ss = 0;
    cm->binary_sp = 0;
    cm->envp = NULL;
    cm->is_source_catch = 0;
    if (cm->jmpbuf)
	WinFree(cm->jmpbuf);
    cm->jmpbuf = 0;
}

int TWIN_Catch(LPBYTE lpCatchBuf, int ErrValue)
{
    HTASK this_task;
    CATCHMAP *cm;
    int catchid;
    int i;
    int positive_stack = stack_grows_positive((unsigned long) &positive_stack);
    unsigned long stack_pos = (unsigned long) &positive_stack;
    
    /* Did we get here as a result of a Throw? */
    if (ErrValue)
	return ErrValue;

    /* Find all invalid catches */
    cm = CatchMap;
    this_task = GetCurrentTask();
    if (positive_stack)
    {
	for (i = 0; i < MAX_ACTIVE_CATCHES; i++, cm++)
	    if (cm->htask == this_task && cm->is_source_catch &&
		cm->stack_pos < stack_pos)
		free_catch(cm);
    }
    else
    {
	for (i = 0; i < MAX_ACTIVE_CATCHES; i++, cm++)
	    if (cm->htask == this_task && cm->is_source_catch && 
		cm->stack_pos > stack_pos)
		free_catch(cm);
    }

    /* Allocate the new catch */
    catchid = TWIN_GetCatchId();
    cm = &CatchMap[catchid];
    cm->binary_ss = 0;
    cm->binary_sp = 0;
    cm->envp = envp_global;
    cm->is_source_catch = 1;
    cm->jmpbuf = *(jmp_buf **)lpCatchBuf;

    PUTWORD(lpCatchBuf+0, 0);
    PUTWORD(lpCatchBuf+2, 0);
    PUTWORD(lpCatchBuf+4, 0);
    PUTWORD(lpCatchBuf+6, 0);
    PUTWORD(lpCatchBuf+8, 0);
    PUTWORD(lpCatchBuf+10, 0);
    PUTWORD(lpCatchBuf+12, 0);
    PUTWORD(lpCatchBuf+14, 0);
    PUTWORD(lpCatchBuf+16, catchid);

    if (envp_global != NULL)
      envp_global->is_catch = 1;
    return 0;
}

void WINAPI
Throw(const int *lpCatchBuf, int nErrorReturn)
{
    int catchid;
    CATCHMAP *cm;
    ENV *envp;

    catchid = GETWORD((LPBYTE)lpCatchBuf+16);
    if (catchid < 0 || catchid >= MAX_ACTIVE_CATCHES)
    {
	exit(1);
    }
    
    cm = &CatchMap[catchid];
    if (cm->htask != GetCurrentTask())
    {
	exit(1);
    }

    /*
     * Binary catch buffer.
     */
    if (GETWORD((LPBYTE)lpCatchBuf+0))
    {
	envp = cm->envp;
	envp->reg.ds = GETWORD((LPBYTE)lpCatchBuf+4);
	envp->reg.ss = GETWORD((LPBYTE)lpCatchBuf+6);
	envp->reg.di = GETWORD((LPBYTE)lpCatchBuf+12);
	envp->reg.si = GETWORD((LPBYTE)lpCatchBuf+14);
	envp->reg.sp = ((unsigned long) GetSelectorAddress(envp->reg.ss) + 
			GETWORD((LPBYTE)lpCatchBuf+8));
	envp->reg.bp = ((unsigned long) GetSelectorAddress(envp->reg.ss) + 
			GETWORD((LPBYTE)lpCatchBuf+10));
	envp->reg.ax = nErrorReturn;
	envp->reg.dx = 0;
	envp->trans_addr = ((((unsigned long) GETWORD((LPBYTE)lpCatchBuf+0)) << 16) |
			    ((unsigned long) GETWORD((LPBYTE)lpCatchBuf+2)));
	envp->return_addr = envp->trans_addr;

	envp_global = envp;
	longjmp(envp->jump_buffer, 1);
    }
    /*
     * Source catch buffer.
     */
    else
    {
	envp_global = cm->envp;
	longjmp(*cm->jmpbuf, nErrorReturn);
    }
}

