/* $Id: validfs.c,v 1.1.1.1 2003/05/21 13:42:54 pasha Exp $ */

static char *SCCSID = "@(#)1.12  9/5/97 12:23:51 src/jfs/utils/validfs/validfs.c, jfsutil, w45.fs32, 990417.1";
/*
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
 *   COMPONENT_NAME: jfsutil
 *
 *   FUNCTIONS: main
 *		read_super
 *		validfs
 *		walk_ait
 *		walk_dir
 *		walk_iag_extent
 *		walk_inode_tree
 *		walk_inoext
 *
 */
#include <os2.h>
#include <jfs_types.h>
#include <jfs_aixisms.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "devices.h"
#include "jfs_filsys.h"
#include "jfs_superblock.h"
#include "jfs_dinode.h"
#include "initmap.h"
#include "inode.h"
#include "super.h"
#include "jfs_xtree.h"
#include "jfs_dtree.h"
#include "jfs_imap.h"
#include "jfs_dmap.h"
#include "dumpfs.h"

#define DISK_LIST	0
#define	FOUND_LIST	1
#define	DISK_AND_FOUND	2
struct list_item {
   int32	key;
   int32	status;
   struct list_item	*next; 
};

static struct superblock	sb;
static uint32	error;

/*
 * Forward references
 */
int32 walk_ait(int device, struct dinode *inode_buffer, boolean_t is_primary);

int32 search_and_add(struct list_item	**top_list,
		     int32		key,
		     int32		status)
{
    struct list_item	*new_item;
    struct list_item	*cur_item = *top_list;
    int32	rc;

    /*
     * If at end just return
     */
    if( key == -1 ) return 0;
    /*
     * We have an item on the iagfree list following this one.  First
     * lets see if it is already on our list.  If it is, and it is found
     * then we know it is okay and we can remove it from the list.  If
     * it is on the list, but not yet found, then we have a problem with
     * this list.  If it is not on the list yet we'll add it to the
     * list, but don't mark it as found yet.
     */
    /*
     * First search down the list looking for the specified item.
     */
    while( cur_item ) {
	if( cur_item->key == key ) {
	    /*
	     * Found a match
	     */
	    break;
	}
	cur_item = cur_item->next;
    }
    
    if( cur_item != NULL ) {
	/*
	 * We found an item on the list already
	 */
	switch( status ) {
	    case DISK_LIST:
		if( cur_item->status == FOUND_LIST ) {
		    /*
		     * Good news we have both determined this item should be on
		     * this list and we have seen it on the list.  Change the
		     * state to show this.
		     */
		    cur_item->status = DISK_AND_FOUND;
		    rc = 0;
		} else {
		    /*
		     * Bad news.  This item was already seen to be on the disk
		     * list.  It is shown twice.
		     */
		    error++;
		    rc = 1;
		}
		break;
	    case FOUND_LIST:
		if( cur_item->status == DISK_LIST ) {
		    /*
		     * Good news we have both determined this item should be on
		     * this list and we have seen it on the list.  Change the
		     * state to show this.
		     */
		    cur_item->status = DISK_AND_FOUND;
		    rc = 0;
		} else {
		    /*
		     * Bad news.  This item was already found.
		     */
		    error++;
		    rc = 1;
		}
		break;
	    case DISK_AND_FOUND:
	    default:
		/*
		 * Bad status passed in
		 */
		rc = 2;
		printf("Bad status for search\n");
	}
    } else {
	/*
	 * There was no item already on the list.  We'll add this one for later
	 * matching.
	 */
	new_item = malloc(sizeof(struct list_item));
	new_item->status = status;
	new_item->key = key;
	new_item->next = *top_list;
	*top_list = new_item;
	rc = 0;
    }
    return( rc );
}


int32 verify_list( struct list_item	*top_list,
		   char	*which_list )
{
    struct list_item	*cur_item = top_list;
    int32	rc = 0;

    while( cur_item != NULL ) {
	if( cur_item->status != DISK_AND_FOUND ) {
	    printf("Bad %s list item: %d\tstatus: %s\n", which_list,
			cur_item->key,
			(cur_item->status == DISK_LIST ) ? "on-disk only"
							: "not on disk" );
	    error++;
	    rc = 1;
	}
	cur_item = cur_item->next;
    }
    return rc;
}


