/* $Id: cbblwsp.c,v 1.2 2004/07/24 00:49:05 pasha Exp $ */

static char *SCCSID = "@(#)1.2  9/12/98 11:51:38 src/jfs/utils/clrbblks/cbblwsp.c, jfscbbl, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		cbblwsp.c
 *
 *   COMPONENT_NAME:		jfscbbl
 *
 *   FUNCTIONS:		baltree_load
 *			baltree_purge
 *			baltree_search
 *			init_agg_record
 *			freelist_insert
 *			retrylist_insert
 *			retrylist_purge
 *			rslvdlist_insert
 *			seqlist_create
 *			seqlist_remove
 *			workspace_release
 *
 *			alloc_wrksp
 *			alloc_wsp_extent 
 *			baltree_search_insert
 *			baltree_stack_pop 
 *			baltree_stack_push
 *			get_bdblk_record 
 *
 */


#include "clrbblks.h"

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
 *	device under clrbblks
 */
extern char	LVName[4];	// device name

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
 *	file system under clrbblks
 */
extern FS_t	FSMount;	// file system descriptor
extern FS_t	*fsMount;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  *	inode allocation map
  */
extern IMap_t	IMap;		// inode allocation map control page
extern IMap_t	*iMap;


/* 
 * bitmap page control 
 */
extern iag_t	iagbuf;  

/* 
 * inode extent buffer 
 */
extern int32	IXNumber;
extern dinode_t ixbuf[];


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * fsck aggregate info structure pointer
  *
  *      defined in xchkdsk.c
  */
extern struct cbbl_agg_record *agg_recptr;

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 *
 * The following are internal to this file
 *
 */
int32 alloc_wrksp_cb ( int32, void  ** );
int32 alloc_wsp_extent_cb ( int32 );
int32 baltree_search_insert ( int64, cbbl_bdblk_recptr * );
int32 baltree_stack_pop ( cbbl_bdblk_recptr * );
int32 baltree_stack_push ( cbbl_bdblk_recptr );
int32 get_bdblk_record ( cbbl_bdblk_recptr * );

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */

/****************************************************************************
 * NAME:	alloc_wrksp
 *
 * FUNCTION:	Allocates and initializes (to guarantee the storage is 
 *		backed) dynamic storage for the caller.
 *
 * PARAMETERS:
 *      length		the number of bytes of storage which are needed
 *
 *      addr_wrksp_ptr	the address of a variable in which this routine
 *			will return the address of the dynamic storage
 *			allocated for the caller
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 alloc_wrksp_cb ( int32   length,
                    void  **addr_wrksp_ptr )
{
  int32 awsp_rc = 0;
  char *wsp_ptr = NULL;
  wsp_ext_rec_ptr this_fer;
  int32 bytes_avail;
  int32 min_length;

  *addr_wrksp_ptr = NULL;      /* initialize return value */
  min_length = ((length + 7) / 8) * 8;  /* round up to an 8 byte boundary */

  while ( (wsp_ptr == NULL) && (awsp_rc == 0) ) {
    this_fer = agg_recptr->wsp_extent_list;

    while( (this_fer != NULL)  &&
           (wsp_ptr == NULL)   &&
           (awsp_rc == 0)  ) {
      bytes_avail = this_fer->extent_length - this_fer->last_byte_used;
      if( bytes_avail >= min_length ) {  /* there's enough here */
        wsp_ptr = this_fer->extent_addr + this_fer->last_byte_used + 1;
        this_fer->last_byte_used += min_length;
        }  /* end there's enough here */
      else {  /* try the next fer */
        this_fer = this_fer->next;
        }  /* end else try the next fer */
      }  /* end while */
    if( (awsp_rc == 0) && (wsp_ptr == NULL) ) {  /*
                            * nothing fatal but we didn't find the
                            * storage yet
                            */
      awsp_rc = alloc_wsp_extent_cb( min_length );  /* will allocate some
                            * number of memory segments and put the fer
                            * describing it on the beginning of the list.
                            */
      }  /* end nothing fatal but we didn't find the storage yet */
    }  /* end while */

  if( awsp_rc == 0 ) {  /* we allocated virtual storage */
        /*
         * now initialize the storage 
         */
    memset( (void *) wsp_ptr, 0, length );

    *addr_wrksp_ptr = (void *) wsp_ptr;  /* set the return value */
    }  /* end we allocated virtual storage */

 return( awsp_rc );
}                                      /* end alloc_wrksp    */

