/*
    LXLoader - Loads LX exe files or DLLs for execution or to extract information from.
    Copyright (C) 2007  Sven Rosén (aka Viking)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PROCESSLX_H
#define PROCESSLX_H

#include <memmgr.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "gcc_os2def.h"
/* Process Information Block structure. */
#ifdef INCL_DOSPROCESS
typedef struct _PIB {
  ULONG     pib_ulpid;     /*  Process identifier. */
  ULONG     pib_ulppid;    /*  Parent process identifier. */
  ULONG     pib_hmte;      /*  Module handle of executable program. */
  PCHAR     pib_pchcmd;    /*  Command line pointer. */
  PCHAR     pib_pchenv;    /*  Environment pointer. */
  ULONG     pib_flstatus;  /*  Process' status bits. */
  ULONG     pib_ultype;    /*  Process' type code. */
} PIB;

typedef PIB *PPIB;
#endif
/*
An OS/2 application that has been loaded into memory and prepared for execution is 
called a process. A process is the code, data, and other resources of an application, 
such as file handles, semaphores, pipes, queues, and so on. The OS/2 operating system 
considers every application it loads to be a process. 

Information about a process is kept in a read/write area of the process address space, 
called the Process Information Block (PIB). The operating system creates and maintains 
a PIB for every process in the system. 

An application can access the PIB of a specific process using DosGetInfoBlocks. 
*/


/* Thread identity.  */
#ifdef INCL_DOSPROCESS
//typedef LHANDLE TID;


/* System-specific Thread Information Block structure.  */

typedef struct _TIB2 {
  ULONG      tib2_ultid;         /* Current thread identifier. */
  ULONG      tib2_ulpri;         /* Current thread priority. */
  ULONG      tib2_version;       /* Version number for this system-specific Thread Information Block.*/
  USHORT     tib2_usMCCount;     /* Must-complete count. */
  USHORT     tib2_fMCForceFlag;  /* Must-complete force flag. */
} TIB2;

typedef TIB2 *PTIB2;


/* Thread Information Block structure. */

typedef struct _TIB {
  PVOID     tib_pexchain;     /*  Head of exception handler chain. */
  PVOID     tib_pstack;       /*  Pointer to the base of the stack. */
  PVOID     tib_pstacklimit;  /*  Pointer to the end of the stack. */
  PTIB2     tib_ptib2;        /*  Pointer to a system-specific thread information block. */
  ULONG     tib_version;      /*  Version number for this Thread Information Block. */
  ULONG     tib_ordinal;      /*  Thread ordinal number. */
} TIB;

typedef TIB *PTIB;

#endif
/*
A thread is a dispatchable unit of executable code that consists of  set of instructions, 
related CPU register values, and a stack. Every process has at least one thread and can 
have many threads running at the same time. The application runs when the operating 
system gives control to a thread in a process. A thread is the basic unit of execution. 

Information about a thread is kept in a read/write area of the process address space, 
called the Thread Information Block (TIB). The operating system creates and maintains 
a TIB for every thread in the system. 

An application can access the TIB of a specific thread using DosGetInfoBlocks. 

Note:  The fields of this data structure should not be modified unless you are switching 
       stacks, in which case only the tib_pstack and tib_pstacklimit should changed. 
*/

struct t_processlx {
	struct LX_module * lx_mod;
	int pid;
	
	void * code_mmap;
	void * stack_mmap;
	struct _PIB * lx_pib;
	struct _TIB * main_tid;
	struct t_mem_area root_mem_area;
};

struct t_processlx * processlx_create(struct LX_module * lx_m);

void processlx_destroy(struct t_processlx * proc);


#ifdef __cplusplus
};
#endif

#endif
