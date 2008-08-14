/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test10a.c                                                     */
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
    HQUEUE hq;
    PSZ pszQueueName = "\\queue\\os2linux\\test\\queue";
    PID pid;

    printf(__FILE__ " main function invoked\n");
    printf("Test DosExecPgm and queues\n");
    ULONG bytesread, priority = 0;
    REQUESTDATA request;
    PVOID msgtxt;
    CHAR fmtspec[80];

    /* create queue */
    apiret = DosOpenQueue(&pid, &hq, pszQueueName);
    printf("DosOpenQueue function returned %d\n", (int)apiret);
    if (apiret) {
        return 0;
    }

    apiret = DosReadQueue(hq, &request, &bytesread, &msgtxt, 0, TRUE,
                             (PBYTE)&priority, NULLHANDLE);
    printf("DosReadQueue function returned %d\n", (int)apiret);
    if (apiret) {
        return 0;
    }
    sprintf(fmtspec, "The message was \"%%0.%ds\"\n", (int)bytesread);
    printf(fmtspec, (PSZ)msgtxt);

    apiret = DosReadQueue(hq, &request, &bytesread, &msgtxt, 0, TRUE,
                             (PBYTE)&priority, NULLHANDLE);
    printf("DosReadQueue function returned %d\n", (int)apiret);
    if (apiret) {
        return 0;
    }
    sprintf(fmtspec, "The message was \"%%0.%ds\"\n", (int)bytesread);
    printf(fmtspec, (PSZ)msgtxt);

    apiret = DosReadQueue(hq, &request, &bytesread, &msgtxt, 0, TRUE,
                             (PBYTE)&priority, NULLHANDLE);
    printf("DosReadQueue function returned %d\n", (int)apiret);
    if (apiret) {
        return 0;
    }
    sprintf(fmtspec, "The message was \"%%0.%ds\"\n", (int)bytesread);
    printf(fmtspec, (PSZ)msgtxt);

    apiret = DosCloseQueue(hq);
    printf("DosCloseQueue function returned %d\n", (int)apiret);

    return 0;
}

