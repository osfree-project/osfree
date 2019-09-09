/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test12.c                                                      */
/*                                                                            */
/* Description: Test Vio functions.                                           */
/*                                                                            */
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
/* VioGetCurPos                                                               */
/* VioSetCurPos                                                               */
/* DosSleep                                                                   */
/*                                                                            */
/*----------------------------------------------------------------------------*/



/* include the standard linux stuff first */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/* now include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_DOSPROCESS
#define INCL_VIO
#include "os2.h"


/*----------------------------------------------------------------------------*/
/* main - the main entry point to the program                                 */
/*----------------------------------------------------------------------------*/

int main(void)
{
    APIRET16 apiret;
    char *str1 = "First string";
    char *str2 = "Second string";

    printf(__FILE__ " main function invoked\n");
    printf("Test VIO functions\n");
    DosSleep(2000);

    /* move the cursor around some */
    apiret = VioSetCurPos(0, 0, 0);
    DosSleep(1000);
    apiret = VioSetCurPos(1, 0, 0);
    DosSleep(1000);
    apiret = VioSetCurPos(2, 0, 0);
    DosSleep(1000);
    apiret = VioSetCurPos(3, 0, 0);
    DosSleep(1000);
    apiret = VioSetCurPos(4, 0, 0);
    DosSleep(1000);
    apiret = VioSetCurPos(5, 0, 0);
    DosSleep(1000);

    /* write some strings */
    VioWrtTTY(str1, strlen(str1), 0);
    DosSleep(2000);
    apiret = VioSetCurPos(6, 0, 0);
    VioWrtTTY(str2, strlen(str2), 0);
    DosSleep(2000);

    return 0;
}

