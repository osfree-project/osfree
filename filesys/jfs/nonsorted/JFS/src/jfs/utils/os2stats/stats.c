/* $Id: stats.c,v 1.1.1.1 2003/05/21 13:42:48 pasha Exp $ */

static char *SCCSID = "@(#)1.2  7/31/98 08:21:25 src/jfs/utils/os2stats/stats.c, jfsutil, w45.fs32, 990417.1";
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
 * Module: fscntl.c
 */

#define INCL_DOSPROCESS
#include <os2.h>

#include "jfs_types.h"
#include "jfs_cntl.h"


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
	if (argc != 2)
		usage(argv[0]);
	if (*argv[1] != '-')
		usage(argv[0]);
	t = argv[1][1];


	/* decode option value */
	switch (t)
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
/*
	case 'd': 
		type = FSCNTL_DCACHE;
		break;

	case 'i': 
		type = FSCNTL_ICACHE;
		break;
*/
	default:
		usage(argv[0]);
	}
	
	/* issue fscntl() request */
	*xarg = type;
        rc = DosFSCtl(  xarg,           /* data area */
                        xargsize,       /* size of data */
                        &dummyd,        /* returned data length */
                        NULL,           /* parameter list */
                        0,              /* size of parameter list */
                        &dummyp,        /* return parm length */
                        JFSCTL_STATS,   /* function code */
                        "JFS",          /* target file system */
                        -1,             /* file handle */
                        FSCTL_FSDNAME); /* method of file system id */
	
	if (rc != 0)
	{
		printf("fscntl call returned %d\n", rc);
		exit(2);
	}

	/* display */
	switch (type)
	{
	    case FSCNTL_JCACHE:
		jStatistics();
		break;

	    case FSCNTL_NCACHE:
		nStatistics();
		break;

	    case FSCNTL_LCACHE:
		lStatistics();
		break;
	}
}
		
/*
 *	jStatistics()
 */
jStatistics()
{
	struct statjcache	*statjcache;

	printf("\n - jcache statistics - \n");

	statjcache = (struct statjcache *)&xarg;
	printf("reclaim: %d\n", statjcache->reclaim);
	printf("read: %d\n", statjcache->read);
	printf("recycle: %d\n", statjcache->recycle);
	printf("lazywrite.awrite: %d\n", statjcache->lazywriteawrite);
	printf("recycle.awrite: %d\n", statjcache->recycleawrite);
	printf("logsync.awrite: %d\n", statjcache->logsyncawrite);
	printf("write: %d\n", statjcache->write);
}
		
/*
 *	nStatistics()
 */
nStatistics()
{
	struct statncache	*statncache;

	printf("\n - ncache statistics - \n");

	statncache = (struct statncache *)&xarg;
	printf("lookup: %d\n", statncache->lookups);
	printf("hit: %d\n", statncache->hits);
	printf("miss: %d\n", statncache->misses);
	printf("enter: %d\n", statncache->enters);
	printf("delete: %d\n", statncache->deletes);
	printf("name2long: %d\n", statncache->nam2long);
}

/*
 *	lStatisrics()
 */
lStatistics()
{
	struct statlcache	*statlcache;

	printf("\n - lcache statistics - \n");

	statlcache = (struct statlcache *)&xarg;
	printf("commit: %d\n", statlcache->commit);
	printf("page.init: %d\n", statlcache->pageinit);
	printf("page.done: %d\n", statlcache->pagedone);
	printf("sync: %d\n", statlcache->sync);
	printf("maxbufcnt: %d\n", statlcache->maxbufcnt);
}


usage(char *n)
{
	printf("usage: %s -[njl]\n", n);
	exit(1);
}
