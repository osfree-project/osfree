/* $Id: fsckxtre.c,v 1.2 2004/03/21 02:43:28 pasha Exp $ */

static char *SCCSID = "@(#)1.17  10/16/98 14:52:59 src/jfs/utils/chkdsk/fsckxtre.c, jfschk, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		fsckxtre.c
 *
 *   COMPONENT_NAME: 	jfschk
 *
 *
 *   FUNCTIONS:
 *              find_first_leaf
 *              init_xtree_root
 *              process_valid_data
 *              xTree_processing
 *              xTree_search
 *
 *              xTree_binsrch_page
 *              xTree_process_internal_extents
 *              xTree_node_first_key
 *              xTree_node_first_in_level
 *              xTree_node_last_in_level
 *              xTree_node_not_first_in_level
 *              xTree_node_not_last_in_level
 *              xTree_process_leaf_extents
 *
 */

/*
 * defines and includes common among the xfsck modules
 */
#include "xfsckint.h"


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * superblock buffer pointer
  *
  *      defined in xchkdsk.c
  */
extern struct superblock *sb_ptr;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * fsck aggregate info structure pointer
  *
  *      defined in xchkdsk.c
  */
extern struct fsck_agg_record *agg_recptr;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * For message processing
  *
  *      defined in xchkdsk.c
  */
extern char message_parm_0[];
extern char message_parm_1[];
extern char message_parm_2[];
extern char message_parm_3[];
extern char message_parm_4[];
extern char message_parm_5[];
extern char message_parm_6[];
extern char message_parm_7[];
extern char message_parm_8[];
extern char message_parm_9[];

extern char *msgprms[];
extern int16 msgprmidx[];

extern char *terse_msg_ptr;
extern char *verbose_msg_ptr;

extern char *MsgText[];

extern char *Vol_Label;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * Device information.
  *
  *      defined in xchkdsk.c
  */
extern HFILE  Dev_IOPort;
extern uint32 Dev_blksize;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * The current processing phase.
  *
  *      defined in xchkdsk.c
  *      constants defining possible values are defined in xfsck.h
  */
extern int current_fsck_phase;


/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 *
 * The following are internal to this file
 *
 */

typedef struct fsck_Xtree_info {
     xtpage_t       *xtp_ptr;
     xad_t          *xad_ptr;
     fsblkidx_t      ext_addr;
     reg_idx_t       ext_length;
     treeQ_elem_ptr  this_Qel;
     treeQ_elem_ptr  next_Qel;
     fsblkidx_t      this_key;
     fsblkidx_t      last_key;
     fsblkidx_t      last_node_addr;
     int8            last_level;
     int8            dense_file;
     int8            leaf_seen;
     } fsck_Xtree_info_t;
typedef fsck_Xtree_info_t *fsck_Xtree_info_ptr;


retcode_t xTree_binsrch_page( xtpage_t *,fsblkidx_t,int8 *,int16 *,int8 * );

retcode_t xTree_process_internal_extents ( xtpage_t *, fsck_inode_recptr,
                                           treeQ_elem_ptr, fsck_msg_info_ptr,
                                           int );

retcode_t xTree_node_first_key ( fsck_Xtree_info_ptr, fsck_inode_recptr,
                                 fsck_msg_info_ptr, int );

retcode_t xTree_node_first_in_level ( fsck_Xtree_info_ptr,
                                          fsck_inode_recptr,
                                          fsck_msg_info_ptr,
                                          int );

retcode_t xTree_node_last_in_level ( fsck_Xtree_info_ptr,
                                         fsck_inode_recptr,
                                         fsck_msg_info_ptr,
                                         int );

retcode_t xTree_node_not_first_in_level ( fsck_Xtree_info_ptr,
                                          fsck_inode_recptr,
                                          fsck_msg_info_ptr,
                                          int );

retcode_t xTree_node_not_last_in_level ( fsck_Xtree_info_ptr,
                                         fsck_inode_recptr,
                                         fsck_msg_info_ptr,
                                         int );

retcode_t xTree_process_leaf_extents ( xtpage_t *, fsck_inode_recptr,
                                       treeQ_elem_ptr, fsck_msg_info_ptr,
                                       int8, int );

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */


/*****************************************************************************
 * NAME: find_first_leaf
 *
 * FUNCTION:  Get the ordinal number of the aggregate block containing the
 *            first leaf node in the B+ Tree, type xTree, rooted in the 
 *            given inode.
 *
 * PARAMETERS:
 *      inoptr           - input - pointer to the inode in which the xTree 
 *                                 is rooted
 *      addr_leaf_ptr    - input - pointer to a variable in which to return
 *                                 the address of the leaf in an fsck buffer.
 *      leaf_agg_offset  - input - offset, from the beginning of the 
 *                                 aggregate, in aggregate blocks, of the
 *                                 leftmost leaf in the xTree 
 *      is_inline        - input - pointer to a variable in which to return:
 *                                 !0 if the inode's data is inline (no leaf)
 *                                  0 if the inode's data is not inline
 *      is_rootleaf      - input - pointer to a variable in which to return:
 *                                 !0 if the xTree's root node is a leaf
 *                                  0 if the xTree's root node is an internal
 *                                        node
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t find_first_leaf( dinode_t    *inoptr,
                           xtpage_t   **addr_leaf_ptr,
                           fsblkidx_t  *leaf_agg_offset,
                           int8        *is_inline,
                           int8        *is_rootleaf
                          )
{
  retcode_t ffl_rc = FSCK_OK;
  xtpage_t *xtpg_ptr;
  xad_t    *xad_ptr;
  fsblkidx_t first_child_addr;

  *is_rootleaf = 0;                /* assume inode has no data */
  *is_inline = 0;                  /* assume inode has no data */
  *addr_leaf_ptr = NULL;           /* assume inode has no data */
  *leaf_agg_offset = 0;            /* assume inode has no data */
  xtpg_ptr = (xtpage_t *) &(inoptr->di_btroot);

  if( xtpg_ptr->header.flag & BT_LEAF )  {  /* it's a root-leaf */
    *is_rootleaf = -1;
    *leaf_agg_offset = addressPXD(&(inoptr->di_ixpxd));
    }  /* end it's a root-leaf */

  else {  /* it's a tree */

    while( (ffl_rc == FSCK_OK) &&
           (*addr_leaf_ptr == NULL)      )  {
      if( xtpg_ptr->header.flag & BT_LEAF )  {  /* found it!  */
        *addr_leaf_ptr = xtpg_ptr;
        *leaf_agg_offset = first_child_addr;
        }  /* end found it!  */
      else {  /* keep moving down the tree */
        xad_ptr = &(xtpg_ptr->xad[XTENTRYSTART]);
        first_child_addr = addressXAD( xad_ptr );
        ffl_rc = node_get( first_child_addr, &xtpg_ptr );
        }  /* end else keep moving down the tree */
      }  /* end while */
    }  /* end it's a tree */

  return( ffl_rc );
}                              /* end of find_first_leaf ()  */


/*****************************************************************************
 * NAME: init_xtree_root
 *
 * FUNCTION:  Initialize the btroot in the given inode as an empty (big) 
 *                 xtree root.  Adjust di_nblocks and di_size to match.
 *
 * PARAMETERS:
 *      inoptr           - input - pointer to the inode in which the xTree 
 *                                 root should be initialized
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t init_xtree_root( dinode_t    *inoptr )
{
  retcode_t ixr_rc = FSCK_OK;
  xtpage_t *xtpg_ptr;
	 
  xtpg_ptr = (xtpage_t *) &(inoptr->di_btroot);
  xtpg_ptr->header.flag = (DXD_INDEX|BT_ROOT|BT_LEAF);
  xtpg_ptr->header.maxentry = XTROOTMAXSLOT;
  xtpg_ptr->header.nextindex = XTENTRYSTART;
  inoptr->di_nblocks = 0;
  inoptr->di_size = 0;
	 
  return( ixr_rc );
}                              /* end of init_xtree_root ()  */


