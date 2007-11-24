/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      test09a.c                                                     */
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
/* DosWrite                                                                   */
/* DosClose                                                                   */
/* DosSleep                                                                   */
/* DosDelete                                                                  */
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
    PSZ filename = "./temp.txt";
    HFILE hfile;
    ULONG action;
    PSZ buf = "Actions speak louder than words.\n"
              "The pen is mightier than the sword.\n"
              "Trueth is not stranger than fistion, it is just better.\n"
              "Revenge is the worst desire, Most hate has its roots in revenge.\n"
              "No good deed goes unpunished.\n"
              "Good has better lubrication than evil. But evil can slide uphill.\n";
    ULONG byteswritten;

    printf(__FILE__ " main function invoked\n");
    printf("Test file manager functions\n");

    apiret = DosOpen(filename, &hfile, &action, 0, 0, OPEN_ACTION_CREATE_IF_NEW,
                     OPEN_ACCESS_WRITEONLY, NULL);
    printf("DosOpen function returned %d\n", (int)apiret);
    if (apiret != NO_ERROR) {
        DosSleep(3000);
        return 0;
    }
    printf("Action returned %d\n", (int)action);

    apiret = DosWrite(hfile, buf, strlen(buf), &byteswritten);
    printf("DosWrite function returned %d\n", (int)apiret);
    printf("byteswritten = %d\n", (int)byteswritten);

    apiret = DosClose(hfile);
    printf("DosClose function returned %d\n", (int)apiret);

    apiret = DosDelete(filename);
    printf("DosDelete function returned %d\n", (int)apiret);

    return 0;
}

