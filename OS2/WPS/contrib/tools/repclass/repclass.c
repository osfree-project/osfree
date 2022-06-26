
/*
 * repclass.c:
 *      this file contains the REPCLASS utility for registering
 *      a new WPS class and replacing an existing class with it
 *      in one step. The command-line syntax is:
 *
 *          repclass <oldclass> <newclass> [<dllname>]
 *
 *      If <dllname> is specified, <newclass> is registered and
 *      replaces <oldclass>.
 *
 *      If <dllname> is omitted, the replacement of <oldclass>
 *      with <newclass> is undone and <newclass> is de-registered.
 *
 *      Initial release: Aug. 12, 1998
 *      Changed: 0.82. Made the output msgs a bit more lucid.
 *      This version: 0.9.0. Updated the startup message.
 */

/*
 *      Copyright (C) 1997-2003 Ulrich M”ller.
 *
 *      This file is part of the XWorkplace source package.
 *      XWorkplace is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published
 *      by the Free Software Foundation, in version 2 as it comes in the
 *      "COPYING" file of the XWorkplace main distribution.
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 */

#include <stdio.h>
#include <os2.h>

#include "bldlevel.h"

/*
 * main:
 *      evaluate the command line, perform error checking
 *      and do all the (de)registering and (un)replacing.
 */

int main(int argc, char *argv[])
{
    int irc = 0;
    /* The argv array will contain the following:
     *         0         1          2            3
     *      repclass <oldclass> <newclass> [<dllname>]
     */

    BOOL rc1, rc2;

    if (argc == 4)
    {
        // four parameters: user wants registering.
        // First register <newclass>
        printf("Registering %s with the WPS class list: ", argv[2]);
        if (WinRegisterObjectClass(argv[2], argv[3]))
            printf("OK\n");
        else
        {
            printf("failed!\n");
            irc = 1;
        }

        // Then replace <oldclass> with <newclass>.
        printf ("Replacing %s with %s: ", argv[1], argv[2]);
        if (WinReplaceObjectClass(argv[1], argv[2], TRUE))
            printf("OK\n");
        else
        {
            printf("failed!\n");
            irc = 2;
        }
    }
    else if (argc == 3)
    {
        // three parameters: user wants deregistering.
        // First undo replacement of <oldclass> with <newclass>.
        printf ("Un-replacing %s with %s: ", argv[2], argv[1]);
        if (WinReplaceObjectClass(argv[1], argv[2], FALSE))
            printf("OK\n");
        else
        {
            printf("failed!\n");
            irc = 3;
        }

        // Then deregister <newclass>.
        printf ("Deregistering %s: ", argv[2]);
        if (WinDeregisterObjectClass(argv[2]))
            printf("OK\n");
        else
        {
            printf("failed!\n");
            irc = 4;
        }
    }
    else
    {
        // neither three nor four parameters: explain ourselves.
        printf("repclass V"BLDLEVEL_VERSION" ("__DATE__") (C) 1998-2003 Ulrich M”ller\n");
        printf("  Part of the XWorkplace package.\n");
        printf("  This is free software under the GNU General Public Licence (GPL).\n");
        printf("  Refer to the COPYING file in the XWorkplace installation dir for details.\n");
        printf("repclass registers and replaces a given Workplace Shell class or undoes an\n");
        printf("existing replacement (and deregisters at the same time).\n");
        printf("Usage: repclass <oldclass> <newclass> [<dllname>]\n");
        printf("with:\n");
        printf("   <oldclass>  being a currently installed WPS class;\n");
        printf("   <newclass>  the name of the new class to replace <oldclass> with;\n");
        printf("   <dllname>   the filename of the DLL which contains <newclass>; if the DLL\n");
        printf("               is not on the LIBPATH, you must specify a full path.\n");
        printf("If <dllname> is specified, <newclass> is registered with the WPS, and \n");
        printf("  <oldclass> is replaced with it.\n");
        printf("If <dllname> is omitted, <newclass> will be removed from the replacement list\n");
        printf("  for <oldclass>, and <newclass> will be deregistered.\n");
        printf("Class names are case-sensitive.\n");
        printf("Examples:\n");
        printf(" repclass WPFolder XFolder xfolder.dll - register XFolder and replace WPFolder\n");
        printf(" repclass WPFolder XFolder        - deregister XFolder and un-replace WPFolder\n");
        irc = 5;
    }

    return (irc);
}

