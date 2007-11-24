/*-- H -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      os2linux.h                                                    */
/*                                                                            */
/* Description: Local include file for the OS2Linx Project. This include      */
/*              file is not needed or used by client programs. It is only     */
/*              needed by the library itself. Client programs should use      */
/*              the OS/2 header files supplied in the project header          */
/*              subdirectory.                                                 */
/*                                                                            */
/* Copyright (C) IBM Corporation 2003. All Rights Reserved.                   */
/* Copyright (C) W. David Ashley 2004, 2005. All Rights Reserved.             */
/*                                                                            */
/* Author(s):                                                                 */
/*      W. David Ashley  <dashley@us.ibm.com>                                 */
/*                                                                            */
/* This software is subject to the terms of the Common Public License v1.0.   */
/* You must accept the terms of this license to use this software.            */
/*                                                                            */
/* This program is distributed in the hope that it will be useful, but        */
/* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY */
/* or FITNESS FOR A PARTICULAR PURPOSE.                                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/


#ifndef OS2LINUX_H

#define OS2LINUX_H

/* include the standard linux stuff */
#define __USE_GNU
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define __USE_GNU
#include <fcntl.h>
#undef __USE_GNU
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>
//#include <sys/msg.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>
#include <dlfcn.h>
//#include <semaphore.h>
#include <syslog.h>
#include <alloca.h>
//#include <sched.h>
//#include <glob.h>
#include <utime.h>
#include <nl_types.h>
//#include <curses.h>

// Support of usleep function
#include <l4/util/util.h>

/*----------------------------------------------------------------------------*/
/* Definitions                                                                */
/*----------------------------------------------------------------------------*/

/* macros */
#define SAVEENV int save_errno = errno
#define RESTOREENV errno = save_errno
#define RESTOREENV_RETURN(x) RESTOREENV; return x

/* number of async timers available in the system */
/* Note: if you increase this number you will need to create an */
/* additional function for each new slot. See the os2.c file.   */
#define TIMERSLOTS 10

/* maximun available process exit functions */
#define MAX_EXIT_LIST 10

/* storage for DosAllocThreadLocalMemory and DosFreeThreadLocalMemory */
typedef struct _THRDSPECIFICMEM {
   void * used[8];
} THRDSPECIFICMEM;
typedef THRDSPECIFICMEM *PTHRDSPECIFICMEM;

/* OS/2 async timer storage */
typedef struct _DOSASYNCTIMERSTRUCT {
    timer_t timerid;
    HEV hSem;
    struct itimerspec value;
    BOOL oneshot;
} DOSASYNCTIMERSTRUCT;
typedef DOSASYNCTIMERSTRUCT *PDOSASYNCTIMERSTRUCT;

/* DosExitList storage */
typedef struct _DOSEXITLISTSTRUCT {
    ULONG used;
    ULONG ordercode;
    PFNEXITLIST func;
} DOSEXITLISTSTRUCT;
typedef DOSEXITLISTSTRUCT *PDOSEXITLISTSTRUCT;

#if 0
!!! glob.h not found
/* storage for Linux HDIR */
typedef struct _LINUXHDIRSTRUCT {
    size_t ctr; /* glob entry counter */
    glob_t hdir;
} LINUXHDIRSTRUCT;
typedef LINUXHDIRSTRUCT *PLINUXHDIRSTRUCT;
#endif

/* storage for file/pipe handles */
typedef struct _FILEHANDLESTRUCT {
    char id[4]; /* set to "FIL" for a file or "PIP" for a pipe */
    int fh; /* actual Linux file handle */
    ULONG openmode; /* OS/2 open mode */
    PSZ npname; /* pipe name if created with DosCreateNPipe */
} FILEHANDLESTRUCT;
typedef FILEHANDLESTRUCT *PFILEHANDLESTRUCT;

/* main global support structure for the OS2Linux library */
typedef struct _DOSLINUXSTRUCT {
    /* OS/2 processes support */
    PIB pib;
    /* thread specific memory support */
//!!    pthread_key_t thrdKey;
    /* thread specific TIB support */
//!!    pthread_key_t thrdTIBKey;
    /* OS/2 process critical section mutes semaphore */
//!!    pthread_mutex_t critSecMutex;
    /* system-wide mutex semaphore for this library */
//!!    sem_t * SysMutexSem;
    /* support for an array of OS/2 async timers */
    PDOSASYNCTIMERSTRUCT timerslot[TIMERSLOTS];
    void (*asyncTimerHandler[TIMERSLOTS]) (int, siginfo_t *, void *);
    /* support for DosExitList */
    ULONG termcode;
    DOSEXITLISTSTRUCT exitList[MAX_EXIT_LIST];
    /* support for DosFindFirst, DosFindNext and DosFindClose */
//!!!    LINUXHDIRSTRUCT syshdir;
} DOSLINUXSTRUCT;
typedef DOSLINUXSTRUCT * PDOSLINUXSTRUCT;


/*----------------------------------------------------------------------------*/
/* Global variables                                                           */
/*----------------------------------------------------------------------------*/

extern DOSLINUXSTRUCT DosLinuxStruct;


/*----------------------------------------------------------------------------*/
/* Global function declarations                                               */
/*----------------------------------------------------------------------------*/

int APIENTRY DosMakeArgv(const char *s, const char *delimiters, char ***argvp);
void APIENTRY DosFreeMakeArgv(char **argv);
void APIENTRY DosNameConversion(PSZ pszName, PUCHAR pSeparator, PUCHAR pRepl,
                                BOOL uc);
VOID APIENTRY DosRequestSysMutexSem(void);
VOID APIENTRY DosReleaseSysMutexSem(void);
APIRET APIENTRY DosLogError(PSZ pszOS2FuncName, PSZ pszCFuncName, int errnum);
key_t APIENTRY DosFtok(PSZ pszName);
void DosAsyncTimerHandler_0(int signo, siginfo_t *info, void *context);
void DosAsyncTimerHandler_1(int signo, siginfo_t *info, void *context);
void DosAsyncTimerHandler_2(int signo, siginfo_t *info, void *context);
void DosAsyncTimerHandler_3(int signo, siginfo_t *info, void *context);
void DosAsyncTimerHandler_4(int signo, siginfo_t *info, void *context);
void DosAsyncTimerHandler_5(int signo, siginfo_t *info, void *context);
void DosAsyncTimerHandler_6(int signo, siginfo_t *info, void *context);
void DosAsyncTimerHandler_7(int signo, siginfo_t *info, void *context);
void DosAsyncTimerHandler_8(int signo, siginfo_t *info, void *context);
void DosAsyncTimerHandler_9(int signo, siginfo_t *info, void *context);
void VioInitCurses (VOID);
void VioDeInitCurses (VOID);


#endif  /* #ifndef OS2LINUX_H */