/*****************************************************************************
 * NAME: process_valid_data
 *
 * FUNCTION:  Perform the desired action on the xTree rooted in the given 
 *            inode, assume that the xTree has a valid structure.  (I.e.,
 *            that the tree has already been validated.)
 *
 * PARAMETERS:
 *      inoptr          - input - pointer to the inode in which the xTree is
 *                                rooted
 *      inoidx          - input - ordinal number of the inode
 *      inorecptr       - input - pointer to the fsck inode record describing
 *                                the inode in which the xTree is rooted
 *      msg_info_ptr    - input - pointer to data needed for messages about
 *                                the inode in which the xTree is rooted
 *      desired_action  - input - { FSCK_RECORD | FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t process_valid_data  ( dinode_t          *inoptr,
                                inoidx_t           inoidx,
                                fsck_inode_recptr  inorecptr,
                                fsck_msg_info_ptr  msg_info_ptr,
                                int                desired_action
                               )
{
  retcode_t pvd_rc = FSCK_OK;
  int xad_idx;
  xtpage_t *xtpage_ptr;
  xtpage_t *this_xtpage;
  xad_t    *xad_ptr;
  fsblkidx_t node_addr_fsblks;
  fsblkidx_t first_child_addr;
  fsblkidx_t first_fsblk;
  fsblkidx_t num_fsblks;
  dxd_t *dxd_ptr;
  int8 extent_is_valid;
  int8 is_EA = 0;
  int8 is_ACL = 0;
  int8 is_rootnode;
  reg_idx_t block_count;

  xtpage_ptr = (xtpage_t *) &(inoptr->di_btroot);
  is_rootnode = -1;

if( xtpage_ptr->header.flag != 0 ) {  /* there is data for this inode */

  while( (pvd_rc == FSCK_OK) &&
         (xtpage_ptr != NULL ) ) {  /* while not done processing the tree */
        /*
         * this node is a first child.  if it isn't a leaf, get
         * the address of its first child
         */
    if( xtpage_ptr->header.flag & BT_LEAF )  {  /* it's a leaf */
      first_child_addr = 0;
      }  /* end it's a leaf */
    else {  /* else it's not a leaf */
      xad_ptr = &(xtpage_ptr->xad[XTENTRYSTART]);    /* the first child */
      first_child_addr = addressXAD( xad_ptr );
      }  /* end else it's not a leaf */

        /*
         * process the current level
         */
    this_xtpage = xtpage_ptr;  /* first node in the level */
    while( (pvd_rc == FSCK_OK) &&
           (this_xtpage != NULL) ) {  /* process all nodes on the level */
      for( xad_idx = XTENTRYSTART;
           ( (xad_idx < xtpage_ptr->header.nextindex) &&
             (pvd_rc == FSCK_OK) );
           xad_idx++ ) {  /* for each xad in the xtpage */
        xad_ptr = &(this_xtpage->xad[xad_idx]);
        first_fsblk = addressXAD( xad_ptr );
        num_fsblks = lengthXAD( xad_ptr );
        pvd_rc = process_extent( inorecptr, num_fsblks, first_fsblk,
                                 is_EA, is_ACL, msg_info_ptr, &block_count, 
                                 &extent_is_valid, desired_action );
        if( (desired_action == FSCK_RECORD)         || 
            (desired_action == FSCK_RECORD_DUPCHECK)  ) {
          agg_recptr->blocks_this_fset += block_count;
          agg_recptr->this_inode.all_blks += block_count;
          if( first_child_addr == 0 )  {  /* this is a leaf */
            agg_recptr->this_inode.data_blks += block_count;
            }  /* end this is a leaf */
          }
        else if( desired_action == FSCK_UNRECORD ) {
          agg_recptr->blocks_this_fset -= block_count;
          agg_recptr->this_inode.all_blks -= block_count;
          if( first_child_addr == 0 )  {  /* this is a leaf */
            agg_recptr->this_inode.data_blks -= block_count;
            }  /* end this is a leaf */
          }
        }  /* end for each xad in the xtpage */

      if( is_rootnode ) {  /* root has no siblings */
        is_rootnode = 0;
        this_xtpage = NULL;
        }  /* end root has no siblings */
      else if( this_xtpage->header.next == ((int64)0) ) {  /* 
                          * this is rightmost 
                          */
        this_xtpage = NULL;
        }  /* end this is rightmost */
      else {  /* else there is a right sibling/cousin */
        node_addr_fsblks = this_xtpage->header.next;
        pvd_rc = node_get( node_addr_fsblks, &this_xtpage );
        }  /* end else there is a right sibling/cousin */
      }  /* end process all nodes on the level */

        /*
         * if not done, go down to the next level of the tree
         */
    if( first_child_addr == 0 )  {  /* done! */
      xtpage_ptr = NULL;
      }  /* end done! */
    else {  /* get the first child/cousin in the next level */
      pvd_rc = node_get( first_child_addr, &xtpage_ptr );
      }  /* end get the first child/cousin in the next level */
    }  /* end while not done processing the tree */
  }  /* end there is data for this inode */

 return( pvd_rc );
}                              /* end of process_valid_data ()  */


/*****************************************************************************
 * NAME: xTree_binsrch_page
 *
 * FUNCTION:  Perform a binary search on the xad's in the given xTree node
 *
 * PARAMETERS:
 *      xtpg_ptr       - input - pointer to the xTree node to search
 *      given_offset   - input - offset to match to an xad key
 *      xad_selected   - input - pointer to a variable in which to return:
 *                               !0 if the search was successful
 *                                0 if the search was not successful
 *      selected_idx   - input - the ordinal value of xad, within the node,
 *                               of the xad whose key matches given_offset 
 *                               (if any)
 *      not_allocated  - input - * currently unused *
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t xTree_binsrch_page( xtpage_t   *xtpg_ptr,
                              fsblkidx_t  given_offset,
                              int8       *xad_selected,
                              int16      *selected_idx,
                              int8       *not_allocated
                             )
{
  retcode_t xbp_rc = FSCK_OK;
  int16      lowidx, mididx, highidx;
  fsblkidx_t this_offset;

  lowidx = XTENTRYSTART;
  highidx = xtpg_ptr->header.nextindex - 1;
  *xad_selected = 0;

  while( (!(*xad_selected)) && (xbp_rc == FSCK_OK) ) {

    if( (highidx == lowidx) || ((highidx - lowidx) == 1) ){  /* 
                         * at most 1 apart 
                         */
      if( given_offset < offsetXAD(&(xtpg_ptr->xad[highidx])) ) {
        *selected_idx = lowidx;
        *xad_selected = -1;
        }
      else {
        *selected_idx = highidx;
        *xad_selected = -1;
        }
      }  /* end at most 1 apart */

    else {  /* far enough apart to continue algorithm */

      mididx = ((highidx - lowidx) >> 1) + lowidx;
      this_offset = offsetXAD(&(xtpg_ptr->xad[mididx]));

      if( given_offset == this_offset ) { /* it's a match */
        *selected_idx = mididx;
        *xad_selected = -1;
        }  /* end it's a match */
      else if( given_offset < this_offset ) {  /* this one is greater */
        if( given_offset > offsetXAD(&(xtpg_ptr->xad[mididx - 1])) ) { /*
                         * and the one before this one is less
                         */
          *selected_idx = mididx - 1;
          *xad_selected = -1;
          }  /* end and the one before this one is less */
        else { /* the one before is not less */
          highidx = mididx;       /* reset the range */
          }  /* end else the one before is not less */
        }  /* end this one is greater */
      else {  /* this one is less */
        if( given_offset < offsetXAD(&(xtpg_ptr->xad[mididx + 1])) ) { /*
                         * and the one after this one is greater
                         */
          *selected_idx = mididx;
          *xad_selected = -1;
          }  /* end and the one before this one is less */
        else { /* the one after is not greater */
          lowidx = mididx;       /* reset the range */
          }  /* end else the one before is not less */
        }  /* end this one is less */
      }  /* end else far enough apart to continue algorithm */
    }  /* end while !xad_selected */

  return( xbp_rc );
}                              /* end of xTree_binsrch_page ()  */


