/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test10.c                                                      */
/*                                                                            */
/* Description: Test DosExecPgm and queues.                                   */
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
/* DosExecPgm                                                                 */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/* include the standard linux stuff first */
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* now include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_DOSPROCESS
#define INCL_DOSQUEUES
#include "os2.h"


/*----------------------------------------------------------------------------*/
/* main - the main entry point to the program                                 */
/*----------------------------------------------------------------------------*/

int main(void)
{
    APIRET apiret;
    CHAR objname[50];
    RESULTCODES rc;
    HQUEUE hq;
    PSZ pszQueueName = "\\queue\\os2linux\\test\\queue";
    PSZ str[3] = {"Test message number one", "Test message number two",
                  "Test message number three"};

    printf(__FILE__ " main function invoked\n");
    printf("Test DosExecPgm and queues\n");

    /* create queue */
    apiret = DosCreateQueue(&hq, 0, pszQueueName);
    printf("DosCreateQueue function returned %d\n", (int)apiret);
    if (apiret) {
        return 0;
    }

    apiret = DosWriteQueue(hq, 1, strlen(str[0]), str[0], 0);
    printf("DosWriteQueue function returned %d\n", (int)apiret);
    if (apiret) {
        return 0;
    }

    apiret = DosWriteQueue(hq, 2, strlen(str[1]), str[1], 0);
    printf("DosWriteQueue function returned %d\n", (int)apiret);
    if (apiret) {
        return 0;
    }

    apiret = DosWriteQueue(hq, 3, strlen(str[2]), str[2], 0);
    printf("DosWriteQueue function returned %d\n", (int)apiret);
    if (apiret) {
        return 0;
    }

    apiret = DosExecPgm(objname, 50, EXEC_SYNC, NULL, NULL, &rc, "./test10a");
    printf("DosExecPgm function returned %d\n", (int)apiret);
    if (apiret) {
        return 0;
    }

    apiret = DosCloseQueue(hq);
    printf("DosCloseQueue function returned %d\n", (int)apiret);

    return 0;
}

