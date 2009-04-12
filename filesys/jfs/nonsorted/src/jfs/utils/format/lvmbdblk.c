/* $Id: lvmbdblk.c,v 1.6 2004/07/24 00:49:04 pasha Exp $ */

static char *SCCSID = "@(#)1.5  3/31/99 16:29:35 src/jfs/utils/format/lvmbdblk.c, jfsformat, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		lvmbdblk.c
 *
 *   COMPONENT_NAME: 	jfsformat
 *
 *   FUNCTIONS:
 *		disable_LVM_BdBlkReloc
 *		enable_LVM_BdBlkReloc
 *		get_LVM_BdBlkLst_count
 *		get_LVM_BadBlockLists
 *		tell_LVM_ClearLists
 *
 */
#define INCL_DOSPROFILE
#define INCL_DOSERRORS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#include <os2.h>
#include "jfs_types.h"
#include <jfs_aixisms.h>
#include <jfs_inode.h>  /* defines IFREG, etc. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <time.h>
#include <sesrqpkt.h>
#include <ioctl.h>
#include <dskioctl.h>
#include <specdefs.h>

#include "jfs_filsys.h"
#include "jfs_dinode.h"
#include "jfs_superblock.h"
#include "super.h"
#include "jfs_dmap.h"
#include "initmap.h"
#include "inode.h"
#include "jfs_xtree.h"
#include "jfs_dtree.h"
#include "devices.h"
#include "inodes.h"
#include "debug.h"
#include "lvmbdblk.h"

extern struct lvm_bdblk_record *bdblk_recptr;	/* defined in xmkfs.c	*/
extern boolean_t  lvm_BdBlk_enabled;			/* defined in xmkfs.c    */
extern boolean_t  do_lvm_BdBlks;			/* defined in xmkfs.c */

/******************************************************************
 * NAME: 	disable_LVM_BdBlkReloc
 *
 * FUNCTION: 	Notify the LVM that if Bad Block Relocation is enabled for
 *		the given partition, it should be disabled.
 *
 * PARAMETERS:	
 *	bdblk_recptr	ptr to the bad block record describing the file
 *			system block which is now allocated to the Bad
 *			Block inode.
 *
 * NOTES:	Each bad block record describes one file system block
 *		which contains one or more bad LV blocks.
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 disable_LVM_BdBlkReloc( int32 dev_handle )
{
    int32 dlb_rc = 0;
    DDI_OS2LVM_param LVMpp;
    DDI_OS2LVM_param *pLVMpp = &LVMpp;
    DDI_OS2LVM_data LVMdp;
    DDI_OS2LVM_data *pLVMdp = &LVMdp;
    ULONG ppLen = 0;
    ULONG dpLen = 0;
    int32 		bufsize = 1024;
	
	/*
	 * initialize the LVM DosDevIOCtl parm
	 */
    pLVMpp->Command = DISABLE_BBR;
    pLVMpp->DriveUnit = 0;
    pLVMpp->TableNumber = 0;
    pLVMdp->ReturnData = 0;
    pLVMdp->UserBuffer = NULL;
    dlb_rc = DosDevIOCtl( dev_handle,		IOC_DC,	
			 IODC_LV,	 	(void *) pLVMpp,
			 sizeof(DDI_OS2LVM_param),
			 &ppLen,		 (void *) pLVMdp,
			 sizeof(DDI_OS2LVM_data),	 &dpLen	
			);
    if( dlb_rc == 0 ) {	
        lvm_BdBlk_enabled = FALSE;
        }

    return( dlb_rc );
}				/* end disable_LVM_BdBlkReloc() */


