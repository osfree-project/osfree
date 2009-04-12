/* $Id: initmap.c,v 1.2 2004/03/21 02:43:21 pasha Exp $ */

static char *SCCSID = "@(#)1.7  9/5/97 12:23:43 src/jfs/utils/validfs/initmap.c, jfsutil, w45.fs32, 990417.1";
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
 *   COMPONENT_NAME: jfsutil
 *
 *   FUNCTIONS:	alloc_map
 *		calc_map_size
 *		compare_maps
 *		initbmap
 *		initctl
 *		initdmap
 *		initmap
 *		markit
 *
 */
#define INCL_DOSERRORS
#include <os2.h>
#include <jfs_types.h>
#include <jfs_aixisms.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "jfs_filsys.h"
#include "jfs_dinode.h"
#include "inode.h"
#include "jfs_dmap.h"
#include "super.h"
#include "jfs_xtree.h"
#include "dumpfs.h"
#include "diskmap.h"
#include "devices.h"
#include "initmap.h"

#ifdef _JFS_OS2
#define bzero(string, count)	memset(string, 0, count)
#endif

#define UZWORD	(0x80000000u)
#define DBBYTE		8	/* number of bits per byte */
#define L2DBBYTE	3	/* log2 of number of bits per byte */
#define	CHAR_ONES	(0xffu)

static char	page_buffer[PSIZE];
static uint32	l2nbperpage;

static struct dinode	block_map_inode;
static dmap_t		*block_map;
static dmap_t		*cur_dmap;
static dbmap_t		*control_page;
static uint32		last_allocated;


/*
 * NAME: initdmap
 *                                                                    
 * FUNCTION: Initialize a dmap for the specified block range (blkno thru
 *	blkno+nblocks-1).
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	dev_ptr	- 
 *	blkno	- Starting disk block number to be covered by this dmap.
 *	nblocks	- Number of blocks covered by this dmap.
 *	treemax	- Return value set as the maximum free string found in this dmap
 *	start	- Logical block address of where this dmap should live on disk.
 *
 * NOTES: The wmap and pmap words along the leaves of the dmap tree are
 *	initialized, with the leaves initialized to the maximum free string of
 *	the wmap word they describe.  With this complete ujfs_adjtree() is
 *	called to combine all appropriate buddies and update the higher level of
 *	the tree to reflect the result of the buddy combination.  The maximum
 *	free string of the dmap (i.e. the root value of the tree) is returned in
 *	treemax.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 */
static int32 initdmap(	int		dev_ptr,
			int64		blkno,
			int64		nblocks,
			signed char	*treemax,
			int32		aggr_block_size )
{
    int8	max;
    dmap_t	*dp, *disk_dmap;
    int		rc;
    int32	this_page;
    int64	page_address;

    /*
     * Get the disk address for this dmap page
     */
    this_page = BLKTODMAP(blkno, l2nbperpage);

    rc = ujfs_rwdaddr( dev_ptr, &page_address, &block_map_inode, this_page, GET,
		aggr_block_size );

    /*
     * Determine which is the next dmap
     */
    dp = cur_dmap;

    ujfs_complete_dmap( dp, blkno, &max );

    /*
     * Read the dmap and compare
     */
    disk_dmap = (dmap_t *)page_buffer;
    rc = ujfs_rw_diskblocks( dev_ptr, page_address, PSIZE, disk_dmap, GET );

    rc = memcmp( cur_dmap, disk_dmap, sizeof( dmap_t ) );
    if( rc != 0 ) {
	printf("Block map level 0 logical offset %d  different.\n", this_page );
	printf("Disk map:\n");
	display_dmap(this_page, disk_dmap);
	printf("Actual map:\n");
	display_dmap(this_page, cur_dmap);
	rc = 1;
    }

    cur_dmap++;
    /*
     * Set the treemax return value with the maximum free described by this dmap
     */
    *treemax = max;

    return(rc);
}


/*
 * NAME: initctl
 *                                                                    
 * FUNCTION: Initialize a dmapctl for the specified block range (blkno thru
 *	blkno+nblocks-1) and level and initialize all dmapctls and dmaps under
 *	this dmapctl.
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	dev_ptr	- 
 *	blkno	- Starting disk block number to be covered by this dmapctl.
 *	nblocks	- Number of blocks covered by this dmapctl.
 *	level	- The level of this dmapctl.
 *	treemax	- Return value set as the maximum free string found in this
 *		  dmapctl.
 *	start	- Logical block address of where this page should live on disk.
 *
 * NOTES: This routine is called recursively. On first invocation it is called
 *	for the top level dmapctl of the tree.  For each leaf of the dmapctl,
 *	the lower level dmap (level == 0) or dmapctl (level > 0) is created for
 *	the block range covered by the leaf and the leaf is set to the maximum
 *	free string found in the lower level object.  If the lower level object
 *	is a dmap, initdmap() is called to handle it's initialization.
 *	Otherwise, initctl() is called recursively to initialize the lower level
 *	dmapctl with the level specified as the current level - 1; once all
 *	leaves have been initialized ujfs_adjtree() is called to combine all
 *	appropriate buddies and update the higher level of the tree to reflect
 *	the result of the buddy combination.  The maximum free string of the
 *	dmapctl (i.e. the root value of the tree) is returned in treemax.
 *
 * DATA STRUCTURES:
 *
 * RETURNS: None.
 */