/****************************************************************************
 * NAME: 	alloc_wsp_extent
 *
 * FUNCTION:	Extend the workspace
 *
 *		Since OS/2 always allocates a new memory segment when
 *		it encounters a malloc(), for optimum use of the
 *		storage we'll always allocate a whole segment.  (Then
 *		the alloc_wrksp routine portions it out as needed.)
 *
 * PARAMETERS:
 *      minimum_length	minimum number of bytes of contiguous storage
 *			needed
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 alloc_wsp_extent_cb ( int32 minimum_length )
{
  int32 awe_rc = 0;
  wsp_ext_rec_ptr new_fer;
  int32 extent_length = MEMSEGSIZE;
  char *extent_addr = NULL;

     /*
      * the user has specified the minimum s/he needs.  We must allocate
      * at least 16 more than that because we're going to use 16 bytes
      * at the beginning to keep track of it.
      */
  while( extent_length < (minimum_length + 16) ) {
    extent_length += MEMSEGSIZE;
    }

  extent_addr = (char *) malloc(extent_length);

  if( extent_addr == NULL ) { /* allocation failure */
    awe_rc = CBBL_ENOMEM1;
    }  /* end allocation failure */

  else {  /* got the dynamic storage  */

        /*
         * use the first 16 bytes of it to keep track of it
         */
    new_fer = (wsp_ext_rec_ptr) extent_addr;
    new_fer->extent_length = extent_length;
    new_fer->extent_addr = extent_addr;
    new_fer->last_byte_used = sizeof(cbbl_extent_record) - 1;

    new_fer->next = agg_recptr->wsp_extent_list;
    agg_recptr->wsp_extent_list = new_fer;
    }  /* end else got the dynamic storage */

 return( awe_rc );
}                                    /* end alloc_wsp_extent    */


/****************************************************************************
 * NAME: 	baltree_load
 *
 * FUNCTION: 	Determine which file system block contains the given LV 
 *		block.  Find (or add) an entry in the balanced tree 
 *		describing that file system block, and note in it the LV 
 *		block and the LV table from which it came.  
 *
 * PARAMETERS: 
 *	LV_blknumber	a logical volume block number from  the LVM's bad
 *			block list for the file system.
 *
 *	LV_tblnumber	The LVM's bad block list is really a series of
 *			tables.  This is the ordinal number of the table
 *			in which this particular bad LV block is recorded.
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 baltree_load ( LSNumber LV_blknumber, int32 LV_tblnumber )
{
    int32 btl_rc = 0;
    int32 lvidx;
    int done = 0;
    int64 fs_blknumber;
    cbbl_bdblk_recptr bblkrecptr;
	
	/*
	 * figure out what file system block contains this 
	 * logical volume block.
	 */
    fs_blknumber = LV_blknumber / agg_recptr->fs_lv_ratio;

	/*
	 * The file system block must be after the fixed position
	 * metadata areas and before the workspace and journal log
	 * areas for us to even try resolving it.
	 */
    if(	(fs_blknumber > agg_recptr->fs_last_metablk) &&
	(fs_blknumber < agg_recptr->fs_first_wspblk) ) {  /*
				* maybe it can be resolved
				*/

		/*
		 * find (or create) a bad block record, in the balanced
		 * tree, for this file system block.
		 */
	btl_rc = baltree_search_insert( fs_blknumber, &bblkrecptr );
	
		/*
		 * Add the LVM's Bad Block Table number to the bad block 
		 * record.
		 * Add this logical volume block number to the list in
		 * the bad block record.
		 */
	if( btl_rc == 0 ) {  /* it's in the tree */
	    done = 0;
	    bblkrecptr->LV_table = LV_tblnumber;
	    for( lvidx = 0; 
	         ((lvidx < MAX_LVBLKS_PER_FSBLK) && (!done)); 
	         lvidx++ ) {
		if( bblkrecptr->LV_blkno[lvidx] == 0 ) {  /*
				* 0 can never be an actual LV block number
				* in this context since the 1st block in
				* the file system is NEVER relocated.
				*/
		    bblkrecptr->LV_blkno[lvidx] = LV_blknumber;
		    done = -1;
		    }
		}  /* end for lvidx = ... */
	    }  /* end it's in the tree */
	}  /* end maybe it can be resolved */
	
  return( btl_rc );
}                                      /* end baltree_load    */


