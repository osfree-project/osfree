/* $Id: cbblwsp.h,v 1.1.1.1 2003/05/21 13:41:06 pasha Exp $ */

/* static char *SCCSID = "@(#)1.2  9/12/98 11:51:46 src/jfs/utils/clrbblks/cbblwsp.h, jfscbbl, w45.fs32, 990417.1";*/
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
 *   MODULE_NAME:		cbblwsp.h
 *
 *   COMPONENT_NAME: 	jfscbbl
 *
 */
#ifndef H_CBBLWSP
#define	H_CBBLWSP


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * clrbblks uses the structures and types in this file 
 * to define its workspace.
 *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

/*----------------------------------------------------------------
 *
 * structure cbbls_bdblk_record describes one entry on an LVM Bad Block
 *				List for this file system.
 *
 */

#define MAX_LVBLKS_PER_FSBLK	MAXBLOCKSIZE/MINBLOCKSIZE
    /*
     * the possible values for list_type
     *   (since inode_type is 4 bits wide, valid range is 0-15)
     */
#define no_list		' '
#define sequen_list	's'
#define free_list	'f'
#define retry_list	'r'
#define resolved_list	'd'


struct cbbl_bdblk_record {
     /* 
      * supporting the balanced-tree used for quick random access
      * and the lists used  
      *		1) for quick sequential access 
      * 	2) to keep track of records for blocks eligible for retry
      */
   int64	fs_blkno;	      		/* 8 : key - block number in terms of
				       	 * 	file system blocks
				       	 */
   struct cbbl_bdblk_record *left;    	/* 4 : for tree - left child  */
   struct cbbl_bdblk_record *right;   	/* 4 : for tree - right child */
   int16  height;                     		/* 2 : for tree */
   int8   balance;                    		/* 1 : for tree */
   char   list_type;                  		/* 1 : for lists and stack */
   struct cbbl_bdblk_record *next;    	/* 4 : for lists and stack */
   struct cbbl_bdblk_record *prev;    	/* 4 : for lists and stack */
   struct cbbl_bdblk_record *tree_next; /* 4 : all elements still in tree */
      /* 
      * the bad block data
      */
   int8 seen;			/* 1 : !0 => has been visited in the tree
				 * 	traversal to build the sequential list
				 */
   int8 retry_list_count;	/* 1 : number of times have failed
				 * 	to relocate the extent
 				 */
   int8 staledata_count;	/* 1 : number of times have failed
				 * 	to relocate the extent containing
				 * 	this block because of stale inode
				 *	data on the device.
				 */
   int8 insuffspace_count;	/* 1 : number of times have failed
				 * 	to relocate the extent containing
				 * 	this block because can't allocate
				 *	a new extent for it.
				 */
   int8 tooslow_count;		/* 1 : number of times have successfully
				 * 	relocated the data in the extent
				 * 	containing this block, but then 
				 * 	couldn't allocate it to the JFS
				 * 	Bad Block inode because it was
				 *	allocated elsewhere first
				 */
   int8 relocated_count;	/* 1 : number of times have successfully
				 * 	relocated the data in the extent
				 * 	containing this block
				 */
   int8 allocated_to_BBino;	/* 1 : !0 => has been allocated to the
				 *	JFS Bad Block inode
				 */
   int8 LVM_notified;		/* 1 : !0 => the LVM has been notified that
				 * 	it can safely remove this block from
				 *	its bad block list.
				 */
   int32  LV_table;		/* 4 : LVM bad block table number */
   int64  LV_blkno[ MAX_LVBLKS_PER_FSBLK ];	/*
				 * 64 : (8 * 8) - block numbers in terms of
				 * 	          logical volume blocks
				 * 	N.B. There are at most 8 lv blocks
				 *	per fs block.  This is the list of
				 *	lv blocks which are BOTH on the
				 *	LVM's bad block list AND which are
				 *	part of this fs block.
				 */
   };


typedef struct cbbl_bdblk_record *cbbl_bdblk_recptr;

#define bdblk_record_length sizeof(struct cbbl_bdblk_record)



/*----------------------------------------------------------------------------
 *
 * type cbbl_bdblk_baltree is a height-balanced tree of fcbbl_bdblk_record.
 *
 *
 * There is one node in this table for each bad block reported by the LVM
 * which we hope to resolve.
 *
 */