static int32 initctl( int		dev_ptr,
		      int64		blkno,
		      int64		nblocks,
		      int32		level,
		      signed char	*treemax,
		      uint32		aggr_block_size )
{
    int		index, rc = 0, l2cblks, nchild, error;
    signed char	*cp, max;
    dmapctl_t	*dcp, *disk_dcp;
    int64	nb, cblks;
    int64	page_address, this_page;

    /*
     * Get the disk address for this control page
     */
    this_page = BLKTOCTL(blkno, l2nbperpage, level);

    error = ujfs_rwdaddr( dev_ptr, &page_address, &block_map_inode, this_page,
		GET, aggr_block_size );

    dcp = malloc( sizeof(dmapctl_t) );

    /*
     * Initialize the dmapctl to zeros.
     */
    bzero( dcp, sizeof(dmapctl_t) );

    dcp->height = 5;
    dcp->leafidx = CTLLEAFIND;
    dcp->nleafs = LPERCTL;
    dcp->l2nleafs = L2LPERCTL;
    dcp->budmin = L2BPERDMAP + level * L2LPERCTL;

    /*
     * Pick up the pointer to the first leaf of the dmapctl tree.
     */
    cp = dcp->stree + dcp->leafidx;

    l2cblks = L2BPERDMAP + level * L2LPERCTL;
    cblks = (1 << l2cblks);

    /*
     * Determine how many lower level dmapctls or dmaps will be described by
     * this dmapctl based upon the number of blocks covered by this dmapctl.
     */
    nchild = nblocks >> l2cblks;
    nchild = (nblocks & (cblks-1)) ? nchild+1 : nchild;

    for( index = 0; index < nchild; index++, nblocks -= nb, blkno += nb ) {
    	/*
	 * Determine how many blocks the lower level dmapctl or dmap will cover.
    	 */
    	nb = MIN( cblks, nblocks );

    	/*
	 * If this is a level 0 dmapctl, initialize the dmap for the block range
	 * (i.e. blkno thru blkno+nb-1).  Otherwise, initialize the lower level
	 * dmapctl for this block range.  in either case, the pointer to the
	 * leaf covering this block range is passed down and will be set to the
	 * length of the maximum free string of blocks found at the lower level.
    	 */
    	if( level == 0 ) {
    		rc += initdmap( dev_ptr, blkno, nb, cp+index, aggr_block_size );
	} else {
    		rc += initctl(dev_ptr, blkno, nb, level-1, cp+index, 
			aggr_block_size);
	}
    }

    /*
     * Initialize the leaves for this dmapctl that were not covered by the
     * specified input block range (i.e. the leaves have no low level dmapctl or
     * dmap.
     */
    for( ; index < LPERCTL; index++ ) {
    	*(cp+index) = NOFREE;
    }

    /*
     * With the leaves initialized, adjust the tree for this dmapctl.
     */
    max = ujfs_adjtree( dcp->stree, L2LPERCTL, l2cblks );

    /*
     * Read this page on disk and compare
     */
    disk_dcp = (dmapctl_t *)page_buffer;
    error = ujfs_rw_diskblocks( dev_ptr, page_address, PSIZE, disk_dcp, GET);

    error = memcmp( dcp, disk_dcp, sizeof(dmapctl_t) );
    if( error != 0 ) {
	printf("Block map level %d logical offset %d different.\n", level,
		this_page );
	printf("Disk map:\n");
	display_level(level, this_page, disk_dcp);
	printf("Actual map:\n");
	display_level(level, this_page, dcp);
	error = 1;
    }
    free( dcp );

    /*
     * Set the treemax return value with the maximum free described by this
     * dmapctl.
     */
    *treemax = max;

    return( rc + error );
}


