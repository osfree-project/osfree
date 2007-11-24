/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test07.c                                                      */
/*                                                                            */
/* Description: Test DosExecPgm.                                              */
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
#include "os2.h"


/*----------------------------------------------------------------------------*/
/* main - the main entry point to the program                                 */
/*----------------------------------------------------------------------------*/

int main(void)
{
    APIRET apiret;
    CHAR objname[50];
    RESULTCODES rc;

    printf(__FILE__ " main function invoked\n");
    printf("Test DosExecPgm\n");

    printf("Executing test07a using EXEC_SYNC\n");
    apiret = DosExecPgm(objname, 50, EXEC_SYNC, NULL, NULL, &rc, "./test07a");
    printf("DosExecPgm function returned %d\n", (int)apiret);

    printf("Executing test07a using EXEC_ASYNC\n");
    apiret = DosExecPgm(objname, 50, EXEC_ASYNC, NULL, NULL, &rc, "./test07a");
    printf("DosExecPgm function returned %d\n", (int)apiret);

    /* sleep a while so test07a can catch up with us */
    printf(__FILE__ " sleeping 3 seconds\n");
    DosSleep(3000);

    return 0;
}

