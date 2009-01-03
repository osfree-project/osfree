/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      queues.c                                                      */
/*                                                                            */
/* Description: This file includes the code for support of queues.            */
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
#define INCL_DOSQUEUES
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_ERRORS
#include "os2.h"

/* include the linux headers and our local stuff */
#include "os2linux.h"


/*============================================================================*/
/* OS/2 APIs for memory manager                                               */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* Local definitions                                                          */
/*----------------------------------------------------------------------------*/

#define MAX_MSG_SIZE 4096

/* standard size message buffer */
typedef struct _mymsg {
    long mtype;
    char mtext[1];
} mymsg;
typedef mymsg * pmymsg;

/* max size message buffer, customized for OS2Linux */
typedef struct _mymaxmsg {
    long mtype;
    ULONG ulData;
    char mtext[MAX_MSG_SIZE];
} mymaxmsg;
typedef mymaxmsg * pmymaxmsg;

/* storage for a queue handle */
typedef struct _MQ {
int qid;
CHAR queueName[_POSIX_PATH_MAX];
BOOL creator;
HEV hev;
mymaxmsg msg;
} MQ;
typedef MQ * PMQ;


/*----------------------------------------------------------------------------*/
/* DosCreatePipe                                                              */
/*    Notes:                                                                  */
/*       - cb is ignored                                                      */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosCreatePipe(PHFILE phfRead, PHFILE phfWrite, ULONG cb)
{
    SAVEENV;
    int fh[2];
    PFILEHANDLESTRUCT pfh[2];
    int rc;

    /* allocate memory for the file handles */
    pfh[0] = malloc(sizeof(FILEHANDLESTRUCT));
    if (pfh[0] == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    pfh[1] = malloc(sizeof(FILEHANDLESTRUCT));
    if (pfh[1] == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    strcpy(pfh[0]->id, "PIP");
    strcpy(pfh[1]->id, "PIP");

    /* create the pipe */
    rc = pipe(fh);
    if (rc) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    pfh[0]-> fh = fh[0];
    pfh[0]-> fh = fh[0];

    /* return the filehandles */
    *phfRead = (HFILE)pfh[0];
    *phfWrite = (HFILE)fh[1];
    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosCloseQueue                                                              */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosCloseQueue(HQUEUE hq)
{
    SAVEENV;
    int rc;

    if (((PMQ)hq)->creator == TRUE) {
        rc = msgctl(((PMQ)hq)->qid, IPC_RMID, NULL);
        if (rc) {
            if (errno == EINVAL) {
                RESTOREENV_RETURN(ERROR_QUE_INVALID_HANDLE);
            }
            else {
                RESTOREENV_RETURN(ERROR_QUE_PROC_NO_ACCESS);
            }
        }
        if(((PMQ)hq)->queueName[0] != '\0') {
            remove(((PMQ)hq)->queueName);
        }
    }
    free((PMQ)hq);

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosCreateQueue                                                             */
/*    Notes:                                                                  */
/*       - queue_flags is ignored                                             */
/*       - the queue is always a FIFO queue                                   */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosCreateQueue(PHQUEUE phq, ULONG queue_flags, PCSZ pszName)
{
    SAVEENV;
    PSZ pszNewName;
    key_t mykey = 0;
    PMQ pmq;

    /* allocate memory for the queue struct */
    pmq = malloc(sizeof(MQ));
    if (pmq == NULL) {
        RESTOREENV_RETURN(ERROR_QUE_NO_MEMORY);
    }
    pmq->hev = NULLHANDLE;

    /* named or unnamed queue? */
    if (pszName == NULL) {
        /* this is an unnamed queue */
        pmq->queueName[0] = '\0';
        pmq->qid = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | S_IRWXU | S_IRWXG);
    }
    else {
        /* this is a named queue */
        pszNewName = alloca(strlen(pszName) + 1);
        if (pszNewName == NULL) {
            free(pmq);
            RESTOREENV_RETURN(ERROR_QUE_NO_MEMORY);
        }
        strcpy(pszNewName, pszName);
        DosNameConversion(pszNewName, "\\", ".", TRUE);
        if (*pszNewName == '.') {
            *pszNewName = '/';
        }
        /* get a key for the queue */
        mykey = DosFtok(pszNewName);
        if (mykey == -1) {
            free(pmq);
            RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
        }
        strcpy(pmq->queueName, "/tmp");
        strcat(pmq->queueName, pszNewName);
        pmq->qid = msgget(mykey, IPC_CREAT | S_IRWXU | S_IRWXG);
    }

    /* get the queue */
    if (pmq->qid == -1) {
        switch (errno) {
        case ENOMEM:
        case ENOSPC:
            free(pmq);
            RESTOREENV_RETURN(ERROR_QUE_NO_MEMORY);
        case ENOENT:
            free(pmq);
            RESTOREENV_RETURN(ERROR_QUE_NAME_NOT_EXIST);
        default:
            free(pmq);
            RESTOREENV_RETURN(ERROR_QUE_PROC_NO_ACCESS);
        }
    }

    pmq->hev = NULLHANDLE;
    pmq->creator = TRUE;
    *phq = (HQUEUE)pmq;
    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosOpenQueue                                                               */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosOpenQueue(PPID ppid, PHQUEUE phq, PCSZ pszName)
{
    SAVEENV;
    PSZ pszNewName;
    key_t mykey = 0;
    PMQ pmq;

    /* allocate memory for the queue struct */
    pmq = malloc(sizeof(MQ));
    if (pmq == NULL) {
        RESTOREENV_RETURN(ERROR_QUE_NO_MEMORY);
    }
    pmq->hev = NULLHANDLE;

    /* named or unnamed queue? */
    if (pszName == NULL) {
        free(pmq);
        RESTOREENV_RETURN(ERROR_QUE_PROC_NO_ACCESS);
    }

    /* this is a named queue */
    pszNewName = alloca(strlen(pszName) + 1);
    if (pszNewName == NULL) {
        free(pmq);
        RESTOREENV_RETURN(ERROR_QUE_NO_MEMORY);
    }
    strcpy(pszNewName, pszName);
    DosNameConversion(pszNewName, "\\", ".", TRUE);
    if (*pszNewName == '.') {
        *pszNewName = '/';
    }

    /* get a key for the queue */
    mykey = DosFtok(pszNewName);
    if (mykey == -1) {
        free(pmq);
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    /* get the queue */
    pmq->qid = msgget(mykey, S_IRWXU | S_IRWXG);
    if (pmq->qid == -1) {
        switch (errno) {
        case ENOMEM:
        case ENOSPC:
            free(pmq);
            RESTOREENV_RETURN(ERROR_QUE_NO_MEMORY);
        case ENOENT:
            free(pmq);
            RESTOREENV_RETURN(ERROR_QUE_NAME_NOT_EXIST);
        default:
            free(pmq);
            RESTOREENV_RETURN(ERROR_QUE_PROC_NO_ACCESS);
        }
    }

    pmq->hev = NULLHANDLE;
    pmq->creator = FALSE;
    pmq->queueName[0] = '\0';
    *phq = (HQUEUE)pmq;
    *ppid = 0;
    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosPeekQueue                                                               */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosPeekQueue(HQUEUE hq, PREQUESTDATA pRequest, PULONG pcbData,
                             PPVOID ppbuf, PULONG element, BOOL32 nowait,
                             PBYTE ppriority, HEV hsem)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosPurgeQueue                                                              */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosPurgeQueue(HQUEUE hq)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosQueryQueue                                                              */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryQueue(HQUEUE hq, PULONG pcbEntries)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosReadQueue                                                               */
/*    Notes:                                                                  */
/*       - element is ignored                                                 */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosReadQueue(HQUEUE hq, PREQUESTDATA pRequest, PULONG pcbData,
                             PPVOID ppbuf, ULONG element, BOOL32 wait,
                             PBYTE ppriority, HEV hsem)
{
    SAVEENV;
    int rc;
    int msgflg = 0;

    if (wait) {
        msgflg = msgflg & IPC_NOWAIT;
        if (((PMQ)hq)->hev == NULLHANDLE) {
            ((PMQ)hq)->hev = hsem;
        }
    }

    /* get the message */
    ((PMQ)hq)->msg.mtype = 1;
    rc = msgrcv(((PMQ)hq)->qid, &((PMQ)hq)->msg, MAX_MSG_SIZE, 0, msgflg);
    if (rc == -1) {
        switch (errno) {
        case E2BIG:
            RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
        case EINVAL:
            RESTOREENV_RETURN(ERROR_QUE_NAME_NOT_EXIST);
        case ENOMSG:
            RESTOREENV_RETURN(ERROR_QUE_EMPTY);
        default:
            RESTOREENV_RETURN(ERROR_QUE_PROC_NO_ACCESS);
        }
    }

    /* set return data */
    pRequest->pid = 0;
    pRequest->ulData = ((PMQ)hq)->msg.ulData;
    *pcbData = (ULONG)rc - sizeof(ULONG);
    memcpy(ppbuf, ((PMQ)hq)->msg.mtype, sizeof(ULONG));
    *ppriority = ((PMQ)hq)->msg.mtype - 1;
    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosWriteQueue                                                              */
/*    Notes:                                                                  */
/*       - priority is ignored                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosWriteQueue(HQUEUE hq, ULONG request, ULONG cbData,
                              PVOID pbData, ULONG priority)
{
    SAVEENV;
    pmymaxmsg mbuf;
    int rc;

    /* check args */
    if (priority > 15) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }
    if (cbData > MAX_MSG_SIZE) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    /* allocate memory for the send buffer */
    mbuf = alloca(sizeof(mymsg) + (sizeof(ULONG) * 2));
    if (mbuf == NULL) {
        RESTOREENV_RETURN(ERROR_QUE_NO_MEMORY);
    }

    /* copy message into the buffer */
    mbuf->mtype = 1;
    mbuf->ulData = request;
    memcpy(mbuf->mtext, &pbData, sizeof(ULONG));

    /* send the message */
    rc = msgsnd(((PMQ)hq)->qid, mbuf, (size_t)cbData, 0);
    if (rc && errno == EINVAL) {
        RESTOREENV_RETURN(ERROR_QUE_INVALID_HANDLE);
    }
    else if (rc) {
        RESTOREENV_RETURN(ERROR_QUE_NO_MEMORY);
    }

    /* if necessary, post semaphore */
    if (((PMQ)hq)->hev != NULLHANDLE) {
        DosPostEventSem(((PMQ)hq)->hev);
    }
    RESTOREENV_RETURN(NO_ERROR);
}