/****************************************************************************
 * NAME: 	baltree_purge
 *
 * FUNCTION:	Move any entries in the balanced tree which are not on 
 *		the retry list and not already on the free list onto the 
 *		free list.
 *
 *		Initialize the balanced tree and the sequential list of 
 *		its active elements.
 *
 * PARAMETERS: 	none
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 baltree_purge ( )
{
  int32 btp_rc = 0;
  cbbl_bdblk_recptr bblkrecptr;
  cbbl_bdblk_recptr next_bblkrecptr;

	/*
	 * move sequential list entries to the free list
	 */
  bblkrecptr = agg_recptr->bdblk_baltree.seq_list;
  while( bblkrecptr != NULL ) {
    next_bblkrecptr = bblkrecptr->next;
    freelist_insert( bblkrecptr );
    bblkrecptr = next_bblkrecptr;
    }

	/*
	 * move resolved list entries to the free list
	 */
  bblkrecptr = agg_recptr->resolved_bdblk_records;
  while( bblkrecptr != NULL ) {
    next_bblkrecptr = bblkrecptr->next;
    freelist_insert( bblkrecptr );
    bblkrecptr = next_bblkrecptr;
    }

	/*
	 * initialize the tree, the sequential list, and 
	 * the resolved list
	 */
  agg_recptr->bdblk_baltree.right = NULL;
  agg_recptr->bdblk_baltree.height = 0;
  agg_recptr->bdblk_baltree.seq_list = NULL;
  agg_recptr->bdblk_baltree.last_recptr = NULL;
  agg_recptr->resolved_bdblk_records = NULL;

  return( btp_rc );
}                                      /* end baltree_purge    */


/****************************************************************************
 * NAME: 	baltree_search
 *
 * FUNCTION:  	Search the balanced binary sort tree for a node containing 
 *		the given key.  Returns the address of the node, still in the
 *		sequential list,  with key equal to or greater than the given 
 *		key (if any).
 *
 * PARAMETERS:
 *      given_key	the key to be matched
 *
 *      addr_bblkrecptr	pointer to a variable in which to return the
 *			address of the node, if found, or NULL if the
 *			search is not successful.
 *
 * NOTES:  (See Knuth)
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 baltree_search ( int64                      given_key,
	                    cbbl_bdblk_recptr    *addr_bblkrecptr )
{
  int32 bts_rc = 0;
  bdblk_baltree_headptr thead;
  cbbl_bdblk_recptr bblkrecptr, last_greater_bblkrecptr;
  int done = 0;

  thead = &(agg_recptr->bdblk_baltree);
  *addr_bblkrecptr = NULL;         /* assume it's not here */

  if( thead->right == NULL ) {  /* empty tree */
    done = -1;
    }
  else {  /* tree is not empty */
    bblkrecptr = thead->right;
    last_greater_bblkrecptr = NULL;
    }

  while( !done ) {

    if( given_key == bblkrecptr->fs_blkno ) {  /* key matches */
      *addr_bblkrecptr = bblkrecptr;	/* return node with equal key */
      done = -1;
      }  /* end key matches */

    else if( given_key < bblkrecptr->fs_blkno ) {  /* key is less */

      if( bblkrecptr->left == NULL ) {
        *addr_bblkrecptr = bblkrecptr;  /* return node with greater key */
        done = -1;                      /* it's not in here */
        }
      else {
        last_greater_bblkrecptr = bblkrecptr;  /*
				* if this key is greater than any in the left
				* subtree, then this record is the one we want 
				* to return
				*/
        bblkrecptr = bblkrecptr->left;  /* maybe it's in the left subtree */
        }
      }  /* end key is less */
	
    else {  /* key is greater */
      if( bblkrecptr->right == NULL ) {
        *addr_bblkrecptr = last_greater_bblkrecptr;  /*
				* return the last record in the tree such that
				* the given key is less (or NULL if none seen)
				*/
        done = -1;     
        }
      else {
        bblkrecptr = bblkrecptr->right;  /* maybe it's in the right subtree */
        }
      }  /* end key is greater */
    }  /* end while */

	/*
	 * now make sure we're returning a record still in the
	 * sequential list.  If not, find the next which is still in
	 * the list (if any).
	 */
    bblkrecptr = *addr_bblkrecptr;
    done = 0;
    while( (bblkrecptr != NULL) && (!done) ) {
	if( bblkrecptr->list_type == sequen_list ) {
	    done = -1;
	    }
	else {
	    bblkrecptr = bblkrecptr->tree_next;
	    }
	}  /* end while */
	
    *addr_bblkrecptr = bblkrecptr;	/* this will have changed if
				* the one selected originally wasn't on 
				* the sequential list.
				*/
  return( bts_rc );
}                                      /* end baltree_search    */


