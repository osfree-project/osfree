/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      process.c                                                     */
/*                                                                            */
/* Description: This file includes the code to support processes and threads. */
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


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* #ifdef HAVE_CONFIG_H

/* now include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_DOSPROCESS
#define INCL_ERRORS
#include "os2.h"

/* include the linux headers and our local stuff */
#include "os2linux.h"


/*----------------------------------------------------------------------------*/
/* Local definitions                                                          */
/*----------------------------------------------------------------------------*/

#define EXEC_PGM_SLEEP 10  /* 10 milliseconds */


/*============================================================================*/
/* C APIs for threads                                                         */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* _beginthread                                                               */
/*    Notes:                                                                  */
/*       - you may have to remove the __cdecl definition from your code       */
/*----------------------------------------------------------------------------*/

unsigned long _beginthread(void(*start_address )( void * ),
                           unsigned stack_size, void *arglist)
{
    SAVEENV;
    pthread_t newThread;
    pthread_attr_t attr;
    int rc;
    size_t newStackSize;

    /* set the attributes for the thread */
    pthread_attr_init(&attr );
    if (stack_size == 0) {
        pthread_attr_setstacksize(&attr, (size_t)4096);
    }
    else {
        newStackSize = ((stack_size / 4096) + 1) * 4096;
        pthread_attr_setstacksize(&attr, (size_t)newStackSize);
    }
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    /* create the thread */
    rc = pthread_create(&newThread, &attr, (void *)start_address, arglist);
    if (rc) {
        RESTOREENV_RETURN(-1);
    }

    /* set return info and return code */
    RESTOREENV_RETURN((unsigned long)newThread);
}


/*----------------------------------------------------------------------------*/
/* _endthread                                                                 */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

void _endthread(void)
{
    SAVEENV;

    pthread_exit(NULL);
    RESTOREENV;
}


/*============================================================================*/
/* OS/2 APIs for processes and threads                                        */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* DosCreateThread                                                            */
/*    Notes:                                                                  */
/*       - all non-zero values for flag have been poisoned                    */
/*       - all possible OS/2 return codes are not used                        */
/*       - ERROR_INVALID_PARAMETER return code has been added                 */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosCreateThread(PTID ptid, PFNTHREAD pfn, ULONG param,
                                ULONG flag, ULONG cbStack)
{
    SAVEENV;
    pthread_t newThread;
    pthread_attr_t attr;
    int rc;
    size_t newStackSize;

    /* check the flag */
    if (flag) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    /* set the attributes for the thread */
    pthread_attr_init(&attr );
    if (cbStack == 0) {
        pthread_attr_setstacksize(&attr, (size_t)4096);
    }
    else {
        newStackSize = ((cbStack / 4096) + 1) * 4096;
        pthread_attr_setstacksize(&attr, (size_t)newStackSize);
    }
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    /* create the thread */
    rc = pthread_create(&newThread, &attr, (void *)pfn, (void *)param);
    if (rc) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }

    /* set return info and return code */
    *ptid = newThread;
    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosResumeThread                                                            */
