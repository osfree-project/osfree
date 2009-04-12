/* $Id: inodemap.c,v 1.2 2004/03/21 02:43:29 pasha Exp $ */

static char *SCCSID = "@(#)1.10  7/10/98 15:20:51 src/jfs/utils/format/inodemap.c, jfsformat, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		inodemap.c
 *
 *   COMPONENT_NAME: 	jfsformat
 *
 *   FUNCTIONS: init_AG_free_list
 *		init_inode_map
 *
 */
#define INCL_DOSERRORS
#include <os2.h>
#include "jfs_types.h"
#include <jfs_aixisms.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jfs_superblock.h"
#include "super.h"
#include "jfs_filsys.h"
#include "jfs_dinode.h"
#include "jfs_dmap.h"
#include "initmap.h"
#include "devices.h"
#include "inodes.h"
#include "jfs_imap.h"
#include "inodemap.h"
#include "message.h"


/*
 * NAME: init_AG_free_list
 *                                                                    
 * FUNCTION: Initialize AG dependent part of inode map control page
 *                                                                    
 * PARAMETERS:
 *	aggr_block_size	- block size for aggregate
 *	inode_table_loc	- block offset for first extent of inode allocation map,
 *			  this will be added to the table at the appropriate
 *			  location
 *	AGsize		- Number of logical blocks in an allocation group, used
 *			  to determine AG boundary for inode extent
 *	in_agctl	- Part of map control page to be initialized
 *	num_inode_init	- Number of inodes initialized for inode map
 *	iagnum		- IAG number of IAG with the allocated inodes
 *
 * RETURNS:
 *	success: 0
 *	failure: any other value
 */
static int32 init_AG_free_list( int32	aggr_block_size,
				int64	inode_table_loc,
				int32	AGsize,
				iagctl_t	*in_agctl,
				uint32	num_inode_init,
				int32	iagnum )
{
    int32	AG_num;
    int64	end_AG;
    int32	index;
    iagctl_t	empty_entry;

    /*
     * Figure out which AG the inode extent belongs in and initialize
     * AG free list entry to point to it
     *
     * All other entries will point to an empty entry
     */
    end_AG = AGsize;
    for( AG_num = 0; AG_num < MAXAG; AG_num++ ) {
	if( inode_table_loc < end_AG ) {
	    break;
	}
	end_AG += AGsize;
    }

    if( inode_table_loc >= end_AG ) {
	/*
	 * Broke out of for loop by hitting the maximum number of aggregates.
	 * Either the AGsize is wrong, or the inode_table_loc value is wrong.
	 */
	fprintf(stderr,
"Internal error: %s(%d): inode table location does not fall in correct range\n",
		__FILE__, __LINE__);
	fprintf(stderr, "\tAGsize = %d\tinode_table_loc = %d\tend_AG = %d\n",
		AGsize, inode_table_loc, end_AG );
	return ERROR_INVALID_ACCESS;
    }

    empty_entry.inofree = -1;
    empty_entry.extfree = -1;
    empty_entry.numinos = 0;
    empty_entry.numfree = 0;

    for( index = 0; index < MAXAG; index++ ) {
	if( index == AG_num ) {
	    /*
	     * This is where the initial inode extent lives
	     */
	    in_agctl[index].inofree = iagnum;
	    in_agctl[index].extfree = iagnum;
	    in_agctl[index].numinos = NUM_INODE_PER_EXTENT;
	    in_agctl[index].numfree = NUM_INODE_PER_EXTENT - num_inode_init;
	} else {
	    /*
	     * No inode extents allocated for this AG
	     */
	    memcpy( &(in_agctl[index]), &(empty_entry), sizeof(empty_entry) );
	}
    }

    return( 0 );
}


/*
 * NAME: init_inode_map
 *                                                                    
 * FUNCTION: Initialize first extent of inode allocation map
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	aggr_block_size		- block size for aggregate
 *	dev_ptr			- open port for device to write to
 *	inode_table_loc		- byte offset of first extent of inode table
 *	inode_table_size	- byte count of first extent of inode table
 *	inode_map_loc		- aggregate block offset of first extent of map
 *	inode_map_size		- byte count of first extent of inode map
 *	num_inode_init		- Number of initialized fileset inodes
 *	AGsize			- Size of allocation groups
 *	which_map		- Indicates either fileset or aggregate
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *	success: 0
 *	failure: any other value
 */