/****************************************************************************
 * NAME: 	baltree_search_insert
 *
 * FUNCTION:  	Search the given balanced binary sort tree for a node 
 *		containing the given key.  If no node is found, insert one.
 *
 * PARAMETERS:
 *      given_key	the key to be matched
 *
 *      addr_bblkrecptr	pointer to a variable in which to return the
 *			address of the node.
 *
 * NOTES: (See Knuth)
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 baltree_search_insert ( int64               given_key,
                              cbbl_bdblk_recptr  *addr_bblkrecptr )
{
  int32 btsi_rc = 0;
  bdblk_baltree_headptr thead;
  int done = 0;
  int rebal;

  cbbl_bdblk_recptr found_bblkrecptr = NULL;
  cbbl_bdblk_recptr Precptr;        /* used to move down the tree */
  cbbl_bdblk_recptr Qrecptr;        /* temp variable */
  cbbl_bdblk_recptr Rrecptr;        /* temp variable */
  cbbl_bdblk_recptr Srecptr;        /* node where balancing may be needed */
  cbbl_bdblk_recptr Trecptr = NULL; /* parent of Srec (null if Srec is root) */
  cbbl_bdblk_recptr *addr_recptr;
  cbbl_bdblk_recptr new_recptr_val;
	
  thead = &(agg_recptr->bdblk_baltree);

  if( thead->right == NULL ) {  /* tree is empty */
       /*
        * create
        */
    btsi_rc = get_bdblk_record ( &Qrecptr );
    if( btsi_rc == 0 ) {  /* got a new record */
      Qrecptr->fs_blkno = given_key;
      thead->right = Qrecptr;
      thead->height = 1;
      found_bblkrecptr = Qrecptr;
      }  /* end got a new record */
    done = -1;
    }  /* end tree is empty */

  else {  /* tree isn't empty */
       /*
        * search and insert
        */
    Srecptr = thead->right;
    Precptr = thead->right;
    }  /* end else tree isn't empty */

  while( (!done) && (found_bblkrecptr == NULL) && (btsi_rc == 0) ) {

    if( given_key == Precptr->fs_blkno ) {  /* found it! */
      found_bblkrecptr = Precptr;
      done = -1;                    /* no rebalancing will be needed
                                     * because no insert done
                                     */
      }  /* end found it! */

    else if( given_key < Precptr->fs_blkno ) {  /* given key is less */
      Qrecptr = Precptr->left;      /* try the left subtree */

      if( Qrecptr == NULL ) {       /* it's not in the tree */
        btsi_rc = get_bdblk_record ( &Qrecptr );
        if( btsi_rc == 0 ) {  /* got a new record */
          Qrecptr->fs_blkno = given_key;
          Precptr->left = Qrecptr;
          found_bblkrecptr = Qrecptr;
          }  /* end got a new record */
        }  /* end it's not in the tree */

      else {  /* it could be in the left subtree */
        if( Qrecptr->balance != 0 ) {  /* might need to rebalance here */
          Trecptr = Precptr;
          Srecptr = Qrecptr;
          }  /* end might need to rebalance here */
        Precptr = Qrecptr;
        }  /* end it could be in the left subtree */
      }  /* end given key is less */

    else {  /* given key is greater */
      Qrecptr = Precptr->right;      /* try the right subtree */

      if( Qrecptr == NULL ) {       /* it's not in the tree */
        btsi_rc = get_bdblk_record ( &Qrecptr );
        if( btsi_rc == 0 ) {  /* got a new record */
          Qrecptr->fs_blkno = given_key;
          Precptr->right = Qrecptr;
          found_bblkrecptr = Qrecptr;
          }  /* end got a new record */
        }  /* end it's not in the tree */

      else {  /* it could be in the right subtree */
        if( Qrecptr->balance != 0 ) {  /* might need to rebalance here */
          Trecptr = Precptr;
          Srecptr = Qrecptr;
          }  /* end might need to rebalance here */
        Precptr = Qrecptr;
        }  /* end it could be in the right subtree */
      }  /* end given key is greater */
    }  /* end while */

  if( (btsi_rc == 0) && (!done) ) {  /* we inserted a new record */

     /*
      * adjust balance indicators
      */

    if( given_key < Srecptr->fs_blkno ) {
      Rrecptr = Srecptr->left;
      Precptr = Srecptr->left;
      }
    else {  /* (given_key >= Srecptr->fs_blkno) */
      Rrecptr = Srecptr->right;
      Precptr = Srecptr->right;
      }  /* end else (given_key >= Srecptr->fs_blkno) */

    while( Precptr != Qrecptr ) {
      if( given_key < Precptr->fs_blkno ) {
        Precptr->balance = -1;
        Precptr = Precptr->left;
        }
      else {  /* (given_key > Precptr->fs_blkno) */
        Precptr->balance = 1;
        Precptr = Precptr->right;
        }  /* end else (given_key > Precptr->fs_blkno) */
      }  /* end while */

     /*
      * rebalance if necessary
      */

    if( given_key < Srecptr->fs_blkno ) {  rebal = -1;  }
    else {  rebal = 1;  }

    if( Srecptr->balance == 0 ) { /* now it's just 1 out of exact balance */
      Srecptr->balance = rebal;
      thead->height = thead->height + 1;  /* so we added height to the tree */
      }  /* end now it's just 1 out of exact balance */
    else if( Srecptr->balance == (-rebal) ) {  /* this makes it in exact balance */
      Srecptr->balance = 0;
      }  /* end this makes it in exact balance */
    else {  /* this makes it 2 out of balance so need to rebalance */

         /*
          * rebalancing is necessary
          */

      if( Rrecptr->balance == rebal ) {  /* >>>>> single rotation needed <<<<< */

        Precptr = Rrecptr;
        if( rebal == 1 ) { 
          Srecptr->right = Rrecptr->left; 
          Rrecptr->left = Srecptr;
          }
        else {  
          Srecptr->left = Rrecptr->right; 
          Rrecptr->right = Srecptr;
          }

        Srecptr->balance = 0;
        Rrecptr->balance = 0;
        } /* end single rotation needed */

      else {  /* >>>>> double rotation needed <<<<< */

        if( (-rebal) == 1 ) {  
          Precptr = Rrecptr->right;  
          Rrecptr->right = Precptr->left; 
          Precptr->left = Rrecptr; 
          Srecptr->left = Precptr->right; 
          Precptr->right = Srecptr; 
          }
        else {  
          Precptr = Rrecptr->left; 
          Rrecptr->left = Precptr->right; 
          Precptr->right = Rrecptr; 
          Srecptr->right = Precptr->left; 
          Precptr->left = Srecptr; 
          }

        if( Precptr->balance == rebal ) {
          Srecptr->balance = -rebal;
          Rrecptr->balance = 0;
          Precptr->balance = 0;
          }
        else if( Precptr->balance == 0 ) {
          Srecptr->balance = 0;
          Rrecptr->balance = 0;
          }
        else {  /* (Precptr->balance == -rebal) */
          Srecptr->balance = 0;
          Rrecptr->balance = rebal;
          Precptr->balance = 0;
          }
        }  /* end else double rotation needed */

         /*
          * add the finishing touch to rebalancing
          *    -- at this point Precptr points to the new root
          *       and Trecptr points to the father of the old root.
          */

      if( Srecptr == thead->right ) {  /* Srecptr was the root */
        thead->right = Precptr;
        }  /* end Srecptr was the root */
      else if( Srecptr == Trecptr->right ) {  /* Srecptr was a right child */
        Trecptr->right = Precptr;
        }  /* end Srecptr was a right child */
      else {  /* Srecptr was a left child */
        Trecptr->left = Precptr;
        }  /* end Srecptr was a left child */
      }  /* end else this makes it 2 out of balance so need to rebalance */
    }  /* end we inserted a new record */

  if( btsi_rc == 0 ) {
    *addr_bblkrecptr = found_bblkrecptr;
    }

  return( btsi_rc );
}                                      /* end baltree_search_insert    */