/*    Notes:                                                                  */
/*       - all possible OS/2 return codes are not used                        */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosResumeThread(TID tid)
{
    SAVEENV;
    int rc;

    pthread_kill((pthread_t)tid, SIGCONT);
    if (rc == ESRCH) {
        RESTOREENV_RETURN(ERROR_INVALID_THREADID);
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosSuspendThread                                                           */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSuspendThread(TID tid)
{
    SAVEENV;
    int rc;

    rc = pthread_kill((pthread_t)tid, SIGTSTP);
    if (rc) {
        RESTOREENV_RETURN(ERROR_INVALID_THREADID);
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosGetInfoBlocks                                                           */
/*    Notes:                                                                  */
/*       - the tib is not done yet                                            */
/*       - the pib is not finished yet (see os2.c the _init function)         */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosGetInfoBlocks(PTIB *pptib, PPIB *pppib)
{
    SAVEENV;
    PTIB ptib;
    PTIB2 ptib2;
    int policy;
    struct sched_param param;

    if (pptib) {
        ptib = pthread_getspecific(DosLinuxStruct.thrdTIBKey);
        if (ptib == NULL) {
            ptib2 = malloc(sizeof(TIB2));
            ptib = malloc(sizeof(TIB));
            ptib->tib_pexchain = NULL; /* not supported */
            ptib->tib_pstack = NULL; /* not supported */
            ptib->tib_pstacklimit = NULL; /* not supported */
            ptib->tib_ptib2 = ptib2;
            ptib->tib_version = 20;
            ptib->tib_ordinal = 0; /* not supported */
            ptib->tib_ptib2->tib2_ultid = (ULONG)pthread_self();
            ptib->tib_ptib2->tib2_version = 20;
            ptib->tib_ptib2->tib2_usMCCount = 0; /* not supported */
            ptib->tib_ptib2->tib2_fMCForceFlag = 0; /* not supported */
            pthread_setspecific(DosLinuxStruct.thrdTIBKey, ptib);
        }

        /* this part of the TIB is dynamic */
        pthread_getschedparam(pthread_self(), &policy, &param);
        ptib->tib_ptib2->tib2_ulpri = (ULONG)param.__sched_priority;

        *pptib = ptib;
    }

    if (pppib) {
        *pppib = &DosLinuxStruct.pib;
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosKillThread                                                              */
/*    Notes:                                                                  */
/*       - all possible OS/2 return codes are not used                        */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosKillThread(TID tid)
{
    SAVEENV;
    int rc;

    rc = pthread_cancel((pthread_t)tid);
    if (rc == ESRCH) {
        RESTOREENV_RETURN(ERROR_INVALID_THREADID);
    } else if (rc) {
        RESTOREENV_RETURN(ERROR_BUSY); /* this is the catch-all error code */
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosAllocThreadLocalMemory                                                  */
/*    Notes:                                                                  */
/*       - we actually will allow more total bytes to be allocated            */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosAllocThreadLocalMemory(ULONG cb, PULONG *p)
{
    SAVEENV;
    PTHRDSPECIFICMEM thrdMem;
    int ctr;

    /* check param */
    if (cb == 0 || cb > 8) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    /* do we have any thread specific memory allocated yet? */
    thrdMem = pthread_getspecific(DosLinuxStruct.thrdKey);
    if (thrdMem == NULL) {
        thrdMem= malloc(sizeof(THRDSPECIFICMEM));
        if (thrdMem == NULL) {
            RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
        }
        memset(thrdMem, 0, sizeof(THRDSPECIFICMEM));
        pthread_setspecific(DosLinuxStruct.thrdKey, (void *)thrdMem);
    }

    /* find an empty allocation spot */
    for (ctr = 0; ctr < 8; ctr++) {
        if (thrdMem->used[ctr] == NULL) {
            thrdMem->used[ctr] = malloc(8 * cb);
            if (thrdMem->used[ctr] == NULL) {
                RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
            }
            memset(thrdMem->used[ctr], 0, 8);
            *p = (PULONG)&thrdMem->used[ctr];
            RESTOREENV_RETURN(NO_ERROR);
        }
    }

    RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
}


/*----------------------------------------------------------------------------*/
/* DosFreeThreadLocalMemory                                                   */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosFreeThreadLocalMemory(ULONG *p)
{
    SAVEENV;
    PTHRDSPECIFICMEM thrdMem;
    thrdMem = pthread_getspecific(DosLinuxStruct.thrdKey);
    int ctr;

    /* reset the pointer slot */
    for (ctr = 0; ctr < 8; ctr++) {
        if (p == thrdMem->used[ctr]) {
            free(thrdMem->used[ctr]);
            thrdMem->used[ctr] = NULL;
            RESTOREENV_RETURN(NO_ERROR);
        }
    }

    /* pointer not found in available slots */
    RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
}


/*----------------------------------------------------------------------------*/
/* DosWaitChild                                                               */
/*    Notes:                                                                  */
/*       - an action of DCWA_PROCESSTREE is not supported                     */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosWaitChild(ULONG action, ULONG option, PRESULTCODES pres,
                             PPID ppid, PID pid)
{
    SAVEENV;
    int stat;
    pid_t retpid;

    /* check param */
    if (action == DCWA_PROCESSTREE) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    /* perform wait */
    if (action == DCWW_NOWAIT) {
        retpid = waitpid(pid, &stat, WNOHANG);
    }
    else if (action == DCWW_WAIT) {
        retpid = waitpid(pid, &stat, 0);
    }
    else {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    /* check return code */
    if (retpid == -1) {
        switch (errno) {
        case ECHILD:
            RESTOREENV_RETURN(ERROR_INVALID_PROCID);
        case EINVAL:
            RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
        default:
            RESTOREENV_RETURN(ERROR_INVALID_DATA);
        }
    }

    /* set return values */
    *ppid = retpid;
    if (WIFSIGNALED(stat)) {
        if (WTERMSIG(stat) == SIGKILL) {
            pres->codeTerminate = TC_KILLPROCESS;
        }
        else {
            pres->codeTerminate = TC_EXCEPTION;
        }
    }
    else {
        pres->codeTerminate = TC_EXIT;
    }
    pres->codeResult = (ULONG)WEXITSTATUS(stat);

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosWaitThread                                                              */
/*    Notes:                                                                  */
/*       - a TID of zero is not allowed (wait for next thread to end)         */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosWaitThread(PTID ptid, ULONG option)
{
    SAVEENV;
    int rc;
    int policy;
    struct sched_param sched;

    /* check param */
    if (*ptid == 0) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    /* check option and wait/nowait for the thread */
    if (option == DCWW_NOWAIT) {
        /* check to see if the thread is still running */
        rc = pthread_getschedparam(*ptid, &policy, &sched);
        if (rc == ESRCH) {
            RESTOREENV_RETURN(ERROR_INVALID_THREADID);
        }
        else if (rc) {
            RESTOREENV_RETURN(ERROR_INTERRUPT);
        }
        else {
            RESTOREENV_RETURN(ERROR_THREAD_NOT_TERMINATED);
        }
    }
    else if (option == DCWW_WAIT) {
        /* wait for the thread to end */
        rc = pthread_join(*ptid, NULL);
        if (rc == ESRCH || rc == EDEADLK) {
            RESTOREENV_RETURN(ERROR_INVALID_THREADID);
        }
        else if (rc == EINVAL) {
            RESTOREENV_RETURN(ERROR_THREAD_NOT_TERMINATED);
        }
        else if (rc) {
            RESTOREENV_RETURN(ERROR_INTERRUPT);
        }
    }
    else {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosSleep                                                                   */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSleep(ULONG msec)
{
    SAVEENV;

    if (usleep(msec * 1000)) {
        RESTOREENV_RETURN(ERROR_TS_WAKEUP);
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosEnterCritSec                                                            */
/*    Notes:                                                                  */
/*       - all possible OS/2 return codes are not used                        */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosEnterCritSec(VOID)
{
    SAVEENV;
    sigset_t allsigs;

    if (sigfillset(&allsigs) == 0) {
        sigprocmask(SIG_BLOCK, &allsigs, NULL);
    }
    pthread_mutex_lock(&DosLinuxStruct.critSecMutex);

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosExitCritSec                                                             */
/*    Notes:                                                                  */
/*       - all possible OS/2 return codes are not used                        */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosExitCritSec(VOID)
{
    SAVEENV;
    sigset_t allsigs;

    if (sigfillset(&allsigs) == 0) {
        sigprocmask(SIG_UNBLOCK, &allsigs, NULL);
    }
    pthread_mutex_unlock(&DosLinuxStruct.critSecMutex);

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosExitList                                                                */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosExitList(ULONG ordercode, PFNEXITLIST pfn)
{
    int i;

    if (ordercode & EXLST_EXIT) {
        /* we do not require any processing here */
        return NO_ERROR;
    }
    else if (ordercode & EXLST_REMOVE) {
        /* remove the pfn entry from the list */
        for (i = 0; i < MAX_EXIT_LIST; i++) {
            if (DosLinuxStruct.exitList[i].func == pfn) {
                DosLinuxStruct.exitList[i].func = NULL;
                DosLinuxStruct.exitList[i].ordercode = 0;
                /* do NOT DosLinuxStruct.exitList[i]used = FALSE */
                /* this will cause problems with lower order exit functions */
                break;
            }
        }
        if (i == MAX_EXIT_LIST) {
            return ERROR_INVALID_FUNCTION;
        }
    }
    else if (ordercode & EXLST_ADD) {
        /* add the pfn entry to the list */
        for (i = 0; i < MAX_EXIT_LIST; i++) {
            if (DosLinuxStruct.exitList[i].func == NULL &&
             DosLinuxStruct.exitList[i].used == FALSE) {
                DosLinuxStruct.exitList[i].func = pfn;
                DosLinuxStruct.exitList[i].ordercode = ordercode >> 8;
                DosLinuxStruct.exitList[i].used = TRUE;
                break;
            }
        }
        if (i == MAX_EXIT_LIST) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    else {
        /* unrecognized order code */
        return ERROR_INVALID_DATA;
    }

    return NO_ERROR;
}


/*----------------------------------------------------------------------------*/
/* DosExecPgm                                                                 */
/*    Notes:                                                                  */
/*       - the pEnv argument must be NULL                                     */
/*       - the execFlag argument can only be EXEC_SYNC and EXEC_ASYNC         */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosExecPgm(PCHAR pObjname, LONG cbObjname, ULONG execFlag,
                           PCSZ pArg, PCSZ pEnv, PRESULTCODES pRes, PCSZ pName)
{
    SAVEENV;
    char **argv = NULL;
    int rc;
    pid_t pid, cpid, npid;
    char * pszArgStr;
    int status;
    APIRET apiret = NO_ERROR;

    /* check args */
    if (pEnv != NULL) {
        RESTOREENV_RETURN(ERROR_BAD_ENVIRONMENT);
    }
    if (execFlag > EXEC_ASYNC) {
        RESTOREENV_RETURN(ERROR_INVALID_FUNCTION);
    }

    /* initialize some variables */
    if (pArg != NULL) {
        pszArgStr = alloca(strlen(pName) + strlen(pArg) + 2);
    }
    else {
        pszArgStr = alloca(strlen(pName) + 1);
    }
    if (pszArgStr == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    strcpy(pszArgStr, pName);
    if (pArg != NULL) {
        strcat(pszArgStr, " ");
        strcat(pszArgStr, pArg);
    }
    rc = DosMakeArgv(pszArgStr, " \t", &argv);
    if (rc == -1) {
        if (argv) {
            DosFreeMakeArgv(argv);
        }
        RESTOREENV_RETURN(ERROR_INVALID_DATA);
    }
    *pObjname = '\0';

    /* fork a child process */
    pid = fork();
    switch (pid) {
    case -1:
        /* fork returned an error */
        switch (errno) {
        case EAGAIN:
        case ENOMEM:
            apiret = ERROR_NOT_ENOUGH_MEMORY;
            break;
        default:
            apiret = ERROR_INVALID_DATA;
        }
        break;
    case 0:
        /* this is the child process - run the specified program */
        npid = setsid();
        if (npid == -1) {
            exit (ERROR_ACCESS_DENIED);
        }
        if (strchr(pName, '/')) {
            /* assume we have the full path name */
            execv(pName, argv);
        }
        else {
            /* assume we do not have the full path name */
            execvp(pName, argv);
        }
        /* if execv or execvp return check the error */
        switch (errno) {
        case EACCES:
        case EPERM:
            exit(ERROR_ACCESS_DENIED);
        case ENOEXEC:
            exit(ERROR_EXE_MARKED_INVALID);
        case ENOENT:
            exit(ERROR_FILE_NOT_FOUND);
        case ENOMEM:
            exit(ERROR_NOT_ENOUGH_MEMORY);
        case ENOTDIR:
            exit(ERROR_PATH_NOT_FOUND);
        case ELOOP:
        case ENFILE:
        case EMFILE:
            exit(ERROR_TOO_MANY_OPEN_FILES);
        case ETXTBSY:
            exit(ERROR_LOCK_VIOLATION);
        default:
            exit(ERROR_BAD_FORMAT);
        }
        break;
    default:
        /* this is the original process */
        switch (execFlag) {
        case EXEC_SYNC:
            cpid = waitpid(pid, &status, 0);
            if (cpid == -1) {
                apiret = ERROR_INVALID_DATA;
                break;
            }
            if (WIFEXITED(status)) {
                pRes->codeTerminate = 0;
                pRes->codeResult = WIFEXITED(status);
            }
            break;
        case EXEC_ASYNC:
            pRes->codeTerminate = pid;
            pRes->codeResult = 0;
            /* sleep for a while so we can get any child errors */
            DosSleep(EXEC_PGM_SLEEP);
            cpid = waitpid(pid, &status, WNOHANG);
            if (cpid == -1) {
                apiret = ERROR_INVALID_DATA;
            }
            else if (cpid == 0) {
                /* do nothing - assume child loaded ok */
            }
            else {
                apiret = WEXITSTATUS(status);
            }
            break;
        default:
            apiret = ERROR_INVALID_FUNCTION;
        }
    }

    DosFreeMakeArgv(argv);
    RESTOREENV_RETURN(apiret);
}


/*----------------------------------------------------------------------------*/
/* DosSetPriority                                                             */
/*    Notes:                                                                  */
/*       - this function only supports thread priority modification           */
/*       - sets absolute values for the priority, not a range value           */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY  DosSetPriority(ULONG scope, ULONG ulClass, LONG delta,
                                ULONG PorTid)
{
    SAVEENV;
    int policy;
    int rc;
    struct sched_param param;

    if (scope != 2) {
        RESTOREENV_RETURN(ERROR_INVALID_SCOPE);
    }

    pthread_getschedparam((TID)PorTid, &policy, &param);

    /* set the policy from the OS/2 class */
    if (ulClass == PRTYC_IDLETIME || ulClass == PRTYC_REGULAR) {
        policy = SCHED_RR;
    }
    else if (ulClass == PRTYC_TIMECRITICAL || ulClass == PRTYC_FOREGROUNDSERVER) {
        policy = SCHED_FIFO;
    }

    /* set the priority from the OS/2 delta */
    if (delta < 0 && delta >= -31) {
        param.__sched_priority = sched_get_priority_min(policy);
    }
    else if (delta > 0 && delta <= 31) {
        param.__sched_priority = sched_get_priority_max(policy);
    }
    else if (delta == 0) {
        param.__sched_priority = (sched_get_priority_min(policy) +
         sched_get_priority_max(policy)) / 2;
    }
    else {
        RESTOREENV_RETURN(ERROR_INVALID_PDELTA);
    }

    /* now change the priority */
    rc = pthread_setschedparam((TID)PorTid, policy, &param);
    if (rc) {
        switch (errno) {
        case EINVAL:
            RESTOREENV_RETURN(ERROR_INVALID_PDELTA);
        case ESRCH:
            RESTOREENV_RETURN(ERROR_INVALID_THREADID);
        default:
            RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
        }
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosKillProcess                                                             */
/*    Notes:                                                                  */
/*       - all non-zero values for action have been poisoned                  */
/*       - all possible OS/2 return codes are not used                        */
/*       - ERROR_INVALID_PARAMETER return code has been added                 */
/*       - ERROR_ACCESS_DENIED return code has been added                     */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosKillProcess(ULONG action, PID pid)
{
    SAVEENV;
    int rc;

    /* check param */
    if (action) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    /* try to kill the process */
    rc = kill(pid, SIGKILL);
    if (rc == ESRCH) {
        RESTOREENV_RETURN(ERROR_INVALID_PROCID);
    }
    else if (rc) {
        RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosBeep                                                                    */
/*    Notes:                                                                  */
/*       - this function ignores all the input arguments and instead rings    */
/*         the machine bell using the ASCII BEL character                     */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosBeep(ULONG freq, ULONG dur)
{
    putc('\a', stderr);
    fflush(stderr);
    return NO_ERROR;
}

