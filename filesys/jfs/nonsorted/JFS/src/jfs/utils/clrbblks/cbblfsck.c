/* $Id: cbblfsck.c,v 1.1.1.1 2003/05/21 13:41:00 pasha Exp $ */

static char *SCCSID = "@(#)1.2  9/12/98 11:51:22 src/jfs/utils/clrbblks/cbblfsck.c, jfscbbl, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		cbblfsck.c
 *
 *   COMPONENT_NAME: 	jfscbbl
 *
 *   FUNCTIONS:		init_cbblfsck_record
 *			complete_cbblfsck_record
 *		 
 *           		read_cbblfsck_record
 *         		write_cbblfsck_record
 *
 */

#include "clrbblks.h"

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *	device and file system under clrbblks
  *
  *      defined in clrbblks.c
  */
extern char	LVName[];	// device name

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * fsck aggregate info structure pointer
  *
  *      defined in clrbblks.c
  */
extern struct cbbl_agg_record *agg_recptr;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *	communication area 
  *
  *      defined in clrbblks.c
  */
extern int64                            cbblfsck_area_byteoff;
extern struct fsckcbbl_record  *cbblfsck_recptr;
extern int                               cbblfsck_area_size;
extern int                               cbblfsck_commun_active;


/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 *
 * The following are internal to this file
 *
 */
int32 read_cbblfsck_record ( void ); 
int32 write_cbblfsck_record ( void ); 

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */


/****************************************************************************
 * NAME: 	complete_cbblfsck_record
 *
 * FUNCTION:	Copies stats from the clrbblks aggregate record into
 *		the communication area buffer, then writes the buffer
 *		to the device.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 complete_cbblfsck_record( ) 
{
    int32	   ccr_rc = 0;

    if( cbblfsck_commun_active ) {
        cbblfsck_recptr->total_bad_blocks = agg_recptr->total_bad_blocks;
        cbblfsck_recptr->resolved_blocks = agg_recptr->resolved_blocks;
        cbblfsck_recptr->reloc_extents = agg_recptr->reloc_extents;
        cbblfsck_recptr->reloc_blocks = agg_recptr->reloc_blocks;
        cbblfsck_recptr->LVM_lists = agg_recptr->LVM_lists;
	/*
	 * we ignore the return code from the write routine.
	 *
	 * It either works or it doesn't, and doesn't really affect
	 * the success of the clrbblks utility, but only chkdsk's 
	 * ability to report and/or log stats from the clrbblks run.
	 */
        write_cbblfsck_record( );
        }

    return( ccr_rc );
}				/* end complete_cbblfsck_record() */


/****************************************************************************
 * NAME: 	init_cbblfsck_record
 *
 * FUNCTION:	Copies stats from the clrbblks aggregate record into
 *		the communication area buffer, then writes the buffer
 *		to the device.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 init_cbblfsck_record( ) 
{
    int32	   icr_rc = 0;

    cbblfsck_area_byteoff = (int64) agg_recptr->fs_first_wspblk * 
				(int64) agg_recptr->fs_blksize;
    icr_rc = read_cbblfsck_record( );
    if( icr_rc != 0 ) {
        cbblfsck_commun_active = 0;
        }
    else {
        cbblfsck_commun_active = -1;
	  
        memset( (void *) cbblfsck_recptr, 0, sizeof(struct fsckcbbl_record) );
        memcpy( (void *) &(cbblfsck_recptr->eyecatcher), 
		(void *) "fsckcbbl", 8 );
        cbblfsck_recptr->cbbl_retcode = -1;
        cbblfsck_recptr->fs_blksize = agg_recptr->fs_blksize;
        cbblfsck_recptr->lv_blksize = agg_recptr->lv_blksize;
        cbblfsck_recptr->fs_lv_ratio = agg_recptr->fs_lv_ratio;
        cbblfsck_recptr->fs_last_metablk = agg_recptr->fs_last_metablk;
        cbblfsck_recptr->fs_first_wspblk = agg_recptr->fs_first_wspblk;
        memcpy( (void *) &(cbblfsck_recptr->bufptr_eyecatcher), 
		(void *) "cbblbufs", 8 );
        cbblfsck_recptr->clrbblks_agg_recptr = agg_recptr;
        cbblfsck_recptr->ImapInoPtr = agg_recptr->ImapInoPtr;
        cbblfsck_recptr->ImapCtlPtr = agg_recptr->ImapCtlPtr;
        cbblfsck_recptr->ImapLeafPtr = agg_recptr->ImapLeafPtr;
        cbblfsck_recptr->iagPtr = agg_recptr->iagPtr;
        cbblfsck_recptr->InoExtPtr = agg_recptr->InoExtPtr;
        }

    return( icr_rc );
}				/* end init_cbblfsck_record() */


/****************************************************************************
 * NAME: 	read_cbblfsck_record
 *
 * FUNCTION:	Copies stats from the clrbblks aggregate record into
 *		the communication area buffer, then writes the buffer
 *		to the device.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 read_cbblfsck_record( ) 
{
    int32	   rcr_rc = 0;

    rcr_rc = ujfs_rw_diskblocks(LVHandle, cbblfsck_area_byteoff, 
				cbblfsck_area_size, 
				(void *)cbblfsck_recptr, GET);
    return( rcr_rc );
}				/* end read_cbblfsck_record() */


/****************************************************************************
 * NAME: 	write_cbblfsck_record
 *
 * FUNCTION:	Copies stats from the clrbblks aggregate record into
 *		the communication area buffer, then writes the buffer
 *		to the device.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 write_cbblfsck_record( ) 
{
    int32	   wcr_rc = 0;

    wcr_rc = ujfs_rw_diskblocks(LVHandle, cbblfsck_area_byteoff, 
				cbblfsck_area_size, 
				(void *)cbblfsck_recptr, PUT);

    return( wcr_rc );
}				/* end write_cbblfsck_record() */