void walk_dtree( int	device,
		 uint64	block,
		 uint32	length,
		 int64	*total_nblocks)
{
    int	rc;
    dtpage_t	dtree_buffer;
    uint8	*stbl;
    idtentry_t	*cur_entry;
    uint64	first_block, cur_block, last_block;
    uint32	cur_length;
    int32	lastindex, index;
    int32	thisindex;

    /*
     * Read the page from disk
     */
    rc = ujfs_rw_diskblocks( device, block << sb.s_l2bsize,
		length << sb.s_l2bsize, &dtree_buffer, GET );

    if( dtree_buffer.header.flag & BT_LEAF ) {
	/*
	 * Nothing to do, since the data here is not pointing to blocks
	 */
	return;
    }

    /*
     * Mark blocks for each entry and visit that page
     */
    lastindex = dtree_buffer.header.nextindex;
    stbl = (uint8 *)&(dtree_buffer.slot[dtree_buffer.header.stblindex]);
    for( index = 0; index < lastindex; index++ ) {
	/*
	 * This is an internal page of the d-tree.  Mark these blocks and
	 * then walk that page
	 */
	thisindex = stbl[index];
	cur_entry = (idtentry_t *)&(dtree_buffer.slot[thisindex]);
	first_block = addressPXD( &(cur_entry->xd) );
	cur_length = lengthPXD( &(cur_entry->xd) );
	*total_nblocks += cur_length;
	last_block = first_block + cur_length;
	for( cur_block = first_block; cur_block < last_block; cur_block++ ) {
	    markit( cur_block, 0 );
	}

	walk_dtree( device, first_block, cur_length, total_nblocks);
    }
}


void walk_dir(	int	device,
		dtroot_t	*root_header,
		int64	*total_nblocks)
{
    int32	index, lastindex;
    uint64	first_block, cur_block, last_block;
    idtentry_t	*cur_entry;	
    uint32	length;

    if( root_header->header.flag & BT_LEAF ) {
	/*
	 * Nothing to do, since the data here is not pointing to blocks
	 */
	return;
    }

    /*
     * Have root of directory inode btree.
     * Walk tree marking all blocks allocated.
     */
    lastindex = root_header->header.nextindex;
    for( index = 0; index < lastindex; index++ ) {
	/*
	 * This is an internal page of the d-tree.  Mark these blocks and
	 * then walk that page
	 */
	cur_entry =
	    (idtentry_t *)&(root_header->slot[root_header->header.stbl[index]]);
	first_block = addressPXD( &(cur_entry->xd) );
	length = lengthPXD( &(cur_entry->xd) );
	*total_nblocks += length;
	last_block = first_block + length;
	for( cur_block = first_block; cur_block < last_block; cur_block++ ) {
	    markit( cur_block, 0 );
	}

	walk_dtree( device, first_block, length, total_nblocks);
    }
}


void walk_internal_page(int	device,
			uint64	block,
			uint32	length,
			int64	*total_nblocks,
			int32	flag)
{
    int	rc;
    xtpage_t	xtree_page;
    int32	lastindex, index;
    uint64	first_block, cur_block, last_block;
    uint32	cur_length;
    int32	leafbad;

    /*
     * Read the internal page
     */
    rc = ujfs_rw_diskblocks( device, block << sb.s_l2bsize,
		length << sb.s_l2bsize, &xtree_page, GET );

    /*
     * Mark the blocks for the page; if internal page walk down page
     */
    leafbad = (xtree_page.header.flag & BT_LEAF) ? flag : 0;
    lastindex = xtree_page.header.nextindex;
    for( index = XTENTRYSTART; index < lastindex; index++ ) {
	/*
	 * This is actual data of the inode, mark these blocks
	 */
	first_block = addressXAD( &(xtree_page.xad[index]) );
	cur_length = lengthXAD( &(xtree_page.xad[index]) );
	*total_nblocks += cur_length;
	last_block = first_block + cur_length;
	for( cur_block = first_block; cur_block < last_block; cur_block++ ) {
	    markit( cur_block, leafbad );
	}

	if( xtree_page.header.flag & BT_INTERNAL ) {
	    /*
	     * This is an internal page of the b-tree.  Walk the page.
	     */
	    walk_internal_page(device, first_block, cur_length, total_nblocks,
			flag);
	}
    }
}