/****************************************************************************
 * NAME:	baltree_stack_pop
 *
 * FUNCTION:  	If the balanced tree stack is not empty, remove the top 
 *		element and return its address.  Otherwise return NULL.
 *
 * PARAMETERS:
 *      addr_bblkrecptr	the address of a variable in which to return
 *			the address of the top element
 *
 * NOTES:  
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 baltree_stack_pop ( cbbl_bdblk_recptr *addr_bblkrecptr )
{
  int32 btsp_rc = 0;

  *addr_bblkrecptr = agg_recptr->inobt_stack;

  if( agg_recptr->inobt_stack != NULL ) {
    agg_recptr->inobt_stack = agg_recptr->inobt_stack->next;
    }

  return( btsp_rc );
}                                      /* end baltree_stack_pop     */


/****************************************************************************
 * NAME:	baltree_stack_push
 *
 * FUNCTION:	Add the given element to the top of the balanced tree stack.
 *
 * PARAMETERS:
 *      bblkrecptr	the address of the element to add to the stack
 *
 * NOTES:  
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 baltree_stack_push ( cbbl_bdblk_recptr bblkrecptr )
{
  int32 btsp_rc = 0;

  bblkrecptr->next = agg_recptr->inobt_stack;
  agg_recptr->inobt_stack = bblkrecptr;

  return( btsp_rc );
}                                      /* end baltree_stack_push    */
	
