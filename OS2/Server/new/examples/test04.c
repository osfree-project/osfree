/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test04.c                                                      */
/*                                                                            */
/* Description: Test thread and named mutex semaphores.                       */
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
/* -------------------------------------------------------------------------- */
/*                                                                            */
/* The following OS/2 APIs are tested here:                                   */
/*                                                                            */
/* DosSleep                                                                   */
/* DosCreateThread                                                            */
/* DosWaitThread                                                              */
/* DosCreateMutexSem                                                          */
/* DosCloseMutexSem                                                           */
/* DosRequestMutexSem                                                         */
/* DosReleaseMutexSem                                                         */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/* include the standard linux stuff first */
#include <errno.h>
#include <stdio.h>
#include <semaphore.h>

/* now include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#include "os2.h"


/*----------------------------------------------------------------------------*/
/* thread1 - wait for mutex semaphore to be available                         */
/*----------------------------------------------------------------------------*/

void thread1(ULONG ulArg)
{
    int i;
    APIRET apiret;

    for (i = 0; i < 4; i++) {
        apiret = DosRequestMutexSem((HMTX)ulArg, SEM_INDEFINITE_WAIT);
        printf("thread1() DosRequestMutexSem function returned %d\n", (int)apiret);
        DosSleep(1500); /* 1.5 seconds */
        apiret = DosReleaseMutexSem((HMTX)ulArg);
        printf("thread1() DosReleaseMutexSem returned %d\n", (int)apiret);
        DosSleep(1500); /* 1.5 seconds */
    }
    return;
}


/*----------------------------------------------------------------------------*/
/* main - the main entry point to the program                                 */
/*----------------------------------------------------------------------------*/

int main(void)
{
    APIRET apiret;
    TID tid;
    HMTX hmtx;
    int i;

    printf(__FILE__ " main function invoked\n");
    printf("Test named mutex semaphores\n");

    apiret = DosCreateMutexSem("\\SEM32\\TEST4SEM", &hmtx, DC_SEM_SHARED,
                               FALSE);
    printf("DosCreateMutexSem function returned %d\n", (int)apiret);

    printf("Starting thread with DosCreateThread\n");
    apiret = DosCreateThread(&tid, thread1, (ULONG)hmtx, 0, 8092);

    for (i = 0; i < 4; i++) {
        apiret = DosRequestMutexSem(hmtx, SEM_INDEFINITE_WAIT);
        printf("main() DosRequestMutexSem function returned %d\n", (int)apiret);
        DosSleep(1500); /* 1.5 seconds */
        apiret = DosReleaseMutexSem(hmtx);
        printf("main() DosReleaseMutexSem returned %d\n", (int)apiret);
        DosSleep(1500); /* 1.5 seconds */
    }

    apiret = DosWaitThread(&tid, DCWW_WAIT);
    printf("Function DosWaitThread returned %d\n", (int)apiret);

    apiret = DosCloseMutexSem(hmtx);
    printf("Function DosCloseMutexSem returned %d\n", (int)apiret);

    return 0;
}

