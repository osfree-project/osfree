/* $Id: extendfs.c,v 1.1.1.1 2003/05/21 13:41:24 pasha Exp $ */

static char *SCCSID = "@(#)1.5  9/25/98 20:53:17 src/jfs/utils/extendfs/extendfs.c, jfsextendfs, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		extendfs.c
 *
 *   COMPONENT_NAME: 	jfsutil
 *
 *   FUNCTIONS:		main
 *			extendfs
 *			whatDoYouWant
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
#include "jfs_superblock.h"
#include "jfs_filsys.h"
#include "jfs_inode.h"
#include "jfs_dmap.h"
#include "jfs_imap.h"
#include "jfs_xtree.h"
#include "jfs_dtree.h"
#include "jfs_cntl.h"
#include "jfs_extendfs.h"
#include "jfs_debug.h"

#include "devices.h"
#include "debug.h"

#include "fssubs.h"
#include "utilsubs.h"
#include "message.h"

/* Global telling openLV to fail silently */
extern	int32	Fail_Quietly;

/* command parameters */
int32	LogSize = 0;
int64	FSSize = 0;
int32	Query = FALSE;

/*
 *	device under extendfs
 */
char	LVName[4];	// device name

/*
 *	Parameter array for messages
 */
#define MAXPARMS	2
#define MAXSTR		80
char	*msg_parms[MAXPARMS];
char	msgstr[MAXSTR];

/*
 * forward references
 */
static int32 extendfs(void);
int32 openLV(char *LVName);
static int32 whatDoYouWant(int32 argc, char **argv);


/*
 * NAME:	main()
 *
 * FUNCTION:	Main routine of extendfs utility.
 */
main(
	int32	argc,
	char	**argv)
{
	char		drive;
	uint32		free_space;
	FSALLOCATE	fsalloc_buf;
	int32	rc;

	DosError(FERR_DISABLEHARDERR);

	/* parse command line arguments */
	if (rc = whatDoYouWant(argc, argv))
	{
		message_user(MSG_JFS_EXTENDFS_USAGE, NULL, 0, STDOUT_CODE,
			     NO_RESPONSE, JFS_MSG);
		goto out;
	}

	if (LVName[0] == '*')
	{
		/* Try to extend all partitions. */

		/* This tells openLV to fail silently */
		Fail_Quietly = 1;

		LVName[1] = ':';
		LVName[2] = 0;

		for (drive = 'C'; drive <= 'Z'; drive++)
		{
			LVName[0] = drive;
			rc = openLV(LVName);
			if (rc == 0)
			{
				extendfs();
				closeFS();
			}
		}
		return 0;
	}

	/* validate and open LV */
	if (rc = openLV(LVName))
		goto out;	

#ifdef	_JFS_DEBUG_EXTENDFS
	/* open FS */
	if (rc = openFS())
		goto out;
#endif	/* _JFS_DEBUG_EXTENDFS */

	/* extend the file system */
	rc = extendfs();

	if (rc == 0)
	{
		message_user(MSG_JFS_EXTENDFS_COMPLETE, NULL, 0, STDOUT_CODE,
			     NO_RESPONSE, JFS_MSG);
		sprintf(msgstr, "%lld", (lvMount->LVSize << lvMount->l2pbsize) >> 10);
		msg_parms[0] = msgstr;
		message_user(MSG_OSO_DISK_SPACE, msg_parms, 1, STDOUT_CODE,
			     NO_RESPONSE, OSO_MSG);
		rc = DosQueryFSInfo((ULONG)(toupper(LVName[0])-'A'+1), 1L,
				    (PVOID)&fsalloc_buf, sizeof(FSALLOCATE));
		if (rc)
		{
			message_user(MSG_JFS_EXTENDFS_FAILED, NULL, 0,
				     STDOUT_CODE, NO_RESPONSE, JFS_MSG);
		}
		else
		{
			free_space = (fsalloc_buf.cSectorUnit *
				      fsalloc_buf.cUnitAvail) /
				     (1024 / fsalloc_buf.cbSector);
			_itoa(free_space, msgstr, 10);
			msg_parms[0] = msgstr;
			message_user(MSG_OSO_FREE_SPACE, msg_parms, 1,
				     STDOUT_CODE, NO_RESPONSE, OSO_MSG);
		}
	}
	else
	{
		message_user(MSG_JFS_EXTENDFS_FAILED, NULL, 0, STDOUT_CODE,
			     NO_RESPONSE, JFS_MSG);
	}
#ifdef	_JFS_DEBUG_EXTENDFS
	/* close FS */
	closeFS();
#endif	/* _JFS_DEBUG_EXTENDFS */

out:
	return rc;
}


/*
 *	extendfs()
 */
static int32 extendfs(void)
{
	int32	rc;
	int32	pList;
	ULONG	pListLen = 0;
	extendfs_t	pData;
	ULONG	pDataLen = 0;

	pDataLen = sizeof(extendfs_t);

	/*
	 *	extend file system
	 */
	pData.flag = 0;
	pData.dev = lvMount->LVNumber;
	pData.LVSize = lvMount->LVSize;
	pData.LogSize = LogSize;
	pData.FSSize = FSSize;

	rc = fscntl(JFSCTL_EXTENDFS,
	       (void *)&pList, &pListLen,
	       (void *)&pData, &pDataLen);

	return rc;
}


/*
 *	whatDoYouWant()
 *
 * function: extend file system;
 *
 * extendfs [-l LogSize] {device_name}
 *
 *   |-------------------------------|----------|----------|
 *   file system space               fsck       inline log
 *                                   workspace  space
 *
 * input:
 *	option:
 *	 -ls:LogSize: new inline log size in megabytes;
 *	device_name - device under extension.
 */
static int32 whatDoYouWant(
	int32	argc,
	char	**argv)
{
	int32	rc = 0;
	int32	i;
	char	*argp;
	int32	devFound = FALSE;

#ifdef NO_DRIVE_NEEDED
	UCHAR	cwd[80];

	/* initialize the disk name to be the current drive */
	getcwd(cwd, 80);
	LVName[0] = cwd[0];
	LVName[1] = cwd[1];
	LVName[2] = '\0';
#endif

	if (argc < 2)
		return EINVAL;

	/* parse each of the command line parameters */
	for (i = 1; i < argc; i++)
	{
		argp = argv[i];

		/* an option ? */
		if (*argp == '/' || *argp == '-')  
		{
			argp++;  /* increment past / or - */
			if (strncmp(argp, "ls", 2) == 0 || 
			    strncmp(argp, "LS", 2) == 0)
			{
				if (argp[2] == ':')
					LogSize = strtoll(argp + 3, NULL, 0);
				else
					return EINVAL;
			}
			else
				return EINVAL;

			continue;
		}

		/* a drive letter */
		if (isalpha(*argp) && argp[1] == ':' && !argp[2])
		{  
			/* only allow one drive letter to be specified */
			if (devFound)  
				return EINVAL;
			else
			{
				strcpy(LVName, argp);
				devFound = TRUE;
			}

			continue;
		}
		if ((*argp == '*') && ! argp[1])
		{
			if (devFound)
				return EINVAL;

			LVName[0] = '*';
			LVName[1] = 0;
			devFound = TRUE;

			continue;
		}

		/* UFO parameter */
		return EINVAL;
	} /* end for */

	if (!devFound)
		rc = EINVAL;

	return (rc);
}
