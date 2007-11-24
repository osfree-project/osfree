/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test07a.c                                                     */
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
/* DosSleep                                                                   */
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

    printf(__FILE__ " main function invoked\n");

    DosSleep(2000);

    printf("Leaving " __FILE__ " function\n");

    return 0;
}