void walk_inode_tree(int	device,
		     xtpage_t	*inode_root,
		     int64	*total_nblocks,
		     int32	flag)
{
    int32	index, lastindex;
    uint64	first_block, cur_block, last_block;
    uint32	length;
    int32	leafbad;

    /*
     * Have root of inode btree. Walk tree marking all blocks allocated.
     */
    leafbad = (inode_root->header.flag & BT_LEAF) ? flag : 0;
    lastindex = inode_root->header.nextindex;
    for( index = XTENTRYSTART; index < lastindex; index++ ) {
	/*
	 * This is actual data of the inode, mark these blocks
	 */
	first_block = addressXAD( &(inode_root->xad[index]) );
	length = lengthXAD( &(inode_root->xad[index]) );
	*total_nblocks += length;
	last_block = first_block + length;
	for( cur_block = first_block; cur_block < last_block; cur_block++ ) {
	    markit( cur_block, leafbad );
	}

	if( inode_root->header.flag & BT_INTERNAL ) {
	    /*
	     * This is an internal page of the b-tree.  Walk the page.
	     */
	    walk_internal_page(device, first_block, length, total_nblocks,
			flag);
	}
    }
}


void walk_inoext( int		device,
		  uint64	address,
		  uint32	length,
		  uint32	wmap,
		  dinomap_t	*control_page,
		  uint32	agno,
		  ino_t		start_inum,
		  uint32	*found_map,
		  int32		inostamp )
{
    dinode_t	*next_inode;
    uint32	left_to_read = length << sb.s_l2bsize;
    uint64	cur_address = address << sb.s_l2bsize;
    int32	index, rc;
    uint32	map = wmap;
    char	page_buffer[PSIZE];
    ino_t	cur_inum = start_inum;
    uint32	mask = HIGHORDER;
    uint64	first_block, cur_block, last_block;
    uint32	num_blocks;
    int64	total_nblocks;

    *found_map = 0;

    while( (left_to_read > 0) ) {
	/*
	 * Read next page of inodes for this extent
	 */
	rc = ujfs_rw_diskblocks( device, cur_address, PSIZE, page_buffer, GET );
	cur_address += PSIZE;
	left_to_read -= PSIZE;

	next_inode = (dinode_t *)page_buffer;
	for( index = 0; index < INOSPERPAGE;
	     index++, next_inode++, cur_inum++, map <<= 1, mask >>= 1 ) {
	    /*
	     * Initialize count for this inode's number of blocks
	     */
	    total_nblocks = 0;

	    /*
	     * If this inode is allocated, mark blocks for its b-tree
	     */
	    if( (map & HIGHORDER) != 0 ) {
		if( next_inode->di_nlink <= 0 ) {
		    error++;
		    printf("Inode %d (fileset: %d) link count bad: %d\n",
			next_inode->di_number, next_inode->di_fileset,
			next_inode->di_nlink);
		} else {
		    *found_map |= mask;
		}

		/*
		 * Account for any blocks used by EA for this inode
		 */
		if( next_inode->di_ea.flag & DXD_EXTENT ) {
		    first_block = addressDXD(&(next_inode->di_ea));
		    num_blocks = lengthDXD(&(next_inode->di_ea));
		    total_nblocks += num_blocks;
		    last_block = first_block + num_blocks;
		    for( cur_block = first_block; cur_block < last_block;
				cur_block++ ) {
			markit( cur_block, 0 );
		    }
		}

		if( (next_inode->di_fileset == AGGREGATE_I) &&
			(next_inode->di_number == FILESYSTEM_I) ) {
		    /*
		     * Need to account for inode map's blocks
		     */
		    walk_inode_tree(device, (xtpage_t *)&next_inode->di_btroot,
					&total_nblocks, 0);

		    /*
		     * Need to walk this tree of inodes
		     */
		    rc = walk_ait( device, next_inode, TRUE );
		    if( rc != 0 ) {
			error++;
			printf("Problem with Fileset Inode Allocation Map.\n");
		    }
		} else if( (next_inode->di_fileset == AGGREGATE_I) &&
			(next_inode->di_number == BADBLOCK_I) ) {
		    walk_inode_tree(device, (xtpage_t *)&next_inode->di_btroot,
					&total_nblocks, BADBLOCK);
		} else if( next_inode->di_mode & IFDIR ) {
		    /*
		     * Need to walk the extents as directory extents
		     */
		    walk_dir( device, (dtroot_t *)&(next_inode->di_btroot),
				&total_nblocks);
		} else {
		    walk_inode_tree(device, (xtpage_t *)&next_inode->di_btroot,
					&total_nblocks, 0);
		}

		/*
		 * Now total_nblocks contains the total number of blocks
		 * actually allocated for this inode.  Compare this to the
		 * on-disk information.
		 */
		if( next_inode->di_nblocks != total_nblocks ) {
		    error++;
		    printf(
	       "Inode %d (fileset: %d) nblocks bad, disk: %lld, actual: %lld\n",
			next_inode->di_number, next_inode->di_fileset,
			next_inode->di_nblocks, total_nblocks);
		}
	    } else {
		if( next_inode->di_number == cur_inum &&
			next_inode->di_inostamp == inostamp &&
			addressPXD(&(next_inode->di_ixpxd)) == address &&
			lengthPXD(&(next_inode->di_ixpxd)) == length &&
			next_inode->di_nlink > 0 ) {
		    error++;
		    printf("Inode %d (fileset: %d) link count bad: %d\n",
				next_inode->di_number, next_inode->di_fileset,
				next_inode->di_nlink);
		    *found_map |= mask;
		}
		control_page->in_numfree++;
		control_page->in_agctl[agno].numfree++;
	    }
	}
    }
}


