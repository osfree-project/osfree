/* $Id: logform.c,v 1.1.1.1 2003/05/21 13:41:52 pasha Exp $ */

static char *SCCSID = "@(#)1.12  3/24/99 15:57:23 src/jfs/utils/libfs/logform.c, jfslib, w45.fs32, 990417.1";
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
 *   MODULE_NAME:               jfs_logform.c
 *
 *   COMPONENT_NAME:         jfslib
 *
 *  format log
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
#include <stdio.h>

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSDEVIOCTL
#include <os2.h>
#include "jfs_types.h"
#include <jfs_aixisms.h>
#include "sysbloks.h"
#include <extboot.h>
#include "bootsec.h"
#include "devices.h"

#include "jfs_superblock.h"
#include "jfs_filsys.h"
#include "jfs_logmgr.h"
#include "logform.h"
#include "message.h"

int32 stdout_redirected;

/*
 * NAME:	jfs_logform
 * FUNCTION:	format file system log
 *
 * RETURN:	0 -	successful
 *		-1 -	error occur 
 *
 */
int32 jfs_logform(
        int32   fd, 	   /* for inline log, this is a file descriptor
			    * for an opened device to write log.
			    * for outline log, it is -1 */
	int32   aggr_blk_size, /* aggregate block size in bytes */
        int32   s_l2bsize, /* log2 of aggregate block size in bytes */
	uint32  s_flag,    /* fs superblock s_flag is passed in   */
        int64   log_start, /* offset of the start of inline log in 
			    * number of aggr. blocks. for outline log
			    * it is set as zero */
        int32   log_len,   /* inline log length in number of aggr. blks 
			    * for outline log, it is zero */
	char *  dev_name,  /* logical volume of the outline log device  
			    * for inline log, it is NULL */
	int32   nblks      /* size of the outline log in 512 byte blocks
			    * for inline log, it is zero */
)
{
        ULONG Action;
        int64	log_len_in_bytes;
        char  parms = 0;
        unsigned long parmlen = sizeof(parms);
        ULONG actual;
        struct DPB dev;
        unsigned long devlen = sizeof(dev);

	int32 oflag,logfd,npages,rc,k,dblks,total_blks;
	char  logpages[4 * LOGPSIZE];
	logpage_t *logp;		/* array of 4 log pages */
	static logsuper_t log_sup;
	struct lrd *lrd_ptr;
	int64  log_begin;  /* the location of the beginning of the log inside
			    * of the file system. ( in bytes )
			    */
	int64 log_contwt;
	char answer;
	int16 inlinelog = (s_flag & JFS_INLINELOG );
	int    Working_counter;
	char *Working[5];

#define LOGBUFSIZE	4 * LOGPSIZE
	logp = (logpage_t *) &logpages;
	Working[0] = "   |\r";
	Working[1] = "   /\r";
	Working[2] = "   -\r";
	Working[3] = "   \\\r";

   /*
    * check to see whether standard out has been redirected, and
    * set the flag accordingly.
    */
  if( (ujfs_stdout_redirected()) != 0 ) {
    stdout_redirected = 1;
    }
  else {
    stdout_redirected = 0;
    }

	/* 
	 * if it is an outline log, do device check and open device
	 */
	if (!inlinelog )
	{
		/*  open the device 
	 	*/
		if (DosOpen(dev_name, (unsigned long *)&logfd, &Action, 0, 0, 
			OPEN_ACTION_OPEN_IF_EXISTS,
			OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR |
			OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE, 0))
                	goto errout;
		fd = logfd;

		/*
		 * Lock the drive
		 */
		rc = DosDevIOCtl(fd, IOCTL_DISK, DSK_LOCKDRIVE, 
				&parms, sizeof(parms),
				&parmlen, &parms, sizeof(parms), &parmlen);

		/*
	 	* validate/determine device size.
	 	* for bringup allow ioctl to fail or to report zero size.
	 	*/
		rc = DosDevIOCtl(fd, IOCTL_DISK, DSK_GETDEVICEPARAMS, &parms,
                         sizeof(parms), &parmlen, &dev, sizeof(dev), &devlen);
		if (rc == NO_ERROR)
		{
			total_blks = dev.dev_bpb.total_sectors +
			dev.dev_bpb.large_total_sectors;
			if (total_blks > 0)
			{
				dblks = (nblks == 0) ? total_blks :
					MIN(nblks,total_blks);
			}
		}

		if ((npages = dblks/(LOGPSIZE/512)) == 0)
		{
			printf("ioctl failed \n");
			printf("try again but specify number of blocks \n");
			return -1;
		}	

		/* before we destroy anything in the log, try to
		 * confirm with the user
	 	 */
			while(TRUE) {
				printf("logform: destroy %s (y)?",dev_name);
				fflush(stdout);
				answer = getchar();
				if(answer == 'n' || answer == 'N') exit(1);
				else if(answer == 'y' || answer == 'Y' ||
							answer == '\n') break;
				while((answer = getchar()) != '\n');
			}
        rc = write_bootsec(fd, &(dev.dev_bpb), "jfslog", 1);

	}
	else {  /* the fs has an inlinelog  */
		log_len_in_bytes = ((int64) log_len) << s_l2bsize;
		npages = log_len_in_bytes / LOGPSIZE; 
		}

		/* 
		 * init log superblock: log page 1
		 */
	log_sup.magic = LOGMAGIC;
	log_sup.version = LOGVERSION;
	log_sup.state = LOGREDONE;
	log_sup.flag = s_flag;  /* assign fs s_flag to log superblock.
				 * currently s_flag carries the inlinelog
				 * info and commit option ( i.e. group commit
				 * or lazy commit, etc.. )
				 */
	log_sup.size = npages;
	log_sup.bsize = aggr_blk_size;
	log_sup.l2bsize = s_l2bsize; 
	log_sup.end = 2*LOGPSIZE + LOGPHDRSIZE + LOGRDSIZE;

	/* find the log superblock location 
	 */
	log_begin = log_start << s_l2bsize;
	rc = ujfs_rw_diskblocks(fd, (log_begin+LOGPSIZE), (unsigned)LOGPSIZE,
					(char *)&log_sup, PUT);
	if ( rc != 0 )
		goto errout;

	/*
	 * init device pages 2 to npages-1 as log data pages:
	 *
	 * log page sequence number (lpsn) initialization:
	 * the N (= npages-2) data pages of the log is maintained as 
	 * a circular file for the log records;
	 * lpsn grows by 1 monotonically as each log page is written 
	 * to the circular file of the log;
	 * Since the AIX DUMMY log record is dropped for this XJFS, 
	 * and setLogpage() will not reset the page number even if
	 * the eor is equal to LOGPHDRSIZE. In order for binary search
	 * still work in find log end process, we have to simulate the
	 * log wrap situation at the log format time.
	 * The 1st log page written will have the highest lpsn. Then
	 * the succeeding log pages will have ascending order of
	 * the lspn starting from 0, ... (N-2)
	 */

	/* 
		initialize 1st 2 log pages to be written: lpsn = N-1, 0
		and also a SYNCPT log record is written to the N-1 page 
	
		Since the log is always an even number of meg, if we
		write 2 pages before entering the loop, we are assured
		that the log will end after a 4 page buffer.
	*/

	logp[0].h.eor = logp[0].t.eor = LOGPHDRSIZE + LOGRDSIZE;
	logp[0].h.page = logp[0].t.page = npages - 3;
	lrd_ptr = (struct lrd *)&logp[0].data;
	lrd_ptr->logtid = 0;
	lrd_ptr->backchain = 0;
	lrd_ptr->type = LOG_SYNCPT;
	lrd_ptr->length = 0;
	lrd_ptr->log.syncpt.sync = 0;

	logp[1].h.eor = logp[1].t.eor = LOGPHDRSIZE;
	logp[1].h.page = logp[1].t.page = 0;
	lrd_ptr = (struct lrd *)&logp[1].data;
	lrd_ptr->logtid = 0;
	lrd_ptr->backchain = 0;
	lrd_ptr->type = LOG_SYNCPT;
	lrd_ptr->length = 0;
	lrd_ptr->log.syncpt.sync = 0;

	rc = ujfs_rw_diskblocks(fd, (log_begin+2*LOGPSIZE), 
					(unsigned) 2*LOGPSIZE,
					(char *)&(logp[0]), PUT);
	if ( rc != 0 )
		goto errout;

	/* initialize buffer to write 4 pages at a time */
	logp[0].h.eor = logp[0].t.eor = LOGPHDRSIZE;

	logp[2].h.eor = logp[2].t.eor = LOGPHDRSIZE;
	lrd_ptr = (struct lrd *)&logp[2].data;
	lrd_ptr->logtid = 0;
	lrd_ptr->backchain = 0;
	lrd_ptr->type = LOG_SYNCPT;
	lrd_ptr->length = 0;
	lrd_ptr->log.syncpt.sync = 0;

	logp[3].h.eor = logp[3].t.eor = LOGPHDRSIZE;
	lrd_ptr = (struct lrd *)&logp[3].data;
	lrd_ptr->logtid = 0;
	lrd_ptr->backchain = 0;
	lrd_ptr->type = LOG_SYNCPT;
	lrd_ptr->length = 0;
	lrd_ptr->log.syncpt.sync = 0;

	/* initialize succeeding log  pages: lpsn = 1, 2, ..., (N-2) */
	Working_counter = 0;
	log_contwt = log_begin + LOGBUFSIZE;
	for ( k = 1; k < npages - 4; k+=4 )
	{
		logp[0].h.page = logp[0].t.page = k;
		logp[1].h.page = logp[1].t.page = k + 1;
		logp[2].h.page = logp[2].t.page = k + 2;
		logp[3].h.page = logp[3].t.page = k + 3;
		rc = ujfs_rw_diskblocks(fd, log_contwt, 
				(unsigned) LOGBUFSIZE, (char *)&(logp[0]), PUT);
		if (rc != 0 )
			goto errout;
		log_contwt += LOGBUFSIZE;

		if( !stdout_redirected ) {
  			Working_counter++;
			switch( Working_counter ) {
			    case( 100 ):
				DosPutMessage( 1, strlen(Working[0]), Working[0] );
				fflush( stdout );
				break;
			    case( 200 ):
				DosPutMessage( 1, strlen(Working[1]), Working[1] );
				fflush( stdout );
				break;
			    case( 300 ):
				DosPutMessage( 1, strlen(Working[2]), Working[2] );
				fflush( stdout );
				break;
			    case( 400 ):
				DosPutMessage( 1, strlen(Working[3]), Working[3] );
				fflush( stdout );
				Working_counter = 0;
				break;
			    default:
				break;
			}
		}
	}
	if (!inlinelog)
	{
        	rc = DosDevIOCtl(fd, IOCTL_DISK, DSK_UNLOCKDRIVE, &parms,
				 sizeof(parms), &parmlen, &parms,
				 sizeof(parms), &parmlen);
        	rc = DosClose(fd);
	}

	return(0);

errout:
	if (rc == ERROR_WRITE_PROTECT)
		message_user(ERROR_WRITE_PROTECT, NULL, 0, STDOUT_CODE,
			     NO_RESPONSE, OSO_MSG);
	return(-1);

}
