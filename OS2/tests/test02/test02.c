/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test02.c                                                      */
/*                                                                            */
/* Description: Test thread starting and ending.                              */
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
/* _beginthread                                                               */
/* _endthread                                                                 */
/* DosSleep                                                                   */
/* DosCreateThread                                                            */
/* DosWaitThread                                                              */
/*                                                                            */
/*----------------------------------------------------------------------------*/


//when cross-compiling for OS/2 under NT
#undef __NT__
/* include the standard library stuff first */
#include <stdio.h>
#include <stdlib.h>
#include <process.h>

/* now include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_DOSPROCESS
#include "os2.h"


/*----------------------------------------------------------------------------*/
/* thread1 - thread started by DosCreateThread                                */
/*----------------------------------------------------------------------------*/

void APIENTRY thread1(ULONG ulArg)
{
    DosSleep(1000); /* 1 second */
    printf("Message from function thread1\n");
    printf("The thread argument was %d\n", (int)ulArg);
    DosSleep(1000); /* 1 second */
    return;
}

/*----------------------------------------------------------------------------*/
/* thread2 - thread started by _beginthread                                   */
/*----------------------------------------------------------------------------*/

void thread2(void * p)
{
    DosSleep(1000); /* 1 second */
    printf("Message from function thread2\n");
    printf("The thread argument was %p\n", p);
    DosSleep(1000); /* 1 second */
    _endthread();
    return;
}


/*----------------------------------------------------------------------------*/
/* main - the main entry point to the program                                 */
/*----------------------------------------------------------------------------*/

int main(void)
{
    APIRET apiret;
    TID tid;
    char           *stack;

    /* test thread1 */
    printf(__FILE__ " main function invoked\n");
    printf("Test basic thread functionality\n");

    printf("Starting first thread with DosCreateThread\n");
    apiret = DosCreateThread(&tid, thread1, 10, 0, 8092);
    printf("Function returned %d\n", (int)apiret);
    printf("The thread id was %d\n", (int)tid);
    apiret = DosWaitThread(&tid, DCWW_WAIT);
    printf("Function DosWaitThread returned %d\n", (int)apiret);

    /* test thread2 */
    printf(__FILE__ " main function invoked\n");
    printf("Starting second thread with _beginthread\n");
    stack = (char *) malloc( 8092 );
    tid = (TID)_beginthread(thread2, stack, 8092, (void *)10);
    printf("The thread id was %d\n", (int)tid);
    apiret = DosWaitThread(&tid, DCWW_WAIT);
    printf("Function DosWaitThread returned %d\n", (int)apiret);

    return 0;
}