/******************************************************************
 * NAME: 	enable_LVM_BdBlkReloc
 *
 * FUNCTION: 	Notify the LVM that it can remove, from its bad block list,
 *		the entries describing the bad LV blocks contained in the
 *		given file system block.
 *
 * PARAMETERS:	
 *	bdblk_recptr	ptr to the bad block record describing the file
 *			system block which is now allocated to the Bad
 *			Block inode.
 *
 * NOTES:	Each bad block record describes one file system block
 *		which contains one or more bad LV blocks.
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 enable_LVM_BdBlkReloc( int32 dev_handle )
{
    int32 elb_rc = 0;
    DDI_OS2LVM_param LVMpp;
    DDI_OS2LVM_param *pLVMpp = &LVMpp;
    DDI_OS2LVM_data LVMdp;
    DDI_OS2LVM_data *pLVMdp = &LVMdp;
    ULONG ppLen = 0;
    ULONG dpLen = 0;
    int32 		bufsize = 1024;

	/*
	 * initialize the LVM DosDevIOCtl parm
	 */
    pLVMpp->Command = ENABLE_BBR;
    pLVMpp->DriveUnit = 0;
    pLVMpp->TableNumber = 0;
    pLVMdp->ReturnData = 0;
    pLVMdp->UserBuffer = NULL;

    elb_rc = DosDevIOCtl( dev_handle,		IOC_DC,	
			 IODC_LV,	 	(void *) pLVMpp,
			 sizeof(DDI_OS2LVM_param),
			 &ppLen,		 (void *) pLVMdp,
			 sizeof(DDI_OS2LVM_data),	 &dpLen	
			);
    if( elb_rc == 0 ) {	
        lvm_BdBlk_enabled = TRUE;
        }

    return( elb_rc );
}				/* end enable_LVM_BdBlkReloc() */
	
	
/******************************************************************
 * NAME: 	get_LVM_BdBlkLst_count
 *
 * FUNCTION: 	Query the LVM for the number of tables composing) its bad
 *		block list for the file system.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 get_LVM_BdBlkLst_count( int32 dev_handle )
{
    int32 glbblc_rc = 0;
	
    DDI_OS2LVM_param LVMpp;
    DDI_OS2LVM_param *pLVMpp = &LVMpp;
    DDI_OS2LVM_data LVMdp;
    DDI_OS2LVM_data *pLVMdp = &LVMdp;
    ULONG ppLen = 0;
    ULONG dpLen = 0;
    int32 		min_bufsize;
    int32 		bufsize = 1024;
    char 		*bufptr = NULL;
    BadBlockInfo	*BB_bufptr = NULL;
    BadBlockTableInfo 	*BBT_bufptr = NULL;
    int32 		tablesize;
    RelocationTable	*RTable = NULL;
	
	/*
	 * initialize the LVM DosDevIOCtl parm and data packets
	 */
    pLVMpp->Command = 0;
    pLVMpp->DriveUnit = 0;
    pLVMpp->TableNumber = 0;
    pLVMpp->LSN = 0;
    pLVMdp->ReturnData = 0;
    pLVMdp->UserBuffer = NULL;
	
	/*
	 * allocate a buffer
	 */
    bufptr = malloc( bufsize );
    if( bufptr == NULL ) {
	return( ENOMEM );
	}
	
	/*
	 * ask the LVM how many bad block lists it has for the filesystem
	 */
    pLVMpp->Command = GET_BBR_INFO;
    pLVMdp->UserBuffer = bufptr;
    glbblc_rc = DosDevIOCtl( dev_handle,	IOC_DC,	
			    IODC_LV,	(void *) pLVMpp,	
			    sizeof(DDI_OS2LVM_param),
			    &ppLen,		(void *) pLVMdp,	
			    sizeof(DDI_OS2LVM_data),	&dpLen
			    );	
    if( glbblc_rc != 0 ) {
	return( glbblc_rc );
	}
    else {  /* DosDevIOCtl successful */
	if( pLVMdp->ReturnData != BBR_SUCCESS ) {	/* cat 8, funct 69 failed */
	    glbblc_rc = pLVMdp->ReturnData;
	    }
	else {
	    BB_bufptr = (BadBlockInfo *) bufptr;
	    bdblk_recptr->num_tables = BB_bufptr->TotalTables;
	    }
	}  /* end else DosDevIOCtl successful */

    free( (void *) bufptr );
				
    return( glbblc_rc );
}				/* end get_LVM_BdBlkLst_count() */
	
	
/******************************************************************
 * NAME: 	get_LVM_BadBlockLists
 *
 * FUNCTION: 	Query the LVM for (the set of tables composing) its bad
 *		block list for the file system.  Save the buffers containing
 *		the tables for use populating the JFS Bad Block Inode later.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 get_LVM_BadBlockLists( int32 dev_handle )
{
    int32 glbbl_rc = 0;
    int8  buffer_allocated = 0;
    int32 bbl_idx;
    struct lbb_bufrec *buf_recptr, *last_buf_recptr = NULL;
    DDI_OS2LVM_param LVMpp;
    DDI_OS2LVM_param *pLVMpp = &LVMpp;
    DDI_OS2LVM_data LVMdp;
    DDI_OS2LVM_data *pLVMdp = &LVMdp;
    ULONG ppLen = 0;
    ULONG dpLen = 0;
    int32 		min_bufsize;
    int32			initial_bufsize = 1024;
    int32 		bufsize;
    char 		*bufptr = NULL;
    BadBlockInfo	*BB_bufptr = NULL;
    BadBlockTableInfo 	*BBT_bufptr = NULL;
    int32 		tablesize;
    RelocationTable	*RTable = NULL;
	
	/*
	 * initialize the LVM DosDevIOCtl parm and data packets
	 */
    pLVMpp->Command = 0;
    pLVMpp->DriveUnit = 0;
    pLVMpp->TableNumber = 0;
    pLVMpp->LSN = 0;
    pLVMdp->ReturnData = 0;
    pLVMdp->UserBuffer = NULL;
				
	/*
	 *
	 * get and save the LVM Bad Block Tables
	 *
	 */
    for( bbl_idx = 0;
	 ( (bbl_idx < bdblk_recptr->num_tables) && (glbbl_rc == 0) );
	 bbl_idx++ ) {
		/*
		 * if no buffer is allocated, allocate one at min size
		 */
	if( !buffer_allocated ) {
	    bufsize = initial_bufsize;
	    bufptr = malloc( bufsize );
	    if( bufptr == NULL ) {
		glbbl_rc = ENOMEM;
		break;
		}
	    else {
		buffer_allocated = -1;
		}
	    }
		/*
		 * ask the LVM how big this table is.
		 */
	pLVMpp->Command = GET_TBL_SIZE;
	pLVMpp->TableNumber = bbl_idx;
	pLVMdp->ReturnData = 0;
	pLVMdp->UserBuffer = bufptr;
	glbbl_rc =
		DosDevIOCtl( dev_handle,	IOC_DC,	
			     IODC_LV,	(void *) pLVMpp,	
			     sizeof(DDI_OS2LVM_param),
			     &ppLen,		(void *) pLVMdp,	
			     sizeof(DDI_OS2LVM_data),	&dpLen	
			    );	
	if( glbbl_rc == 0 ) {  /* DosDevIOCtl successful */
	    if( pLVMdp->ReturnData != BBR_SUCCESS ) { /* cat 8, funct 69 failed */
		glbbl_rc = pLVMdp->ReturnData;
		break;
		}
	    else {
		BBT_bufptr = (BadBlockTableInfo *) bufptr;
		tablesize = BBT_bufptr->ActiveRelocations;
		}
	    }  /* end else DosDevIOCtl successful */
	
		/*
		 *
		 * If the table isn't empty
		 *
		 */
	if( (glbbl_rc == 0) && (tablesize != 0) ) {  /*
				* successful so far and the
				* current table isn't empty
				*/
		/*
		 * if the current buffer isn't big enough for the table
		 * release it and allocate a bigger one.
		 */
	    min_bufsize = tablesize * sizeof( LSNumber );
	    if( min_bufsize > bufsize ) {  /* more buffer is needed */
		free( (void *) bufptr );
		buffer_allocated = 0;
		bufsize = ((min_bufsize + 1023) / 1024) * 1024;
		bufptr = malloc( bufsize );
		if( bufptr == NULL ) {
		    glbbl_rc = ENOMEM;
		    break;
		    }
	    	else {
		    buffer_allocated = -1;
		    }
		}
	
			/*
			 * ask the LVM for the table
	 		 */
	    pLVMpp->Command = GET_SECT_LIST;
	    pLVMpp->TableNumber = bbl_idx;
	    pLVMdp->ReturnData = 0;
	    pLVMdp->UserBuffer = bufptr;
	    glbbl_rc = DosDevIOCtl( dev_handle,	IOC_DC,	
				     IODC_LV,	(void *) pLVMpp,	
				     sizeof(DDI_OS2LVM_param),
				     &ppLen,		(void *) pLVMdp,	
				     sizeof(DDI_OS2LVM_data),	&dpLen	
				    );	
	    if( glbbl_rc != 0 ) {
		glbbl_rc = -1;
		break;
		}
	    else  {  /* DosDevIOCtl successful */
		if( pLVMdp->ReturnData != BBR_SUCCESS ) {
		    glbbl_rc = -1;
		    break;
		    }
		else {
		    RTable = (RelocationTable *) bufptr;
		    }
		}  /* end DosDevIOCtl successful */
	
	    if( glbbl_rc == 0 ) {  /* the buffer contains a table */
			/*
			 * add the buffer to the end of the list.
			 *
			 * We need to process bad blocks in ascending order
			 * so that, in case 2 bad LSNs in the same fs block are
			 * on different LVM tables, we recognize that they are
			 * in the same block.
			 */
		buf_recptr = (struct lbb_bufrec *) malloc( sizeof(struct lbb_bufrec));
		if( bufptr == NULL ) {
		    return( ENOMEM );
		    }
		buf_recptr->next = NULL;
		buf_recptr->buf_addr = bufptr;
		buf_recptr->buflen = bufsize;
		buf_recptr->tblnum = bbl_idx;
		buf_recptr->tblsize = tablesize;
		if( bdblk_recptr->bufs == NULL ) {
		    bdblk_recptr->bufs = buf_recptr;
		    }
		else {
		    last_buf_recptr->next = buf_recptr;
		    }
		last_buf_recptr = buf_recptr;
		buffer_allocated = 0;
		}  /* end we got a list */
	    }  /* end successful so far and table not empty */	
	}  /* end for bbl_idx */

    if( buffer_allocated ) {
	free( (void *) bufptr );
	}
	
    return( glbbl_rc );
}				/* end get_LVM_BadBlockLists() */


