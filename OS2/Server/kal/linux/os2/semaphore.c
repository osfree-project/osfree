/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      semaphore.c                                                   */
/*                                                                            */
/* Description: This file includes the code to support semaphores.            */
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
#define INCL_DOSSEMAPHORES
#define INCL_DOSPROCESS
#define INCL_ERRORS
#include "os2.h"

/* include the linux headers and our local stuff */
#include "os2linux.h"


/*----------------------------------------------------------------------------*/
/* Local definitions                                                          */
/*----------------------------------------------------------------------------*/

#define MAX_SEMS 64
#define SEM_WAIT_PERIOD 100 /* POSIX says this should be 10ms */

/* storage to hold semaphore handles */
typedef struct _OS2Sem {
    char id[4];            /* always set to "SEM" for identification purposes */
    char type[4];          /* type of semaphore, "EVE", "MTX" or "MUX" */
    sem_t * posixSem;      /* the real semaphore */
    unsigned long named;   /* TRUE if this is a named semaphore */
    /* the following fields are only used by muxwait semaphores */
    BOOL waitall;                /* wait for all sems? */
    int numSems;                 /* number of semaphore records */
    SEMRECORD semrec[MAX_SEMS];  /* semaphore records */
} OS2Sem;
typedef OS2Sem * POS2Sem;


