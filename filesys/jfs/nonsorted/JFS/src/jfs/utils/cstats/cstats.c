/* $Id: cstats.c,v 1.1.1.1 2003/05/21 13:41:12 pasha Exp $ */

static char *SCCSID = "@(#)1.1  9/15/99 09:40:29 src/jfs/utils/cstats/cstats.c, jfsutil, w45.fs32, fixbld";
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
 *   MODULE_NAME:		cstats.c
 *
 *   COMPONENT_NAME: 	jfsutil
 *
 *   FUNCTIONS:		main
 *		
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define INCL_DOS
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSDEVIOCTL
#define INCL_DOSMEMMGR

#include <os2.h>

#include "jfs_types.h"
#include "jfs_aixisms.h"
#include "jfs_inode.h"
#include "jfs_cntl.h"
#include "jfs_cachemgr.h"

/*
 * NAME:	main()
 *
 * FUNCTION:	Main routine of cstats utility.
 */
main(
	int32	argc,
	char	**argv)
{
	struct cachestats cs;
	int32	other;
	int32	rc;
	ULONG	pParmLen = 0;
	ULONG	pDataLen = 0;

	pDataLen = sizeof(cs);

	rc = DosFSCtl((PVOID)&cs, pDataLen, &pDataLen, 0, 0, &pParmLen,
		      JFSCTL_CACHESTATS, "JFS", -1, FSCTL_FSDNAME);

	if (rc)
	{
		printf("DosFSCtl returned %d\n", rc);
		return rc;
	}

	other = cs.cachesize -
	       (cs.nfreecbufs + cs.cbufs_protected + cs.cbufs_probationary +
		cs.cbufs_inuse + cs.cbufs_io + cs.jbufs_protected +
		cs.jbufs_probationary + cs.jbufs_inuse + cs.jbufs_io +
		cs.jbufs_nohomeok);

	printf("cachesize  %8d   cbufs_protected    %8d\n", cs.cachesize,
	       cs.cbufs_protected);
	printf("hashsize   %8d   cbufs_probationary %8d\n", cs.hashsize,
	       cs.cbufs_probationary);
	printf("nfreecbufs %8d   cbufs_inuse        %8d\n", cs.nfreecbufs,
	       cs.cbufs_inuse);
	printf("minfree    %8d   cbufs_io           %8d\n", cs.minfree,
	       cs.cbufs_io);
	printf("maxfree    %8d   jbufs_protected    %8d\n", cs.maxfree,
	       cs.jbufs_protected);
	printf("numiolru   %8d   jbufs_probationary %8d\n", cs.numiolru,
	       cs.jbufs_probationary);
	printf("slrun      %8d   jbufs_inuse        %8d\n", cs.slrun,
	       cs.jbufs_inuse);
	printf("slruN      %8d   jbufs_io           %8d\n", cs.slruN,
	       cs.jbufs_io);
	printf("Other      %8d   jbufs_nohomeok     %8d\n", other,
	       cs.jbufs_nohomeok);
	return 0;
}
