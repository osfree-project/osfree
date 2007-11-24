/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test09c.c                                                     */
/*                                                                            */
/* Description: Test file manager functions.                                  */
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
/* DosFindFirst                                                               */
/* DosFindNext                                                                */
/* DosFindClose                                                               */
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
#define INCL_ERRORS
#include "os2.h"


/*----------------------------------------------------------------------------*/
/* main - the main entry point to the program                                 */
/*----------------------------------------------------------------------------*/

int main(void)
{
    APIRET apiret;
    PSZ filespec = "./*.c";
    HDIR hdir = HDIR_CREATE;
    ULONG entries = 3;
    FILEFINDBUF3 findbuf[3];
    int i;

    printf(__FILE__ " main function invoked\n");
    printf("Test file manager functions\n");

    apiret = DosFindFirst(filespec, &hdir, 0, findbuf, sizeof(findbuf),
                          &entries, FIL_STANDARD);
    printf("DosFindFirst function returned %d\n", (int)apiret);
    printf("%d entries returned\n", (int)entries);
    for (i = 0; i < entries; i ++) {
        printf("%s\n", findbuf[i].achName);
    }

    while (apiret == NO_ERROR) {
        apiret = DosFindNext(hdir, findbuf, sizeof(findbuf), &entries);
        printf("DosFindNext function returned %d\n", (int)apiret);
        printf("%d entries returned\n", (int)entries);
        for (i = 0; i < entries; i ++) {
            printf("%s\n", findbuf[i].achName);
        }
    }
    printf("The last DosFindNext function should return 18\n");

    apiret = DosFindClose(hdir);
    printf("DosFindClose function returned %d\n", (int)apiret);

    return 0;
}

