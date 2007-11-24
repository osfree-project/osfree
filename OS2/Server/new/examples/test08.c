/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test08.c                                                      */
/*                                                                            */
/* Description: Test some miscellaneous functions.                            */
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
/* DosPutMessage                                                              */
/* DosQuerySysInfo                                                            */
/* DosScanEnv                                                                 */
/* DosSetExtLIBPATH                                                           */
/* DosQueryExtLIBPATH                                                         */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/* include the standard linux stuff first */
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* now include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_DOSMISC
#include "os2.h"


/*----------------------------------------------------------------------------*/
/* main - the main entry point to the program                                 */
/*----------------------------------------------------------------------------*/

int main(void)
{
    APIRET apiret;
    char * msg = "Message from DosPutMessage";
    ULONG val;
    PSZ env_val;
    CHAR newpath[128];

    printf(__FILE__ " main function invoked\n");
    printf("Test miscellaneous functions\n");

    apiret = DosPutMessage((HFILE)1, strlen(msg), msg);
    printf("DosPutMessage function returned %d\n", (int)apiret);

    apiret = DosQuerySysInfo(QSV_MAX_TEXT_SESSIONS, QSV_MAX_TEXT_SESSIONS, &val,
                             sizeof(val));
    printf("DosQuerySysInfo function returned %d\n", (int)apiret);
    printf("QSV_MAX_TEXT_SESSIONS = %d\n", (int)val);

    apiret = DosQuerySysInfo(QSV_MAX_PATH_LENGTH, QSV_MAX_PATH_LENGTH, &val,
                             sizeof(val));
    printf("DosQuerySysInfo function returned %d\n", (int)apiret);
    printf("QSV_MAX_PATH_LENGTH = %d\n", (int)val);

    apiret = DosScanEnv("LD_LIBRARY_PATH", &env_val);
    printf("DosScanEnv function returned %d\n", (int)apiret);
    printf("Env var LD_LIBRARY_PATH = %s\n", env_val);

    apiret = DosSetExtLIBPATH("./", 0);
    printf("DosSetExtLIBPATH function returned %d\n", (int)apiret);

    apiret = DosQueryExtLIBPATH(newpath, 0);
    printf("DosQueryExtLIBPATH function returned %d\n", (int)apiret);
    printf("The new LIBPATH is %s\n", newpath);

    return 0;
}

