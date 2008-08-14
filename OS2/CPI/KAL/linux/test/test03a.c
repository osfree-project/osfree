/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test03a.c                                                     */
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
/* DosCreateEventSem                                                          */
/* DosPostEventSem                                                            */
/* DosCloseEventSem                                                           */
/* DosQueryEventSem                                                           */
/* DosWaitEventSem                                                            */
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
/* thread1 - wait for event semaphore to post (indefinite waits)              */
/*----------------------------------------------------------------------------*/

void thread1(ULONG ulArg)
{
    int i;
    ULONG ulPostCt;
    APIRET apiret;

    DosQueryEventSem((HEV)ulArg, &ulPostCt);
    printf("The event semaphore post count is %d\n", (int)ulPostCt);
    for (i = 0; i < 4; i++) {
        apiret = DosWaitEventSem((HEV)ulArg, SEM_INDEFINITE_WAIT);
        printf("DosWaitEventSem (indefinite wait) returned %d\n", (int)apiret);
    }
    return;
}


/*----------------------------------------------------------------------------*/
/* thread2 - wait for event semaphore to post (timed waits)                   */
/*----------------------------------------------------------------------------*/

void thread2(ULONG ulArg)
{
    int i;
    ULONG ulPostCt;
    APIRET apiret;

    DosQueryEventSem((HEV)ulArg, &ulPostCt);
    printf("The event semaphore post count is %d\n", (int)ulPostCt);
    /* note the one extra DosWaitEventSem in the loop */
    /* the last one should time out */
    for (i = 0; i < 5; i++) {
        apiret = DosWaitEventSem((HEV)ulArg, 5000); /* 5 seconds */
        printf("DosWaitEventSem (timed wait) returned %d\n", (int)apiret);
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
    HEV hev;
    int i;

    /* test thread1 */
    printf(__FILE__ " main function invoked\n");
    printf("Test unnamed event semaphores\n");

    apiret = DosCreateEventSem(NULL, &hev, DC_SEM_SHARED, FALSE);
    printf("DosCreateEventSem function returned %d\n", (int)apiret);

    /* post to thread 1 */
    printf("Starting thread with DosCreateThread\n");
    apiret = DosCreateThread(&tid, thread1, (ULONG)hev, 0, 8092);

    for (i = 0; i < 4; i++) {
        apiret = DosPostEventSem(hev);
        printf("Function DosPostEventSem returned %d\n", (int)apiret);
        DosSleep(1500); /* 1.5 seconds */
    }

    apiret = DosWaitThread(&tid, DCWW_WAIT);
    printf("Function DosWaitThread returned %d\n", (int)apiret);

    /* post to thread 2 */
    printf("Starting thread with DosCreateThread\n");
    apiret = DosCreateThread(&tid, thread2, (ULONG)hev, 0, 8092);

    for (i = 0; i < 4; i++) {
        apiret = DosPostEventSem(hev);
        printf("Function DosPostEventSem returned %d\n", (int)apiret);
        DosSleep(1500); /* 1.5 seconds */
    }

    apiret = DosWaitThread(&tid, DCWW_WAIT);
    printf("Function DosWaitThread returned %d\n", (int)apiret);
    printf("The last DosWaitEventSem should have returned 640\n");

    apiret = DosCloseEventSem(hev);
    printf("Function DosCloseEventSem returned %d\n", (int)apiret);

    return 0;
}