/*
 * Read the specified extent as an extent of IAG's
 * If its offset is 0 skip the first page since this is a control page.
 * For all other IAGs need to mark blocks:
 *	- mark the blocks for any allocated extents for the IAG
 *	- read the extent and mark blocks for any allocated inodes
 * Note: the blocks owned by the table itself will be marked when the inode for
 *	 the table is seen.
 */
void walk_iag_extent( int	device,
		      xad_t	*extent,
		      boolean_t	is_primary,
		      dinomap_t	*control_page,
		      dinomap_t	*disk_cp,
		      struct list_item	**top_iagfree,
		      struct list_item	**top_inofree,
		      struct list_item	**top_extfree,
		      int32	inostamp )
{
    uint64	offset, address, count, end;
    uint32	length, page_length;
    iag_t	iag_buffer;
    int32	index, rc, extdx;
    pxd_t	*inoext_ptr;
    uint32	map, found_map;
    uint32	agno;
    ino_t	start_inum;
    uint32	mymap[EXTSPERIAG];
    int16	seen_extent = 0, free_inodes = 0;

    offset = offsetXAD( extent );
    address = addressXAD( extent );
    length = lengthXAD( extent );
    page_length = PSIZE >> sb.s_l2bsize;

    if( offset == 0 ) {
	/*
	 * Read in the disk control page now.  We will compare it after all the
	 * other pages of the map have been processed.
	 */
	rc = ujfs_rw_diskblocks( device, address << sb.s_l2bsize,
				 sizeof(dinomap_t), disk_cp, GET );
	if( rc != 0 ) exit(rc);

	address += page_length;
	length -= page_length;
    }

    while( length > 0 ) {
	/*
	 * Clear map to use for tracking inodes seen
	 */
	memset( mymap, 0, EXTSPERIAG * sizeof(uint32));

	/*
	 * Read next IAG
	 */
	rc = ujfs_rw_diskblocks( device, address << sb.s_l2bsize, PSIZE,
				 &iag_buffer, GET );
	if( rc != 0 ) exit(rc);
	length -= page_length;
	address += page_length;

	control_page->in_nextiag = iag_buffer.iagnum + 1;
	if( iag_buffer.iagfree != -1 ) {
	    /*
	     * We have an item on the iagfree list following this one.
	     */
	    rc = search_and_add(top_iagfree, iag_buffer.iagfree, DISK_LIST);
	    if( rc != 0 ) {
		printf("Bad iagfree item on-disk: %d\n", iag_buffer.iagfree);
	    }
	}

	agno = iag_buffer.agstart / sb.s_agsize;
	if( iag_buffer.extfreefwd != -1 ) {
	    /*
	     * We have an item on the extfree list following this one.
	     */
	    rc = search_and_add(&(top_extfree[agno]), iag_buffer.extfreefwd,
			DISK_LIST);
	    if( rc != 0 ) {
		printf("Bad extfree[%d] item on-disk: %d\n", agno,
			iag_buffer.extfreefwd);
	    }
	}

	if( iag_buffer.inofreefwd != -1 ) {
	    /*
	     * We have an item on the inofree list following this one.
	     */
	    rc = search_and_add(&(top_inofree[agno]), iag_buffer.inofreefwd,
			DISK_LIST);
	    if( rc != 0 ) {
		printf("Bad inofree[%d] item on-disk: %d\n", agno,
			iag_buffer.inofreefwd);
	    }
	}

	/*
	 * Mark blocks for any allocated inode extents
	 */
	for( index = 0; index < SMAPSZ; index++ ) {
	    map = iag_buffer.extsmap[index];
	    inoext_ptr = iag_buffer.inoext + (index * EXTSPERSUM);
	    for( extdx = 0; extdx < EXTSPERSUM, map != 0; extdx++, map <<= 1) {
		if( (map & HIGHORDER) != 0 ) {
		    seen_extent++;

		    /*
		     * Count inodes for allocated inode extents
		     */
		    control_page->in_numinos += NUM_INODE_PER_EXTENT;
		    control_page->in_agctl[agno].numinos +=
					NUM_INODE_PER_EXTENT;

		    address = count = addressPXD(inoext_ptr + extdx);
		    end = count + inoext_ptr[extdx].len;
		    for( ; count < end; count++) {
			markit( count, 0 );
		    }

		    if( is_primary == TRUE ) {
			/*
			 * Now need to read inodes and mark blocks for them
			 * Only do this for the primary inode table
			 */
			start_inum = (iag_buffer.iagnum << L2INOSPERIAG) +
				(index << (L2EXTSPERSUM + L2INOSPEREXT)) +
				(extdx << L2INOSPEREXT);
			walk_inoext( device, address, inoext_ptr[extdx].len,
				iag_buffer.wmap[(index * EXTSPERSUM) + extdx],
				control_page, agno, start_inum, &found_map,
				inostamp );
			mymap[(index * EXTSPERSUM) + extdx] = found_map;
			if( ~found_map != 0 ) free_inodes = 1;
		    }
		}
	    }
	}

	if( seen_extent == 0 ) {
	    /*
	     * No extents for this IAG, add it to iagfree list
	     */
	    rc = search_and_add(top_iagfree, iag_buffer.iagnum, FOUND_LIST);
	    if( rc != 0 ) {
		printf("Bad iagfree item found: %d\n", iag_buffer.iagnum);
	    }
	} else if( seen_extent != EXTSPERIAG ) {
	    /*
	     * Have free extents in this IAG, add to AG free extent list
	     */
	    rc = search_and_add(&(top_extfree[agno]), iag_buffer.iagnum,
			FOUND_LIST);
	    if( rc != 0 ) {
		printf("Bad extfree[%d] item found: %d\n", agno,
			iag_buffer.iagnum);
	    }
	}
	if( free_inodes != 0 ) {
	    /*
	     * We have some free inodes in the extent
	     */
	    rc = search_and_add(&(top_inofree[agno]), iag_buffer.iagnum,
			FOUND_LIST);
	    if( rc != 0 ) {
		printf("Bad inofree[%d] item found: %d\n", agno,
			iag_buffer.iagnum);
	    }
	}

	if( is_primary ) {
	    /*
	     * Compare map found by walking extents to the on-disk version
	     */
	    rc = memcmp( mymap, iag_buffer.wmap, EXTSPERIAG * sizeof(uint32));
	    if( rc != 0 ) {
		error++;
		printf("Miscompare of inode wmap of IAG %d.\n",
			iag_buffer.iagnum);
		print_uint_array ("Found map:", mymap, EXTSPERIAG);
		print_uint_array ("Disk wmap:", iag_buffer.wmap, EXTSPERIAG);
	    }

	    rc = memcmp( mymap, iag_buffer.pmap, EXTSPERIAG * sizeof(uint32));
	    if( rc != 0 ) {
		error++;
		printf("Miscompare of inode pmap of IAG %d.\n",
			iag_buffer.iagnum);
		print_uint_array ("Found map:", mymap, EXTSPERIAG);
		print_uint_array ("Disk pmap:", iag_buffer.pmap, EXTSPERIAG);
	    }
	}
    }
}