/*
 * NAME: initbmap
 *                                                                    
 * FUNCTION: Initialize the disk block allocation map for an aggregate.
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	dev_ptr	- 
 *	nblocks	- Number of blocks within the aggregate.
 *	aggr_block_size	- aggregate block size
 *
 * NOTES: The bmap control page is created.  Next, the number dmapctl level
 *	required to described the aggregate size (number of blocks within the
 *	aggregate) is determined. initctl() is then called to initialize the
 *	appropriate dmapctl levels and corresponding dmaps.
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 */
static int32 initbmap(	int	dev_ptr,
			int64	nblocks,
			uint32	aggr_block_size )
{
    int		level, rc = 0, error;
    uint64	nb, old_length;
    int32	next_xad, lblkno;
    int64	page_address;
    dbmap_t	*disk_cp;

    /*
     * Get the disk address for the control page
     */
    error = ujfs_rwdaddr(dev_ptr, &page_address, &block_map_inode, 0, GET,
		aggr_block_size );

    /*
     * Get the level for the top dmapctl for this disk map.  Levels are zero
     * origin.  This means that the level for the dmapctls right above the dmaps
     * is zero.
     * Determine the logical page number of the first page at this level
     */
    level = BMAPSZTOLEV( nblocks );

    /*
     * Initialize the dmapctls for this disk map and the dmaps they describe.
     */
    error = initctl(dev_ptr, 0, nblocks, level, &control_page->dn_maxfreebud,
		aggr_block_size);

    /*
     * Read control page and compare
     */
    disk_cp = (dbmap_t *)page_buffer;
    rc = ujfs_rw_diskblocks( dev_ptr, page_address, PSIZE, disk_cp, GET );

    /*
     * Since we can't tell what was the last thing allocated we will just set
     * this field the same as on disk for the comparison
     */
    control_page->dn_agpref = disk_cp->dn_agpref;

    rc = memcmp( control_page, disk_cp, sizeof(dbmap_t));

    if( rc != 0 ) {
	printf("Block Map control page is not the same.\n");
	printf("Disk map:\n");
	display_cpage(disk_cp);
	printf("Actual map:\n");
	display_cpage(control_page);
	rc = 1;
    }

    return( rc + error );
}


/*
 * NAME: alloc_map
 *                                                                    
 * FUNCTION: Allocate and initialize to zero the memory for dmap pages of block
 *	map.  Allocate and initialize to zero the memory for the control page of
 *	the block map.
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	num_dmaps	- Indicates number of dmaps to allocate
 *
 * NOTES:
 *
 * DATA STRUCTURES: Initializes file static variable block_map
 *
 * RETURNS: 0 for success
 */
static int alloc_map ( int32	num_dmaps )
{
    printf("Sizeof bmap = %d\n", sizeof(dbmap_t));
    printf("Sizeof dmapctl = %d\n", sizeof(dmapctl_t));
    printf("Sizeof dmap = %d\n", sizeof(dmap_t));

    if( num_dmaps <= 0 )
	return EINVAL;

    block_map = malloc( num_dmaps * sizeof( dmap_t ) );
    if( block_map == NULL )
	return ENOMEM;

    memset( block_map, 0, num_dmaps * sizeof( dmap_t ) );

    control_page = malloc( sizeof( dbmap_t ) );
    if( control_page == NULL ) {
	return ENOMEM;
    }

    memset( control_page, 0, sizeof( dbmap_t ) );

    return 0;
}


/*
 * NAME: initmap
 *                                                                    
 * FUNCTION: Initialize dmap pages and control page
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	nblocks	- Number of blocks covered by this map
 *	ag_size	- Will be filled in with AG size in blocks
 *	aggr_block_size		- Aggregate block size
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 */
static void initmap( uint64	nblocks,
		     uint32	ag_size,
		     uint32	aggr_block_size,
		     uint32	ndmaps )
{
    int32	index, nb, l2nl, n;
    int64	num_blocks_left;

    control_page->dn_agsize = ag_size;
    control_page->dn_agl2size = log2shift( ag_size );
    control_page->dn_l2nbperpage = log2shift(PSIZE/aggr_block_size);

    /*
     * Initialize control page
     */
    control_page->dn_mapsize = control_page->dn_nfree = nblocks;

    /*
     * Initialize the allocation group information.
     */
    control_page->dn_numag = nblocks / control_page->dn_agsize;
    control_page->dn_numag += (nblocks % control_page->dn_agsize) ? 1 : 0;
    for( index = 0, nb = nblocks; index < control_page->dn_numag;
	 index++, nb -= ag_size ) {
		control_page->dn_agfree[index] = MIN( nb, ag_size );
    }
    control_page->dn_maxlevel = BMAPSZTOLEV(nblocks);
    control_page->dn_aglevel = BMAPSZTOLEV(control_page->dn_agsize);
    l2nl = control_page->dn_agl2size -
		(L2BPERDMAP + control_page->dn_aglevel * L2LPERCTL);
    control_page->dn_agheigth = l2nl >> 1;
    control_page->dn_agwidth = 1 << (l2nl - (control_page->dn_agheigth << 1));
    for( index = 5 - control_page->dn_agheigth, control_page->dn_agstart = 0,
	 n = 1; index > 0; index-- ) {
	control_page->dn_agstart += n;
	n <<= 2;
    }

    /*
     * Initialize each dmap page
     */
    num_blocks_left = nblocks;
    for( index = 0; index < ndmaps; index++, num_blocks_left -= BPERDMAP ) {
	nb = MIN( num_blocks_left, BPERDMAP );
	ujfs_idmap_page( block_map + index, nb );
    }
}