/****************************************************************************
 * NAME:	init_agg_record
 *
 * FUNCTION:	initializes the global record, cbbl_aggregate
 *
 * PARAMETERS:	none
 *
 * NOTES:  
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 init_agg_record_cb( )
{
  int32 iar_rc = 0;
	
  memset ( (void *) agg_recptr, 0, sizeof(cbbl_agg_record) );
	
  memcpy( (void *) &(agg_recptr->eyecatcher), (void *) "cbblsagg", 8 );

  memcpy( (void *) &(agg_recptr->scan_eyecatcher), (void *) "InodScan", 8 );

  memcpy( (void *) &(agg_recptr->bdblk_baltree.eyecatcher),
          (void *) "BdBlBalT", 8 );

  memcpy( (void *) &(agg_recptr->bufptr_eyecatcher), (void *) "ScanBufs", 8 );

  agg_recptr->ImapInoPtr = &DIIMap;
  agg_recptr->ImapCtlPtr = &IMap;
  agg_recptr->ImapLeafPtr = &IMapXtreeLeaf;
  agg_recptr->iagPtr = &iagbuf;
  agg_recptr->InoExtPtr = &(ixbuf[0]);

  return( iar_rc );
}                                 /* end init_agg_record       */

/****************************************************************************
 * NAME:	freelist_insert
 *
 * FUNCTION:	Insert the given entry into the list of free bad 
 *		block records
 *
 * PARAMETERS:
 *      bbrecptr	a pointer to the record to insert
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 freelist_insert ( cbbl_bdblk_recptr bbrecptr )
{
  int32 fli_rc = 0;

	/*
	 * set the specified element
	 */
  bbrecptr->list_type = free_list;
  bbrecptr->next = agg_recptr->free_bdblk_records;
  bbrecptr->prev = NULL;
	/*
	 * adjust the list to insert the specified element at the front
	 */
  if( agg_recptr->free_bdblk_records != NULL ) { /* list not empty */
    agg_recptr->free_bdblk_records->prev = bbrecptr; /* 
				* former 1st has new prev 
				*/
    }
  agg_recptr->free_bdblk_records = bbrecptr;

  return( fli_rc );
}                                      /* end freelist_insert    */


/****************************************************************************
 * NAME:	get_bdblk_record
 *
 * FUNCTION:	Allocate a bad block record and return it's address
 *
 * PARAMETERS:
 *      addr_bbrecptr	address of a variable in which to return 
 *			a pointer to the allocated record 
 *
 * NOTES:  
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 get_bdblk_record ( cbbl_bdblk_recptr *addr_bbrecptr )
{
  int32 gbbr_rc = 0;
	
  if( agg_recptr->free_bdblk_records != NULL ) {
    	*addr_bbrecptr = agg_recptr->free_bdblk_records;  /* take 1st */
	agg_recptr->free_bdblk_records = (*addr_bbrecptr)->next;  /* new 1st */
	if( agg_recptr->free_bdblk_records != NULL ) {     /* wasn't last */
	    agg_recptr->free_bdblk_records->prev = NULL;   /* 1st has no prev */
	    }
	(*addr_bbrecptr)->list_type = no_list;
	(*addr_bbrecptr)->next = NULL;
	(*addr_bbrecptr)->prev = NULL;
    	}
	
  else {  /* the free list is empty */
    	gbbr_rc = alloc_wrksp_cb( bdblk_record_length, (void **) addr_bbrecptr );
    	}
	/*
	 * initialize fields 
	 */
  if( gbbr_rc == 0 ) {  /* we got one */
  	memset( *addr_bbrecptr, 0, bdblk_record_length );
  	(*addr_bbrecptr)->left = NULL;
  	(*addr_bbrecptr)->right = NULL;
  	(*addr_bbrecptr)->next = NULL;
  	(*addr_bbrecptr)->prev = NULL;
  	(*addr_bbrecptr)->list_type = no_list;
  	(*addr_bbrecptr)->balance = 0;
  	(*addr_bbrecptr)->seen = 0;
  	(*addr_bbrecptr)->retry_list_count = 0;
  	(*addr_bbrecptr)->insuffspace_count = 0;
  	(*addr_bbrecptr)->relocated_count = 0;
  	(*addr_bbrecptr)->allocated_to_BBino = 0;
  	(*addr_bbrecptr)->LVM_notified = 0;
	}  /* end we got one */
	
  return( gbbr_rc );
}                                      /* end get_bdblk_record   */