/*****************************************************************************
 * NAME: xTree_node_first_key
 *
 * FUNCTION:  Helper routine for xTree_processing
 *
 * PARAMETERS:
 *      xtiptr          - input - pointer to an fsck record describing the
 *                                xTree
 *      inorecptr       - input - pointer to the fsck inode record describing
 *                                the inode in which the xTree is rooted
 *      msg_info_ptr    - input - pointer to data needed for messages about 
 *                                the inode in which the xTree is rooted
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t xTree_node_first_key ( fsck_Xtree_info_ptr  xtiptr,
                                 fsck_inode_recptr    inorecptr,
                                 fsck_msg_info_ptr    msg_info_ptr,
                                 int                  desired_action
                                )
{
 retcode_t xnfk_rc = FSCK_OK;

      /*
       * the key in the 1st xad must match the key in the parent
       * node's xad describing this node
       */
  if( xtiptr->this_key != xtiptr->this_Qel->node_first_offset ) {  /*
                             * invalid key in 1st xad
                             */
    inorecptr->ignore_alloc_blks = 1;
    if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported yet */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_inotyp;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = msg_info_ptr->msg_inopfx;
      sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%d", 5 );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      fsck_send_msg( fsck_BADKEYS, 0, 4 );
      }  /* end not reported yet */
    }  /* end invalid key in 1st xad */

  else {  /* 1st xad might be ok */
    if( xtiptr->last_level == xtiptr->this_Qel->node_level ) {  /*
                             * not 1st in level
                             */
      if( (xtiptr->dense_file) &&
          (xtiptr->xtp_ptr->header.flag & BT_LEAF) ) {  /*
                             * a leaf node in a dense file
                             */
        if( xtiptr->this_key != (xtiptr->last_key + 1) ) {  /*
                             * a gap in a dense file
                             */
          inorecptr->ignore_alloc_blks = 1;
          if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* 
                             * not reported yet 
                             */
            msgprms[0] = message_parm_0;
            msgprmidx[0] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            sprintf( message_parm_2, "%lld", xtiptr->last_key );
            msgprms[2] = message_parm_2;
            msgprmidx[2] = 0;
            fsck_send_msg( fsck_BADINOINTERNGAP, 0, 3 );
            }  /* end not reported yet */
          }  /* end a gap in a dense file */
        }  /* end a leaf node in a dense file */

      else {  /* not a leaf node in a dense file */
        if( xtiptr->this_key <= xtiptr->last_key )  {  /*
                             * the extents overlap!
                             */
          inorecptr->ignore_alloc_blks = 1;
          if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
                             * not reported yet
                             */
            msgprms[0] = message_parm_0;
            msgprmidx[0] = msg_info_ptr->msg_inotyp;
            msgprms[1] = message_parm_1;
            msgprmidx[1] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
            msgprms[2] = message_parm_2;
            msgprmidx[2] = 0;
            sprintf( message_parm_3, "%d", 6 );
            msgprms[3] = message_parm_3;
            msgprmidx[3] = 0;
            fsck_send_msg( fsck_BADKEYS, 0, 4 );
            }  /* end not reported yet */
          }  /* end the extents overlap! */
        }  /* end else not a leaf node in a dense file */
      }  /* end not 1st in level */
    }  /* end 1st xad might be ok */

 return( xnfk_rc );
}                              /* end of xTree_node_first_key ()  */


/*****************************************************************************
 * NAME: xTree_node_first_in_level
 *
 * FUNCTION:  Helper routine for xTree_processing
 *
 * PARAMETERS:
 *      xtiptr          - input - pointer to an fsck record describing the
 *                                xTree
 *      inorecptr       - input - pointer to the fsck inode record describing
 *                                the inode in which the xTree is rooted
 *      msg_info_ptr    - input - pointer to data needed for messages about 
 *                                the inode in which the xTree is rooted
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t xTree_node_first_in_level ( fsck_Xtree_info_ptr  xtiptr,
                                          fsck_inode_recptr    inorecptr,
                                          fsck_msg_info_ptr    msg_info_ptr,
                                          int                  desired_action
                                         )
{
 retcode_t xnfil_rc = FSCK_OK;

  if( xtiptr->xtp_ptr->header.prev != 0 ) {  /*
                             * bad back ptr!
                             */
    inorecptr->ignore_alloc_blks = 1;

    if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported yet */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_inotyp;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = msg_info_ptr->msg_inopfx;
      sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum);
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_BADBSBLCHN, 0, 3 );
      }  /* end not reported yet */
    }  /* end bad back ptr! */

 return( xnfil_rc );
}                         /* end of xTree_node_first_in_level ()  */


/*****************************************************************************
 * NAME: xTree_node_last_in_level
 *
 * FUNCTION:  Helper routine for xTree_processing
 *
 * PARAMETERS:
 *      xtiptr          - input - pointer to an fsck record describing the
 *                                xTree
 *      inorecptr       - input - pointer to the fsck inode record describing
 *                                the inode in which the xTree is rooted
 *      msg_info_ptr    - input - pointer to data needed for messages about
 *                                the inode in which the xTree is rooted
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t xTree_node_last_in_level ( fsck_Xtree_info_ptr  xtiptr,
                                         fsck_inode_recptr    inorecptr,
                                         fsck_msg_info_ptr    msg_info_ptr,
                                         int                  desired_action
                                        )
{
 retcode_t xnlil_rc = FSCK_OK;

  if( xtiptr->xtp_ptr->header.next != 0 ) {  /*
                             * bad forward ptr!
                             */
    inorecptr->ignore_alloc_blks = 1;
    if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
                             * not reported yet
                             */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_inotyp;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = msg_info_ptr->msg_inopfx;
      sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_BADFSBLCHN, 0, 3 );
      }  /* end not reported yet */
    }  /* end bad forward ptr!  */

 return( xnlil_rc );
}                         /* end of xTree_node_last_in_level ()  */


/*****************************************************************************
 * NAME: xTree_node_not_first_in_level
 *
 * FUNCTION:  Helper routine for xTree_processing
 *
 * PARAMETERS:
 *      xtiptr          - input - pointer to an fsck record describing the
 *                                xTree
 *      inorecptr       - input - pointer to the fsck inode record describing
 *                                the inode in which the xTree is rooted
 *      msg_info_ptr    - input - pointer to data needed for messages about 
 *                                the inode in which the xTree is rooted
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t xTree_node_not_first_in_level ( fsck_Xtree_info_ptr  xtiptr,
                                          fsck_inode_recptr    inorecptr,
                                          fsck_msg_info_ptr    msg_info_ptr,
                                          int                  desired_action
                                         )
{
 retcode_t xnnfil_rc = FSCK_OK;

  if( xtiptr->xtp_ptr->header.prev != xtiptr->last_node_addr ) {  /*
                             * bad back ptr!
                             */
    inorecptr->ignore_alloc_blks = 1;

    if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported yet */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_inotyp;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = msg_info_ptr->msg_inopfx;
      sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum);
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_BADBSBLCHN, 0, 3 );
      }  /* end not reported yet */
    }  /* end bad back ptr! */

 return( xnnfil_rc );
}                         /* end of xTree_node_not_first_in_level ()  */


