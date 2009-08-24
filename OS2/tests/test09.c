/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test09.c                                                      */
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
/* DosOpen                                                                    */
/* DosRead                                                                    */
/* DosClose                                                                   */
/* DosSleep                                                                   */
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
    HFILE hfile;
    ULONG action;
    char buf[32000];
    ULONG bytesread;
    ULONG newpos;

    printf(__FILE__ " main function invoked\n");
    printf("Test file manager functions\n");

    apiret = DosOpen("./test01.c", &hfile, &action, 0, 0, OPEN_ACTION_OPEN_IF_EXISTS,
                     OPEN_ACCESS_READONLY, NULL);
    printf("DosOpen function returned %d\n", (int)apiret);
    if (apiret != NO_ERROR) {
        DosSleep(3000);
        return 0;
    }
    printf("Action returned %d\n", (int)action);

    apiret = DosRead(hfile, buf, sizeof(buf) - 1, &bytesread);
    printf("DosRead function returned %d\n", (int)apiret);
    printf("bytesread = %d\n", (int)bytesread);
    buf[bytesread] = '\0';

    printf("\nFile contents: *****************************\n");
    printf("%s", buf);
    printf("End file contents: *****************************\n\n");

    DosSleep(1500);

    apiret = DosSetFilePtr(hfile, -200, FILE_CURRENT, &newpos);
    printf("DosSetFilePtr function returned %d\n", (int)apiret);

    apiret = DosRead(hfile, buf, sizeof(buf) - 1, &bytesread);
    printf("DosRead function returned %d\n", (int)apiret);
    printf("bytesread = %d\n", (int)bytesread);
    buf[bytesread] = '\0';

    printf("\nFile contents (last 200 bytes): *****************************\n");
    printf("%s", buf);
    printf("End file contents(last 200 bytes): *****************************\n\n");

    apiret = DosClose(hfile);
    printf("DosClose function returned %d\n", (int)apiret);

    return 0;
}

