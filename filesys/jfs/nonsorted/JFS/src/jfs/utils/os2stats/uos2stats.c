/* $Id: uos2stats.c,v 1.1.1.1 2003/05/21 13:42:48 pasha Exp $ */

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


// int		xarg[1024];

/*
 *	main()
 */
int32 uos2stats(
                unsigned char t,
                PVOID out
               )
{
	int32 rc;
	int type;
       int *xarg = (int *)out;
       unsigned long dummyd = 0;
       unsigned long dummyp = 0;
       int	xargsize = 4096;

	/* decode option value */
	switch (type)
	{
	case 'n': 
	case 'N': 
		type = FSCNTL_NCACHE;
		break;

	case 'j': 
	case 'J': 
		type = FSCNTL_JCACHE;
		break;

	case 'l': 
	case 'L': 
		type = FSCNTL_LCACHE;
		break;

	case 'd': 
	case 'D': 
		type = FSCNTL_DCACHE;
		break;

	case 'i': 
	case 'I': 
		type = FSCNTL_ICACHE;
		break;
	default:
             return 87;                    // Invalid parameter
	}
	/* issue fscntl() request */
//	type = FSCNTL_NCACHE;
	 *xarg  = type;
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
	return(rc);
}