int32 init_inode_map( int32	aggr_block_size,
		      HFILE	dev_ptr,
		      int64	inode_table_loc,
		      int32	inode_table_size,
		      int64	inode_map_loc,
		      int32	inode_map_size,
		      uint16	num_inode_init,
		      int32	AGsize,
		      uint32	which_map )
{
    iag_t	*first_iag;
    dinomap_t	*control_page;
    void	*imap;
    int64	index, first_block, last_block;
    int32	rc = 0;
    uint32	*wmap_word;
    int32	*inosmap_ptr, *extsmap_ptr;
    int16	mapindex, bitindex;
    pxd_t	*inoext_ptr;
    int16	numbits;

    /*
     * Initialize space to hold control page for inode allocation map as well as
     * the initial page of inode allocation map
     */
    imap = calloc( 1, inode_map_size );
    if( imap == NULL ) {
      message_user(MSG_OSO_INSUFF_MEMORY, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
      return( ENOMEM );
      }

    /*
     * Initialize control page
     */
    control_page = imap;
    control_page->in_freeiag = -1;
    control_page->in_nextiag = 1;
    control_page->in_numinos = NUM_INODE_PER_EXTENT;
    control_page->in_numfree = NUM_INODE_PER_EXTENT - num_inode_init;
    control_page->in_nbperiext = INODE_EXTENT_SIZE / aggr_block_size;
    control_page->in_l2nbperiext = log2shift(control_page->in_nbperiext);

    /*
     * Initialize space for first extent of fileset inode map
     */
    first_iag = (iag_t *)((int)imap + SIZE_OF_MAP_PAGE); 

    /*
     * Initialize control section of first IAG of inode allocation map
     */
    first_iag->iagnum = 0;
    first_iag->agstart = ((inode_table_loc/aggr_block_size) / AGsize) * AGsize;
    first_iag->inofreefwd = -1;
    first_iag->inofreeback = -1;
    first_iag->extfreefwd = -1;
    first_iag->extfreeback = -1;
    first_iag->iagfree = -1;
    first_iag->nfreeinos = NUM_INODE_PER_EXTENT - num_inode_init;
    first_iag->nfreeexts = EXTSPERIAG - 1;

    /*
     * Initialize working and persistent maps of first IAG
     */
    if( which_map == AGGREGATE_I ) {
	first_iag->wmap[0] = first_iag->pmap[0] = 0xf8008000;
    } else {
	first_iag->wmap[0] = first_iag->pmap[0] = 0xf0000000;
    }

    /*
     * Initialize first inode extent of IAG
     */
    PXDlength(&(first_iag->inoext[0]), inode_table_size/aggr_block_size);
    PXDaddress(&(first_iag->inoext[0]), inode_table_loc/aggr_block_size);

    /*
     * Initialize extent summary map from the inode extents of the iag
     * Initialize inode summary map from working/permanent map of the iag
     */
    numbits = sizeof( first_iag->extsmap[0] ) << 3;
    for( inoext_ptr = &(first_iag->inoext[0]),
	 wmap_word = &(first_iag->wmap[0]), mapindex = 0;
	 mapindex < SMAPSZ; mapindex++ ) {
	extsmap_ptr = &(first_iag->extsmap[mapindex]);
	inosmap_ptr = &(first_iag->inosmap[mapindex]);
	for( bitindex = 0; bitindex < numbits;
	     bitindex++, inoext_ptr++, wmap_word++) {
	    *extsmap_ptr <<= 1;
	    *inosmap_ptr <<= 1;
	    if( addressPXD(inoext_ptr) != 0 ) {
		/*
		 * Extent allocated
		 */
		*extsmap_ptr |= 1;

		if( (*wmap_word ^ 0xffffffff) == 0 ) {
		    /*
		     * All allocated
		     */
		    *inosmap_ptr |= 1;
		}
	    } else {
		/*
		 * Extent is not allocated; want to mark the inode summary map
		 * as allocated to simplify the searching in the runtime
		 * filesystem code.  This allows just the inosmap to be searched
		 * when looking for a free backed inode.
		 */
		*inosmap_ptr |= 1;
	    }
	}
    }

    /*
     * Initialize the AG free list
     */
    rc = init_AG_free_list( aggr_block_size, inode_table_loc/aggr_block_size,
				AGsize, control_page->in_agctl, num_inode_init,
				first_iag->iagnum );
    if( rc != 0 ) return(rc);

    /*
     * Write control page and first extent of inode map to disk
     */
    rc = ujfs_rw_diskblocks( dev_ptr, inode_map_loc * aggr_block_size,
			     inode_map_size, imap, PUT);
    free( imap );
    if( rc != 0 ) return rc;

    /*
     * Mark blocks allocated in block allocation map
     */
    first_block = inode_map_loc;
    last_block = inode_map_loc + inode_map_size/aggr_block_size;
    for( index = first_block; ((index < last_block) && (rc==0)); index++ ) {
	rc = markit( index, ALLOC );
    }

    return( rc );
}
