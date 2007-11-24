/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test09b.c                                                     */
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
/* DosCreateDir                                                               */
/* DosDeleteDir                                                               */
/* DosQueryCurrentDir                                                         */
/* DosQueryPathInfo                                                           */
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
    PSZ dirname = "./temp";
    char dirbuf[256];
    ULONG bufsize;
    FILESTATUS3 filestat;

    printf(__FILE__ " main function invoked\n");
    printf("Test file manager functions\n");

    apiret = DosCreateDir(dirname, NULL);
    printf("DosCreateDir function returned %d\n", (int)apiret);
    if (apiret != NO_ERROR) {
        DosSleep(3000);
        return 0;
    }

    apiret = DosDeleteDir(dirname);
    printf("DosDeleteDir function returned %d\n", (int)apiret);
    if (apiret != NO_ERROR) {
        DosSleep(3000);
        return 0;
    }

    bufsize = sizeof(dirbuf);
    apiret = DosQueryCurrentDir(0, dirbuf, &bufsize);
    printf("DosQueryCurrentDir function returned %d\n", (int)apiret);
    if (apiret != NO_ERROR) {
        DosSleep(3000);
        return 0;
    }
    printf("The current directory is %s\n", dirbuf);

    apiret = DosQueryPathInfo(__FILE__, FIL_STANDARD, &filestat,
                              sizeof(filestat));
    printf("DosQueryPathInfo function returned %d\n", (int)apiret);
    if (apiret != NO_ERROR) {
        DosSleep(3000);
        return 0;
    }
    printf("The file size of %s is %d\n", __FILE__, (int)filestat.cbFile);

    return 0;
}

