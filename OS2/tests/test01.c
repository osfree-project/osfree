/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test01.c                                                      */
/*                                                                            */
/* Description: Test the os2.so library. Only basic linking tests are         */
/*              performed.                                                    */
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
/* DosGetDateTime                                                             */
/* DosBeep                                                                    */
/*                                                                            */
/*----------------------------------------------------------------------------*/


/* include the standard linux stuff first */
#include <stdio.h>

/* now include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_DOSPROCESS
#define INCL_DOSDATETIME
#include "os2.h"


/*----------------------------------------------------------------------------*/
/* displaytime - print out the date and timem                                 */
/*----------------------------------------------------------------------------*/

void displaytime(PDATETIME pdt)
{
    char * days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

    printf("The current time is %s %02d-%02d-%d:%02d:%02d:%02d\n",
           days[pdt->weekday], (int)pdt->month, (int)pdt->day, (int)pdt->year,
           (int)pdt->hours, (int)pdt->minutes, (int)pdt->seconds);
    return;
}


/*----------------------------------------------------------------------------*/
/* main - the main entry point to the program                                 */
/*----------------------------------------------------------------------------*/

int main(void)
{
    APIRET apiret;
    DATETIME dt;
    ULONG sec = 5;

    printf(__FILE__ " main function invoked\n");
    printf("Test basic library functioality\n");

    printf("Three beeps should follow this message\n");
    DosBeep(0, 0);
    DosSleep(125);
    DosBeep(0, 0);
    DosSleep(125);
    DosBeep(0, 0);

    apiret = DosGetDateTime(&dt);
    displaytime(&dt);

    printf("Sleeping for %d seconds\n", (int)sec);
    DosSleep(sec * 1000);

    apiret = DosGetDateTime(&dt);
    displaytime(&dt);
    return 0;
}

