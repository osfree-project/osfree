/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test05.c                                                      */
/*                                                                            */
/* Description: Test thread, event semaphores and asyncronous timers.         */
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
/* DosCloseEventSem                                                           */
/* DosWaitEventSem                                                            */
/* DosStartTimer                                                              */
/* DosStopTimer                                                               */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/* include the standard linux stuff first */
#include <errno.h>
#include <stdio.h>
//#include <semaphore.h>

/* now include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_DOSDATETIME
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#include "os2.h"


/*----------------------------------------------------------------------------*/
/* thread1 - wait for event semaphore to post                                 */
/*----------------------------------------------------------------------------*/

void APIENTRY thread1(ULONG ulArg)
{
    int i;
    APIRET apiret;

    for (i = 0; i < 4; i++) {
        apiret = DosWaitEventSem((HEV)ulArg, SEM_INDEFINITE_WAIT);
        printf("DosWaitEventSem returned %d\n", (int)apiret);
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
    HTIMER htimer;

    printf(__FILE__ " main function invoked\n");
    printf("Test timers\n");

    apiret = DosCreateEventSem("\\SEM32\\TEST5SEM", &hev, DC_SEM_SHARED, FALSE);
    printf("DosCreateEventSem function returned %d\n", (int)apiret);

    apiret = DosStartTimer(1000, (HSEM)hev, &htimer);
    printf("DosStartTimer function returned %d\n", (int)apiret);

    apiret = DosCreateThread(&tid, thread1, (ULONG)hev, 0, 8092);
    printf("DosCreateThread returned %d\n", (int)apiret);

    apiret = DosWaitThread(&tid, DCWW_WAIT);
    printf("Function DosWaitThread returned %d\n", (int)apiret);

    apiret = DosStopTimer(htimer);
    printf("DosStopTimer function returned %d\n", (int)apiret);

    apiret = DosCloseEventSem(hev);
    printf("Function DosCloseEventSem returned %d\n", (int)apiret);

    return 0;
}