/*****************************************************************************
 * NAME: xTree_node_not_last_in_level
 *
 * FUNCTION:  Helper routine for xTree_processing
 *
 * PARAMETERS:
 *      xtiptr          - input - pointer to an fsck record describing the
 *                                xTree
 *      inorecptr       - input - pointer to the fsck inode record describing
 *                                the inode in which the xTree is rooted
 *      msg_info_ptr    - input - pointer to data needed for messages about
 *                                the inode in which the xTree is rooted
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t xTree_node_not_last_in_level ( fsck_Xtree_info_ptr  xtiptr,
                                         fsck_inode_recptr    inorecptr,
                                         fsck_msg_info_ptr    msg_info_ptr,
                                         int                  desired_action
                                        )
{
 retcode_t xnnlil_rc = FSCK_OK;

  if( xtiptr->xtp_ptr->header.next != xtiptr->next_Qel->node_addr ) {  /*
                             * bad forward ptr!
                             */
    inorecptr->ignore_alloc_blks = 1;
    if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
                             * not reported yet
                             */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_inotyp;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = msg_info_ptr->msg_inopfx;
      sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_BADFSBLCHN, 0, 3 );
      }  /* end not reported yet */
    }  /* end bad forward ptr!  */

  else {  /* forward sibling pointer is correct */

    if( xtiptr->this_Qel->last_ext_uneven )  {  /* last extent described
                             * by this node is not an even number of
                             * 4096 pages but it can't be the last extent
                             * allocated to the inode
                             */
      inorecptr->ignore_alloc_blks = 1;
      if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
                             * not reported yet
                             */
        xtiptr->xad_ptr = &( xtiptr->xtp_ptr->xad[
                                   xtiptr->xtp_ptr->header.nextindex - 1
                                                    ] );
        xtiptr->this_key = offsetXAD(xtiptr->xad_ptr);
        xtiptr->ext_length = lengthXAD(xtiptr->xad_ptr);
        msgprms[0] = message_parm_0;
        msgprmidx[0] = msg_info_ptr->msg_inotyp;
        msgprms[1] = message_parm_1;
        msgprmidx[1] = msg_info_ptr->msg_inopfx;
        sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", xtiptr->this_key );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", xtiptr->ext_length );
        msgprms[4] = message_parm_4;
        msgprmidx[4] = 0;
        fsck_send_msg( fsck_BADINOODDINTRNEXT, 0, 5 );
        }  /* end not reported yet */
      }  /* end last extent described by this node is ... */
    }  /* end else forward sibling pointer is correct */

 return( xnnlil_rc );
}                         /* end of xTree_node_not_last_in_level ()  */


/*****************************************************************************
 * NAME: xTree_process_internal_extents
 *
 * FUNCTION:  Helper routine for xTree_processing
 *
 * PARAMETERS:
 *      xtpg_ptr        - input - pointer to the internal node in an fsck
 *                                buffer
 *      ino_recptr      - input - pointer to the fsck inode record describing
 *                                the inode in which the xTree is rooted
 *      Q_elptr         - input - address of an fsck Q element pointer 
 *                                describing the internal node
 *      msg_info_ptr    - input - pointer to data needed for messages about
 *                                the inode in which the xTree is rooted
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t xTree_process_internal_extents ( xtpage_t          *xtpg_ptr,
                                           fsck_inode_recptr  ino_recptr,
                                           treeQ_elem_ptr     Q_elptr,
                                           fsck_msg_info_ptr  msg_info_ptr,
                                           int                desired_action
                                          )
{
 retcode_t xpie_rc = FSCK_OK;
 fsblkidx_t last_key, this_key;
 reg_idx_t xadidx;
 treeQ_elem_ptr new_Qelptr;
 reg_idx_t ext_length, adjusted_length;
 fsblkidx_t ext_addr;
 int8 ext_ok;
 int8 is_EA = 0;
 int8 is_ACL = 0;
 xad_t *xad_ptr;
 uint8  flag_mask;
	 
 flag_mask = ~(XAD_NEW|XAD_EXTENDED|XAD_COMPRESSED|
                    XAD_NOTRECORDED|XAD_COW);

 last_key = -1;

 for( xadidx = XTENTRYSTART;
      ( (xadidx < xtpg_ptr->header.nextindex) &&
        (xpie_rc == FSCK_OK)           &&
        (!ino_recptr->ignore_alloc_blks)          );
      xadidx++
     )  {
   xad_ptr = &(xtpg_ptr->xad[xadidx]);

   if( (xad_ptr->flag & flag_mask) ) {  /* bad flag value */
     ino_recptr->ignore_alloc_blks = 1;
     }
	  
   this_key = offsetXAD(xad_ptr);
   if( !ino_recptr->ignore_alloc_blks )  {  
     if( this_key <= last_key ) {  /* these keys MUST ascend */
       ino_recptr->ignore_alloc_blks = 1;
       if( desired_action == FSCK_RECORD_DUPCHECK )  {  /* first detection */
         msgprms[0] = message_parm_0;
         msgprmidx[0] = msg_info_ptr->msg_inotyp;
         msgprms[1] = message_parm_1;
         msgprmidx[1] = msg_info_ptr->msg_inopfx;
         sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
         msgprms[2] = message_parm_2;
         msgprmidx[2] = 0;
         sprintf( message_parm_3, "%d", 7 );
         msgprms[3] = message_parm_3;
         msgprmidx[3] = 0;
         fsck_send_msg( fsck_BADKEYS, 0, 4 );
         }  /* end first detection */
       }  /* end these keys MUST ascend */
     else {  /* key looks ok from here */
       last_key = this_key;

       ext_addr = addressXAD(xad_ptr);
       ext_length = lengthXAD(xad_ptr);
       xpie_rc = process_extent( ino_recptr, ext_length, ext_addr,
                                            is_EA, is_ACL, msg_info_ptr, &adjusted_length, 
                                            &ext_ok, desired_action );
       if( (xpie_rc == FSCK_OK) &&
           (ext_ok) ) {  /* extent is good */
  
         if( (desired_action == FSCK_RECORD)         || 
             (desired_action == FSCK_RECORD_DUPCHECK)  ) {
           agg_recptr->blocks_this_fset += adjusted_length;
           agg_recptr->this_inode.all_blks += adjusted_length;
           } 
         else if( desired_action == FSCK_UNRECORD ) {
           agg_recptr->blocks_this_fset -= adjusted_length;
           agg_recptr->this_inode.all_blks -= adjusted_length;
           }

         xpie_rc = treeQ_get_elem( &new_Qelptr );
         if( xpie_rc == FSCK_OK ) {  /* got a queue element */
           new_Qelptr->node_level = Q_elptr->node_level + 1;
           new_Qelptr->node_addr = ext_addr;
           PXDaddress(&(new_Qelptr->node_pxd),ext_addr);
           PXDlength(&(new_Qelptr->node_pxd),ext_length);
           new_Qelptr->node_first_offset = this_key;
           xpie_rc = treeQ_enqueue( new_Qelptr );
           }  /* end got a queue element */
         }  /* end extent is good */
       }  /* end else key looks ok from here */
     }    
   }  /* end for xadidx */

 return( xpie_rc );
}                         /* end of xTree_process_internal_extents ()  */