/******************************************************************
 * NAME: 	tell_LVM_ClearLists
 *
 * FUNCTION: 	Notify the LVM that it can remove all entries from the bad
 *		block tables which form its bad block list for this partition.
 *
 * PARAMETERS:	none
 *
 * NOTES:		none
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 tell_LVM_ClearLists( int32 dev_handle )
{
    int32 tlcl_rc = 0;
    int32 lvidx;
    DDI_OS2LVM_param LVMpp;
    DDI_OS2LVM_param *pLVMpp = &LVMpp;
    DDI_OS2LVM_data LVMdp;
    DDI_OS2LVM_data *pLVMdp = &LVMdp;
    ULONG ppLen = 0;
    ULONG dpLen = 0;
    int32 bufsize = 1024;

	/*
	 * initialize the LVM DosDevIOCtl parm and data packets
	 */
    pLVMpp->Command = CLEAR_TBL;
    pLVMpp->DriveUnit = 0;
    pLVMdp->ReturnData = 0;
    pLVMdp->UserBuffer = NULL;

	/*
	 * Tell the LVM to clear its tables for this partition
	 */
    for( lvidx = 0;
	((lvidx < bdblk_recptr->num_tables) && (tlcl_rc == 0));
	lvidx++ ) {
		/*
		 * tell the LVM to forget all the entries on this list
		 */
	pLVMpp->TableNumber = (USHORT) lvidx;
	tlcl_rc = DosDevIOCtl( 	dev_handle,		IOC_DC,	
				IODC_LV,	 	(void *) pLVMpp,
				sizeof(DDI_OS2LVM_param),
				&ppLen,		 (void *) pLVMdp,
				sizeof(DDI_OS2LVM_data),	 &dpLen	
				);
	}  /* end for lvidx = ... */

    return( tlcl_rc );
}				/* end tell_LVM_ClearLists() */
