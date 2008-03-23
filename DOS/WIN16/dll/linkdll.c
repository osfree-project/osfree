/*    
	linkdll.c	1.2	1/29/96 create linked list of initialization points 
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

 */

/*
 *	Program to take a list of linked DLL modules, and create a
 *	table containing all of the startup routines that need to
 *	be called from within main().  (See TwinRT.c for the actual
 *	call.)
 *
 *	Programs that don't need this table use the default table,
 *	available in TwinDeflib.c, which is an empty table.  This
 *	program is only needed for systems that do not support a
 *	mechanism for auto-initialized code, and which link against
 *	DLL modules.  Two such systems are AIX and SunOS 4.1.3.
 */

#include <stdio.h>
#include <time.h>

#include <string.h>

#define LINKDLL_VERSION "1.0"

main(int argc, char *argv[])
{
    int first;
    int i;
    time_t curtime;
    FILE *fp;

    /*
     *  Format of command is 'linkdll [-o outfile] lib1 [lib2 [...]]'.
     *  If no output file is given, the default is standard output.
     */

    if (argc < 2)
    {
	usage(argv[0]);
	return(1);
    }
    first = 1;
    if (strcmp(argv[1], "-o") == 0)
    {
	if (argc < 4)
	{
	    usage(argv[0]);
	    return(1);
	}
	fp = freopen(argv[2], "w", stdout);
	if (fp == NULL)
	{
	    fprintf(stderr,"Unable to open %s for output\n",argv[2]);
	    return(1);
	}
	first = 3;
    }

    /*
     *  First create the required headers.
     */
    time(&curtime);
    printf ("/*\n");
    printf (" *  Created by linkdll version %s\n", LINKDLL_VERSION);
    printf (" *  Date: %s", ctime(&curtime));
    printf (" */\n");

    printf ("#include \"windows.h\"\n");
    printf ("#include \"Resources.h\"\n");
    printf ("#include \"Module.h\"\n");
    printf ("\n");

    /*
     *  Now create the function prototype for each entry routine.
     */
    for (i=first; i<argc; i++)
	printf("long int TWIN_LibEntry_%s();\n", argv[i]);

    /*
     *  Now create the actual table.
     */
    printf ("\n");
    printf ("ENTRYTAB TWIN_LibInitTable[] =\n");
    printf ("{\n");
    for (i=first; i<argc; i++)
	printf("\t{\"%s\",0,0,TWIN_LibEntry_%s},\n", argv[i], argv[i]);
    printf("\t{0,0,0,0}\n");
    printf("};\n");
    printf("\n");

    return(0);
}

usage(char *p)
{
    fprintf(stderr, "usage: %s [-o outputfile] lib1 [libname [...]]\n", p);
    return(0);
}