struct cbbl_bdblk_baltree_head {
   char               eyecatcher[8]; 	/* 8 */
   cbbl_bdblk_recptr  right;		/* 4 : for tree - right child - points
						* to the root node of the 
						* balanced tree 
						*/
   int32              height;		/*  4 : tree height */
   cbbl_bdblk_recptr  seq_list;		/*  4 : ptr to first node in the 
						* sequential list
						*/
   cbbl_bdblk_recptr  last_recptr;	/*  4 : ptr to last node returned in 
						* sequential traversal of list
						*/
   char               rsrvd[12];	/* 8 : pad to quadword boundary */
   } ;					/* (32) */

typedef struct cbbl_bdblk_baltree_head 	 cbbl_bdblk_baltree;
typedef cbbl_bdblk_baltree 		*bdblk_baltree_headptr;


/*----------------------------------------------------------------------------
 * type cbbl_extent_record describes one extent of storage which clrbblks
 *                         has allocated for its workspace.
 *
 */
typedef struct wsp_ext_rec {
   struct wsp_ext_rec *next;
   int32    extent_length;
   char     *extent_addr;
   int32    last_byte_used;
   } cbbl_extent_record;

typedef struct wsp_ext_rec *wsp_ext_rec_ptr;


/*----------------------------------------------------------------------------
 * structure cbbl_agg_record describes the aggregate
 */

typedef struct cbbl_agg_record {
   char		eyecatcher[8];
   int32		fs_blksize;	/* aggregate block size */
   int32		lv_blksize;	/* device block size	*/
   int32 		fs_lv_ratio;	/* fs_blksize/lv_blksize */
   char		rsrvda[4];	/* 4 : pad to quadword boundary */
   int64		fs_last_metablk;  /* 
				* last fs block we won't try to relocate 
				* because it holds fixed-location metadata
				*/
   int64		fs_first_wspblk; /* 
				* first fs block we won't try to relocate 
				* because it holds fsck workspace or the
				* inline journal log 
				*/
     int32	total_bad_blocks; /* count of bad blocks in LVM's list 
				* at beginning of Bad Block List utility
				* processing
				*/
   int32		resolved_blocks; /* count of bad blocks:
				* - for which the data has been relocated,
				* - which are now allocated to the bad block
				*   inode, and 
				* - which the LVM has been told to forget
				*/
   int32		reloc_extents;	/* count of relocated extents */
   int64		reloc_blocks;	/* count of blocks in relocated extents */
   int32		LVM_lists;	/* count of bad block lists maintained by LVM
				* according to the last query
				*/
   wsp_ext_rec_ptr   wsp_extent_list; /* linked list of records describing
                               * the workspace extents.
                               */
   char			rsrvdb[4];	/* 4 : pad to quadword boundary */

   char		scan_eyecatcher[8];
   int32		iag_idx;	/* current iag number	                 */
   int32		extent_idx;	/* current extent (within current iag)   */
   int32		inode_idx;	/* current inode (within current extent) */
   int32		this_inonum; 	/* the actual inode number of current inode */
   int32		this_pass;	/* the number of times the fileset inodes have
				* already been scanned for ownership of data 
				* residing in a bad block.
				*/
   char			rsrvdc[4];	/* 4 : pad to quadword boundary */
	
   cbbl_bdblk_recptr	inobt_stack; 	/* stack for balanced tree */
   cbbl_bdblk_recptr	bdblk_retry_list; 
   cbbl_bdblk_recptr	free_bdblk_records; 
   cbbl_bdblk_recptr	resolved_bdblk_records; 
	
   cbbl_bdblk_baltree	bdblk_baltree;  /* header for  balanced tree 
					*/
   char		bufptr_eyecatcher[8];
   dinode_t	*ImapInoPtr;		/* addr of imap inode buffer		*/
   IMap_t	*ImapCtlPtr;		/* addr of imap control page buffer	*/
   xtpage_t	*ImapLeafPtr;		/* addr of imap leaf page buffer	*/
   iag_t		*iagPtr;		/* addr of iag buffer			*/
   dinode_t	*InoExtPtr;		/* addr of inode extent buffer		*/
   char		rsrvdd[4];		/* 4 : pad to quadword boundary */
   } cbbl_agg_record;


#endif

