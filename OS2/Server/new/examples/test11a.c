/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test11a.c                                                     */
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
/* DosOpen                                                                    */
/* DosRead                                                                    */
/* DosClose                                                                   */
/* DosSleep                                                                   */
/* DosOpenEventSem                                                                           */
/* DosPostEventSem                                                                           */
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
    ULONG action, byteswritten;
    PSZ msg[3] = {"Message number one", "Message number two",
                  "Message number three"};

    printf(__FILE__ " main function invoked\n");
    printf("Test named pipe functions\n");

    apiret = DosOpen(pipeName, (PHFILE)&hpipe, &action, 0, FILE_NORMAL,
                        OPEN_ACTION_FAIL_IF_NEW, 0, NULL);
    printf("DosOpen function returned %d\n", (int)apiret);
    if (apiret != NO_ERROR) {
        return 0;
    }

    apiret = DosOpenEventSem(semName, &hev);
    printf("DosOpenEventSem function returned %d\n", (int)apiret);

    apiret = DosWrite((HFILE)hpipe, msg[0], strlen(msg[0]), &byteswritten);
    printf("DosWrite function returned %d\n", (int)apiret);

    apiret = DosPostEventSem(hev);
    DosSleep(2000);

    apiret = DosWrite((HFILE)hpipe, msg[1], strlen(msg[1]), &byteswritten);
    printf("DosWrite function returned %d\n", (int)apiret);

    apiret = DosPostEventSem(hev);
    DosSleep(2000);

    apiret = DosWrite((HFILE)hpipe, msg[2], strlen(msg[2]), &byteswritten);
    printf("DosWrite function returned %d\n", (int)apiret);

    apiret = DosPostEventSem(hev);
    DosSleep(2000);

    apiret = DosCloseEventSem(hev);
    printf("DosCloseEventSem function returned %d\n", (int)apiret);

    apiret = DosClose((HFILE)hpipe);
    printf("DosClose function returned %d\n", (int)apiret);

    return 0;
}

