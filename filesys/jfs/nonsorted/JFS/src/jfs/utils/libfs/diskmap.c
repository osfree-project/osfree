/* $Id: diskmap.c,v 1.1.1.1 2003/05/21 13:41:46 pasha Exp $ */

static char *SCCSID = "@(#)1.2  10/4/96 10:14:51 src/jfs/utils/libfs/diskmap.c, jfslib, w45.fs32, 990417.1";
/* @(#)diskmap.c	1.1  @(#)diskmap.c	1.1 8/30/96 08:17:36 */
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
 *   COMPONENT_NAME: cmdufs
 *
 *   FUNCTIONS: ujfs_adjtree
 *		ujfs_complete_dmap
 *		ujfs_getagl2size
 *		ujfs_idmap_page
 *		ujfs_maxbuddy
 *
 */
#include "jfs_types.h"
#include "jfs_dmap.h"

/*
 * budtab[]
 *
 * used to determine the maximum free string in a character
 * of a wmap word.  the actual bit values of the character
 * serve as the index into this array and the value of the
 * array at that index is the max free string.
 *
 */
static int8 budtab[256] = {
	3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0,
	2, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, -1};


/*
 * NAME: ujfs_maxbuddy
 *                                                                    
 * FUNCTION: Determines the maximum string of free blocks within a word of the
 *	wmap or pmap consistent with binary buddy.
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	cp	- Pointer to wmap or pmap word.
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: Maximum string of free blocks within word.
 */
int8 ujfs_maxbuddy( char *cp )
{
    /*
     * Check if the wmap or pmap word is all free. If so, the free buddy size is
     * BUDMIN.
     */
    if( *((uint32 *)cp) == 0 ) {
    	return(BUDMIN);
    }

    /*
     * Check if the wmap or pmap word is half free. If so, the free buddy size
     * is BUDMIN-1.
     */
    if( *((uint16 *)cp) == 0 || *((uint16 *)cp+1) == 0 ) {
    	return(BUDMIN-1);
    }

    /*
     * Not all free or half free. Determine the free buddy size through table
     * lookup using quarters of the wmap or pmap word.
     */
    return( MAX( MAX( budtab[*cp], budtab[*(cp+1)]),
		 MAX( budtab[*(cp+2)], budtab[*(cp+3)] )));
}


/*
 * NAME: ujfs_adjtree
 *                                                                    
 * FUNCTION: Calculate the tree of a dmap or dmapctl.
 *                                                                    
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *	cp	- Pointer to the top of the tree
 *	l2leaves- Number of leaf nodes as a power of 2
 *	l2min	- Number of disk blocks actually covered by a leaf of the tree;
 *		  specified as a power of 2
 *
 * NOTES: This routine first works against the leaves of the tree to calculate
 *	the maximum free string for leaf buddys.  Once this is accomplished the
 *	values of the leaf nodes are bubbled up the tree.
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 */
int8 ujfs_adjtree( int8	*treep,
		    int32	l2leaves,
		    int32	l2min )
{
    int32	nleaves, leaf_index, l2max, nextb, bsize, index;
    int32	l2free, leaf, num_this_level, nextp;
    int8	*cp0, *cp1, *cp = treep;

    /*
     * Determine the number of leaves of the tree and the
     * index of the first leaf.
     * Note: I don't know why the leaf_index calculation works, but it does.
     */
    nleaves = (1 << l2leaves);
    leaf_index = (nleaves - 1) / 3;

    /*
     * Determine the maximum free string possible for the leaves.
     */
    l2max = l2min + l2leaves;

    /*
     * Try to combine buddies starting with a buddy size of 1 (i.e. two leaves).
     * At a buddy size of 1 two buddy leaves can be combined if both buddies
     * have a maximum free of l2min; the combination will result in the
     * left-most buddy leaf having a maximum free of l2min+1.  After processing
     * all buddies for a certain size, process buddies at the next higher buddy
     * size (i.e. current size * 2) and the next maximum free
     * (current free + 1).  This continues until the maximum possible buddy
     * combination yields maximum free.
     */
    for( l2free = l2min, bsize = 1; l2free < l2max; l2free++, bsize = nextb ) {
    	nextb = bsize << 1;

    	for( cp0 = cp + leaf_index, index = 0; index < nleaves;
	     index += nextb, cp0 += nextb ) {
		if( *cp0 == l2free && *(cp0 + bsize) == l2free ) {
    		    *cp0 = l2free + 1;
		    *(cp0 + bsize) = -1;
		}
    	}
    }

    /*
     * With the leaves reflecting maximum free values bubble this information up
     * the tree.  Starting at the leaf node level, the four nodes described by
     * the higher level parent node are compared for a maximum free and this
     * maximum becomes the value of the parent node.  All lower level nodes are
     * processed in this fashion then we move up to the next level (parent
     * becomes a lower level node) and continue the process for that level.
     */
    for( leaf = leaf_index, num_this_level = nleaves >> 2; num_this_level > 0;
	 num_this_level >>= 2, leaf = nextp ) {
    	nextp = (leaf - 1) >> 2;

    	/*
	 * Process all lower level nodes at this level setting the value of the
	 * parent node as the maximum of the four lower level nodes.
    	 */
    	for( cp0 = cp + leaf, cp1 = cp + nextp, index = 0;
	     index < num_this_level; index++, cp0 += 4, cp1++ ) {
    		*cp1 = TREEMAX(cp0);
	}
    }

    return(*cp);
}


