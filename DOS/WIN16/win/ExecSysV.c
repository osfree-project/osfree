/*    
	ExecSysV.c	1.7
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
#include <signal.h>
#include <siginfo.h>
#include <sys/regset.h>
#include <sys/ucontext.h>
#include <setjmp.h>
#include <errno.h>
#include <memory.h>
#include <sys/sysi86.h>

#include "Exec.h"

#ifndef UCONTEXT_REGS
#define UCONTEXT_REGS regs
#endif

#define	STACK_DWORDS	0x4000

static struct sigaction sact;
static unsigned long TWIN_Stack[STACK_DWORDS];
static int altstackdefined = 0;

static void
TWIN_GPFSigHandler(int sig, siginfo_t *si, ucontext_t *uc)
{
    struct TWIN_i386_context_s  *sc;
    
    sc = (struct TWIN_i386_context_s *) uc->uc_mcontext.UCONTEXT_REGS;
    TWIN_GPFHandler(sig, sc);
    TWIN_ValidateSegmentRegisters(sc);
}

static void
TWIN_DebugSigHandler (int sig, siginfo_t *si, ucontext_t *uc)
{
    struct TWIN_i386_context_s  *sc;

    sc = (struct TWIN_i386_context_s *) uc->uc_mcontext.UCONTEXT_REGS;

    if (sig == SIGTRAP)
	sc->eip--;
    
    wine_debug(sig, sc);
    TWIN_ValidateSegmentRegisters(sc);
}

int
TWIN_RegisterGPFSignal(int sig)
{
    if (!altstackdefined)
    {
	stack_t ss;
	
	altstackdefined = 1;
	ss.ss_sp = TWIN_Stack;
	ss.ss_size = sizeof(TWIN_Stack);
	ss.ss_flags = 0;
	if (sigaltstack(&ss, NULL) < 0)
	    perror("sigaltstack");
    }
    
    sact.sa_handler = TWIN_GPFSigHandler;
    sact.sa_flags = SA_SIGINFO | SA_ONSTACK;
    return sigaction(sig, &sact, NULL);
}

int
TWIN_RegisterDebugSignal(int sig)
{
    if (!altstackdefined)
    {
	stack_t ss;
	
	altstackdefined = 1;
	ss.ss_sp = TWIN_Stack;
	ss.ss_size = sizeof(TWIN_Stack);
	ss.ss_flags = 0;
	if (sigaltstack(&ss, NULL) < 0)
	    perror("sigaltstack");
    }
    
    sact.sa_handler = TWIN_DebugSigHandler;
    sact.sa_flags = SA_SIGINFO | SA_ONSTACK;
    return sigaction(sig, &sact, NULL);
}