/*****************************************************************************
 * NAME: xTree_process_leaf_extents
 *
 * FUNCTION:  Helper routine for xTree_processing
 *
 * PARAMETERS:
 *      xtpg_ptr        - input - pointer to the leaf node in an fsck buffer
 *      inorecptr       - input - pointer to the fsck inode record describing
 *                                the inode in which the xTree is rooted
 *      Q_elptr         - input - address of an fsck Q element pointer 
 *                                describing the leaf
 *      msg_info_ptr    - input - pointer to data needed for messages about the 
 *                                inode in which the xTree is rooted
 *      dense_file      - input - !0 => the xTree describes a dense file
 *                                 0 => the xTree describes a file which may 
 *                                      be sparse
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t xTree_process_leaf_extents ( xtpage_t          *xtpg_ptr,
                                       fsck_inode_recptr  ino_recptr,
                                       treeQ_elem_ptr     Q_elptr,
                                       fsck_msg_info_ptr  msg_info_ptr,
                                       int8               dense_file,
                                       int                desired_action
                                      )
{
 retcode_t xple_rc = FSCK_OK;
 fsblkidx_t last_key, this_key;
 reg_idx_t xadidx;
 reg_idx_t ext_length, adjusted_length;
 fsblkidx_t ext_addr;
 int8 ext_ok;
 int8 is_EA = 0;
 int8 is_ACL = 0;
 xad_t *xad_ptr;
 reg_idx_t ext_pages;
 uint8 flag_mask;
	 
 flag_mask = ~(XAD_NEW|XAD_EXTENDED|XAD_COMPRESSED|
                    XAD_NOTRECORDED|XAD_COW);
	 
 last_key = -1;

 for( xadidx = XTENTRYSTART;
      ( (xadidx < xtpg_ptr->header.nextindex) &&
        (xple_rc == FSCK_OK)           &&
        (!ino_recptr->ignore_alloc_blks)         );
      xadidx++
     ) {

   xad_ptr = &(xtpg_ptr->xad[xadidx]);

   if( (xad_ptr->flag & flag_mask) ) {  /* bad flag value */
     ino_recptr->ignore_alloc_blks = 1;
     }
	  
   this_key = offsetXAD(xad_ptr);
	  
   if( (last_key != -1) && (!ino_recptr->ignore_alloc_blks) )  {  /* 
                                * not the first key 
                                */
     if( this_key <= last_key ) {  /* these keys MUST ascend */
       ino_recptr->ignore_alloc_blks = 1;
       if( desired_action == FSCK_RECORD_DUPCHECK )  {  /* first detection */
         msgprms[0] = message_parm_0;
         msgprmidx[0] = msg_info_ptr->msg_inotyp;
         msgprms[1] = message_parm_1;
         msgprmidx[1] = msg_info_ptr->msg_inopfx;
         sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
         msgprms[2] = message_parm_2;
         msgprmidx[2] = 0;
         sprintf( message_parm_3, "%d", 8 );
         msgprms[3] = message_parm_3;
         msgprmidx[3] = 0;
         fsck_send_msg( fsck_BADKEYS, 0, 4 );
         }  /* end first detection */
       }  /* end these keys MUST ascend */
     else {  /* the keys do ascend */
       if( (dense_file) && (this_key != (last_key + 1)) ) {  /*
                                * a dense file with a gap!
                                */
         ino_recptr->ignore_alloc_blks = 1;
         if( desired_action == FSCK_RECORD_DUPCHECK )  {  /* first detection */
           msgprms[0] = message_parm_0;
           msgprmidx[0] = msg_info_ptr->msg_inopfx;
           sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
           msgprms[1] = message_parm_1;
           msgprmidx[1] = 0;
           sprintf( message_parm_2, "%lld", msg_info_ptr->msg_inonum );
           msgprms[2] = message_parm_2;
           msgprmidx[2] = 0;
           fsck_send_msg( fsck_BADINOINTERNGAP, 0, 3 );
           }  /* end first detection */
         }  /* end a dense file with a gap! */
       }  /* end else the keys do ascend */
     }  /* end not the first key */

   if( !ino_recptr->ignore_alloc_blks ) {  /* no problems detected yet */
     ext_addr = addressXAD(xad_ptr);
     ext_length = lengthXAD(xad_ptr);
     last_key = this_key + ext_length - 1;
     agg_recptr->this_inode.data_size = (last_key + 1) * sb_ptr->s_bsize;

        /*
         * all extents (except the very last one for the inode) must
         * be in full (4096 byte) pages.
         */
     ext_pages = ext_length >> agg_recptr->log2_blksperpg;
     if( (ext_length != (ext_pages << agg_recptr->log2_blksperpg)) &&
	 (!(ino_recptr->badblk_inode))  )  {  /*
                                * this one is an odd size and isn't 
				* owned by the bad block inode
                                */
       if( xadidx == (xtpg_ptr->header.nextindex - 1) ) {  /*
                                * this is the last extent for the node
                                * and might be the last for the inode
                                */
         Q_elptr-> last_ext_uneven = -1;  /* flag it */
         }  /* end this is the last extent for the node and might be ... */
       else {  /* not the last extent for the xtpage */
         ino_recptr->ignore_alloc_blks = 1;
         if( desired_action == FSCK_RECORD_DUPCHECK )  {  /* first detection */
           msgprms[0] = message_parm_0;
           msgprmidx[0] = msg_info_ptr->msg_inotyp;
           msgprms[1] = message_parm_1;
           msgprmidx[1] = msg_info_ptr->msg_inopfx;
           sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
           msgprms[2] = message_parm_2;
           msgprmidx[2] = 0;
           sprintf( message_parm_3, "%lld", this_key );
           msgprms[3] = message_parm_3;
           msgprmidx[3] = 0;
           sprintf( message_parm_4, "%ld", ext_length );
           msgprms[4] = message_parm_4;
           msgprmidx[4] = 0;
           fsck_send_msg( fsck_BADINOODDINTRNEXT, 0, 5 );
           }  /* end first detection */
         }  /* end else not the last extent for the xtpage */
       }  /* end this one is an odd size and isn't owned by the ... */

     if( !ino_recptr->ignore_alloc_blks ) {  /* so far so good */
       xple_rc = process_extent( ino_recptr, ext_length, ext_addr,
                                 is_EA, is_ACL, msg_info_ptr, &adjusted_length, 
                                 &ext_ok, desired_action );
       if( (desired_action == FSCK_RECORD)         || 
           (desired_action == FSCK_RECORD_DUPCHECK)  ) {
         agg_recptr->blocks_this_fset += adjusted_length;
         agg_recptr->this_inode.all_blks += adjusted_length;
         agg_recptr->this_inode.data_blks += adjusted_length;
         }
       else if( desired_action == FSCK_UNRECORD ) {
         agg_recptr->blocks_this_fset -= adjusted_length;
         agg_recptr->this_inode.all_blks -= adjusted_length;
         agg_recptr->this_inode.data_blks -= adjusted_length;
         }
       }  /* end so far so good */
     }  /* end no problems detected yet */
   }  /* end for xadidx */

 return( xple_rc );
}                              /* end of xTree_process_leaf_extents ()  */


