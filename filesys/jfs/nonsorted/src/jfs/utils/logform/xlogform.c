/* $Id: xlogform.c,v 1.2 2004/03/21 02:43:37 pasha Exp $ */

static char *SCCSID = "@(#)1.1  10/11/96 08:20:45 src/jfs/utils/logform/xlogform.c, jfsutil, w45.fs32, 990417.1";
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
 *   COMPONENT_NAME: cmdufs
 *
 *   FUNCTIONS:  jfs_logform.c: format log
 *
 *
 *
 * Description:
 *
 * This is the main() for the main function jfs_logform() in libfs.
 *
 * In OS/2 there is no mount/umount concept, the logical partition
 * name ( e.g. C:, D: ) are dynamically assigned at the system boot
 * time. At the boot time, OS/2 sequencially scan each partition and
 * make the assignment. 
 * Since the logical partition name could change each time the system
 * boot, so XJFS decided to put log inside the file system until this 
 * problem is solved. 
 * The following logform utility is used to support both inline log file 
 * system and outline log file system.
 * For logform command, it should only be used when it is an outline
 * log file system.
 * in addtion, logform is called from mkfs or logredo, or extendfs 
 * (kernel code).
 *
 * command:
 *	xlogform lvname [nblocks]
 *
 * where 
 * lvname	- the name of a logical volume of the outline log device;
 * [nblocks]	- the size of the log in 512 byte blocks
 * 		  (optional paramter);
 *		  this parameter is here for bringup purpose. For
 *		  normal use, always put zero.
 *	
 * the following disk pages starting from the beginning of the log
 * are formatted:
 *
 * page 0 - not changed              
 * page 1 - log superblock
 * page 2 - A SYNC log record is written into this page at logform time
 * pages 3-N  - set to empty log pages
 */

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <stdio.h>
#include <unistd.h>
#include <jfs/fsdefs.h>

#include "jfs_types.h"
#include "jfs_filsys.h"
#include "jfs_superblock.h"
#include "logform.h"
#include "dosstubs.h"
#include "devices.h"

extern errno;

/*
 * FUNCITON:	logform
 *		The main routine opens the device using the dev_name
 *		argument. It reads the first SIZE_OF_SUPER bytes to
 *		see if this is a file system superblock. 
 *		If yes, return error since the command logform only
 *		process a outline log logical volume.
 *		If no, assume it is a log device, then call
 *		jfs_logform() to do the actually log format.  
 */
main(argc,argv)
int argc;
char *argv[];
{
	char *dev_name;
	int32 dblks,rc, oflag;
	struct stat st;
	int32 fd;
	uint32 Action;
	union {
		struct superblock fs_super;
		char block[PAGESIZE];
	}super;

	/* get device name */
	dev_name = argv[1];

	/* 
	 * open the device 
	 */
/***
	oflag = O_RDWR;
	if ((fd = open(dev_name,oflag)) < 0)
	{
		fprintf(stderr,"logform: open device failed errno=%d\n", errno);
		return(-1);
	}
***/
        if (DosOpen(dev_name, (unsigned long *)&fd, &Action, 0, 0,
                        OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                        OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR |
                        OPEN_SHARE_DENYREADWRITE |OPEN_ACCESS_READWRITE, 0))
        {
                fprintf(stderr,"logform: open device  %s failed errno = %d\n",
                                        errno, dev_name);
                return(-1);
        }

        /* 
	 * read in the first SIZE_OF_SUPER bytes of the opened device
         * to determine if the device is an outline log device
	 * If it is a file system device, return error since the logform
	 * command should not be called when the file system uses inlinelog;  
	 * For outlinelog, then the dev_name should be the log device name.
         */
	rc = ujfs_rw_diskblocks(fd, (uint64)(SUPER1_OFF),
                         (unsigned)SIZE_OF_SUPER, super.block, GET);
	if ( rc != 0)
	{
		fprintf(stderr,"logform: read device failed errno=%d\n", errno);
		return(-1);
        }

	if (strncmp(super.fs_super.s_magic,JFS_MAGIC,(unsigned)strlen(JFS_MAGIC)) == 0 )
	{
		fprintf(stderr,
		 "logform: Cannot format a file system device %s\n",  dev_name);
		return(-1);
	}
	return(jfs_logform(-1,0,-1, 0, 0, 0, dev_name,dblks)); 

}
