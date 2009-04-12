/* $Id: cachejfs.c,v 1.1.1.1 2003/05/21 13:38:52 pasha Exp $ */

static char *SCCSID = "@(#)1.3  11/1/99 11:36:52 src/jfs/utils/cachejfs/cachejfs.c, jfsutil, w45.fs32, fixbld";
/*
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 *
 * Module: cachejfs.c
 *
 */

/*
 * Change History :
 *
 */

#define INCL_DOSPROCESS
#define INCL_DOSERRORS
#include <os2.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "jfs_types.h"
#include "jfs_cntl.h"

int32	report(void);
void	usage(void);
/*
 * This program queries or sets lazywrite parameters
 */

main(int argc, char *argv[])
{
        int rc;
	struct LWPARMS lwparms;
	unsigned long cbData = sizeof(struct LWPARMS);
	unsigned long datalen = 0;
	unsigned long dummyp = 0;
	char *string;
	char *token;
// BEGIN D230860
	int i;

	/*
	 * Initialize lwparms in case we're only modifying minfree or maxfree
	 */
	rc = DosFSCtl(  (PVOID) &lwparms,	/* data area */
			cbData,		/* size of data */
			&datalen,	/* returned data length */
			NULL,		/* parameter list */
			0,		/* size of parameter list */
			&dummyp,	/* return parm length */
			JFSCTL_LW_GET,	/* function code */
			"JFS",		/* target file system */
			-1,		/* file handle */
			FSCTL_FSDNAME);	/* method of file system id */
// END D230860

	/* Parse same as IFS command line.  /L*: is optional
	 */
	if (argc == 1)
	{
		return report();
	}
	for (i = 1; i < argc; i++)				// D2308860
	{
		string = argv[i];
		if ((*string == '/') || (*string == '-'))
		{
			string++;
// BEGIN D230860
			if ((*string == 'm') || (*string == 'M'))
			{
				string++;
				if ((*string == 'i') || (*string == 'I'))
				{
					/* MINFREE! */
					while (*(++string) && (*string != ':'));
					if (*string)
						string++;
					lwparms.MinFree = strtoul(string, 0, 0);
				}
				else if ((*string == 'a') || (*string == 'A'))
				{
					/* MAXFREE! */
					while (*(++string) && (*string != ':'));
					if (*string)
						string++;
					lwparms.MaxFree = strtoul(string, 0, 0);
				}
				else
				{
					usage();
					return ERROR_INVALID_PARAMETER;
				}
				continue;
			}
// END D230860
			if ((*string != 'l') && (*string != 'L'))
			{
				usage();
				return ERROR_INVALID_PARAMETER;
			}
			while (*(++string) && (*string != ':'));
			if (*string)
				string++;	/* skip past colon */
		}
		/* string is pointing just past /LAZYWRITE: if it was there */

		if ((strcmp(string, "OFF") == 0) ||
		    (strcmp(string, "off") == 0))
		{
			lwparms.LazyOff = 1;
			lwparms.SyncTime = 1;
			lwparms.MaxAge = 0;
			lwparms.BufferIdle = 0;
			continue;				// D230860
		}
		lwparms.LazyOff = 0;

		token = strtok(string, ",");

		if (token == 0)
		{
			/* No parameters */
			usage();
			return ERROR_INVALID_PARAMETER;
		}

		if ((lwparms.SyncTime = strtoul(token,0,0)) <= 0)
			lwparms.SyncTime = 1;

		token = strtok(0, ",");

		if (token == 0)
		{
			lwparms.MaxAge = lwparms.SyncTime << 2;
			lwparms.BufferIdle = lwparms.SyncTime >> 3;
			continue;				// D230860
		}

		if ((lwparms.MaxAge = strtoul(token,0,0)) < 0)
			lwparms.MaxAge = 0;

		token = strtok(0, ",");

		if (token == 0)
		{
			lwparms.BufferIdle = lwparms.SyncTime >> 3;
			continue;				// D230860
		}

		if ((lwparms.BufferIdle = strtoul(token,0,0)) < 0)
			lwparms.BufferIdle = 0;
	}

	datalen = sizeof(lwparms);
	
	rc = DosFSCtl((PVOID) &lwparms,	/* data area */
			cbData,		/* size of data */
			&datalen,	/* returned data length */
			NULL,		/* parameter list */
			0,		/* size of parameter list */
			&dummyp,	/* return parm length */
			JFSCTL_LW_SET,	/* function code */
			"JFS",		/* target file system */
			-1,		/* file handle */
			FSCTL_FSDNAME);	/* method of file system id */

	if (rc)
		fprintf(stderr,"DosFSCTL returned %d.\n", rc);

	report();

        return rc;
}

int32 report()
{
	unsigned long	cbData = sizeof(struct LWPARMS);
	unsigned long	datalen = 0;
	unsigned long	dummyp = 0;
	struct LWPARMS	lwparms;
	int32		rc;

	rc = DosFSCtl(  (PVOID) &lwparms,	/* data area */
			cbData,		/* size of data */
			&datalen,	/* returned data length */
			NULL,		/* parameter list */
			0,		/* size of parameter list */
			&dummyp,	/* return parm length */
			JFSCTL_LW_GET,	/* function code */
			"JFS",		/* target file system */
			-1,		/* file handle */
			FSCTL_FSDNAME);	/* method of file system id */

	if (rc)
		fprintf(stderr,"DosFSCTL returned %d.\n", rc);
	else
	{
		printf("\n          SyncTime:%8d seconds\n",
			lwparms.SyncTime);
		printf("            MaxAge:%8d seconds\n",
			lwparms.MaxAge);
		printf("        BufferIdle:%8d seconds\n",
			lwparms.BufferIdle);
		printf("        Cache Size:%8d kbytes\n",
			lwparms.CacheSize/1024);
// BEGIN D230860
		if (datalen >= sizeof(lwparms))
		{
			printf("        Min Free buffers:%8d (%8d K)\n",
			       lwparms.MinFree, lwparms.MinFree*4);
			printf("        Max Free buffers:%8d (%8d K)\n",
			       lwparms.MaxFree, lwparms.MaxFree*4);
		}
// END D230860
		if (lwparms.LazyOff)
			printf("Lazy Write is disabled\n\n");
		else
			printf("Lazy Write is enabled\n\n");
	}
	return rc;
}

void	usage()
{
	fprintf(stderr, "CACHEJFS [[/LAZYWRITE:]{OFF|synctime[,maxage[,bufferidle]]}]\n");
	fprintf(stderr, "         [/MINFREE:minfree] [/MAXFREE:maxfree]\n"); // D230860
}
