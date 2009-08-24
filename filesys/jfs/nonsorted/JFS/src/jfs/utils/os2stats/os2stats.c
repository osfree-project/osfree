/* $Id: os2stats.c,v 1.1.1.1 2003/05/21 13:42:48 pasha Exp $ */

static char *SCCSID = "@(#)1.2  7/31/98 08:21:25 src/jfs/utils/os2stats/os2stats.c, jfsutil, w45.fs32, 990417.1";
/*
 *
 *
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
 * Module: os2stat.c
 */

#define INCL_DOSPROCESS
#include <os2.h>

#include "jfs_types.h"
#include "jfs_cntl.h"
#include "stats.h"


int		xarg[1024];
int		xargsize = 4096;

/*
 *	main()
 */
main(argc, argv)
int argc;
char **argv;
{
	int rc;
	int t, type;
        unsigned long dummyd = 0;
        unsigned long dummyp = 0;

	/* parse options */
/*
	if (argc != 2)
		usage(argv[0]);
	if (*argv[1] != '-')
		usage(argv[0]);
	t = argv[1][1];
*/

	/* decode option value */
/*	switch (t)
	{
	case 'n': 
		type = FSCNTL_NCACHE;
		break;

	case 'j': 
		type = FSCNTL_JCACHE;
		break;

	case 'l': 
		type = FSCNTL_LCACHE;
		break;

	case 'd': 
		type = FSCNTL_DCACHE;
		break;

	case 'i': 
		type = FSCNTL_ICACHE;
		break;

	default:
		usage(argv[0]);
	}
*/	
	/* issue fscntl() request */
	type = FSCNTL_NCACHE;
	*xarg = type;
        rc = DosFSCtl(  xarg,           /* data area */
                        xargsize,       /* size of data */
                        &dummyd,        /* returned data length */
                        NULL,           /* parameter list */
                        0,              /* size of parameter list */
                        &dummyp,        /* return parm length */
                        JFSCTL_CACHESTATS,   /* function code */
                        "JFS",          /* target file system */
                        -1,             /* file handle */
                        FSCTL_FSDNAME); /* method of file system id */
	
	if (rc != 0)
	{
		printf("fscntl call returned %d\n", rc);
		exit(2);
	}
      nStatistics();
      type = FSCNTL_JCACHE;
	*xarg = type;
        rc = DosFSCtl(  xarg,           /* data area */
                        xargsize,       /* size of data */
                        &dummyd,        /* returned data length */
                        NULL,           /* parameter list */
                        0,              /* size of parameter list */
                        &dummyp,        /* return parm length */
                        JFSCTL_CACHESTATS,   /* function code */
                        "JFS",          /* target file system */
                        -1,             /* file handle */
                        FSCTL_FSDNAME); /* method of file system id */
	
	if (rc != 0)
	{
		printf("fscntl call returned %d\n", rc);
		exit(2);
	}
	jStatistics();
	type = FSCNTL_LCACHE;
	*xarg = type;
        rc = DosFSCtl(  xarg,           /* data area */
                        xargsize,       /* size of data */
                        &dummyd,        /* returned data length */
                        NULL,           /* parameter list */
                        0,              /* size of parameter list */
                        &dummyp,        /* return parm length */
                        JFSCTL_CACHESTATS,   /* function code */
                        "JFS",          /* target file system */
                        -1,             /* file handle */
                        FSCTL_FSDNAME); /* method of file system id */
	
	if (rc != 0)
	{
		printf("fscntl call returned %d\n", rc);
		exit(2);
	}
	lStatistics();
	type = FSCNTL_ICACHE;
	*xarg = type;
        rc = DosFSCtl(  xarg,           /* data area */
                        xargsize,       /* size of data */
                        &dummyd,        /* returned data length */
                        NULL,           /* parameter list */
                        0,              /* size of parameter list */
                        &dummyp,        /* return parm length */
                        JFSCTL_CACHESTATS,   /* function code */
                        "JFS",          /* target file system */
                        -1,             /* file handle */
                        FSCTL_FSDNAME); /* method of file system id */
	
	if (rc != 0)
	{
		printf("fscntl call returned %d\n", rc);
		exit(2);
	}
	iStatistics();

	/* display */
}
		
/*
 *	jStatistics()
 */
jStatistics()
{
	struct statJCache *statjcache;

//	printf("\n - Cache statistics - \n");

	statjcache = (struct statjcache *)&xarg;
	printf("JCache:\treclaim: %d\n", statjcache->reclaim);
	printf("\tread: %d\n", statjcache->read);
	printf("\trecycle: %d\n", statjcache->recycle);
	printf("\tlazywrite.awrite: %d\n", statjcache->lazywriteawrite);
	printf("\trecycle.awrite: %d\n", statjcache->recycleawrite);
	printf("\tlogsync.awrite: %d\n", statjcache->logsyncawrite);
	printf("\twrite: %d\n", statjcache->write);
}
		
/*
 *	nStatistics()
 */
nStatistics()
{
	struct statNCache	*statncache;

//	printf("\n - ncache statistics - \n");

	statncache = (struct statncache *)&xarg;
	printf("NCache:\tlookup: %d\n", statncache->lookups);
	printf("\thit: %d\n", statncache->hits);
	printf("\tmiss: %d\n", statncache->misses);
	printf("\tenter: %d\n", statncache->enters);
	printf("\tdelete: %d\n", statncache->deletes);
	printf("\tname2long: %d\n", statncache->nam2long);
}

/*
 *	lStatisrics()
 */
lStatistics()
{
	struct statLCache	*statlcache;

//	printf("\n - lcache statistics - \n");

	statlcache = (struct statlcache *)&xarg;
	printf("LCache:\tcommit: %d\n", statlcache->commit);
	printf("\tpage.init: %d\n", statlcache->pageinit);
	printf("\tpage.done: %d\n", statlcache->pagedone);
	printf("\tsync: %d\n", statlcache->sync);
	printf("\tmaxbufcnt: %d\n", statlcache->maxbufcnt);
}
iStatistics()
{
	struct statICache	*staticache;

//	printf("\n - icache statistics - \n");

	staticache = (struct statICache *)&xarg;
	printf("ICache:\tn.inode: %d\n", staticache->ninode);
	printf("\treclaim: %d\n", staticache->reclaim);
	printf("\trecycle: %d\n", staticache->recycle);
	printf("\trelease: %d\n", staticache->release);
}


usage(char *n)
{
	printf("usage: %s -[njli]\n", n);
	exit(1);
}