void walk_internal_iag( int		device,
			xad_t		*top_page,
			boolean_t	is_primary,
			dinomap_t	*control_page,
			dinomap_t	*disk_cp,
			struct list_item	**top_iagfree,
			struct list_item	**top_inofree,
			struct list_item	**top_extfree,
			int32		inostamp )
{
    int32	rc, index, lastindex;
    uint64	block;
    uint32	length;
    xtpage_t	xtree_page;

    block = addressXAD(top_page);
    length = lengthXAD(top_page);

    rc = ujfs_rw_diskblocks( device, block << sb.s_l2bsize,
		length << sb.s_l2bsize, &xtree_page, GET );

    /*
     * Walk the IAG page unless this is another internal page, then we need to
     * walk it as an internal page again.
     */
    lastindex = xtree_page.header.nextindex;
    for( index = XTENTRYSTART; index < lastindex; index++ ) {
	if( xtree_page.header.flag & BT_LEAF ) {
	    walk_iag_extent( device, &(xtree_page.xad[index]), is_primary,
				control_page, disk_cp, top_iagfree,
				top_inofree, top_extfree, inostamp );
	} else {
	    walk_internal_iag( device, &(xtree_page.xad[index]), is_primary,
				control_page, disk_cp, top_iagfree,
				top_inofree, top_extfree, inostamp );
	}
    }
}