/****************************************************************************
 * NAME:	retrylist_insert
 *
 * FUNCTION:	Insert the given entry into the bad block retry list
 *
 * PARAMETERS:
 *      bbrecptr	a pointer to the record to insert
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 retrylist_insert ( cbbl_bdblk_recptr bbrecptr )
{
  int32 rli_rc = 0;
	
	/*
	 * set the specified element
	 */
  bbrecptr->list_type = retry_list;
  bbrecptr->next = agg_recptr->bdblk_retry_list;
  bbrecptr->prev = NULL;
	/*
	 * adjust the list to insert the specified element at the front
	 */
  if( agg_recptr->bdblk_retry_list != NULL ) { /* list not empty */
    agg_recptr->bdblk_retry_list->prev = bbrecptr; /* 
				* former 1st has new prev 
				*/
    }
  agg_recptr->bdblk_retry_list = bbrecptr;

  return( rli_rc );
}                                      /* end retrylist_insert    */


/****************************************************************************
 * NAME:	retrylist_purge
 *
 * FUNCTION: 	Move any entries in the retry list into the balanced tree.
 *	     	Initialize the retry list.
 *
 * PARAMETERS:	none
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 retrylist_purge ( )
{
  int32 rlp_rc = 0;
  int32 blkno_idx = 0;
  cbbl_bdblk_recptr bblkrecptr;
  cbbl_bdblk_recptr new_bblkrecptr;
  cbbl_bdblk_recptr next_bblkrecptr;
	
  bblkrecptr = agg_recptr->bdblk_retry_list;
	
	/*
	 * move retry list entries into the balanced tree
	 */
  while( bblkrecptr != NULL ) {
    next_bblkrecptr = bblkrecptr->next;
    if( bblkrecptr->retry_list_count <= CLRBBLKS_RETRY_LIMIT ) { /*
				* we haven't passed the limit on retries
				* yet, so we'll put a record for it
				* into the tree
				*/
	baltree_search_insert( bblkrecptr->fs_blkno, &new_bblkrecptr );
	
		/* 
		 * initialize the new record
		 */
	new_bblkrecptr->retry_list_count = bblkrecptr->retry_list_count;
	new_bblkrecptr->staledata_count = bblkrecptr->staledata_count;
	new_bblkrecptr->insuffspace_count = bblkrecptr->insuffspace_count;
	new_bblkrecptr->tooslow_count = bblkrecptr->tooslow_count;
	new_bblkrecptr->relocated_count = bblkrecptr->relocated_count;
	for( blkno_idx = 0; blkno_idx < MAX_LVBLKS_PER_FSBLK; blkno_idx++ ) {
	    new_bblkrecptr->LV_blkno[blkno_idx] = 
					bblkrecptr->LV_blkno[blkno_idx];
	    }  /* end for */
	}  /* end we haven't passed the limit on retries ... */
	
	/* 
	 * release the old record to the freelist
	 */
    freelist_insert( bblkrecptr );
    bblkrecptr = next_bblkrecptr;
    }  /* end while */
	
	/*
	 * initialize the retry list
	 */
  agg_recptr->bdblk_retry_list = NULL;
	
  return( rlp_rc );
}                                      /* end retrylist_purge    */


/****************************************************************************
 * NAME:	rslvdlist_insert
 *
 * FUNCTION:	Insert the given entry into the resolved bad blocks list
 *
 * PARAMETERS:
 *      bbrecptr	a pointer to the record to insert
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 rslvdlist_insert ( cbbl_bdblk_recptr bbrecptr )
{
  int32 rli_rc = 0;
	
	/*
	 * set the specified element
	 */
  bbrecptr->list_type = resolved_list;
  bbrecptr->next = agg_recptr->resolved_bdblk_records;
  bbrecptr->prev = NULL;
	/*
	 * adjust the list to insert the specified element at the front
	 */
  if( agg_recptr->resolved_bdblk_records != NULL ) { /* list not empty */
    agg_recptr->resolved_bdblk_records->prev = bbrecptr; /* 
				* former 1st has new prev 
				*/
    }
  agg_recptr->resolved_bdblk_records = bbrecptr;
	
  return( rli_rc );
}                                      /* end rslvdlist_insert    */