/*****************************************************************************
 * NAME: xTree_processing
 *
 * FUNCTION: Validate the structure of the xTree rooted in the given inode
 *           and perform the desired_action on the nodes in the xTree.
 *           Stop processing the xTree if and when any symptom of corruption
 *           is detected.
 *
 * PARAMETERS:             
 *      inoptr        - input - pointer to the inode in which the xTree is
 *                                   rooted
 *      inoidx        - input - ordinal number of the inode
 *      inorecptr    - input - pointer to the fsck inode record describing
 *                                   the inode
 *      msg_info_ptr    - input - pointer to data needed for messages about
 *                                        the inode
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                           FSCK_UNRECORD | FSCK_QUERY |  
 *                                           FSCK_FSIM_RECORD_DUPCHECK  |
 *                                           FSCK_FSIM_UNRECORD | FSCK_FSIM_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t xTree_processing ( dinode_t          *inoptr,
                             inoidx_t           inoidx,
                             fsck_inode_recptr  inorecptr,
                             fsck_msg_info_ptr  msg_info_ptr,
                             int                desired_action
                           )
{
 retcode_t xp_rc = FSCK_OK;
 int8 old_ignore_alloc_blks;
 int ixpxd_unequal = 0;
 int is_root = -1;
 int not_fsim_tree = -1;
 fsck_Xtree_info_t   xtinfo;
 fsck_Xtree_info_ptr xtiptr;

  xtiptr = &xtinfo;

  xtiptr->this_Qel = NULL;
  xtiptr->next_Qel = NULL;
  xtiptr->last_level = -1;          /* -1 so the root will be recognized
                                     * as 1st node in level 0
                                     */
  xtiptr->dense_file = 0;
  xtiptr->leaf_seen = 0;

  if( !(inoptr->di_mode & ISPARSE) ) {
    xtiptr->dense_file = -1;
    }

  switch( desired_action ) {
    case( FSCK_FSIM_RECORD_DUPCHECK ):
      not_fsim_tree = 0;
      desired_action = FSCK_RECORD_DUPCHECK;
      break;
    case( FSCK_FSIM_UNRECORD ):
      not_fsim_tree = 0;
      desired_action = FSCK_UNRECORD;
      break;
    case( FSCK_FSIM_QUERY):
      not_fsim_tree = 0;
      desired_action = FSCK_QUERY;
      break;
    default:
      break;
    }  /* end switch */

  xtiptr->xtp_ptr = (xtpage_t *) &(inoptr->di_btroot);
  if( (xtiptr->xtp_ptr->header.maxentry != XTROOTINITSLOT) &&
      (xtiptr->xtp_ptr->header.maxentry != XTROOTMAXSLOT)     ) {  /*
                             * bad maxentry field
                             */
    inorecptr->ignore_alloc_blks = 1;
    if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
                             * not reported yet
                             */
      msgprms[0] = "45";
      msgprmidx[0] = 0;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = msg_info_ptr->msg_inotyp;
      msgprms[2] = message_parm_2;
      msgprmidx[2] = msg_info_ptr->msg_inopfx;
      sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
      }  /* end not reported yet */
    }  /* end bad maxentry field */
  else if( xtiptr->xtp_ptr->header.nextindex > 
                                                      xtiptr->xtp_ptr->header.maxentry ) {  /*
                             * bad nextindex field
                             */
    inorecptr->ignore_alloc_blks = 1;
    if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
                             * not reported yet
                             */
      msgprms[0] = "46";
      msgprmidx[0] = 0;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = msg_info_ptr->msg_inotyp;
      msgprms[2] = message_parm_2;
      msgprmidx[2] = msg_info_ptr->msg_inopfx;
      sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
      }  /* end not reported yet */
    }  /* end bad nextindex field */
	 
  if( (! inorecptr->ignore_alloc_blks)  &&
      (xtiptr->xtp_ptr->header.nextindex > XTENTRYSTART) ) {  /*
                             * data length > 0
                             */
    if( desired_action != FSCK_RECORD_DUPCHECK ) {  /* not the first pass */
         /*
          * The first time through we stopped processing allocated
          * blocks if and when we discovered the tree to be corrupt.
          * On a 2nd pass we want to stop at the same place.
          */
      if( inorecptr->ignore_alloc_blks )  {  /* the bit is on */
        old_ignore_alloc_blks = -1;          /* set the flag */
        inorecptr->ignore_alloc_blks = 0;    /* turn the bit off */
        }  /* end the bit is on */
      }  /* end not the first pass */

    xtiptr->this_key = offsetXAD( &(xtiptr->xtp_ptr->xad[XTENTRYSTART]) );
	 
    if( xtiptr->dense_file && (xtiptr->this_key != ((int64)0)) ) {  /*
                             * a dense file with a gap at the front
                             */
      inorecptr->ignore_alloc_blks = 1;
      if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported yet */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = msg_info_ptr->msg_inopfx;
        sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        fsck_send_msg( fsck_BADINOFRONTGAP, 0, 2 );
        }  /* end not reported yet */
      }  /* end a dense file with a gap at the front */

    else {  /* not a dense file with a gap at the front */
      xp_rc = treeQ_get_elem( &xtiptr->this_Qel );
      if( xp_rc == FSCK_OK )  {  /* got a queue element */
        xtiptr->this_Qel->node_level = 0;

        if( xtiptr->xtp_ptr->header.flag & BT_LEAF ) {  /* root leaf */
          if( not_fsim_tree ) {  /* not the FileSet Inode Map tree */
            xp_rc = xTree_process_leaf_extents( xtiptr->xtp_ptr, inorecptr,
                                                                xtiptr->this_Qel,
                                                                msg_info_ptr,
                                                                xtiptr->dense_file,
                                                                desired_action );
            }  /* end not the FileSet Inode Map tree */
          xtiptr->xad_ptr = &( xtiptr->xtp_ptr->xad[
                                                         xtiptr->xtp_ptr->header.nextindex - 1
                                                                    ] );
          agg_recptr->this_inode.data_size = 
                                     (int64)(offsetXAD( xtiptr->xad_ptr ) +
                                                lengthXAD( xtiptr->xad_ptr )   
                                               ) * sb_ptr->s_bsize;  /* 
                             * By definition, a root-leaf is the last leaf 
                             * for the inode
                             */
          }  /* end root leaf */
        else {  /* root is not a leaf */
          if( xtiptr->xtp_ptr->header.flag & BT_INTERNAL ) {  /* root internal */
            xp_rc = xTree_process_internal_extents( xtiptr->xtp_ptr,
                                                                      inorecptr,
                                                                      xtiptr->this_Qel,
                                                                      msg_info_ptr,
                                                                      desired_action );
            }  /* end root internal */
          else {  /* invalid flag value! */
            inorecptr->ignore_alloc_blks = 1;
            if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
                             * not reported yet
                             */
              msgprms[0] = "50";
              msgprmidx[0] = 0;
              msgprms[1] = message_parm_1;
              msgprmidx[1] = msg_info_ptr->msg_inotyp;
              msgprms[2] = message_parm_2;
              msgprmidx[2] = msg_info_ptr->msg_inopfx;
              sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
              msgprms[3] = message_parm_3;
              msgprmidx[3] = 0;
              fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
              }  /* end not reported yet */
            }  /* end else invalid flag value! */
          }  /* end else root is not a leaf */
        }  /* end got a queue element */
      }  /* end else not a dense file with a gap at the front */

    while( (xp_rc == FSCK_OK)       &&     /* nothing fatal and */
           (!inorecptr->ignore_alloc_blks) &&     /* tree looks ok and */
           (agg_recptr->treeQ_back != NULL)   ) {  /* queue not empty */

      xp_rc = treeQ_dequeue( &xtiptr->next_Qel );

      if( xp_rc == FSCK_OK )  {  /*
                             * got another element from the queue
                             */
        if( xtiptr->this_Qel->node_level == xtiptr->next_Qel->node_level )  {  /*
                             * it's not the last in its level
                             */
          xp_rc = xTree_node_not_last_in_level( xtiptr, inorecptr,
                                                  msg_info_ptr,
                                                  desired_action );
          }  /* end it's not the last in its level */
        else {  /* it is the last in its level */
          if( !is_root ) {
            xp_rc = xTree_node_last_in_level( xtiptr, inorecptr,
                                                  msg_info_ptr,
                                                  desired_action );
            }
          }  /* end else it is the last in its level */
        }  /* end got another element from the queue */

      if( (xp_rc == FSCK_OK) &&
          (!inorecptr->ignore_alloc_blks) ) {  /* still in business */

             /*
              * save some info about the node already processed
              * and then move on to the new node
              */
        xtiptr->last_level = xtiptr->this_Qel->node_level;
        xtiptr->last_node_addr = xtiptr->this_Qel->node_addr;
        xtiptr->xad_ptr =
            &( xtiptr->xtp_ptr->xad[xtiptr->xtp_ptr->header.nextindex - 1] );
        if( xtiptr->xtp_ptr->header.flag & BT_LEAF ) {  /* it's a leaf */
          xtiptr->last_key = offsetXAD( xtiptr->xad_ptr ) +
                             lengthXAD( xtiptr->xad_ptr ) - 1;
          }  /* end it's a leaf */
        else {  /* it's an internal node */
          xtiptr->last_key = offsetXAD( xtiptr->xad_ptr );
          }  /* end else it's an internal node */

        xp_rc = treeQ_rel_elem( xtiptr->this_Qel );
        if( xp_rc == FSCK_OK ) {    /* released the older element */
          xtiptr->this_Qel = xtiptr->next_Qel; /* promote newer element */
          xtiptr->next_Qel = NULL;         /* to avoid releasing it twice */
          is_root = 0;
          xp_rc = node_get( xtiptr->this_Qel->node_addr, &xtiptr->xtp_ptr );
	if( xp_rc != FSCK_OK ) {  /* bad read! */
            inorecptr->ignore_alloc_blks = 1;
            if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
                             * not reported yet
                             */
	      msgprms[0] = "42";
	      msgprmidx[0] = 0;
	      msgprms[1] = message_parm_1;
	      msgprmidx[1] = msg_info_ptr->msg_inotyp;
	      msgprms[2] = message_parm_2;
	      msgprmidx[2] = msg_info_ptr->msg_inopfx;
	      sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
	      msgprms[3] = message_parm_3;
	      msgprmidx[3] = 0;
	      fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
              }  /* end not reported yet */
	    }  /* end bad read */
          else {  /* got the new node */
            if( xtiptr->xtp_ptr->header.maxentry != XTPAGEMAXSLOT ) {  /*
                             * bad maxentry field
                             */
              inorecptr->ignore_alloc_blks = 1;
              if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
                             * not reported yet
                             */
                msgprms[0] = "43";
                msgprmidx[0] = 0;
                msgprms[1] = message_parm_1;
                msgprmidx[1] = msg_info_ptr->msg_inotyp;
                msgprms[2] = message_parm_2;
                msgprmidx[2] = msg_info_ptr->msg_inopfx;
                sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
                msgprms[3] = message_parm_3;
                msgprmidx[3] = 0;
                fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
                }  /* end not reported yet */
              }  /* end bad maxentry field */
            else if( xtiptr->xtp_ptr->header.nextindex > 
                                                      xtiptr->xtp_ptr->header.maxentry ) {  /*
                             * bad nextindex field
                             */
              inorecptr->ignore_alloc_blks = 1;
              if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
                             * not reported yet
                             */
                msgprms[0] = "44";
                msgprmidx[0] = 0;
                msgprms[1] = message_parm_1;
                msgprmidx[1] = msg_info_ptr->msg_inotyp;
                msgprms[2] = message_parm_2;
                msgprmidx[2] = msg_info_ptr->msg_inopfx;
                sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
                msgprms[3] = message_parm_3;
                msgprmidx[3] = 0;
                fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
                }  /* end not reported yet */
              }  /* end bad nextindex field */
	  
            if( ! inorecptr->ignore_alloc_blks ) {  /* nothing major so far */
             if( xtiptr->last_level != xtiptr->this_Qel->node_level )  {  /*
                             * this is a new level
                             */
              xtiptr->last_key = 0;
              xp_rc = xTree_node_first_in_level( xtiptr, inorecptr,
                                               msg_info_ptr,
                                               desired_action );
              }  /* end this is a new level */

             else {  /* not 1st in level */
              xp_rc = xTree_node_not_first_in_level( xtiptr, inorecptr,
                                               msg_info_ptr,
                                               desired_action );
              }  /* end else not 1st in level */

             ixpxd_unequal = memcmp( (void *) &(xtiptr->xtp_ptr->header.self),
                                                  (void *) &(xtiptr->this_Qel->node_pxd),
                                                  sizeof(pxd_t) );       /* returns 0 if equal */
             if( ixpxd_unequal ) {  /* bad self field in header */
              inorecptr->ignore_alloc_blks = 1;
              if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
                             * not reported yet
                             */
                msgprms[0] = message_parm_0;
                msgprmidx[0] = msg_info_ptr->msg_inotyp;
                msgprms[1] = message_parm_1;
                msgprmidx[1] = msg_info_ptr->msg_inopfx;
                sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
                msgprms[2] = message_parm_2;
                msgprmidx[2] = 0;
                fsck_send_msg( fsck_BADINONODESELF, 0, 3 );
                }  /* end not reported yet */
              }  /* end bad self field in header */
             else if( xtiptr->xtp_ptr->header.nextindex > XTENTRYSTART )  {
              if( xtiptr->xtp_ptr->header.flag & BT_LEAF ) {  /* it's a leaf */
                xtiptr->xad_ptr =
                          &( xtiptr->xtp_ptr->xad[
                                   xtiptr->xtp_ptr->header.nextindex - 1
                                                  ] );
                agg_recptr->this_inode.data_size = 
                                     (int64)(offsetXAD( xtiptr->xad_ptr ) +
                                              lengthXAD( xtiptr->xad_ptr )   
                                             ) * sb_ptr->s_bsize;  /* 
                             * Just in case this is the last leaf for the
                             * inode
                             */
                }  /* end it's a leaf */

              xtiptr->xad_ptr = &(xtiptr->xtp_ptr->xad[XTENTRYSTART]);
              xtiptr->this_key = offsetXAD(xtiptr->xad_ptr);
              xtiptr->ext_length = lengthXAD(xtiptr->xad_ptr);
              }
             else {  /* an empty non-root node */
              inorecptr->ignore_alloc_blks = 1;
              if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
                             * not reported yet
                             */
                msgprms[0] = message_parm_0;
                msgprmidx[0] = msg_info_ptr->msg_inotyp;
                msgprms[1] = message_parm_1;
                msgprmidx[1] = msg_info_ptr->msg_inopfx;
                sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
                msgprms[2] = message_parm_2;
                msgprmidx[2] = 0;
                fsck_send_msg( fsck_BADINOMTNODE, 0, 3 );
                }  /* end not reported yet */
              }  /* end else an empty non-root node */
             }  /* end nothing major so far */
            }  /* end else got the new node */
          }  /* end released the older element */
        }  /* end still in business */

      if( (xp_rc == FSCK_OK) &&
          (!inorecptr->ignore_alloc_blks) ) {  /* no problems yet */

        xp_rc = xTree_node_first_key( xtiptr, inorecptr,
                                      msg_info_ptr, desired_action );

        }  /* end no problems yet */

      if( (xp_rc == FSCK_OK) &&
          (!inorecptr->ignore_alloc_blks) ) {  /*
                             * tree still interesting
                             */
        if( xtiptr->xtp_ptr->header.flag & BT_LEAF ) {  /* a leaf node */
          if( not_fsim_tree ) {  /* not the FileSet Inode Map tree */
            xp_rc = xTree_process_leaf_extents( xtiptr->xtp_ptr, inorecptr,
                                                                xtiptr->this_Qel,
                                                                msg_info_ptr,
                                                                xtiptr->dense_file,
                                                                desired_action );
            }  /* end not the FileSet Inode Map tree */
          }  /* end a leaf node */
        else {  /* not a leaf node */
          if( xtiptr->xtp_ptr->header.flag & BT_INTERNAL ) {  /* an internal node */
            xp_rc = xTree_process_internal_extents( xtiptr->xtp_ptr,
                                                                     inorecptr,
                                                                     xtiptr->this_Qel,
                                                                     msg_info_ptr,
                                                                     desired_action );
            }  /* end an internal node */
          else {  /* an invalid flag value! */
            inorecptr->ignore_alloc_blks = 1;
            if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
                             * not reported yet
                             */
              msgprms[0] = "51";
              msgprmidx[0] = 0;
              msgprms[1] = message_parm_1;
              msgprmidx[1] = msg_info_ptr->msg_inotyp;
              msgprms[2] = message_parm_2;
              msgprmidx[2] = msg_info_ptr->msg_inopfx;
              sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
              msgprms[3] = message_parm_3;
              msgprmidx[3] = 0;
              fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
              }  /* end not reported yet */
            }  /* end else an invalid flag value! */
          }  /* end else not a leaf node */
        }  /* end tree still interesting */
      if( (xp_rc == FSCK_OK)       &&     /* nothing fatal and */
          (!inorecptr->ignore_alloc_blks) &&     /* tree looks ok and */
          (agg_recptr->treeQ_back == NULL)   ) {  /* queue is empty */
        xp_rc = xTree_node_last_in_level( xtiptr, inorecptr,
                                                  msg_info_ptr,
                                                  desired_action );
        }
      }  /* end while nothing fatal and tree looks ok and ... */

        /*
         * there's at least 1 more Q element to release for this node, and
         * if the tree is bad there may still be some on the queue as well.
         *
         * (If there's a processing error all the dynamic storage is going
         * to be released so there's no point in preparing these elements
         * for reuse.)
         */
    if( xp_rc == FSCK_OK)  {
      if( xtiptr->this_Qel != NULL )  {
        xp_rc = treeQ_rel_elem( xtiptr->this_Qel );
        }
      }

    if( xp_rc == FSCK_OK)  {
      if( xtiptr->next_Qel != NULL )  {
        xp_rc = treeQ_rel_elem( xtiptr->next_Qel );
        }
      }

    agg_recptr->treeQ_back = NULL;

    while( (xp_rc == FSCK_OK) &&
           (agg_recptr->treeQ_front != NULL) ) {
      xtiptr->this_Qel = agg_recptr->treeQ_front;
      agg_recptr->treeQ_front = xtiptr->this_Qel->next;
      xp_rc = treeQ_rel_elem( xtiptr->this_Qel );
      }  /* end while */

    if( xp_rc == FSCK_OK )  {  /* not planning to quit */
      if( desired_action != FSCK_RECORD_DUPCHECK )  {  /*
                            * we altered the corrupt tree bit
                            */
        if( old_ignore_alloc_blks &&
            !inorecptr->ignore_alloc_blks )  {  /*
                            * the flag is set but the bit didn't get
                            * turned back on.  This means that the first
                            * time we went through this tree we decided
                            * it was corrupt but this time it looked ok.
                            */
          xp_rc = FSCK_INTERNAL_ERROR_8;
          }  /* end the flag is set but the bit didn't get turned ... */
        else if( !old_ignore_alloc_blks &&
                inorecptr->ignore_alloc_blks )  {  /*
                            * the flag is off but the bit got turned on.
                            * This means that the first time we went
                            * through this tree it looked ok but this
                            * time we decided that it is corrupt.
                            */
          xp_rc = FSCK_INTERNAL_ERROR_9;
          }  /* end the flag is off but the bit got turned on ... */
        }  /* end we altered the corrupt tree bit */
      }  /* not planning to quit */
    }  /* end data length > 0 */

 return( xp_rc );
}                              /* end of xTree_processing ()  */