/*
 * NAME: calc_map_size
 *                                                                    
 * FUNCTION: Calculates the size of a block map and initializes memory for dmap
 * pages of map.  Later when we are ready to write the map to disk we will
 * initialize the rest of the map pages.
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	number_of_blocks	- Number of blocks in aggregate
 *	aggr_inodes		- Array of aggregate inodes
 *	aggr_block_size		- Aggregate block size
 *	ag_size			- Will be filled in with AG size in blocks
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: 0 for success
 */
int calc_map_size( uint64	number_of_blocks,
		   uint32	aggr_block_size,
		   uint32	ag_size )
{
    int		rc;
    int32	ndmaps;
    int64	cur_block, end_block;

    /*
     * Round number of blocks to number of blocks as covered by dmap pages.
     */
    ndmaps = (number_of_blocks + BPERDMAP - 1)/BPERDMAP;

    /*
     * Make sure the number of dmaps needed is within the supported range
     */
    if( (ndmaps * BPERDMAP) > MAXMAPSIZE ) {
	return( EINVAL );
    }

    /*
     *  Allocate dmap pages and initialize them for the aggregate blocks
     */
    if( (rc = alloc_map( ndmaps )) < 0 )
	return rc;
    initmap( number_of_blocks, ag_size, aggr_block_size, ndmaps );

    return 0;
}


/*
 * NAME: markit
 *                                                                    
 * FUNCTION: Mark specified block allocated/unallocated in block map
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	block	- Map object to set or clear
 *	flag	- BADBLOCK indicates this is a bad block and should not be used
 *		  to determine max ag
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: NONE
 */
void markit( uint64	block,
	     int32	flag)
{
    uint32	page, rem, word, bit, mask;
    dmap_t	*p1;
    int32	agno;

    if( block >  control_page->dn_mapsize ) {
	printf(
	"Trying to mark block not in aggregate. block: %lld aggr_size: %lld\n",
		block, control_page->dn_mapsize );
	return;
    }

    /*
     * Keep track of the last allocated block to be filled into block map inode
     */
    if( block > last_allocated && !(flag & BADBLOCK) ) {
	last_allocated = block;
    }

    /*
     *  calculate page number in map, and word and bit number in word.
     */
    page = block / BPERDMAP;
    rem = block - page * BPERDMAP;
    word = rem >> L2DBWORD;
    bit = rem - (word << L2DBWORD);

    p1 = block_map + page;

    agno = block >> control_page->dn_agl2size;

    /*
     * Check if the block is already marked allocated
     */
    mask = (UZWORD >> bit);
    if( (p1->pmap[word] & mask) || (p1->wmap[word] & mask) ) {
	printf("DOUBLY marked block: %lld\n", block);
    }
    p1->pmap[word] |= mask;
    p1->wmap[word] |= mask;

    /*
     * Update the stats
     */
    p1->nfree--;
    control_page->dn_nfree--;
    control_page->dn_agfree[agno]--;
}


/*
 * The bitmaps are initialized. Now we will figure out the rest of the map
 * information and compare the pages to the pages on disk.
 */
int32 compare_maps( int		dev_ptr,
		    uint64	number_of_blocks,
		    uint32	aggr_block_size )
{
    int		rc;
    uint64	location;
    uint32	nbperpage;

    /*
     * At this point all of the dmaps have been initialzed except for their
     * trees.  Now we need to build the other levels of the map and adjust the
     * tree for each of the dmaps.
     */
    cur_dmap = block_map;
    control_page->dn_maxag = last_allocated / control_page->dn_agsize;

    nbperpage = PSIZE/aggr_block_size;
    l2nbperpage = log2shift(nbperpage);

    location = AITBL_OFF + (BMAP_I * sizeof(struct dinode));
    ujfs_rw_diskblocks( dev_ptr, location, sizeof(struct dinode),
			&block_map_inode, GET );

    rc = initbmap( dev_ptr, number_of_blocks, aggr_block_size );

    return(rc);
}