/*============================================================================*/
/* OS/2 APIs for event semaphores                                             */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* DosCreateEventSem                                                          */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosCreateEventSem (PCSZ pszName, PHEV phev, ULONG flAttr,
                                   BOOL32 fState)
{
    SAVEENV;
    int rc;
    POS2Sem os2_sem;
    PSZ pszSemName;

    /* check argument */
    if (pszName != NULL && flAttr > 1) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    /* perform the function */
    os2_sem = (POS2Sem)malloc(sizeof(OS2Sem));
    if (os2_sem == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    strcpy(os2_sem->id, "SEM");
    strcpy(os2_sem->type, "EVE");
    if (pszName == NULL) {
        /* this is an unnamed semaphore */
        os2_sem->posixSem = (sem_t *)malloc(sizeof(sem_t));
        rc = sem_init(os2_sem->posixSem, (int)flAttr, 0);
        if (rc == -1) {
            free(os2_sem->posixSem);
            free(os2_sem);
            RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
        }
        os2_sem->named = FALSE;
        os2_sem->numSems = 0;
    }
    else {
        /* this is a named semaphore */
        pszSemName = alloca(strlen(pszName) + 1);
        if (pszSemName == NULL) {
            free(os2_sem->posixSem);
            RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
        }
        strcpy(pszSemName, pszName);
        DosNameConversion(pszSemName, "\\", ".", TRUE);
        if (*pszSemName == '.') {
            *pszSemName = '/';
        }
        sem_unlink(pszSemName);
        os2_sem->posixSem = sem_open(pszSemName, (O_CREAT | O_EXCL),
                                       (S_IRWXU | S_IRWXG), 0);
        if (os2_sem->posixSem == SEM_FAILED ) {
            switch (errno) {
            case EEXIST:
                free(os2_sem);
                RESTOREENV_RETURN(ERROR_DUPLICATE_NAME);
            case EINVAL:
            case ENAMETOOLONG:
                free(os2_sem);
                RESTOREENV_RETURN(ERROR_INVALID_NAME);
            case ENFILE:
            case EMFILE:
                free(os2_sem);
                RESTOREENV_RETURN(ERROR_TOO_MANY_HANDLES);
            default:
                free(os2_sem);
                RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
            }
        }
        os2_sem->named = TRUE;
        os2_sem->numSems = 0;
    }
    if (fState) {
        rc = sem_post(os2_sem->posixSem);
    }

    /* now assign the address of the semaphore to the OS/2 data type */
    *phev = (HEV)os2_sem;

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosOpenEventSem                                                            */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosOpenEventSem (PCSZ pszName, PHEV phev)
{
    SAVEENV;
    POS2Sem os2_sem;
    PSZ pszSemName;

    /* perform the function */
    os2_sem = (POS2Sem)malloc(sizeof(OS2Sem));
    if (os2_sem == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    strcpy(os2_sem->id, "SEM");
    strcpy(os2_sem->type, "EVE");
    if (pszName == NULL) {
        /* this is an unnamed semaphore, no problem, just dup it */
        os2_sem->named = FALSE;
        memcpy(os2_sem, (POS2Sem)phev, sizeof(OS2Sem));
    }
    else {
        /* this is a named semaphore */
        pszSemName = alloca(strlen(pszName) + 1);
        if (pszSemName == NULL) {
            free(os2_sem->posixSem);
            RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
        }
        strcpy(pszSemName, pszName);
        DosNameConversion(pszSemName, "\\", ".", TRUE);
        if (*pszSemName == '.') {
            *pszSemName = '/';
        }
        os2_sem->posixSem = sem_open(pszSemName, 0);
        if (os2_sem->posixSem == SEM_FAILED ) {
            switch (errno) {
            case EEXIST:
                free(os2_sem);
                RESTOREENV_RETURN(ERROR_DUPLICATE_NAME);
            case EINVAL:
            case ENAMETOOLONG:
                free(os2_sem);
                RESTOREENV_RETURN(ERROR_INVALID_NAME);
            case ENFILE:
            case EMFILE:
                free(os2_sem);
                RESTOREENV_RETURN(ERROR_TOO_MANY_HANDLES);
            default:
                free(os2_sem);
                RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
            }
        }
        os2_sem->named = TRUE;
    }

    /* now assign the address of the semaphore to the OS/2 data type */
    *phev = (HEV)os2_sem;

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosCloseEventSem                                                           */
/*    Notes:                                                                  */
/*       - not all the return codes are used                                  */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosCloseEventSem (HEV hev)
{
    SAVEENV;
    POS2Sem os2_sem;

    os2_sem = (POS2Sem)hev;
    if (strcmp(os2_sem->id, "SEM")) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }

    if (os2_sem->named == FALSE) {
        /* this is an unnamed semaphore so we must free the target */
        free(os2_sem->posixSem);
    }
    else {
        /* this is a named semaphore */
        if (sem_close(os2_sem->posixSem)) {
            if (errno == EINVAL) {
                RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
            }
            else if (errno) {
                RESTOREENV_RETURN(ERROR_SEM_BUSY);
            }
        }
    }

    /* now free the main structure */
    free(os2_sem);

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosResetEventSem                                                           */
/*    Notes:                                                                  */
/*       - !!!!!!If the sem_t structure ever changes then this code may need  */
/*         to be repaired                                                     */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosResetEventSem (HEV hev, PULONG pulPostCt)
{
    SAVEENV;
    POS2Sem os2_sem;
    unsigned int v;

    os2_sem = (POS2Sem)hev;
    if (strcmp(os2_sem->id, "SEM")) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }

    sem_getvalue(os2_sem->posixSem, &v);
    if (v == 0) {
      RESTOREENV_RETURN(ERROR_ALREADY_RESET);
    }

    sem_init(os2_sem->posixSem, 1, 0);
    if (pulPostCt != NULL) {
      *pulPostCt = v;
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosPostEventSem                                                            */
/*    Notes:                                                                  */
/*       - this function will accept multiple posts without returning an      */
/*         error                                                              */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosPostEventSem (HEV hev)
{
    SAVEENV;
    int rc;
    POS2Sem os2_sem;

    os2_sem = (POS2Sem)hev;
    if (strcmp(os2_sem->id, "SEM")) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }
    rc = sem_post(os2_sem->posixSem);
    if (rc) {
        if (errno == EINVAL) {
            RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
        }
        else {
            RESTOREENV_RETURN(ERROR_TOO_MANY_POSTS);
        }
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosWaitEventSem                                                            */
/*    Notes:                                                                  */
/*       - specified timeouts are achieved using timed waits                  */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosWaitEventSem (HEV hev, ULONG ulTimeout)
{
    SAVEENV;
    int rc;
    POS2Sem os2_sem;

    os2_sem = (POS2Sem)hev;
    if (strcmp(os2_sem->id, "SEM")) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }
    if (ulTimeout < SEM_WAIT_PERIOD) {
        ulTimeout = SEM_WAIT_PERIOD;
    }
    if (ulTimeout != SEM_INDEFINITE_WAIT) {
        /* this looping construct will cause us to wait longer than the */
        /* specified timeout due to the latency involved in the loop,   */
        /* but that cannot be helped                                    */
        while (ulTimeout > 0) {
            rc = sem_trywait(os2_sem->posixSem);
            if (rc == 0) {
                break;
            }
            DosSleep(SEM_WAIT_PERIOD);
            ulTimeout -= SEM_WAIT_PERIOD;
        }
    }
    else {
        rc = sem_wait(os2_sem->posixSem);
    }
    if (rc) {
        switch (errno) {
        case EAGAIN:
            RESTOREENV_RETURN(ERROR_TIMEOUT);
        case EINVAL:
            RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
        case EINTR:
            RESTOREENV_RETURN(ERROR_INTERRUPT);
        default:
            RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
        }
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosQueryEventSem                                                           */
/*    Notes:                                                                  */
/*       - see notes about the sem_getvalue function                          */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryEventSem (HEV hev, PULONG pulPostCt)
{
    SAVEENV;
    int rc;
    POS2Sem os2_sem;
    int val;

    os2_sem = (POS2Sem)hev;
    if (strcmp(os2_sem->id, "SEM")) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }
    rc = sem_getvalue(os2_sem->posixSem, &val);
    if (rc) {
        if (errno == EINVAL) {
            RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
        }
        else {
            RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
        }
    }
    if (pulPostCt != NULL) {
        *pulPostCt = (ULONG)val;
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosCreateMutexSem                                                          */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosCreateMutexSem (PCSZ pszName, PHMTX phmtx, ULONG flAttr,
                                   BOOL32 fState)
{
    SAVEENV;
    APIRET apiret;
    POS2Sem os2_sem;

    /* create the semaphore */
    apiret = DosCreateEventSem(pszName, (PHEV)phmtx, flAttr, FALSE);
    if (!apiret) {
        os2_sem = (POS2Sem)*phmtx;
        strcpy(os2_sem->type, "MTX");
        if (!fState) {
            apiret = DosPostEventSem((HEV)*phmtx);
        }
    }

    RESTOREENV_RETURN(apiret);
}


/*----------------------------------------------------------------------------*/
/* DosOpenMutexSem                                                            */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosOpenMutexSem (PCSZ pszName, PHMTX phmtx)
{
    SAVEENV;
    APIRET apiret;
    POS2Sem os2_sem;

    /* open the semaphore */
    apiret = DosOpenEventSem(pszName, (PHEV)phmtx);
    if (!apiret) {
        os2_sem = (POS2Sem)*phmtx;
        strcpy(os2_sem->type, "MTX");
    }

    RESTOREENV_RETURN(apiret);
}


/*----------------------------------------------------------------------------*/
/* DosCloseMutexSem                                                           */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosCloseMutexSem (HMTX hmtx)
{
    return DosCloseEventSem((HEV)hmtx);
}


/*----------------------------------------------------------------------------*/
/* DosRequestMutexSem                                                         */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosRequestMutexSem (HMTX hmtx, ULONG ulTimeout)
{
    return DosWaitEventSem((HEV)hmtx, ulTimeout);
}


/*----------------------------------------------------------------------------*/
/* DosReleaseMutexSem                                                         */
/*    Notes:                                                                  */
/*       - !!!!! In order for this to work properly the DosRequestSysMutexSem */
/*         function needs to be paired with the DosReleaseSysMutex function   */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosReleaseMutexSem (HMTX hmtx)
{
    SAVEENV;
    APIRET apiret;
    ULONG postCnt;

    /* enter critical section */
    DosRequestSysMutexSem();

    /* do not post the semaphore if it is already posted */
    apiret = DosQueryEventSem((HEV)hmtx, &postCnt);
    if (apiret) {
        apiret = ERROR_LOCK_VIOLATION;
    }
    else if (postCnt == 0) {
        apiret = DosPostEventSem((HEV)hmtx);
    }

    /* exit critical section */
    DosReleaseSysMutexSem();

    RESTOREENV_RETURN(apiret);
}


/*----------------------------------------------------------------------------*/
/* DosQueryMutexSem                                                           */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryMutexSem (HMTX hmtx, PID *ppid, TID *ptid,
                                  PULONG pulCount)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosCreateMuxWaitSem                                                        */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosCreateMuxWaitSem (PCSZ pszName, PHMUX phmux,
                                     ULONG cSemRec, PSEMRECORD pSemRec,
                                     ULONG flAttr)
{
    SAVEENV;
    APIRET apiret;
    POS2Sem os2_sem;
    ULONG i;

    /* create the semaphore */
    apiret = DosCreateEventSem(pszName, (PHEV)phmux, 0, FALSE);
    if (apiret) {
        RESTOREENV_RETURN(apiret);
    }
    os2_sem = (POS2Sem)*phmux;
    strcpy(os2_sem->type, "MUX");

    /* now add the semaphore records */
    if (cSemRec > 0) {
        for (i = 0; i < cSemRec; i++) {
            os2_sem ->semrec[i] = pSemRec[i];
        }
    }
    os2_sem->numSems = cSemRec;
    if (flAttr & DCMW_WAIT_ALL) {
        os2_sem->waitall = TRUE;
    }
    else {
        os2_sem->waitall = FALSE;
    }

    RESTOREENV_RETURN(apiret);
}


/*----------------------------------------------------------------------------*/
/* DosOpenMuxWaitSem                                                          */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosOpenMuxWaitSem (PCSZ pszName, PHMUX phmux)
{
    SAVEENV;
    APIRET apiret;
    POS2Sem os2_sem;

    /* open the semaphore */
    apiret = DosOpenEventSem(pszName, (PHEV)phmux);
    if (!apiret) {
        os2_sem = (POS2Sem)*phmux;
        strcpy(os2_sem->type, "MUX");
    }

    RESTOREENV_RETURN(apiret);
}


/*----------------------------------------------------------------------------*/
/* DosCloseMuxWaitSem                                                         */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosCloseMuxWaitSem (HMUX hmux)
{
    return DosCloseEventSem((HEV)hmux);
}


/*----------------------------------------------------------------------------*/
/* DosAddMuxWaitSem                                                           */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosAddMuxWaitSem (HMUX hmux, PSEMRECORD pSemRec)
{
    SAVEENV;
    POS2Sem os2_sem;
    POS2Sem os2_sem_add;
    ULONG i;

    /* check to make sure we can add the new record */
    os2_sem = (POS2Sem)hmux;
    if (strcmp(os2_sem->id, "SEM")) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }
    if (strcmp(os2_sem->type, "MUX")) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }
    if (os2_sem->numSems == MAX_SEMS) {
        RESTOREENV_RETURN(ERROR_TOO_MANY_SEMAPHORES);
    }
    if (os2_sem->numSems > 0) {
        os2_sem_add = (POS2Sem)pSemRec->hsemCur;
        if (strcmp(os2_sem->id, os2_sem_add->id) != 0) {
            RESTOREENV_RETURN(ERROR_WRONG_TYPE);
        }
        for (i = 0; i < os2_sem->numSems; i++) {
            if (os2_sem->semrec[i].hsemCur == pSemRec->hsemCur) {
                RESTOREENV_RETURN(ERROR_DUPLICATE_HANDLE);
            }
        }
    }

    /* everthing checks out so we can add the sem record */
    os2_sem ->semrec[os2_sem->numSems] = pSemRec[0];
    os2_sem->numSems++;

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosDeleteMuxWaitSem                                                        */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosDeleteMuxWaitSem (HMUX hmux, HSEM hSem)
{
    SAVEENV;
    POS2Sem os2_sem;
    ULONG i, delsemrec = -1;

    /* find the record to delete */
    os2_sem = (POS2Sem)hmux;
    if (strcmp(os2_sem->id, "SEM")) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }
    if (strcmp(os2_sem->type, "MUX")) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }
    if (os2_sem->numSems == 0) {
        RESTOREENV_RETURN(ERROR_EMPTY_MUXWAIT);
    }
    for (i = 0; i < os2_sem->numSems; i++) {
        if (os2_sem->semrec[i].hsemCur == hSem) {
            delsemrec = i;
            break;
        }
    }
    if (i == -1) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }

    /* we found the record so move everything around */
    if (delsemrec != os2_sem->numSems - 1) {
        for (i = delsemrec; i < os2_sem->numSems - 1; i++) {
            os2_sem->semrec[i] = os2_sem->semrec[i + 1];
        }
    }
    os2_sem->numSems--;

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosWaitMuxWaitSem                                                          */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosWaitMuxWaitSem (HMUX hmux, ULONG ulTimeout,
                                   PULONG pulUser)
{
    SAVEENV;
    POS2Sem os2_sem;
    POS2Sem os2_sem_test;
    ULONG i;
    APIRET apiret[64];
    APIRET combined_apiret;

    /* check the things out */
    os2_sem = (POS2Sem)hmux;
    if (strcmp(os2_sem->id, "SEM")) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }
    if (strcmp(os2_sem->type, "MUX")) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }
    if (os2_sem->numSems == 0) {
        RESTOREENV_RETURN(ERROR_EMPTY_MUXWAIT);
    }
    if (ulTimeout < SEM_WAIT_PERIOD) {
        ulTimeout = SEM_WAIT_PERIOD;
    }

    /* initialize the return code array */
    for (i = 0; i < 64; i++) {
        apiret[i] = ERROR_TIMEOUT;
    }

    /* loop to traverse the list of semaphores */
    /* this looping construct will cause us to wait longer than the */
    /* specified timeout due to the latency involved in the loop,   */
    /* but that cannot be helped                                    */
    while (ulTimeout >= SEM_WAIT_PERIOD) {
        combined_apiret = NO_ERROR;
        for (i = 0; i < os2_sem->numSems; i ++) {
            /* if we already have this semaphore then skip it */
            if (apiret[i] == NO_ERROR) {
                continue;
            }
            /* test the semaphore */
            os2_sem_test = (POS2Sem)os2_sem->semrec[i].hsemCur;
            if (strcmp(os2_sem_test->type, "EVE") == 0) {
                /* this is an event semaphore */
                apiret[i] = DosWaitEventSem((HEV)os2_sem->semrec[i].hsemCur,
                                            SEM_IMMEDIATE_RETURN);
            }
            else {
                /* this is a mutex semaphore */
                apiret[i] = DosRequestMutexSem((HMTX)os2_sem->semrec[i].hsemCur,
                                               SEM_IMMEDIATE_RETURN);
            }
            /* if we only need one semaphore then return it */
            if (os2_sem->waitall == FALSE && apiret[i] == NO_ERROR) {
                if (pulUser != NULL) {
                    *pulUser = os2_sem->semrec[i].ulUser;
                }
                RESTOREENV_RETURN(NO_ERROR);
            }
            combined_apiret |= apiret[i];
        }
        /* if all the semaphores are posted/owned then return */
        if (combined_apiret == NO_ERROR) {
            if (pulUser != NULL) {
                *pulUser = os2_sem->semrec[i].ulUser;
            }
            RESTOREENV_RETURN(NO_ERROR);
        }
        /* if not, then sleep and try again */
        if (ulTimeout != SEM_INDEFINITE_WAIT) {
            DosSleep(SEM_WAIT_PERIOD);
            ulTimeout -= SEM_WAIT_PERIOD;
        }
    }

    RESTOREENV_RETURN(ERROR_TIMEOUT);
}


/*----------------------------------------------------------------------------*/
/* DosQueryMuxWaitSem                                                         */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryMuxWaitSem (HMUX hmux, PULONG pcSemRec,
                                    PSEMRECORD pSemRec, PULONG pflAttr)
{
    SAVEENV;
    POS2Sem os2_sem;
    ULONG i;

    /* check the args */
    os2_sem = (POS2Sem)hmux;
    if (strcmp(os2_sem->id, "SEM")) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }
    if (strcmp(os2_sem->type, "MUX")) {
        RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
    }
    if (pcSemRec == NULL || pSemRec == NULL) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }
    if (*pcSemRec < os2_sem->numSems) {
        RESTOREENV_RETURN(ERROR_PARAM_TOO_SMALL);
    }

    /* do the attribute first */
    if (pflAttr != NULL) {
        if (os2_sem->named == TRUE) {
            *pflAttr = DC_SEM_SHARED;
        }
        else {
            *pflAttr = 0;
        }
        if (os2_sem->waitall == TRUE) {
            *pflAttr |= DCMW_WAIT_ALL;
        }
        else {
            *pflAttr |= DCMW_WAIT_ANY;
        }
    }

    /* now copy the array */
    if (os2_sem->numSems > 0) {
        for (i = 0; i < os2_sem->numSems; i++) {
            pSemRec[i] = os2_sem->semrec[i];
        }
    }
    *pcSemRec = os2_sem->numSems;

    RESTOREENV_RETURN(NO_ERROR);
}