/*
 * Walk aggregate inode table marking blocks:
 *	- For inode map structures
 *	- For inode extents belonging to this table
 *	- For blocks belonging to inode's belonging to this table
 */
int32 walk_ait( int	device,
		struct dinode	*inode_buffer,
		boolean_t	is_primary )
{
    int32	rc = 0;
    xtpage_t	*btree_page;
    int32	index, lastindex;
    uint64	cur_block, last_block;
    dinomap_t	control_page, disk_cp;
    struct list_item	*top_freeiag = NULL;
    struct list_item	*top_inofree[MAXAG];
    struct list_item	*top_extfree[MAXAG];

    /*
     * Initialize control page and lists
     */
    memset( &control_page, 0, sizeof(dinomap_t) );
    memset( &top_inofree, 0, MAXAG * sizeof(struct list_item*) );
    memset( &top_extfree, 0, MAXAG * sizeof(struct list_item*) );

    btree_page = (xtpage_t *)&inode_buffer->di_btroot;

    /*
     * Mark blocks for the inode map structures (i.e. IAG's)
     */
    lastindex = btree_page->header.nextindex;
    for( index = XTENTRYSTART; index < lastindex; index++ ) {
	if( btree_page->header.flag & BT_LEAF ) {
	    /*
	     * Read the extent as IAG's and mark its blocks
	     */
	    walk_iag_extent( device, &(btree_page->xad[index]), is_primary,
				&control_page, &disk_cp, &top_freeiag,
				top_inofree, top_extfree,
				inode_buffer->di_inostamp );

	} else {
	    /*
	     * This is an internal page of the b-tree.  Mark these blocks and
	     * then walk that page
	     */
	    walk_internal_iag( device, &(btree_page->xad[index]), is_primary,
				&control_page, &disk_cp, &top_freeiag,
				top_inofree, top_extfree,
				inode_buffer->di_inostamp );
	}
    }

    /*
     * Now compare the computed control page to the on-disk one.
     * Right now we are only checking inode number counts, so we can't do a
     * straight memory comparison
     */
    if( is_primary ) {
#if 0
	rc = memcmp( &control_page, &disk_cp, sizeof(dinomap_t));
	if( rc != 0 ) {
	    error++;
	    printf("Miscompare of control page.\n");
	}
#else
	if( control_page.in_numinos != disk_cp.in_numinos) {
	    error++;
	    printf("Miscompare on in_numinos; found: %d\tdisk: %d\n",
			control_page.in_numinos, disk_cp.in_numinos );
	}
	if( control_page.in_numfree != disk_cp.in_numfree) {
	    error++;
	    printf("Miscompare on in_numfree; found: %d\tdisk: %d\n",
			control_page.in_numfree, disk_cp.in_numfree );
	}
	for( index = 0; index < MAXAG; index++ ) {
	    if( control_page.in_agctl[index].numinos !=
		disk_cp.in_agctl[index].numinos) {
		error++;
		printf("Miscompare on AG (%d) numinos; found: %d\tdisk: %d\n",
			index, control_page.in_agctl[index].numinos,
			disk_cp.in_agctl[index].numinos );
	    }
	    if( control_page.in_agctl[index].numfree !=
		disk_cp.in_agctl[index].numfree) {
		error++;
		printf("Miscompare on AG (%d) numfree; found: %d\tdisk: %d\n",
			index, control_page.in_agctl[index].numfree,
			disk_cp.in_agctl[index].numfree );
	    }
	}
	if( control_page.in_nextiag != disk_cp.in_nextiag) {
	    error++;
	    printf("Miscompare on in_nextiag; found: %d\tdisk: %d\n",
			control_page.in_nextiag, disk_cp.in_nextiag );
	}
#endif

	/*
	 * Add the on-disk start of the lists from the control page
	 */
	rc = search_and_add(&top_freeiag, disk_cp.in_freeiag, DISK_LIST);
	if( rc != 0 ) {
	    error++;
	    printf("Bad iagfree item on-disk: %d\n", disk_cp.in_freeiag);
	}
	for( index = 0; index < MAXAG; index++ ) {
	    rc = search_and_add(&(top_extfree[index]),
			disk_cp.in_agctl[index].extfree, DISK_LIST);
	    if( rc != 0 ) {
		error++;
		printf("Bad extfree[%d] item on-disk: %d\n", index,
			disk_cp.in_agctl[index].extfree);
	    }
	    rc = search_and_add(&(top_inofree[index]),
			disk_cp.in_agctl[index].inofree, DISK_LIST);
	    if( rc != 0 ) {
		error++;
		printf("Bad inofree[%d] item on-disk: %d\n", index,
			disk_cp.in_agctl[index].inofree);
	    }
	}

	/*
	 * Verify the lists
	 */
	verify_list(top_freeiag, "IAG free");
	for( index = 0; index < MAXAG; index++ ) {
	    verify_list(top_extfree[index], "AG Extent free");
	    verify_list(top_inofree[index], "AG Inode free");
	}
    }

    return 0;
}


