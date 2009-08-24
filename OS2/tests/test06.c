/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test06.c                                                      */
/*                                                                            */
/* Description: Test thread and unnamed muxwait semaphores.                   */
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
/* DosCreateMuxWaitSem                                                        */
/* DosWaitMuxWaitSem                                                          */
/* DosCloseMuxWaitSem                                                         */
/* DosAddMuxWaitSem                                                           */
/* DosDeleteMuxWaitSem                                                        */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/* include the standard linux stuff first */
#include <errno.h>
#include <stdio.h>
//#include <semaphore.h>

/* now include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#include "os2.h"


HEV hev[3];


/*----------------------------------------------------------------------------*/
/* thread1 - post the event sems                                              */
/*----------------------------------------------------------------------------*/

void APIENTRY thread1(ULONG ulArg)
{
    APIRET apiret;

    DosSleep(1500);

    apiret = DosPostEventSem(hev[0]);
    printf("Function DosPostEventSem returned %d\n", (int)apiret);
    DosSleep(1500);

    apiret = DosPostEventSem(hev[1]);
    printf("Function DosPostEventSem returned %d\n", (int)apiret);
    DosSleep(1500);

    apiret = DosPostEventSem(hev[2]);
    printf("Function DosPostEventSem returned %d\n", (int)apiret);

    apiret = DosPostEventSem((HEV)ulArg);
    printf("Function DosPostEventSem returned %d\n", (int)apiret);
    return;
}


/*----------------------------------------------------------------------------*/
/* main - the main entry point to the program                                 */
/*----------------------------------------------------------------------------*/

int main(void)
{
    APIRET apiret;
    TID tid;
    int i;
    SEMRECORD rec[3];
    SEMRECORD newrec;
    HMUX hmux;
    ULONG user;

    printf(__FILE__ " main function invoked\n");
    printf("Test unnamed muxwait semaphores\n");

    for (i = 0; i < 3; i++) {
        apiret = DosCreateEventSem(NULL, &hev[i], DC_SEM_SHARED, FALSE);
        printf("DosCreateEventSem function returned %d\n", (int)apiret);
        rec[i].hsemCur = (HSEM)hev[i];
        rec[i].ulUser = i;
    }

    apiret = DosCreateMuxWaitSem(NULL, &hmux, 3, rec, DCMW_WAIT_ALL);
    printf("DosCreateMuxWaitSem function returned %d\n", (int)apiret);

    apiret = DosCreateEventSem(NULL, (PHEV)&newrec.hsemCur, DC_SEM_SHARED, FALSE);
    printf("DosCreateEventSem function returned %d\n", (int)apiret);
    newrec.ulUser = 3;

    apiret = DosAddMuxWaitSem(hmux, &newrec);
    printf("DosAddMuxWaitSem function returned %d\n", (int)apiret);

    apiret = DosDeleteMuxWaitSem(hmux, rec[1].hsemCur);
    printf("DosDeleteMuxWaitSem function returned %d\n", (int)apiret);

    printf("Starting thread with DosCreateThread\n");
    apiret = DosCreateThread(&tid, thread1, (ULONG)newrec.hsemCur, 0, 8092);

    apiret = DosWaitMuxWaitSem(hmux, SEM_INDEFINITE_WAIT, &user);
    printf("Function DosWaitMuxWaitSem returned %d, user = %d\n",
           (int)apiret, (int)user);

    apiret = DosWaitThread(&tid, DCWW_WAIT);
    printf("Function DosWaitThread returned %d\n", (int)apiret);

    for (i = 0; i < 3; i++) {
        apiret = DosCloseEventSem(hev[i]);
        printf("Function DosCloseEventSem returned %d\n", (int)apiret);
    }

    apiret = DosCloseMuxWaitSem(hmux);
    printf("Function DosCloseMuxWaitSem returned %d\n", (int)apiret);

    return 0;
}

