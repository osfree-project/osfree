/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test11.c                                                      */
/*                                                                            */
/* Description: Test named pipe functions.                                    */
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
/* DosRead                                                                    */
/* DosClose                                                                   */
/* DosSleep                                                                   */
/* DosCreateNPipe                                                             */
/* DosCreateEventSem                                                                           */
/* DosWaitEventSem                                                                           */
/* DosCloseEventSem                                                                           */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/* include the standard linux stuff first */
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* now include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_DOSPROCESS
#define INCL_DOSFILEMGR
#define INCL_DOSNMPIPES
#define INCL_ERRORS
#define INCL_DOSSEMAPHORES
#include "os2.h"


/*----------------------------------------------------------------------------*/
/* main - the main entry point to the program                                 */
/*----------------------------------------------------------------------------*/

int main(void)
{
    APIRET apiret;
    HPIPE hpipe;
    PSZ pipeName = "\\pipe\\testpipe";
    PSZ semName = "\\sem32\\pipesem";
    HEV hev;
    CHAR objname[50];
    RESULTCODES rc;
    ULONG bytesread;
    CHAR buf[50];

    printf(__FILE__ " main function invoked\n");
    printf("Test named pipe functions\n");

    apiret = DosCreateNPipe(pipeName, &hpipe, 0, NP_WAIT, 0, 0, 0);
    printf("DosCreateNPipe function returned %d\n", (int)apiret);

    apiret = DosCreateEventSem(semName, &hev, DC_SEM_SHARED, FALSE);
    printf("DosCreateEventSem function returned %d\n", (int)apiret);

    apiret = DosExecPgm(objname, 50, EXEC_ASYNC, NULL, NULL, &rc, "./test11a");
    printf("DosExecPgm function returned %d\n", (int)apiret);

    apiret = DosWaitEventSem(hev, 5000);
    printf("DosWaitEventSem function returned %d\n", (int)apiret);

    apiret = DosRead((HFILE)hpipe, buf, sizeof(buf) - 1, &bytesread);
    printf("DosRead function returned %d\n", (int)apiret);
    buf[bytesread] = '\0';
    printf("Message was \"%s\"\n", buf);

    apiret = DosWaitEventSem(hev, 5000);
    printf("DosWaitEventSem function returned %d\n", (int)apiret);

    apiret = DosRead((HFILE)hpipe, buf, sizeof(buf) - 1, &bytesread);
    printf("DosRead function returned %d\n", (int)apiret);
    buf[bytesread] = '\0';
    printf("Message was \"%s\"\n", buf);

    apiret = DosWaitEventSem(hev, 5000);
    printf("DosWaitEventSem function returned %d\n", (int)apiret);

    apiret = DosRead((HFILE)hpipe, buf, sizeof(buf) - 1, &bytesread);
    printf("DosRead function returned %d\n", (int)apiret);
    buf[bytesread] = '\0';
    printf("Message was \"%s\"\n", buf);

    /* wait for test11a to catch up with us */
    DosSleep(2000);

    apiret = DosCloseEventSem(hev);
    printf("DosCloseEventSem function returned %d\n", (int)apiret);

    apiret = DosClose((HFILE)hpipe);
    printf("DosClose function returned %d\n", (int)apiret);

    return 0;
}

