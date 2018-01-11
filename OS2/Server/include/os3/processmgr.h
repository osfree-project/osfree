/*
    LXLoader - Loads LX exe files or DLLs for execution or to extract information from.
    Copyright (C) 2007  Sven Rosén (aka Viking)

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
    Or see <http://www.gnu.org/licenses/>
*/

#ifndef PROCESSLX_H
#define PROCESSLX_H

#ifdef __cplusplus
  extern "C" {
#endif

//#if defined(__WIN32__) || defined(__LINUX__)
/* OS/2 API includes */
#define  INCL_BASE
#define  INCL_DOS
#define  INCL_BSEDOS
#define  INCL_DOSPROCESS
#include <os2.h>
//#endif

/* osFree OS/2 personality internal */
#include <os3/thread.h>
#include <os3/memmgr.h>
#include <os3/modmgr.h>
#include <os3/ixfmgr.h>

#ifdef __l4env__
#include <l4/semaphore/semaphore.h>
#endif

#ifndef __OS2__
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
//#endif /* INCL_DOSPROCESS */
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
  // private fields:
  ULONG       tib_eip_saved;
  ULONG       tib_esp_saved;
} TIB;

typedef TIB *PTIB;


 
struct _RESULTCODES {
    ULONG codeTerminate;
    ULONG codeResult;
};

typedef struct _RESULTCODES RESULTCODES;
typedef struct _RESULTCODES *PRESULTCODES;

#endif

#endif

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

struct t_os2process {
        //struct LX_module * lx_mod;
        ULONG hmte;
        int pid;

        char exec_sync;  // synchronous execution flag (whether to use term_sem or not)
#ifdef __l4env__
        l4semaphore_t startup_sem; // child program startup wait semaphore
        l4semaphore_t term_sem;    // child program termination wait semaphore
#endif
        l4_os3_task_t task;
        ULONG term_code; // termination code of last child program
        void *ip;
        void *sp;
        PPIB lx_pib;
        //PTIB main_tib;
        PTIB tib_array[128]; // array of pointers to TIB with (TID-1) index
        l4_os3_thread_t tid_array[128]; // array of l4thread ID's   with (TID-1) index
        char curdisk;
        char curdir[256];
        struct t_mem_area root_mem_area;
        struct t_os2process *next;
        struct t_os2process *prev;
};

#define MAX_TID 128

APIRET CDECL PrcCreatePIB(PPIB *addr, PSZ prg, PSZ arg, PSZ env);
APIRET CDECL PrcCreateTIB(PTIB *addr);
APIRET CDECL PrcDestroyTIB(PID pid, TID tid);

struct t_os2process *PrcGetProc(ULONG pid);

struct t_os2process *PrcGetProcNative(l4_os3_thread_t thread);
TID PrcGetTIDNative(l4_os3_thread_t thread);
APIRET CDECL PrcNewTIB(PID pid, TID tid, l4_os3_thread_t id);
l4_os3_thread_t PrcGetNativeID(PID pid, TID tid);

// APIRET APIENTRY PrcCreate(IXFModule ixfModule,
//                           struct t_os2process * process);
//struct t_os2process *PrcCreate(IXFModule *ixfModule);
struct t_os2process *PrcCreate(ULONG ppid, PSZ pPrg, PSZ pArg, PSZ pEnv);

void PrcDestroy(struct t_os2process * proc);

APIRET PrcSetArgsEnv(PSZ pPrg, PSZ pArg, PSZ pEnv, struct t_os2process *proc);

#define EXEC_SYNC           0
#define EXEC_ASYNC          1
#define EXEC_ASYNCRESULT    2
#define EXEC_TRACE          3
#define EXEC_BACKGROUND     4
#define EXEC_LOAD           5
#define EXEC_ASYNCRESULTDB  6

APIRET APIENTRY PrcExecuteModule(char * pObjname,
                                 long cbObjname,
                                 unsigned long execFlag,
                                 char * pArg,
                                 char * pEnv,
                                 struct _RESULTCODES *pRes, /*PRESULTCODES */
                                 char * pName,
                                 unsigned long ppid);

#ifdef __cplusplus
  }
#endif

#endif