/*
 * NAME: ujfs_complete_dmap
 *                                                                    
 * FUNCTION: Fill in rest of dmap fields from wmap/pmap already initialized.
 *                                                                    
 * PARAMETERS:
 *	dmap_page	- dmap to complete
 *	blkno		- starting block number for this dmap
 *	treemax		- will be filled in with max free for this dmap
 *
 * RETURNS: NONE
 */
void ujfs_complete_dmap( dmap_t	*dmap_page,
			 int64	blkno,
			 int8	*treemax )
{
    dmaptree_t	*tp;
    int8	*cp;
    int32	index;

    dmap_page->start = blkno;

    tp = &dmap_page->tree;
    tp->height = 4;
    tp->leafidx = LEAFIND;
    tp->nleafs = LPERDMAP;
    tp->l2nleafs = L2LPERDMAP;
    tp->budmin = BUDMIN;

    /*
     * Pick up the pointer to the first leaf of the dmap tree.
     */
    cp = tp->stree + tp->leafidx;

    /*
     * Set the initial state for the leaves of the dmap tree.  They will reflect
     * the current allocation state of the wmap words.
     */
    for( index = 0; index < LPERDMAP; index++ ) {
    	*(cp + index) = ujfs_maxbuddy((char *)&dmap_page->wmap[index]);
    }

    /*
     * With the leaves of the dmap initialized adjust (initialize) the dmap's
     * tree.
     */
    *treemax = ujfs_adjtree( tp->stree, L2LPERDMAP, BUDMIN );
}


/*
 * NAME: ujfs_idmap_page
 *                                                                    
 * FUNCTION: Initialize one dmap page
 *                                                                    
 * POST CONDITIONS: Blocks which don't actually exist in the aggregate will be
 *	marked as allocated in the dmap page.  The total number of blocks will
 *	only account for the actually existing blocks.
 *
 * PARAMETERS:
 *	map_page	- pointer to page of map
 *	nblocks		- number of blocks this page
 *
 * RETURNS: NONE
 */
void ujfs_idmap_page( dmap_t	*map_page,
		      uint32	nblocks )
{
    uint32	index, nwords, bit;

    map_page->nblocks = map_page->nfree = nblocks;

    /*
     * Partial dmap page?
     * If there are not enough blocks to cover an entire dmap page the ones
     * which represent blocks which don't exist will be marked as allocated.
     *
     * nwords will indicate the first word beyond the end of existing blocks
     * bit will indicate if this block does not fall on a 32-bit boundary
     */
    nwords = nblocks/DBWORD;
    bit = nblocks % DBWORD;

    if( bit ) {
	/*
	 * Need to mark a partial word allocated
	 */
	map_page->wmap[nwords] = map_page->pmap[nwords] = ONES >> bit;
	nwords++;
    }

    /*
     * Set the rest of the words in the page to ONES.
     */
    for( index = nwords; index < LPERDMAP; index++ ) {
	map_page->pmap[index] = map_page->wmap[index] = ONES;
    }
}


/*
 * NAME: ujfs_getagl2size
 *                                                                    
 * FUNCTION: Determine log2(allocation group size) based on size of aggregate
 *                                                                    
 * PARAMETERS:
 *	size		- Number of blocks in aggregate
 *	aggr_block_size	- Aggregate block size
 *
 * RETURNS: log2(allocation group size) in aggregate blocks
 */
int32 ujfs_getagl2size( int64	size,
			int32	aggr_block_size )
{
    int64	sz;
    int64	m;
    int32	l2sz;
    int32	l2ag_block_size;

    if( size < BPERDMAP * MAXAG ) {
    	return(L2BPERDMAP);
    }

    m = ((uint64)1 << (BITSPERINT64-1));
    for( l2sz = BITSPERINT64, l2sz >= 0; l2sz--; m >>= 1 ) {
	if( m & size ) {
    	    break;
	}
    }

    sz = (int64)1 << l2sz;
    if( sz < size ) {
	l2sz += 1;
    }

    return(l2sz - L2MAXAG);
}