/****************************************************************************
 * NAME:	seqlist_create
 *
 * FUNCTION: 	Create a linked list, with keys ascending, of the nodes 
 *		in the given balanced binary sort tree.
 *
 * PARAMETERS:
 *      thead	a pointer to the header record of the balanced 
 *		binary sort tree to be linked.
 *
 * NOTES:  
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 seqlist_create ( )
{
  int32 slc_rc = 0;
  cbbl_bdblk_recptr bblkrecptr;
  cbbl_bdblk_recptr prev_bblkrecptr;
  cbbl_bdblk_recptr *addr_prev_nextptr;

  agg_recptr->inobt_stack = NULL;  /* initialize the stack */

  addr_prev_nextptr = &(agg_recptr->bdblk_baltree.seq_list);
  bblkrecptr = agg_recptr->bdblk_baltree.right; /* the balanced tree root */
  prev_bblkrecptr = NULL;	    /* 1st has no previous */

  while( (bblkrecptr != NULL) && (slc_rc == 0) ) {

    bblkrecptr->seen = -1;

    if( (bblkrecptr->left != NULL) && (!(bblkrecptr->left->seen)) ) {  /*
                                 * the current element has a left child
                                 * which hasn't been visited yet
                                 */
      slc_rc = baltree_stack_push( bblkrecptr );
      bblkrecptr = bblkrecptr->left;
      }  /* end the current element has a left child which hasn't ... */

    else {  /* no unseen left children */
      /*
       * process the current element by adding it to the end of the
       * sequential list
       */
      *addr_prev_nextptr = bblkrecptr;
      addr_prev_nextptr = &(bblkrecptr->next);
      bblkrecptr->prev = prev_bblkrecptr;
      bblkrecptr->list_type = sequen_list;
      prev_bblkrecptr = bblkrecptr;

      /*
       * If the current element has a right child, process it next.
       * Otherwise, pop one off the stack.
       */
      if( bblkrecptr->right != NULL ) {
        bblkrecptr = bblkrecptr->right;
        }
      else {
        slc_rc = baltree_stack_pop( &bblkrecptr );
        }
      }  /* end no unseen left children */
    }  /* end while */

  *addr_prev_nextptr = NULL;         /* the next pointer in the last element */
	/*
	 * now create the sequential list of all tree elements.
	 * unlike the 'regular' sequential list, elements are not removed
	 * from this list when they are placed on the retry or resolved lists.
	 */
  bblkrecptr = agg_recptr->bdblk_baltree.seq_list;
  while( bblkrecptr != NULL ) {
      bblkrecptr->tree_next = bblkrecptr->next;
      bblkrecptr = bblkrecptr->next;
      }  /* end while */
	
  return( slc_rc );
}                                      /* end seqlist_create    */


/****************************************************************************
 * NAME:	seqlist_remove
 *
 * FUNCTION:	Remove the given entry from the sequential list of 
 *		bad block records. 
 *
 * PARAMETERS:
 *      bbrecptr	a pointer to the record in the sequential list
 *			which should be removed from the list
 *
 * NOTES:  
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 seqlist_remove ( cbbl_bdblk_recptr  bbrecptr )
{
  int32 slr_rc = 0;
	
	/*
	 * adjust the list to drop the specified element
	 */
  if( bbrecptr == agg_recptr->bdblk_baltree.seq_list ) { /* 
				* it's the 1st in list 
				*/
    agg_recptr->bdblk_baltree.seq_list = bbrecptr->next;
    if( agg_recptr->bdblk_baltree.seq_list != NULL ) {
      agg_recptr->bdblk_baltree.seq_list->prev = NULL;	/* 
				* new 1st has no prev 
				*/
      }
    }  /* end it's the 1st in list */
  else { /* not 1st in list */
    bbrecptr->prev->next = bbrecptr->next;
    if( bbrecptr->next != NULL ) {
      bbrecptr->next->prev = bbrecptr->prev;
      }
    }
	/*
	 * clear the specified element
	 */
  bbrecptr->list_type = no_list;
  bbrecptr->next = NULL;
  bbrecptr->prev = NULL;
	
  return( slr_rc );
}                                      /* end seqlist_remove    */
	
/*****************************************************************************
 * NAME:	workspace_release
 *
 * FUNCTION:  	Release the pool of storage allocated for fsck workspace
 *            	storage.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: 0
 *      failure: something else
 */
int32 workspace_release_cb ( )
{
  int32 wr_rc = 0;
  wsp_ext_rec_ptr this_fer, next_fer;
  char *extent_addr;
	
  /*
   * release the allocated extents
   */
  this_fer = agg_recptr->wsp_extent_list;
  while( this_fer != NULL ) {  /* for each extent record */
    next_fer = this_fer->next; /* the one after this one (if any) */
    free( (void *) this_fer->extent_addr );  /* free the extent this
                              * fer describes (and occupies)
                              */
    this_fer = next_fer;     /* go on to the next one in the list */
    }  /* end for each extent record */
	
  return( wr_rc );
}                              /* end of workspace_release ()  */