int32 read_super( int device )
{
    int32 rc;

    rc = ujfs_get_superblk( device, &sb, TRUE );
    if( rc != 0 ) {
	printf("Failed reading primary superblock, will try secondary.\n");

	rc = ujfs_get_superblk( device, &sb, FALSE );
	if( rc != 0 ) {
	    printf("Failed reading secondary superblock.\n");
	    return(rc);
	}
    }

    rc = ujfs_validate_super( &sb );

    if( rc != 0 ) {
	printf("Not a recognized XJFS filesystem.\n");
    }
    return(rc);
}


int32 validfs( HFILE device )
{
    uint64	num_log_blocks;
    int32	rc;
    uint64	first_block, last_block;
    uint32	length, inode_address;
    uint64	index;
    struct dinode	inode_buffer;
    int64	total_nblocks;

    /*
     * Initialize internal block map
     */
    num_log_blocks = sb.s_size >> sb.s_l2bfactor;

    rc = calc_map_size( num_log_blocks, sb.s_bsize, sb.s_agsize );
    if( rc != 0 ) {
	printf("Failure creating internal block map.\n");
    }

    /*
     * Mark fixed items allocated; these are only the items which aren't mapped
     * by one of the inode tables.
     */
    /*
     * Reserved blocks
     */
    length = AGGR_RSVD_BYTES >> sb.s_l2bsize;
    first_block = 0;
    last_block = first_block + length;
    for( index = first_block; index < last_block; index++ ) {
	markit( index, 0 );
    }

    /*
     * Primary superblock
     */
    length = SIZE_OF_SUPER >> sb.s_l2bsize;
    first_block = SUPER1_OFF >> sb.s_l2bsize;
    last_block = first_block + length;
    for( index = first_block; index < last_block; index++ ) {
	markit( index, 0 );
    }

    /*
     * Secondary superblock
     */
    first_block = SUPER2_OFF >> sb.s_l2bsize;
    last_block = first_block + length;
    for( index = first_block; index < last_block; index++ ) {
	markit( index, 0 );
    }

    /*
     * Walk aggregate inode table; marking blocks seen
     */
    rc = ujfs_rwinode( device, &inode_buffer, AGGREGATE_I, GET, sb.s_bsize,
			AGGREGATE_I );
    if( rc != 0 ) return(rc);

    rc = walk_ait( device, &inode_buffer, TRUE );
    if( rc != 0 ) {
	printf(
	"Failed walking aggregate inode table, or difference in inode maps.\n");
    }

    /*
     * Walk secondary aggregate inode table; marking blocks seen
     */
    inode_address = (AGGREGATE_I * sizeof(struct dinode)) +
			( addressPXD(&(sb.s_ait2)) << sb.s_l2bsize );
    rc = ujfs_rw_diskblocks( device, inode_address, sizeof(struct dinode),
				&inode_buffer, GET );
    if( rc != 0 ) return(rc);

    rc = walk_ait( device, &inode_buffer, FALSE );
    if( rc != 0 ) {
	printf(
	"Failed walking secondary inode table, or difference in inode maps.\n");
    }

    /*
     * Since we don't walk the inodes of the secondary inode table we need to
     * be sure and mark the blocks for the map's addressing structure
     */
    total_nblocks = 0;
    walk_inode_tree(device, (xtpage_t *)&inode_buffer.di_btroot,
			&total_nblocks, 0);
    if( inode_buffer.di_nblocks != total_nblocks )
    {
	error++;
	printf(
 "Secondary AIT Inode %d (fileset: %d) nblocks bad, disk: %lld, actual: %lld\n",
		inode_buffer.di_number, inode_buffer.di_fileset,
		inode_buffer.di_nblocks, total_nblocks);
    }

    /*
     * Now the bitmaps are marked, fill in the rest of the maps and compare
     * with the maps on disk
     */
    rc = compare_maps( device, num_log_blocks, sb.s_bsize );

    return(rc);
}


int main( int argc,
	  char **argv)
{
    int rc = 0;
    int	c;
    int	debug_wait;
    HFILE	device_handle;
    int32	phys_block_size;

    if( argc != 2 ) {
	printf("usage: validfs <fs device> ...\n");
	exit(1);
    }

    rc = ujfs_open_device(argv[1], &device_handle, &phys_block_size, READONLY);
    if( rc != 0 ) {
	printf("open of device %s failed.\n", argv[1] );
	exit(1);
    }

    rc = read_super( device_handle );
    if( rc != 0 ) {
	printf("Failed reading superblock.\n");
	exit(rc);
    }

    rc = validfs( device_handle );

    if( rc == 0 && error == 0 ) {
	printf("XJFS filesystem %s looks GOOD!\n", argv[1]);
    }

    ujfs_close(device_handle);
    exit(rc);
}