/*****************************************************************************
 * NAME: xTree_search
 *
 * FUNCTION:  Search the xTree rooted in the given inode for an xad in a
 *            leaf node which describes the given file offset.
 *
 * PARAMETERS:
 *      inoptr        - input - pointer to the inode in which the xTree is
 *                              rooted
 *      given_key     - input - the key (file offset) to match
 *      addr_xad_ptr  - input - pointer to a variable in which to return the
 *                              address of the xad whose key matches given_key
 *      match_found   - input - pointer to a variable in which to return 
 *                              !0 if a matching xad is found
 *                               0 if no match is found
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t xTree_search( dinode_t    *inoptr,
                        fsblkidx_t   given_key,
                        xad_t      **addr_xad_ptr,
                        int8        *match_found
                       )
{
  retcode_t  xs_rc = FSCK_OK;
  xtpage_t  *xtpg_ptr;
  xad_t     *xad_ptr;
  int8       extent_located = 0;
  int8       not_there = 0;
  int16      chosen_idx;
  int8       xad_chosen;
  fsblkidx_t last_offset;

  xtpg_ptr = (xtpage_t *) &(inoptr->di_btroot);

  while( (!extent_located) &&
         (!not_there) &&
         (xs_rc == FSCK_OK) ) {

    if( given_key >
             offsetXAD(&(xtpg_ptr->xad[xtpg_ptr->header.nextindex - 1])) ) { /*
                         * follows the start of the last allocation described
                         */
      chosen_idx = xtpg_ptr->header.nextindex - 1;
      xad_chosen = -1;
      }  /* end else follows the start of the last allocation described */

    else if( given_key < offsetXAD(&(xtpg_ptr->xad[XTENTRYSTART])) ) { /*
                         * precedes the 1st allocation described
                         */
      not_there = -1;
      }  /* end precedes the 1st allocation described */


    else {  /* it's somewhere in between */
      xs_rc = xTree_binsrch_page( xtpg_ptr, given_key,
                                  &xad_chosen, &chosen_idx, &not_there );
      }  /* end else it's somewhere in between */

    if( (xs_rc == FSCK_OK) && (xad_chosen) ) {  /* picked one */

      if( xtpg_ptr->header.flag & BT_LEAF ) {  /* it's this one or none */
        last_offset = offsetXAD(&(xtpg_ptr->xad[chosen_idx])) +
                      lengthXAD(&(xtpg_ptr->xad[chosen_idx])) - 1; /*
                         * the last offset in the extent described
                         */
        if( given_key <= last_offset ) {  /* it's in the range described */
          extent_located = -1;
          }  /* end it's in the range described */
        else {
          not_there = -1;
          }
        }  /* end it's this one or none */
      else {  /* this xad describes a B+ Tree node on the next level down */
        xs_rc = node_get( addressXAD( &(xtpg_ptr->xad[chosen_idx]) ),
                          &xtpg_ptr );       /* read in the next node */
        }  /* end else this xad describes a B+ Tree node on the next ... */
      }  /* end picked one */
    }  /* end while !extent_located and !not_there */

  if( (extent_located) && (xs_rc == FSCK_OK) ) {  /* found it! */
    *addr_xad_ptr = &(xtpg_ptr->xad[chosen_idx]);
    *match_found = -1;
    }  /* end found it! */
  else {  /* no luck */
    *addr_xad_ptr = NULL;
    *match_found = 0;
    }  /* end else no luck */

  return( xs_rc );
}                              /* end of xTree_search ()  */
