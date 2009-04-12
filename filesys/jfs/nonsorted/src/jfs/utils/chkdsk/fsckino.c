/* $Id: fsckino.c,v 1.2 2004/03/21 02:43:22 pasha Exp $ */

static char *SCCSID = "@(#)1.27  12/18/99 05:05:12 src/jfs/utils/chkdsk/fsckino.c, jfschk, w45.fs32, fixbld";
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
 *   MODULE_NAME:               fsckino.c
 *
 *   COMPONENT_NAME:    jfschk
 *
 *   FUNCTIONS:
 *              backout_ACL
 *              backout_EA
 *              calculate_dasd_used
 *              clear_ACL_field
 *              clear_EA_field
 *              display_path
 *              display_paths
 *              first_ref_check_inode
 *              get_path
 *              in_inode_data_check
 *              inode_is_in_use
 *              parent_count
 *              record_valid_inode
 *              release_inode
 *              unrecord_valid_inode
 *              validate_dasd_used
 *              validate_data
 *              validate_dir_data
 *              validate_ACL
 *              validate_EA
 *              validate_record_fileset_inode
 *
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
  * For unicode conversion
  *
  *      defined in xchkdsk.c
  */

extern UconvObject uconv_object;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * For message processing
  *
  *      defined in xchkdsk.c
  */
extern UniChar uni_message_parm[];
extern UniChar *uni_msgprm;

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


extern int32   Uni_Name_len;
extern UniChar Uni_Name[];
extern int32   Str_Name_len;
extern char    Str_Name[];

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 *
 * The following are internal to this file
 *
 */

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */


/*****************************************************************************
 * NAME: backout_ACL
 *
 * FUNCTION: Unrecord all storage allocated for the access control list
 *               (ACL) of the current inode.
 *
 * PARAMETERS:
 *      ino_ptr      - input - pointer to the current inode
 *      ino_recptr  - input - pointer to an fsck inode record describing the
 *                            current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t backout_ACL ( dinode_t          *ino_ptr,
                                    fsck_inode_recptr  ino_recptr
                                   )
{
  retcode_t bacl_rc = FSCK_OK;
  reg_idx_t block_count = 0;
  reg_idx_t extent_length;
  fsblkidx_t extent_address;
  int8 extent_is_valid;
    /*
     * the following will be passed to extent_unrecord() which will
     * ignore them.
     */
  int8 range_adjusted = 0;
  int8 is_EA = 0;
  int8 is_ACL = 1;
  fsck_msg_info_ptr msg_info_ptr = NULL;

  /*
   * if the ACL is in an out-of-line extent, release the blocks
   * allocated for it.
   */
  if( ino_ptr->di_acl.flag == DXD_EXTENT ) {
    extent_length = lengthDXD( &(ino_ptr->di_acl) );   /* extract length */
    extent_address = addressDXD( &(ino_ptr->di_acl) );  /* extract address */
    bacl_rc = process_extent( ino_recptr, extent_length, extent_address,
                             is_EA, is_ACL, msg_info_ptr, &block_count,
                             &extent_is_valid, FSCK_UNRECORD );
    }

  /*
   * backout the blocks in the ACL extent from the running totals for
   * fileset and inode, but not for the object
   * represented by the object (because they were never added to that).
   */
  agg_recptr->blocks_this_fset -= block_count;
  agg_recptr->this_inode.all_blks -= block_count;

  return( bacl_rc );
}                              /* end of backout_ACL_field ()  */


/*****************************************************************************
 * NAME: backout_EA
 *
 * FUNCTION: Unrecord all storage allocated for the extended attributes
 *           (ea) of the current inode.
 *
 * PARAMETERS:
 *      ino_ptr     - input - pointer to the current inode
 *      ino_recptr  - input - pointer to an fsck inode record describing the
 *                            current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t backout_EA ( dinode_t          *ino_ptr,
                       fsck_inode_recptr  ino_recptr
                       )
{
  retcode_t bea_rc = FSCK_OK;
  reg_idx_t block_count = 0;
  reg_idx_t extent_length;
  fsblkidx_t extent_address;
  int8 extent_is_valid;
    /*
     * the following will be passed to extent_unrecord() which will
     * ignore them.
     */
  int8 range_adjusted = 0;
  int8 is_EA = 1;
  int8 is_ACL = 0;
  fsck_msg_info_ptr msg_info_ptr = NULL;

  /*
   * if the EA is in an out-of-line extent, release the blocks
   * allocated for it.
   */
  if( ino_ptr->di_ea.flag == DXD_EXTENT ) {
    extent_length = lengthDXD( &(ino_ptr->di_ea) );   /* extract length */
    extent_address = addressDXD( &(ino_ptr->di_ea) );  /* extract address */
    bea_rc = process_extent( ino_recptr, extent_length, extent_address,
                             is_EA, is_ACL, msg_info_ptr, &block_count,
                             &extent_is_valid, FSCK_UNRECORD );
    }

  /*
   * backout the blocks in the EA extent from the running totals for
   * fileset and inode, but not for the object
   * represented by the object (because they were never added to that).
   */
  agg_recptr->blocks_this_fset -= block_count;
  agg_recptr->this_inode.all_blks -= block_count;

  return( bea_rc );
}                              /* end of backout_EA_field ()  */


/*                                                                                      begin @F1 */
/*****************************************************************************
 * NAME: calculate_dasd_used
 *
 * FUNCTION: Calculate the cumulative amount of dasd used for each directory
 *           in the file system.
 *
 * PARAMETERS:  none
 *
 * NOTES:               none
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t calculate_dasd_used ( )
{
  retcode_t cdu_rc = FSCK_OK;
  inoidx_t ino_idx;
  fsck_inode_recptr this_inorec, this_child_inorec;
  fsck_inode_recptr this_parent_inorec, next_parent_inorec;
  int aggregate_inode = 0;
  int alloc_ifnull = 0;
  int32 this_level;

    /*
     * In the first step we make a single pass on the inode records:
     *
     *     if the inode represents a file which is not an orphan then the
     *     number of blocks used by the file is added to the cummulative
     *     total for the parent inode.
     *
     *     if the inode represents a directory then we figure out the
     *     distance, in the fileset directory tree, between this inode and
     *     the root directory inode.
     */
  cdu_rc = get_inorecptr_first( aggregate_inode, &ino_idx, &this_inorec );

  while( (cdu_rc == FSCK_OK) &&
         (this_inorec != NULL) &&
         (ino_idx < FILESET_OBJECT_I) ) { /*
                               * not interesting until we get past the root inode
                               * and the special fileset inodes.
                               */
    cdu_rc = get_inorecptr_next( aggregate_inode, &ino_idx, &this_inorec );
    }  /* end while */

  while( (cdu_rc == FSCK_OK) && (this_inorec != NULL) ) {

    if( (this_inorec->in_use) &&
        (!this_inorec->selected_to_rls) )  {  /*
                               * inode in use and not selected to release
                               */
      if( this_inorec->inode_type != directory_inode ) {  /* it's a file */
        if( this_inorec->parent_inonum != 0 ) {  /* it's not an orphan */
               cdu_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                                              this_inorec->parent_inonum,
                                              &this_parent_inorec );
               if( (cdu_rc == FSCK_OK) && (this_parent_inorec == NULL) )  {  /* uh oh....*/
                cdu_rc = FSCK_INTERNAL_ERROR_63;
                }  /* end uh oh.... */
               else if( cdu_rc == FSCK_OK ) {  /* located the parent's inode record */
                   this_parent_inorec->cumm_blocks += this_inorec->cumm_blocks;
                }  /* end else located the parent's inode record */
               }  /* end it's not an orphan */
        }  /* end it's a file */

      else {  /* it's a directory */
          if( this_inorec->dtree_level == -1 ) {  /* the distance not known yet */
              if( this_inorec->parent_inonum == 0 ) {  /* it's an orphan */
                  this_inorec->dtree_level = 0;
                  if( this_inorec->dtree_level > agg_recptr->tree_height ) {
                      agg_recptr->tree_height = this_inorec->dtree_level;
                      }
                 }  /* end it's an orphan */
              else {  /* it's not an orphan */
                   cdu_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                                                    this_inorec->parent_inonum,
                                                    &this_parent_inorec );
                   if( (cdu_rc == FSCK_OK) && (this_parent_inorec == NULL) )  {
                       cdu_rc = FSCK_INTERNAL_ERROR_64;
                       }
                   else if( cdu_rc == FSCK_OK ) {  /* located the parent's inode rec */
                       agg_recptr->treeStack = NULL;
                       cdu_rc = treeStack_push( this_inorec );
                       while( (this_parent_inorec->dtree_level == -1) &&
                                 (cdu_rc == FSCK_OK)                              ) {
                           if( this_parent_inorec->parent_inonum == 0 ) {  /* orphan */
                               this_parent_inorec->dtree_level = 0;
                               }  /* end orphan */
                           else {  /* not an orphan */
                               cdu_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                                                            this_parent_inorec->parent_inonum,
                                                            &next_parent_inorec );
                               if( (cdu_rc == FSCK_OK) && (next_parent_inorec == NULL) )  {
                                   cdu_rc = FSCK_INTERNAL_ERROR_65;
                                   }
                               else if( cdu_rc == FSCK_OK ) {  /*
                                        * located next parent's inode rec
                                        */
                                   cdu_rc = treeStack_push( this_parent_inorec );
                                   this_parent_inorec = next_parent_inorec;
                                   }  /* end else located next parent's inode rec */
                               }  /* end else not an orphan */
                           }  /* end while parent tree level == -1 */
                       if( cdu_rc == FSCK_OK ) {  /* nothing fatal yet */
                           this_level = this_parent_inorec->dtree_level;  /*
                                        * this is the first known level number
                                        * (distance) in the chain of directories
                                        * from here to the root.
                                        */
                           while( agg_recptr->treeStack != NULL ) {
                               this_level++;
                               cdu_rc = treeStack_pop( &this_child_inorec );
                               this_child_inorec->dtree_level = this_level;
                               if( this_child_inorec->dtree_level >
                                                               agg_recptr->tree_height ) {
                                   agg_recptr->tree_height =
                                                               this_child_inorec->dtree_level;
                                   }
                               }  /* while treeStack not empty */
                           }  /* end nothing fatal yet */
                    }  /* end else located the parent's inode rec */
                   }  /* end else it's not an orphan */
               }  /* end the distance not known yet */
          }  /* end else it's a directory */
      }  /* end inode in use and not selected to release  */

    if( cdu_rc == FSCK_OK ) {
      cdu_rc = get_inorecptr_next( aggregate_inode, &ino_idx, &this_inorec );
      }
    }  /* end while */

    /*
     *
     * In the second step we make one pass on the inode records for each
     * level in the fileset directory tree, since we must accumulate the
     * dasd used values from the bottom up.
     *
     */
  this_level = agg_recptr->tree_height;
  while( (cdu_rc == FSCK_OK) && (this_level > 0) ) {
      cdu_rc = get_inorecptr_first( aggregate_inode, &ino_idx, &this_inorec );

      while( (cdu_rc == FSCK_OK) &&
                (this_inorec != NULL) &&
                (ino_idx < FILESET_OBJECT_I) ) { /*
                               * not interesting until we get past the root inode
                               * and the special fileset inodes.
                               */
          cdu_rc = get_inorecptr_next( aggregate_inode, &ino_idx, &this_inorec );
           }  /* end while */

      while( (cdu_rc == FSCK_OK) && (this_inorec != NULL) ) {

          if( (this_inorec->in_use) &&
              (!this_inorec->selected_to_rls) &&
              (this_inorec->inode_type == directory_inode) &&
              (this_inorec->dtree_level == this_level)          ) {  /*
                               * inode in use and not selected to release and
                               * type directory and dtree level is a match
                               */
              cdu_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                                                   this_inorec->parent_inonum,
                                                   &this_parent_inorec );
              if( (cdu_rc == FSCK_OK) && (this_parent_inorec == NULL) )  {  /* uh oh....*/
                  cdu_rc = FSCK_INTERNAL_ERROR_66;
                  }  /* end uh oh.... */
              else if( cdu_rc == FSCK_OK ) {  /*
                        * located the parent's inode record
                        */

                   this_parent_inorec->cumm_blocks += this_inorec->cumm_blocks;
                   }  /* end else located the parent's inode record */
              }  /* end inode in use and not selected to release and ... */

          if( cdu_rc == FSCK_OK ) {
              cdu_rc = get_inorecptr_next( aggregate_inode, &ino_idx, &this_inorec );
              }

          }  /* end while */
      this_level --;
      }  /* end while this_level > 0 */

  return( cdu_rc );
}                              /* end of calculate_dasd_used ()  */
/*                                                                                           end @F1 */


/*****************************************************************************
 * NAME: clear_ACL_field
 *
 * FUNCTION: Unrecord all storage allocated for the access control list
 *           (ACL) of the current inode.  Clear the inode ACL field to show
 *           the inode owns no ACL.
 *
 * PARAMETERS:
 *      ino_recptr  - input - pointer to an fsck inode record describing the
 *                            current inode
 *      ino_ptr     - input - pointer to the current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t clear_ACL_field ( fsck_inode_recptr  ino_recptr,
                                       dinode_t          *ino_ptr
                                       )
{
  retcode_t caf_rc = FSCK_OK;
  dxd_t *dxd_ptr;
  reg_idx_t block_count = 0;
  reg_idx_t extent_length;
  fsblkidx_t extent_address;
  int8 extent_is_valid;
    /*
     * the following will be passed to extent_unrecord() which will
     * ignore them.
     */
  int8 range_adjusted = 0;
  int8 is_EA = 0;
  int8 is_ACL = -1;
  fsck_msg_info_ptr msg_info_ptr = NULL;

  dxd_ptr = &(ino_ptr->di_acl);     /* locate the EA field in the inode */

  /*
   * if the ACL is in an out-of-line extent, release the blocks
   * allocated for it.
   */
  if( (dxd_ptr->flag == DXD_EXTENT) &&      /* out of line single extent */
      (!ino_recptr->ignore_acl_blks) &&
      (!ino_recptr->ignore_alloc_blks) ) {  /* not flagged to ignore  */
    extent_length = lengthDXD( dxd_ptr );   /* extract length */
    extent_address = addressDXD( dxd_ptr );  /* extract address */
    caf_rc = process_extent( ino_recptr, extent_length, extent_address,
                             is_EA, is_ACL, msg_info_ptr, &block_count,
                             &extent_is_valid, FSCK_UNRECORD );
    ino_ptr->di_nblocks -= block_count;
    agg_recptr->blocks_for_acls -= block_count;
    }  /* end out of line single extent */

  /*
   * Clear the ACL field
   */
  if( caf_rc == FSCK_OK ) {  /* nothing fatal yet */
    dxd_ptr->flag = DXD_CORRUPT;        /* set the flag byte */
    DXDlength( dxd_ptr, 0 );        /* clear the data length */
    DXDaddress( dxd_ptr, 0 );       /* clear the data address */

    agg_recptr->blocks_this_fset -= block_count;
    }  /* end nothing fatal yet */

  return( caf_rc );
}                              /* end of clear_ACL_field ()  */


/*****************************************************************************
 * NAME: clear_EA_field
 *
 * FUNCTION: Unrecord all storage allocated for the extended attributes
 *           (ea) of the current inode.  Clear the inode ea field to show
 *           the inode owns no ea.
 *
 * PARAMETERS:
 *      ino_recptr  - input - pointer to an fsck inode record describing the
 *                            current inode
 *      ino_ptr     - input - pointer to the current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t clear_EA_field ( fsck_inode_recptr  ino_recptr,
                           dinode_t          *ino_ptr
                         )
{
  retcode_t cef_rc = FSCK_OK;
  dxd_t *dxd_ptr;
  reg_idx_t block_count = 0;
  reg_idx_t extent_length;
  fsblkidx_t extent_address;
  int8 extent_is_valid;
    /*
     * the following will be passed to extent_unrecord() which will
     * ignore them.
     */
  int8 range_adjusted = 0;
  int8 is_EA = -1;
  int8 is_ACL = 0;
  fsck_msg_info_ptr msg_info_ptr = NULL;

  dxd_ptr = &(ino_ptr->di_ea);     /* locate the EA field in the inode */

  /*
   * if the EA is in an out-of-line extent, release the blocks
   * allocated for it.
   */
  if( (dxd_ptr->flag == DXD_EXTENT) &&      /* out of line single extent */
      (!ino_recptr->ignore_ea_blks) &&
      (!ino_recptr->ignore_alloc_blks) ) {  /* not flagged to ignore  */
    extent_length = lengthDXD( dxd_ptr );   /* extract length */
    extent_address = addressDXD( dxd_ptr );  /* extract address */
    cef_rc = process_extent( ino_recptr, extent_length, extent_address,
                             is_EA, is_ACL, msg_info_ptr, &block_count,
                             &extent_is_valid, FSCK_UNRECORD );
    agg_recptr->blocks_for_eas -= block_count;
    }  /* end out of line single extent */

  /*
   * Clear the EA field
   */
  if( cef_rc == FSCK_OK ) {  /* nothing fatal yet */
    dxd_ptr->flag = 0;              /* clear the flag byte */
    DXDlength( dxd_ptr, 0 );        /* clear the data length */
    DXDaddress( dxd_ptr, 0 );       /* clear the data address */

    ino_ptr->di_nblocks -= block_count;
    agg_recptr->blocks_this_fset -= block_count;
    }  /* end nothing fatal yet */

  return( cef_rc );
}                              /* end of clear_EA_field ()  */


/*****************************************************************************
 * NAME: display_path
 *
 * FUNCTION: Issue a message to display the given inode path.
 *
 * PARAMETERS:
 *      inoidx      - input - ordinal number of the inode as an integer
 *      inopfx      - input - index (into message catalog) of prefix for
 *                            inode number when displayed in message
 *                            { A | <blank> }
 *      ino_parent  - input - the inode number for the (parent) directory
 *                            whose entry to the current inode is described
 *                            by the contents of inopath.
 *      inopath     - input - pointer to the UniCharacter path which is
 *                            to be displayed.
 *      ino_recptr  - input - pointer to an fsck inode record describing the
 *                            current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t display_path( inoidx_t           inoidx,
                        int                inopfx,
                        ino_t              ino_parent,
                        char              *inopath,
                        fsck_inode_recptr  ino_recptr
                       )
{
  retcode_t dip_rc = FSCK_OK;

  if( (!ino_recptr->unxpctd_prnts) ||
      (!(ino_recptr->inode_type == directory_inode)) ) {  /*
                               * not directory w/ mult parents
                               */
    msgprms[0] = message_parm_0;
    if( ino_recptr->inode_type == directory_inode) {
      msgprmidx[0] = fsck_directory;
      }
    else {
      msgprmidx[0] = fsck_file;
      }
    msgprms[1] = message_parm_1;
    msgprmidx[1] = inopfx;
    sprintf( message_parm_2, "%ld", inoidx );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    msgprms[3] = inopath;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_INOPATHOK, 0, 4 );
    }  /* end not directory w/ mult parents */

  else {  /* else a directory w/ multiple parents */

    if( ino_parent == ino_recptr->parent_inonum ) { /* expected parent */
      msgprms[0] = inopath;
      msgprmidx[0] = 0;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = inopfx;
      sprintf( message_parm_2, "%ld", inoidx );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_INOPATHCRCT, 0, 3 );
      }  /* end expected parent */

    else { /* this is an illegal hard link */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = fsck_directory;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = inopfx;
      sprintf( message_parm_2, "%ld", inoidx );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      msgprms[3] = inopath;
      msgprmidx[3] = 0;
      fsck_send_msg( fsck_INOPATHBAD, 0, 4 );
      }  /* end else this is an illegal hard link */
    }  /* end else a directory w/ multiple parents */

  return( dip_rc );
}                              /* end of display_path ()  */


/*****************************************************************************
 * NAME: display_paths
 *
 * FUNCTION:  Display all paths to the specified inode.
 *
 * PARAMETERS:
 *      inoidx        - input - ordinal number of the inode as an integer
 *      ino_recptr    - input - pointer to an fsck inode record describing the
 *                              current inode
 *      msg_info_ptr  - input - pointer to a record with information needed
 *                              to issue messages about the current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t display_paths( inoidx_t          inoidx,
                         fsck_inode_recptr ino_recptr,
                         fsck_msg_info_ptr msg_info_ptr
                        )
{
  retcode_t dips_rc = FSCK_OK;
  retcode_t intermed_rc;
  ino_t this_parent;
  fsck_inode_extptr this_ext = NULL;
  char *this_path_ptr;
  char **this_path_ptr_ptr;
  fsck_inode_recptr parent_inorecptr;
  int a_path_displayed = 0;

  this_path_ptr_ptr = &this_path_ptr;

  if( inoidx == 2 ) {  /* it's the root directory */
    intermed_rc = get_path( inoidx, this_parent, this_path_ptr_ptr, ino_recptr );
    a_path_displayed = -1;
    dips_rc = display_path( inoidx, msg_info_ptr->msg_inopfx,
                                     this_parent, this_path_ptr, ino_recptr );

    }  /* end it's the root directory */
  else if( ino_recptr->parent_inonum != (ino_t) 0 ) { /*
                                *at least one parent was observed
                                */
    /*
     * if this is a directory with illegal hard links then the
     * inode number in the fsck inode record is the one stored in
     * the inode on disk.  This routine displays only messages
     * for parents observed by fsck.
     */
    if( (ino_recptr->inode_type == directory_inode) &&
        (ino_recptr->unxpctd_prnts) ) {  /* dir with multiple parents */

      this_ext = ino_recptr->ext_rec;
      while( (this_ext != NULL) &&
             (this_ext->ext_type != parent_extension) ) {
        this_ext = this_ext->next;
        }  /* end while */

      if( this_ext == NULL )  {  /* something is terribly wrong! */
        dips_rc = FSCK_INTERNAL_ERROR_1;
        }  /* end something is terribly wrong! */
      }  /* end dir with multiple parents */
    else {  /* not a dir with multiple parents */
      /*
       * the 1st parent observed is in the inode record.  Any others are
       * in extension records.
       */
      this_parent = ino_recptr->parent_inonum;
      if( (this_parent != ROOT_I) || (!agg_recptr->rootdir_rebuilt) ) {  /*
                                * either this parent isn't the root or else
                                * the root dir has not been rebuilt
                                */
        intermed_rc = get_inorecptr ( 0, 0, this_parent, &parent_inorecptr );
        if( intermed_rc != FSCK_OK ) {  /* it's fatal */
          dips_rc = intermed_rc;
          }  /* end it's fatal */
        else if( (parent_inorecptr->in_use) &&
                 (!parent_inorecptr->selected_to_rls) &&
                 (!parent_inorecptr->ignore_alloc_blks)    ) { /*
                                * got parent record and parent seems ok so far
                                */
           intermed_rc = get_path( inoidx, this_parent, this_path_ptr_ptr, ino_recptr );
           if( intermed_rc != FSCK_OK ) {  /* unable to obtain 1st path */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = msg_info_ptr->msg_inopfx;
          sprintf( message_parm_1, "%ld", this_parent );
          msgprms[1] = message_parm_1;
          msgprmidx[1] = 0;
          msgprms[2] = message_parm_2;
          msgprmidx[2] = msg_info_ptr->msg_inotyp;
          msgprms[3] = msgprms[0];
          msgprmidx[3] = msgprmidx[0];
          sprintf( message_parm_4, "%ld", inoidx );
          msgprms[4] = message_parm_4;
          msgprmidx[4] = 0;
          fsck_send_msg( fsck_INOCNTGETPATH, 0, 5 );
          if( intermed_rc < 0 ) {  /* it's fatal */
            dips_rc = intermed_rc;
            }  /* end it's fatal */
          }  /* end unable to obtain 1st path */
          else { /* 1st path obtained */
             a_path_displayed = -1;
          dips_rc = display_path( inoidx, msg_info_ptr->msg_inopfx,
                                this_parent, this_path_ptr, ino_recptr );

            /*
             * if there are any more paths to the inode, find the next parent
             */
          this_ext = ino_recptr->ext_rec;
          while( (this_ext != NULL) &&
                    (this_ext->ext_type != parent_extension) ) {
            this_ext = this_ext->next;
            }  /* end while */
          }  /* end 1st path obtained */
          }  /* end got parent record and parent seems ok so far */
        }  /* end either this parent isn't the root or else root dir has not ... */
      }  /* end else not a dir with multiple parents */

    while( (dips_rc == FSCK_OK ) &&
           (this_ext != NULL) ) { /* there may be more parents */

      if( this_ext->ext_type == parent_extension ) { /* parent extension */
        this_parent = this_ext->inonum;
        if( (this_parent != ROOT_I) || (!agg_recptr->rootdir_rebuilt) ) {  /*
                                * either this parent isn't the root or else
                                * the root dir has not been rebuilt
                                */
        intermed_rc = get_inorecptr ( 0, 0, this_parent, &parent_inorecptr );
        if( intermed_rc != 0 ) {  /* it's fatal */
          dips_rc = intermed_rc;
          }  /* end it's fatal */
        else if( (parent_inorecptr->in_use) &&
                     (!parent_inorecptr->selected_to_rls) &&
                     (!parent_inorecptr->ignore_alloc_blks)    ) {  /*
                                * got parent record and parent seems ok so far
                                */
          intermed_rc = get_path( inoidx, this_parent,
                                            this_path_ptr_ptr, ino_recptr );
          if( intermed_rc == FSCK_OK ) { /* next path obtained */
            a_path_displayed = -1;
            dips_rc = display_path( inoidx, msg_info_ptr->msg_inopfx,
                                  this_parent, this_path_ptr, ino_recptr );
            }  /* end next path obtained */
          else {  /* unable to obtain next path */
            msgprms[0] = message_parm_0;
            msgprmidx[0] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_1, "%ld", this_parent );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            msgprms[2] = message_parm_2;
            msgprmidx[2] = msg_info_ptr->msg_inotyp;
            msgprms[3] = msgprms[0];
            msgprmidx[3] = msgprmidx[0];
            sprintf( message_parm_4, "%ld", inoidx );
            msgprms[4] = message_parm_4;
            msgprmidx[4] = 0;
            fsck_send_msg( fsck_INOCNTGETPATH, 0, 5 );
            if( intermed_rc < 0 ) {  /* it's fatal */
              dips_rc = intermed_rc;
              }  /* end it's fatal */
            }  /* end else unable to obtain next path */
             }  /* end got parent record and parent seems ok so far */
            }  /* end either this parent isn't the root or else root dir has not ... */
        }  /* end parent extension */

      this_ext = this_ext->next;

      }  /* end while there may be more parents */
    }  /* end at least one parent was observed */

        /*
         * if nothing unexpected happened but we
         * couldn't display a path, issue a message
         * and go on.
         */
  if( (dips_rc == FSCK_OK) && (!a_path_displayed) ) {
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_inotyp;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_2, "%ld", inoidx );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    fsck_send_msg( fsck_INOCANTNAME, 0, 3 );
    }

  return( dips_rc );
}                              /* end of display_paths ()  */


/*****************************************************************************
 * NAME: first_ref_check_inode
 *
 * FUNCTION:  Determine whether storage allocated to the given inode
 *            includes any multiply-allocated blocks for which the
 *            first reference is still unresolved.
 *
 * PARAMETERS:
 *      inoptr       - input - pointer to the current inode
 *      inoidx       - input - ordinal number of the inode as an integer
 *      inorec_ptr   - input - pointer to an fsck inode record describing the
 *                             current inode
 *      msginfo_ptr  - input - pointer to a record with information needed
 *                             to issue messages about the current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t first_ref_check_inode ( dinode_t           *inoptr,
                                  inoidx_t                inoidx,
                                  fsck_inode_recptr  inorec_ptr,
                                  fsck_msg_info_ptr  msginfo_ptr
                                )
{
  retcode_t frcvi_rc = FSCK_OK;
  int xad_idx;
  xtpage_t *xtpage_ptr;
  xtpage_t *this_xtpage;
  xad_t    *xad_ptr;
  reg_idx_t block_count;
  fsblkidx_t  node_addr_fsblks;
  fsblkidx_t  first_child_addr;
  fsblkidx_t  first_fsblk;
  reg_idx_t  num_fsblks;
  int8 extent_is_valid;
  dxd_t *dxd_ptr;
  int is_EA;
  int is_ACL;

    /*
     * check the extent (if any) containing the EA
     */
  if( inoptr->di_ea.flag == DXD_EXTENT )  {  /* there is an ea to record */
    dxd_ptr = &(inoptr->di_ea);
    first_fsblk = addressDXD( dxd_ptr );
    num_fsblks = lengthDXD( dxd_ptr );

    is_EA = -1;
    is_ACL = 0;
    msginfo_ptr->msg_dxdtyp = fsck_EA;

    frcvi_rc = process_extent( inorec_ptr, num_fsblks, first_fsblk,
                               is_EA, is_ACL, msginfo_ptr, &block_count,
                               &extent_is_valid, FSCK_QUERY );
    }  /* end there is an ea to check */

    /*
     * check the extent (if any) containing the ACL
     */
  if( inoptr->di_acl.flag == DXD_EXTENT )  {  /* there is an ACL to record */
    dxd_ptr = &(inoptr->di_acl);
    first_fsblk = addressDXD( dxd_ptr );
    num_fsblks = lengthDXD( dxd_ptr );

    is_EA = 0;
    is_ACL = -1;
    msginfo_ptr->msg_dxdtyp = fsck_ACL;

    frcvi_rc = process_extent( inorec_ptr, num_fsblks, first_fsblk,
                               is_EA, is_ACL, msginfo_ptr, &block_count,
                               &extent_is_valid, FSCK_QUERY );
    }  /* end there is an ACL to check */


    /*
     * check the extents (if any) described as data
     */

  if( frcvi_rc == FSCK_OK ) {
    if( inoptr->di_size > 0 ) {  /* there is data there */     /* @D1 */
      if( inorec_ptr->inode_type == directory_inode ) {
        frcvi_rc = process_valid_dir_data( inoptr, inoidx,
                                           inorec_ptr, msginfo_ptr,
                                           FSCK_QUERY );
        }
      else {
        frcvi_rc = process_valid_data( inoptr, inoidx,
                                     inorec_ptr, msginfo_ptr,
                                     FSCK_QUERY );
        }
      }
    }

  return( frcvi_rc );
}                          /* end of first_ref_check_inode ()  */


/*****************************************************************************
 * NAME: get_path
 *
 * FUNCTION:  Construct the unicode path from the root directory through
 *            the entry in the specified parent directory.
 *
 * PARAMETERS:
 *      inode_idx       - input - ordinal number of the inode as an integer
 *      parent_inonum   - input - the inode number for the (parent) directory
 *                                whose entry to the current inode is to be
 *                                described by the contents of inopath.
 *      addr_path_addr  - input - pointer to a variable in which to return
 *                                the address of the path (in UniChars)
 *      ino_recptr      - input - pointer to an fsck inode record describing
 *                                the current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t get_path ( inoidx_t            inode_idx,
                     ino_t               parent_inonum,
                     char              **addr_path_addr,
                     fsck_inode_recptr   ino_recptr
                   )
{
  retcode_t gip_rc = FSCK_OK;
  ino_t inode_inonum;
  ino_t this_parent_inonum;
  ino_t root_inonum;
  int bytes_used;
  int this_name_length = 0;
  int length_in_UniChars;
  UniChar uniname[256];
  char *path_ptr;
  int path_idx;
  int path_completed = 0;
  inoidx_t parent_idx;
  fsck_inode_recptr parent_inorecptr;
  int aggregate_inode = 0;
  int alloc_ifnull = 0;

  *addr_path_addr = NULL;                 /* initialize return value */

  bytes_used = sizeof(char);              /* the string terminator */
  path_idx = JFS_PATH_MAX - 1;            /* the last character in
                                           * the buffer
                                           */
  agg_recptr->path_buffer[path_idx] = '\0'; /* terminate the string
                                           * which will contain the
                                           * requested path
                                           */
  path_ptr = &(agg_recptr->path_buffer[path_idx]);  /* initialize    */

  inode_inonum = (ino_t) inode_idx;       /* change the inode index to an
                                           * inode number
                                           */
  this_parent_inonum = parent_inonum;     /* the first parent must be given
                                           * since the object may not be
                                           * a directory
                                           */
  if( inode_idx == 2 ) {  /* it's the root directory */
    path_completed = -1;
    path_idx --;
    path_ptr --;
    agg_recptr->path_buffer[path_idx] = agg_recptr->delim_char;
    }  /* end it's the root directory */

  while( (!path_completed) && (gip_rc == FSCK_OK) ) {

    gip_rc = direntry_get_objnam(this_parent_inonum,
                                 inode_inonum,
                                 &length_in_UniChars,
                                 uniname);
    uniname[length_in_UniChars] = 0;

    if( gip_rc != FSCK_OK ) {  /* didn't get the name */
      path_completed = -1;
      if( path_idx != (JFS_PATH_MAX - 1)) {  /* we got part of the path */
        path_ptr ++;           /* remove the foreslash from the
                                * beginning of the path we have at this
                                * beginning as now assembled it implies
                                * that the unconnected dir parent is
                                * connected to the fileset root directory.
                                */
        }  /* end we got part of the path */
      }  /* end didn't get the name */
    else if (UniStrFromUcs( uconv_object, Str_Name, uniname, 256 )
             != ULS_SUCCESS)

      gip_rc = FSCK_FAILED_DIRENTRYBAD;

    else {      /* got the name */

      this_name_length = strlen( Str_Name );

      if( (bytes_used + this_name_length + sizeof(char)) > JFS_PATH_MAX ) { /*
                                            * the path is beyond the
                                            * legal length
                                            */
        path_completed = -1;
        if( path_idx == (JFS_PATH_MAX - 1)) {  /* the very first segment
                                * is too long to be valid
                                */
          gip_rc = FSCK_FAILED_DIRENTRYBAD;
          }
        else {  /* we got part of the path */
          path_ptr ++;         /* remove the foreslash from the
                                * beginning of the path we have at
                                * this point since as now assembled
                                * it implies that the unconnected dir
                                * parent is connected to the fileset
                                        * root directory.
                                */
          }  /* end else we got part of the path */
        }  /* end the path is beyond the legal length */
      else {  /* the path is still ok */

        bytes_used += this_name_length;
        path_idx -= this_name_length;
        path_ptr -= this_name_length;
        Str_Name_len = this_name_length;
        memcpy( (void *) path_ptr, (void *) &Str_Name, Str_Name_len );
        bytes_used += sizeof( char );
        path_idx --;
        path_ptr --;
        agg_recptr->path_buffer[path_idx] = agg_recptr->delim_char; /*
                                * assume that we'll find a parent dir
                                * for the path segment just copied into
                                * the path buffer.
                                */
        if( this_parent_inonum == ROOT_I ) {
        path_completed = -1;
            }
        else {  /* haven't gotten up to root yet */
          parent_idx = (inoidx_t) this_parent_inonum;
          inode_inonum = this_parent_inonum;
          gip_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                                parent_idx, &parent_inorecptr );
          if( (gip_rc == FSCK_OK) && (parent_inorecptr == NULL) ) {
            gip_rc = FSCK_INTERNAL_ERROR_21;
            }
          else if( gip_rc == FSCK_OK ) {
            this_parent_inonum = parent_inorecptr->parent_inonum;

            if( (this_parent_inonum == (ino_t) 0)                  ||
                (parent_inorecptr->selected_to_rls) ||
                (!(parent_inorecptr->in_use))       ||
                (parent_inorecptr->inode_type == metadata_inode)
              ) {
              path_completed = -1;
              path_ptr++;      /* remove the foreslash from the beginning
                                * of the path we have at this point since
                                * as now assembled it implies that the
                                * unconnected dir parent is connected to
                                * the fileset root directory.
                                */
              }  /* end if unconnected directory */
            }
          }  /* end else haven't gotten to root yet */
        }  /* end else the path is still ok */
      }  /* end else got the name */
    }  /* end while this parent isn't the root */

  if( gip_rc == FSCK_OK ) {
    *addr_path_addr = path_ptr; /* indicate where to find the 1st char of
                                * the path just assembled
                                */
    }

  return( gip_rc );
}                              /* end of get_path ()  */


/*****************************************************************************
 * NAME: in_inode_data_check
 *
 * FUNCTION:  Verify that the fields in the current inode which describe
 *            inline data (that is, storage within the inode itself) do
 *            not overlap.
 *
 * PARAMETERS:
 *      msg_info_ptr  - input - pointer to a record with information needed
 *                              to issue messages about the current inode
 *
 * NOTES:  The data regarding inline data for the inode is stored in
 *         the global aggregate record, fields in the this_inode record.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t in_inode_data_check  ( fsck_inode_recptr  inorecptr,
                                               fsck_msg_info_ptr  msg_info_ptr )
{
 retcode_t iidc_rc = FSCK_OK;
 int16 size16;
 int16 ea_last;
 dinode_t an_inode;

 size16 = sizeof(an_inode._data3._data);  /* if in-inode data (or
                             * description of data) overflows this then
                             * the EA can NOT be inline
                             */
 if( agg_recptr->this_inode.in_inode_data_length > size16 ) {  /*
                             * extra long inline data
                             */
   if( agg_recptr->this_inode.ea_inline ) {  /* conflict */
     inorecptr->selected_to_rls = 1;
     inorecptr->inline_data_err = 1;
     inorecptr->clr_ea_fld = 1;
     agg_recptr->corrections_needed = 1;
     msgprms[0] = message_parm_0;
     msgprmidx[0] = msg_info_ptr->msg_inotyp;
     msgprms[1] = message_parm_1;
     msgprmidx[1] = msg_info_ptr->msg_inopfx;
     sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
     msgprms[2] = message_parm_2;
     msgprmidx[2] = 0;
     sprintf( message_parm_3, "%d", fsck_longdata_and_otherinline );
     msgprms[3] = message_parm_3;
     msgprmidx[3] = 0;
     fsck_send_msg( fsck_INOINLINECONFLICT, 0, 4 );
     }  /* end conflict */

   if( agg_recptr->this_inode.acl_inline ) {  /* conflict */
     inorecptr->selected_to_rls = 1;
     inorecptr->inline_data_err = 1;
     inorecptr->clr_acl_fld = 1;
     agg_recptr->corrections_needed = 1;
     msgprms[0] = message_parm_0;
     msgprmidx[0] = msg_info_ptr->msg_inotyp;
     msgprms[1] = message_parm_1;
     msgprmidx[1] = msg_info_ptr->msg_inopfx;
     sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
     msgprms[2] = message_parm_2;
     msgprmidx[2] = 0;
     sprintf( message_parm_3, "%d", fsck_longdata_and_otherinline );
     msgprms[3] = message_parm_3;
     msgprmidx[3] = 0;
     fsck_send_msg( fsck_INOINLINECONFLICT, 0, 4 );
     }  /* end conflict */
   }  /* end extra long data */
 else {
   if( agg_recptr->this_inode.ea_inline &&
       agg_recptr->this_inode.acl_inline ) {  /* conflict */
     inorecptr->clr_ea_fld = 1;
     inorecptr->clr_acl_fld = 1;
     agg_recptr->corrections_needed = 1;
     msgprms[0] = message_parm_0;
     msgprmidx[0] = msg_info_ptr->msg_inotyp;
     msgprms[1] = message_parm_1;
     msgprmidx[1] = msg_info_ptr->msg_inopfx;
     sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
     msgprms[2] = message_parm_2;
     msgprmidx[2] = 0;
     sprintf( message_parm_3, "%d", fsck_longdata_and_otherinline );
     msgprms[3] = message_parm_3;
     msgprmidx[3] = 0;
     fsck_send_msg( fsck_INOINLINECONFLICT, 0, 4 );
     }  /* end conflict */
   }

 return( iidc_rc );
}                              /* end of in_inode_data_check ()  */


/*****************************************************************************
 * NAME: inode_is_in_use
 *
 * FUNCTION:  Determine whether the specified inode is currently being
 *            used to represent a file system object.
 *
 * PARAMETERS:
 *      inode_ptr  - input - pointer to the current inode
 *      inode_num  - input - ordinal number of the inode in the internal
 *                           JFS format
 *
 * NOTES:
 *
 * RETURNS:
 *      0:  if inode is not in use
 *      1:  if inode is in use
 */
retcode_t inode_is_in_use ( dinode_t      *inode_ptr,
                            ino_t          inode_num
                          )
{
  retcode_t iiiu_result;
  int ixpxd_unequal = 0;
  ixpxd_unequal = memcmp( (void *) &(inode_ptr->di_ixpxd),
                          (void *) &(agg_recptr->ino_ixpxd),
                          sizeof(pxd_t) );       /* returns 0 if equal */

  iiiu_result = ( (inode_ptr->di_inostamp == agg_recptr->inode_stamp) &&
                  (inode_ptr->di_number == inode_num) &&
                  (inode_ptr->di_fileset == agg_recptr->ino_fsnum) &&
                  (!ixpxd_unequal) &&
                  (inode_ptr->di_nlink != 0)
                );

  return( iiiu_result );
}                              /* end of inode_is_in_use ()  */


/*****************************************************************************
 * NAME: parent_count
 *
 * FUNCTION: Count the number of directory entries fsck has observed which
 *           refer to the specified inode.
 *
 * PARAMETERS:
 *      this_inorec  - input - pointer to an fsck inode record describing the
 *                             current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      the number of parent directories observed for the inode
 */
retcode_t parent_count ( fsck_inode_recptr this_inorec )
{
  retcode_t pc_result = 0;
  fsck_inode_extptr this_ext;

  if( this_inorec->parent_inonum != (ino_t) 0 ) {
    pc_result++;
    }

  this_ext = this_inorec->ext_rec;
  while (this_ext != NULL ) {  /* extension records to check */
    if( this_ext->ext_type == parent_extension ) {
      pc_result++;
      }
    this_ext = this_ext->next;
    }  /* end extension records to check */

  return( pc_result );
}                              /* end of parent_count ()  */


/*****************************************************************************
 * NAME: record_valid_inode
 *
 * FUNCTION:  Record, in the fsck workspace block map, all aggregate blocks
 *            allocated to the specified inode.  The inode structures have
 *            already been validated, no error checking is done.
 *
 * PARAMETERS:
 *      inoptr        - input - pointer to the current inode
 *      inoidx        - input - ordinal number of the inode as an integer
 *      inorecptr     - input - pointer to an fsck inode record describing the
 *                              current inode
 *      msg_info_ptr  - input - pointer to a record with information needed
 *                              to issue messages about the current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t record_valid_inode ( dinode_t          *inoptr,
                               inoidx_t           inoidx,
                               fsck_inode_recptr  inorecptr,
                               fsck_msg_info_ptr  msg_info_ptr
                             )
{
  retcode_t rvi_rc = FSCK_OK;
  fsblkidx_t  first_fsblk, last_fsblk;
  reg_idx_t  num_fsblks;
  dxd_t *dxd_ptr;

    /*
     * record the extent (if any) containing the EA
     */
  if( inoptr->di_ea.flag == DXD_EXTENT )  {  /* there is an ea to record */
    dxd_ptr = &(inoptr->di_ea);
    first_fsblk = addressDXD( dxd_ptr );
    num_fsblks = lengthDXD( dxd_ptr );
    last_fsblk = first_fsblk + num_fsblks - 1;
    extent_record( first_fsblk, last_fsblk );
    agg_recptr->this_inode.all_blks += num_fsblks;
    agg_recptr->blocks_this_fset += num_fsblks;
    }  /* end there is an ea to record */

    /*
     * record the extent (if any) containing the ACL
     */
  if( inoptr->di_acl.flag == DXD_EXTENT )  {  /* there is an acl to record */
    dxd_ptr = &(inoptr->di_acl);
    first_fsblk = addressDXD( dxd_ptr );
    num_fsblks = lengthDXD( dxd_ptr );
    last_fsblk = first_fsblk + num_fsblks - 1;
    extent_record( first_fsblk, last_fsblk );
    agg_recptr->this_inode.all_blks += num_fsblks;
    agg_recptr->blocks_this_fset += num_fsblks;
    }  /* end there is an acl to record */


    /*
     * record the extents (if any) described as data
     */
  process_valid_data ( inoptr, inoidx, inorecptr, msg_info_ptr, FSCK_RECORD );

  return( rvi_rc );
}                          /* end of record_valid_inode ()  */


/*****************************************************************************
 * NAME: release_inode
 *
 * FUNCTION:  Release all aggregate blocks allocated to the specified inode.
 *            Reset the link count, in the inode on the device, to zero
 *            to make it available for reuse.
 *
 * PARAMETERS:
 *      inoidx      - input - ordinal number of the inode as an integer
 *      ino_recptr  - input - pointer to an fsck inode record describing the
 *                            current inode
 *      inoptr      - input - pointer to the current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t release_inode ( inoidx_t          inoidx,
                          fsck_inode_recptr ino_recptr,
                          dinode_t         *inoptr
                         )
{
  retcode_t ri_rc = FSCK_OK;
  dinode_t  *this_inode;

  int aggregate_inode = 0;      /* going for fileset inodes only */
  int which_it = 0;             /* in release 1 there is only fileset 0 */
  struct fsck_ino_msg_info  ino_msg_info;
  fsck_msg_info_ptr msg_info_ptr;

  msg_info_ptr = &ino_msg_info;
  msg_info_ptr->msg_inopfx = fsck_fset_inode; /* all fileset owned */
  msg_info_ptr->msg_inonum = inoidx;
  if( ino_recptr->inode_type == directory_inode ) {
    msg_info_ptr->msg_inotyp = fsck_directory;
    }
  else if( ino_recptr->inode_type == symlink_inode ) {
    msg_info_ptr->msg_inotyp = fsck_symbolic_link;
    }
  else {  /* a regular file */
    msg_info_ptr->msg_inotyp = fsck_file;
    }

  if( ino_recptr->in_use ) { /* the inode is 'in use' */
    ri_rc = inode_get( aggregate_inode, which_it, inoidx, &this_inode );

    if( ri_rc == FSCK_OK ) {  /* inode read successfully */
      this_inode->di_nlink = 0;
      ri_rc = inode_put( this_inode );

      if( (ri_rc == FSCK_OK) &&
          (!ino_recptr->ignore_alloc_blks) )  {
        ri_rc = unrecord_valid_inode( this_inode, inoidx, ino_recptr, msg_info_ptr );
        }
      }  /* end inode read successfully */
    }  /* end the inode  is in use */

  return( ri_rc );
}                              /* end of release_inode ()  */


/*****************************************************************************
 * NAME: unrecord_valid_inode
 *
 * FUNCTION: Unrecord, in the fsck workspace block map, all aggregate blocks
 *           allocated to the specified inode.  The inode structures have
 *           already been validated, no error checking is done.
 *
 * PARAMETERS:
 *      inoptr        - input - pointer to the current inode
 *      inoidx        - input - ordinal number of the inode as an integer
 *      inorecptr     - input - pointer to an fsck inode record describing
 *                              the current inode
 *      msg_info_ptr  - input - pointer to a record with information needed
 *                              to issue messages about the current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t unrecord_valid_inode ( dinode_t          *inoptr,
                                 inoidx_t           inoidx,
                                 fsck_inode_recptr  inorecptr,
                                 fsck_msg_info_ptr  msg_info_ptr
                               )
{
  retcode_t uvi_rc = FSCK_OK;
  fsblkidx_t ea_blocks = 0;
  fsblkidx_t acl_blocks = 0;

    /*
     * unrecord the extent (if any) containing the EA
     */
  if( (inoptr->di_ea.flag == DXD_EXTENT) &&
       (!inorecptr->ignore_ea_blks)             ) {
    ea_blocks = lengthDXD( &(inoptr->di_ea) );   /* extract length */
    agg_recptr->blocks_for_eas -= ea_blocks;
    uvi_rc = backout_EA( inoptr, inorecptr );
    }

    /*
     * unrecord the extent (if any) containing the ACL
     */
  if( (inoptr->di_acl.flag == DXD_EXTENT) &&
       (!inorecptr->ignore_acl_blks)             ) {
    acl_blocks = lengthDXD( &(inoptr->di_acl) );   /* extract length */
    agg_recptr->blocks_for_acls -= acl_blocks;
    uvi_rc = backout_ACL( inoptr, inorecptr );
    }

    /*
     * unrecord the extents (if any) describing data
     *
     * note that the tree is valid or we'd be ignoring these allocated blocks.
     */
  if( uvi_rc == FSCK_OK )  {

    if( inorecptr->inode_type == directory_inode )  {
      agg_recptr->blocks_for_dirs -= inoptr->di_nblocks - ea_blocks;
      uvi_rc = process_valid_dir_data( inoptr, inoidx, inorecptr,
                                        msg_info_ptr, FSCK_UNRECORD );
      }
    else {
      agg_recptr->blocks_for_files -= inoptr->di_nblocks - ea_blocks;
      uvi_rc = process_valid_data( inoptr, inoidx, inorecptr,
                                                msg_info_ptr, FSCK_UNRECORD );
      }
    }

  return( uvi_rc );
}                          /* end of unrecord_valid_inode ()  */


/*****************************************************************************
 * NAME: validate_ACL
 *
 * FUNCTION: Determine whether the structures in the specified inode to
 *           describe ACL data owned by the inode are consistent and (as
 *           far as fsck can tell) correct.
 *
 * PARAMETERS:
 *      inoptr        - input - pointer to the current inode
 *      inoidx        - input - ordinal number of the inode as an integer
 *      inorecptr     - input - pointer to an fsck inode record describing
 *                              the current inode
 *      msg_info_ptr  - input - pointer to a record with information needed
 *                              to issue messages about the current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t validate_ACL ( dinode_t          *inoptr,
                                    inoidx_t           inoidx,
                                    fsck_inode_recptr  inorecptr,
                                    fsck_msg_info_ptr  msg_info_ptr
                                  )
{
 retcode_t vacl_rc = FSCK_OK;
 retcode_t intermed_rc = FSCK_OK;
 dxd_t *dxd_ptr;
 reg_idx_t recorded_length, shortest_valid, longest_valid;
 reg_idx_t ext_length;
 agg_byteidx_t ext_byte_length;
 fsblkidx_t ext_address;
 char *ext_buf_addr;
 int8 extent_is_valid = 0;
 uint16 size16;
 dinode_t an_inode;
 ULONG  eafmt_error = 0;

 dxd_ptr = &(inoptr->di_acl);
 msg_info_ptr->msg_dxdtyp = fsck_ACL;

 if( dxd_ptr->flag != 0 ) {  /* there is an ACL for this inode */
   if( (dxd_ptr->flag != DXD_EXTENT) &&     /* not a single extent AND */
       (dxd_ptr->flag != DXD_INLINE)&&     /* not inline AND */
       (dxd_ptr->flag != DXD_CORRUPT) ) {    /* not already reported */

     msgprms[0] = message_parm_0;
     msgprmidx[0] = msg_info_ptr->msg_inotyp;
     msgprms[1] = message_parm_1;
     msgprmidx[1] = msg_info_ptr->msg_inopfx;
     sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
     msgprms[2] = message_parm_2;
     msgprmidx[2] = 0;
     msgprms[3] = message_parm_3;
     msgprmidx[3] = msg_info_ptr->msg_dxdtyp;
     fsck_send_msg( fsck_BADINODXDFLDD, 0, 4 );
     inorecptr->clr_acl_fld = 1;
     inorecptr->ignore_acl_blks = 1;
     agg_recptr->corrections_needed = 1;
     }  /* end invalid ACL dxd flag */
   else {  /* else the acl flag is ok */

     if( dxd_ptr->flag == DXD_INLINE ) {   /* ACL is inline  */

       size16 = sizeof(an_inode._data4._data);  /* ACL must fit into here */

       agg_recptr->this_inode.acl_inline = 1;
       agg_recptr->this_inode.inline_acl_length = (uint16) dxd_ptr->size;
       agg_recptr->this_inode.inline_acl_offset = (uint16) addressDXD( dxd_ptr );

       if( (dxd_ptr->size == 0) ||
           (dxd_ptr->size > (size16 - agg_recptr->this_inode.inline_acl_offset))  ) { /*
                                       * the length extends
                                       * beyond the end of the inode
                                       */
         msgprms[0] = message_parm_0;
         msgprmidx[0] = msg_info_ptr->msg_inotyp;
         msgprms[1] = message_parm_1;
         msgprmidx[1] = msg_info_ptr->msg_inopfx;
         sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
         msgprms[2] = message_parm_2;
         msgprmidx[2] = 0;
         msgprms[3] = message_parm_3;
         msgprmidx[3] = msg_info_ptr->msg_dxdtyp;
         fsck_send_msg( fsck_BADINODXDFLDL, 0, 4 );

         inorecptr->clr_acl_fld = 1;
         agg_recptr->corrections_needed = 1;
         }  /* end the length extends beyond the end of the inode */
       }  /* end the ACL is inline */

     else if( dxd_ptr->flag == DXD_EXTENT ) {  /* else the ACL is a single extent */

       ext_length = lengthDXD( dxd_ptr );   /* extract the extent length */
       shortest_valid = (ext_length -1) * sb_ptr->s_bsize + 1;
       longest_valid = ext_length * sb_ptr->s_bsize;
       if( (ext_length == 0) ||
           (dxd_ptr->size < shortest_valid) || (dxd_ptr->size > longest_valid) ) {  /*
                                        * invalid length
                                        */
         msgprms[0] = message_parm_0;
         msgprmidx[0] = msg_info_ptr->msg_inotyp;
         msgprms[1] = message_parm_1;
         msgprmidx[1] = msg_info_ptr->msg_inopfx;
         sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
         msgprms[2] = message_parm_2;
         msgprmidx[2] = 0;
         msgprms[3] = message_parm_3;
         msgprmidx[3] = msg_info_ptr->msg_dxdtyp;
         fsck_send_msg( fsck_BADINODXDFLDL, 0, 4 );

         inorecptr->clr_acl_fld = 1;
         inorecptr->ignore_acl_blks = 1;
         agg_recptr->corrections_needed = 1;
         recorded_length = 0;
         extent_is_valid = 0;
         }  /* end invalid length */
       else {  /* length and size might be ok */
         agg_recptr->this_inode.acl_blks = ext_length;

         ext_address = addressDXD( dxd_ptr ); /* extract the extent address */
         vacl_rc = process_extent( inorecptr, ext_length, ext_address, 0, -1,
                                msg_info_ptr, &recorded_length,
                                &extent_is_valid, FSCK_RECORD_DUPCHECK );
         /*
          * add the blocks in the ACL extent to the running totals for
          * the fileset and inode, but not for the object represented
          * by the object.
          */
         agg_recptr->blocks_this_fset += recorded_length;
         agg_recptr->this_inode.all_blks += recorded_length;
         }  /* end else length and size might be ok */

       if( !extent_is_valid ) {
         inorecptr->clr_acl_fld = 1;
         agg_recptr->corrections_needed = 1;
         }
       }  /* end else the ACL is a single extent */
     }  /* end else the ACL flag is ok */
   }  /* end there is an ACL for this inode */

 return( vacl_rc );
}                              /* end of validate_ACL ()  */


/*                                                                                         begin @F1 */
/*****************************************************************************
 * NAME: validate_dasd_used
 *
 * FUNCTION: Verify that, for each directory with a dasd limit set, the cumulative
 *           amount of dasd used matches the value stored in the inode.
 *
 * PARAMETERS:  none
 *
 * NOTES:               none
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t validate_dasd_used ( )
{
  retcode_t vdu_rc = FSCK_OK;
  inoidx_t ino_idx;
  dinode_t *inoptr;
  fsck_inode_recptr this_inorec;
  int aggregate_inode = 0;      /* going for fileset inodes only */
  int alloc_ifnull = 0;
  int which_it = 0;             /* in release 1 there is only fileset 0 */
  int64 dasd_used = 0;

    /*
     * We make a single pass on the inode records:
     *
     *     if the inode record shows that the inode has a dasd limit,
     *     verify that the dasd used amount stored in the inode matches
     *     the amount calculated for the inode by chkdsk.
     */
  vdu_rc = get_inorecptr_first( aggregate_inode, &ino_idx, &this_inorec );

  while( (vdu_rc == FSCK_OK) && (this_inorec != NULL) && (ino_idx < ROOT_I) ) {  /*
                               * not interesting until we get to the root inode
                               */
    vdu_rc = get_inorecptr_next( aggregate_inode, &ino_idx, &this_inorec );
    }  /* end while */

  while( (vdu_rc == FSCK_OK) && (this_inorec != NULL) ) {

    if( (this_inorec->in_use) &&
        (!this_inorec->selected_to_rls) )  {  /*
                               * inode in use and not selected to release
                               */
        if( this_inorec->inode_type == directory_inode ) {  /*
                        * it's a directory
                        */
            vdu_rc = inode_get( aggregate_inode, which_it, ino_idx, &inoptr );
            if( vdu_rc == FSCK_OK ) {  /* got the inode */
                dasd_used = DASDUSED( &(inoptr->di_DASD) );
                if( dasd_used != this_inorec->cumm_blocks ) {  /*
                        * it isn't right!
                        */
                    this_inorec->crrct_cumm_blks = 1;
                    agg_recptr->corrections_needed = 1;
                    agg_recptr->corrections_approved = 1;
                    }  /* end it isn't right! */
                }  /* end got the inode */
            }  /* end it's a directory */
        }  /* end inode in use and not selected to release  */

    if( vdu_rc == FSCK_OK ) {
        vdu_rc = get_inorecptr_next( aggregate_inode, &ino_idx, &this_inorec );
        }
    }  /* end while */

  return( vdu_rc );
}                              /* end of validate_dasd_used ()  */
/*                                                                                           end @F1 */


/*****************************************************************************
 * NAME: validate_data
 *
 * FUNCTION: Determine whether the structures in, or rooted in, the specified
 *           non-directory inode to describe data owned by the inode are
 *           consistent and (as far as fsck can tell) correct.
 *
 * PARAMETERS:
 *      inoptr        - input - pointer to the current inode
 *      inoidx        - input - ordinal number of the inode as an integer
 *      inorecptr     - input - pointer to an fsck inode record describing
 *                              the current inode
 *      msg_info_ptr  - input - pointer to a record with information needed
 *                              to issue messages about the current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t validate_data  ( dinode_t          *inoptr,
                           inoidx_t           inoidx,
                           fsck_inode_recptr  inorecptr,
                           fsck_msg_info_ptr  msg_info_ptr
                          )
{
 retcode_t vd_rc = FSCK_OK;
 retcode_t intermed_rc = FSCK_OK;
 dxd_t *dxd_ptr;
 reg_idx_t adj_length;
 int8 extent_is_valid = 0;
 reg_idx_t ext_length;
 fsblkidx_t ext_addr;
 fsblkidx_t ext_offset;
 uint16 size16;
 dinode_t an_inode;
 xtpage_t *xtp_ptr;


 dxd_ptr = &(inoptr->di_dxd);
 msg_info_ptr->msg_dxdtyp = fsck_objcontents;  /* the data root dxd */

 /*
  * examine the data field
  */
 if( dxd_ptr->flag != 0 ) {  /* there is data for this inode */

   if( (dxd_ptr->flag == (DXD_INDEX|BT_ROOT|BT_LEAF))    ||
       (dxd_ptr->flag == (DXD_INDEX|BT_ROOT|BT_INTERNAL))  ) { /*
                               * to be valid, it has to be a B-tree node,
                               * either root-leaf or root-internal
                               */
       /*
        * figure out how much space the root occupies in the inode itself
        */
     xtp_ptr = (xtpage_t *) (&inoptr->di_btroot);
     agg_recptr->this_inode.in_inode_data_length =
                         (xtp_ptr->header.maxentry - 2) * sizeof( xad_t ); /*
                               * the dxd actually starts 32 bytes (== 2 * length of
                               * an xad) before the boundary.
                               * the 0th and 1st entries in the xad array are
                               * really the header
                               */

       /*
        * validate the tree contents and record the extents it
        * describes until and unless the tree is found to be corrupt
        */
     vd_rc = xTree_processing ( inoptr, inoidx, inorecptr,
                                msg_info_ptr, FSCK_RECORD_DUPCHECK );

     if( vd_rc >= FSCK_OK ) {  /* nothing fatal */

       if( inorecptr->selected_to_rls && inode_is_metadata(inorecptr) ) {
         vd_rc = FSCK_BADMDDATAIDX;
         }
       else if( inorecptr->ignore_alloc_blks ) { /*
                                       * the tree info can't be used
                                       */
         if( inode_is_metadata(inorecptr) ) {
           vd_rc = FSCK_BADMDDATAIDX;
           }
           /*
            * reverse the notations made when recording the extents
            * for the tree.  Again, stop when the point of corruption
            * is found since that's where the recording process was
            * stopped.
            */
         intermed_rc = xTree_processing ( inoptr, inoidx, inorecptr,
                                          msg_info_ptr, FSCK_UNRECORD );
         if( intermed_rc < 0 ) {  /* something fatal here */
           vd_rc = intermed_rc;
           }  /* end something fatal here */
         else {  /* nothing fatal */
           if( intermed_rc != FSCK_OK ) { /* but something wrong */
             if( vd_rc == FSCK_OK ) {  /* first problem */
               vd_rc = intermed_rc;
               }  /* end first problem */
             }  /* end but something wrong */

           if( !inorecptr->ignore_ea_blks ) {
             intermed_rc = backout_EA( inoptr, inorecptr );
             if( intermed_rc < 0 )  {  /* something fatal backing out EA */
               vd_rc = intermed_rc;
               }  /* end something fatal backing out EA */
             else {  /* nothing fatal backing out EA */
               if( intermed_rc != FSCK_OK ) { /* but something wrong */
                 if( vd_rc == FSCK_OK ) {  /* first problem seen */
                 vd_rc = intermed_rc;
                 }  /* end first problem seen */
                 }  /* end but something wrong */

               if( !inorecptr->ignore_acl_blks ) {
                 intermed_rc = backout_ACL( inoptr, inorecptr );
                 if( intermed_rc < 0 )  {  /* something fatal backing out EA */
                   vd_rc = intermed_rc;
                   }  /* end something fatal backing out ACL */
                 else {  /* nothing fatal backing out ACL */
                   if( intermed_rc != FSCK_OK ) { /* but something wrong */
                     if( vd_rc == FSCK_OK ) {  /* first problem seen */
                     vd_rc = intermed_rc;
                     }  /* end first problem seen */
                     }  /* end but something wrong */
                   }  /* end else nothing fatal backing out ACL */
                 }  /* end if !inorecptr->ignore_acl_blks */
               }  /* end else nothing fatal backing out EA */
             }  /* end if !inorecptr->ignore_ea_blks */
           }  /* end else nothing fatal */
          }  /* end the tree info can't be used */
       }  /* end nothing fatal */
     }  /* end to be valid, it has to be ...  */
   else {  /* else not B+ Tree index */

           /*
            * the data root is not valid...the info cannot be trusted
            */
     if( inode_is_metadata(inorecptr) ) {
       vd_rc = FSCK_BADMDDATA;                 /* bail out */
       }
     else {
       inorecptr->selected_to_rls = 1;
       inorecptr->ignore_alloc_blks = 1;
       agg_recptr->corrections_needed = 1;
       }
     msgprms[0] = message_parm_0;
     msgprmidx[0] = msg_info_ptr->msg_inotyp;
     msgprms[1] = message_parm_1;
     msgprmidx[1] = msg_info_ptr->msg_inopfx;
     sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
     msgprms[2] = message_parm_2;
     msgprmidx[2] = 0;
     msgprms[3] = message_parm_3;
     msgprmidx[3] = msg_info_ptr->msg_dxdtyp;
     fsck_send_msg( fsck_BADINODXDFLDD, 0, 4 );
     }  /* end else not B+ Tree index */
   }  /* end there is data for this inode */

 return( vd_rc );
}                              /* end of validate_data ()  */


/*****************************************************************************
 * NAME: validate_dir_data
 *
 * FUNCTION: Determine whether the structures in, or rooted in, the
 *           specified directory inode to describe data owned by the
 *           inode are consistent and (as far as fsck can tell)
 *           correct.
 *
 * PARAMETERS:
 *      inoptr        - input - pointer to the current inode
 *      inoidx        - input - ordinal number of the inode as an integer
 *      inorecptr     - input - pointer to an fsck inode record describing the
 *                              current inode
 *      msg_info_ptr  - input - pointer to a record with information needed
 *                              to issue messages about the current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t validate_dir_data  ( dinode_t          *inoptr,
                               inoidx_t           inoidx,
                               fsck_inode_recptr  inorecptr,
                               fsck_msg_info_ptr  msg_info_ptr
                              )
{
 retcode_t vdd_rc = FSCK_OK;
 retcode_t intermed_rc = FSCK_OK;
 dxd_t *dxd_ptr;
 reg_idx_t adj_length;
 int8 extent_is_valid = 0;
 reg_idx_t ext_length;
 fsblkidx_t ext_addr;
 fsblkidx_t ext_offset;
 uint16 size16;
 dinode_t an_inode;

 dxd_ptr = &(inoptr->di_dxd);
 msg_info_ptr->msg_dxdtyp = fsck_objcontents;  /* the data root dxd */

 /*
  * examine the data field
  */
 if( dxd_ptr->flag != 0 ) {  /* there is data for this inode */

   if( (dxd_ptr->flag == (DXD_INDEX|BT_ROOT|BT_LEAF))    ||
       (dxd_ptr->flag == (DXD_INDEX|BT_ROOT|BT_INTERNAL))  ) { /*
                               * to be valid, it has to be a B-tree node,
                               * either root-leaf or root-internal
                               */
       /*
        * figure out how much space the root occupies in the inode itself
        */
     agg_recptr->this_inode.in_inode_data_length = (DTROOTMAXSLOT - 1) *
                                                         sizeof( dtslot_t ); /*
                                * The root actually starts 32 bytes (aka the length
                                * of 1 slot) before the boundary.
                                * the 1st slot is really the header
                                */

       /*
        * validate the tree contents and record the extents it
        * describes until and unless the tree is found to be corrupt
        */
     vdd_rc = dTree_processing ( inoptr, inoidx, inorecptr,
                                  msg_info_ptr, FSCK_RECORD_DUPCHECK );

     if( vdd_rc >= FSCK_OK ) {  /* nothing fatal */

       if( inorecptr->ignore_alloc_blks ) { /*
                                       * the tree info can't be used
                                       */
           /*
            * reverse the notations made when recording the extents
            * for the tree.  Again, stop when the point of corruption
            * is found since that's where the recording process was
            * stopped.
            */
         intermed_rc = dTree_processing ( inoptr, inoidx, inorecptr,
                                          msg_info_ptr, FSCK_UNRECORD );
         if( intermed_rc < 0 ) {  /* something fatal here */
           vdd_rc = intermed_rc;
           }  /* end something fatal here */
         else {  /* nothing fatal */
           if( intermed_rc != FSCK_OK ) { /* but something wrong */
             if( vdd_rc == FSCK_OK ) {  /* first problem */
               vdd_rc = intermed_rc;
               }  /* end first problem */
             }  /* end but something wrong */

           if( !inorecptr->ignore_ea_blks ) {
             intermed_rc = backout_EA( inoptr, inorecptr );
             if( intermed_rc < 0 )  {  /* something fatal backing out EA */
               vdd_rc = intermed_rc;
               }  /* end something fatal backing out EA */
             else {  /* nothing fatal backing out EA */
               if( intermed_rc != FSCK_OK ) { /* but something wrong */
                 if( vdd_rc == FSCK_OK ) {  /* first problem seen */
                 vdd_rc = intermed_rc;
                 }  /* end first problem seen */
                 }  /* end but something wrong */

               if( !inorecptr->ignore_ea_blks ) {
                 intermed_rc = backout_ACL( inoptr, inorecptr );
                 if( intermed_rc < 0 )  {  /* something fatal backing out ACL */
                   vdd_rc = intermed_rc;
                   }  /* end something fatal backing out ACL */
                 else {  /* nothing fatal backing out ACL */
                   if( intermed_rc != FSCK_OK ) { /* but something wrong */
                     if( vdd_rc == FSCK_OK ) {  /* first problem seen */
                     vdd_rc = intermed_rc;
                     }  /* end first problem seen */
                     }  /* end but something wrong */
                   }  /* end else nothing fatal backing out ACL */
                 }  /* end if !inorecptr->ignore_ea_blks */
               }  /* end else nothing fatal backing out EA */
             }  /* end if !inorecptr->ignore_ea_blks */
           }  /* end else nothing fatal */
         }  /* end the tree info can't be used */
       }  /* end nothing fatal */
     }  /* end to be valid, it has to be ...  */
   else {  /* else not B+ Tree index */

           /*
            * the data root is not valid...the info cannot be trusted
            */
     inorecptr->selected_to_rls = 1;
     inorecptr->ignore_alloc_blks = 1;
     agg_recptr->corrections_needed = 1;

     msgprms[0] = message_parm_0;
     msgprmidx[0] = msg_info_ptr->msg_inotyp;
     msgprms[1] = message_parm_1;
     msgprmidx[1] = msg_info_ptr->msg_inopfx;
     sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
     msgprms[2] = message_parm_2;
     msgprmidx[2] = 0;
     msgprms[3] = message_parm_3;
     msgprmidx[3] = msg_info_ptr->msg_dxdtyp;
     fsck_send_msg( fsck_BADINODXDFLDD, 0, 4 );
     }  /* end else not B+ Tree index */
   }  /* end there is data for this inode */

 return( vdd_rc );
}                              /* end of validate_dir_data ()  */


/*****************************************************************************
 * NAME: validate_EA
 *
 * FUNCTION: Determine whether the structures in the specified inode to
 *           describe ea data owned by the inode are consistent and (as
 *           far as fsck can tell) correct.
 *
 * PARAMETERS:
 *      inoptr        - input - pointer to the current inode
 *      inoidx        - input - ordinal number of the inode as an integer
 *      inorecptr     - input - pointer to an fsck inode record describing
 *                              the current inode
 *      msg_info_ptr  - input - pointer to a record with information needed
 *                              to issue messages about the current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t validate_EA ( dinode_t          *inoptr,
                        inoidx_t           inoidx,
                        fsck_inode_recptr  inorecptr,
                        fsck_msg_info_ptr  msg_info_ptr
                      )
{
 retcode_t vea_rc = FSCK_OK;
 retcode_t intermed_rc = FSCK_OK;
 dxd_t *dxd_ptr;
 reg_idx_t recorded_length, shortest_valid, longest_valid;
 reg_idx_t ext_length;
 agg_byteidx_t ext_byte_length;
 fsblkidx_t ext_address;
 char *ext_buf_addr;
 int8 extent_is_valid = 0;
 int8 ea_format_bad = 0;
 uint16 size16;
 dinode_t an_inode;
 ULONG  eafmt_error = 0;

 dxd_ptr = &(inoptr->di_ea);
 msg_info_ptr->msg_dxdtyp = fsck_EA;

 if( dxd_ptr->flag != 0 ) {  /* there is an EA for this inode */
   if( (dxd_ptr->flag != DXD_EXTENT) &&     /* not a single extent AND */
       (dxd_ptr->flag != DXD_INLINE) ) {    /* not inline */

     msgprms[0] = message_parm_0;
     msgprmidx[0] = msg_info_ptr->msg_inotyp;
     msgprms[1] = message_parm_1;
     msgprmidx[1] = msg_info_ptr->msg_inopfx;
     sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
     msgprms[2] = message_parm_2;
     msgprmidx[2] = 0;
     msgprms[3] = message_parm_3;
     msgprmidx[3] = msg_info_ptr->msg_dxdtyp;
     fsck_send_msg( fsck_BADINODXDFLDD, 0, 4 );
     inorecptr->clr_ea_fld = 1;
     inorecptr->ignore_ea_blks = 1;
     agg_recptr->corrections_needed = 1;
     }  /* end invalid EA dxd flag */
   else {  /* else the ea flag is ok */

     if( dxd_ptr->flag == DXD_INLINE ) {   /* EA is inline  */

       size16 = sizeof(an_inode._data4._data);  /* EA must fit into here */

       agg_recptr->this_inode.ea_inline = 1;
       agg_recptr->this_inode.inline_ea_length = dxd_ptr->size;
       agg_recptr->this_inode.inline_ea_offset = (uint16) addressDXD( dxd_ptr );

       if( (dxd_ptr->size == 0) ||
           (dxd_ptr->size > (size16 - agg_recptr->this_inode.inline_ea_offset)) ) { /*
                                       * the length extends
                                       * beyond the end of the inode
                                       */
         msgprms[0] = message_parm_0;
         msgprmidx[0] = msg_info_ptr->msg_inotyp;
         msgprms[1] = message_parm_1;
         msgprmidx[1] = msg_info_ptr->msg_inopfx;
         sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
         msgprms[2] = message_parm_2;
         msgprmidx[2] = 0;
         msgprms[3] = message_parm_3;
         msgprmidx[3] = msg_info_ptr->msg_dxdtyp;
         fsck_send_msg( fsck_BADINODXDFLDL, 0, 4 );

         inorecptr->clr_ea_fld = 1;
         agg_recptr->corrections_needed = 1;
         }  /* end the length extends beyond the end of the inode */

       else {  /* the inline ea has a valid length.  verify its format. */
         vea_rc = jfs_ValidateFEAList( (FEALIST *) &(inoptr->_data4._data),
                                       &eafmt_error );
         if( (vea_rc != FSCK_OK) || (eafmt_error != 0) ) {  /* ea format is bad */
           ea_format_bad = -1;
           }  /* end ea format is bad */
         }  /* end the inline ea has a valid length.  verify its format.  */
       }  /* end the EA is inline */

     else {  /* else the EA is a single extent */

       ext_length = lengthDXD( dxd_ptr );   /* extract the extent length */
       shortest_valid = (ext_length -1) * sb_ptr->s_bsize + 1;
       longest_valid = ext_length * sb_ptr->s_bsize;
       if( (ext_length == 0) ||
           (dxd_ptr->size < shortest_valid) || (dxd_ptr->size > longest_valid) ) {  /*
                                        * invalid length
                                        */
         msgprms[0] = message_parm_0;
         msgprmidx[0] = msg_info_ptr->msg_inotyp;
         msgprms[1] = message_parm_1;
         msgprmidx[1] = msg_info_ptr->msg_inopfx;
         sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
         msgprms[2] = message_parm_2;
         msgprmidx[2] = 0;
         msgprms[3] = message_parm_3;
         msgprmidx[3] = msg_info_ptr->msg_dxdtyp;
         fsck_send_msg( fsck_BADINODXDFLDL, 0, 4 );

         extent_is_valid = 0;
         inorecptr->ignore_ea_blks = 1;
         }  /* end invalid length */
       else {  /* length and size might be ok */
         agg_recptr->this_inode.ea_blks = ext_length;
         ext_address = addressDXD( dxd_ptr ); /* extract the extent address */
         vea_rc = process_extent( inorecptr, ext_length, ext_address, -1, 0,
                                msg_info_ptr, &recorded_length,
                                &extent_is_valid, FSCK_RECORD_DUPCHECK );
         /*
        * add the blocks in the EA extent to the running totals for
        * the filese and inode, but not for the object represented
        * by the object.
        */
         agg_recptr->blocks_this_fset += recorded_length;
         agg_recptr->this_inode.all_blks += recorded_length;
         }  /* end else length and size might be ok */

       if( !extent_is_valid ) {
         inorecptr->clr_ea_fld = 1;
         agg_recptr->corrections_needed = 1;
         }
       else {  /* the extent looks ok so need to check ea data structure */
         ext_byte_length = ext_length * sb_ptr->s_bsize;

         if( ext_byte_length > agg_recptr->ea_buf_length ) {  /*
                                * extra large ea  -- can't check it
                                */
           inorecptr->cant_chkea = 1;
           agg_recptr->warning_pending = 1;
           }  /* end extra large ea  -- can't check it */

         else {  /* regular size ea */
           intermed_rc = ea_get( ext_address, ext_byte_length,
                                 agg_recptr->ea_buf_ptr,
                                 &(agg_recptr->ea_buf_length),
                                 &(agg_recptr->ea_buf_data_len),
                                 &(agg_recptr->ea_agg_offset) );
           if( intermed_rc != FSCK_OK ) {  /* read failed */
             msgprms[0] = message_parm_0;
             msgprmidx[0] = msg_info_ptr->msg_inotyp;
             msgprms[1] = message_parm_1;
             msgprmidx[1] = msg_info_ptr->msg_inopfx;
             sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
             msgprms[2] = message_parm_2;
             msgprmidx[2] = 0;
             msgprms[3] = message_parm_3;
             msgprmidx[3] = msg_info_ptr->msg_dxdtyp;
             fsck_send_msg( fsck_BADINODXDFLDO, 0, 4 );

             inorecptr->clr_ea_fld = 1;
             agg_recptr->corrections_needed = 1;
             }  /* end read failed */
           else {  /* the ea has been read into the regular buffer */
             vea_rc = jfs_ValidateFEAList( (FEALIST *) agg_recptr->ea_buf_ptr,
                                            &eafmt_error
                                           );
             if( (vea_rc != FSCK_OK) || (eafmt_error != 0) ) {  /* ea format is bad */
               ea_format_bad = -1;
               }  /* end ea format is bad */
             }  /* end else the ea has been read into the regular buffer */
           }  /* end else regular size ea */
         }  /* end else the extent looks ok so need to check ea data ... */
       }  /* end else the EA is a single extent */
     }  /* end else the ea flag is ok */

   if( ea_format_bad ) {  /* bad ea but haven't notified anyone */
     msgprms[0] = message_parm_0;
     msgprmidx[0] = msg_info_ptr->msg_inotyp;
     msgprms[1] = message_parm_1;
     msgprmidx[1] = msg_info_ptr->msg_inopfx;
     sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
     msgprms[2] = message_parm_2;
     msgprmidx[2] = 0;
     fsck_send_msg( fsck_EAFORMATBAD, 0, 3 );
     inorecptr->clr_ea_fld = 1;
     agg_recptr->corrections_needed = 1;
     vea_rc = FSCK_OK;
     }  /* end bad ea but haven't notified anyone */

   }  /* end there is an EA for this inode */

 return( vea_rc );
}                              /* end of validate_EA ()  */


/*****************************************************************************
 * NAME: validate_record_fileset_inode
 *
 * FUNCTION:  Determine whether structures in and/or rooted in the specified
 *            fileset owned inode are consistent and (as far as fsck can tell)
 *            correct.  Record, in the fsck workspace block map, all storage
 *            allocated to the inode.
 *
 * PARAMETERS:
 *      inonum            - input - ordinal number of the inode in the
 *                                  internal JFS format
 *      inoidx            - input - ordinal number of the inode as an integer
 *      inoptr            - input - pointer to the current inode
 *      ino_msg_info_ptr  - input - pointer to a record with information needed
 *                                  to issue messages about the current inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t validate_record_fileset_inode ( ino_t             inonum,
                                          inoidx_t          inoidx,
                                          dinode_t         *inoptr,
                                          fsck_msg_info_ptr ino_msg_info_ptr
                                         )
{
  retcode_t vrfi_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  struct dtroot_t *dtroot_ptr;
  fsck_inode_recptr inorecptr;
  int8 bad_size = 0;
  int aggregate_inode = 0;
  int alloc_ifnull = -1;
  agg_byteidx_t min_size, max_size;
  int16 size16;
  int8 dinode_sect4_avail = 0;
  dinode_t an_inode;


  ino_msg_info_ptr->msg_inonum = inonum;

  vrfi_rc = get_inorecptr( aggregate_inode, alloc_ifnull, inoidx, &inorecptr );

  if( vrfi_rc == FSCK_OK )  {  /* nothing fatal yet */

    inorecptr->in_use = 1;

    if( !(inode_type_recognized( inoptr )) ) { /* bad type */
      inorecptr->inode_type = unrecognized_inode;
      ino_msg_info_ptr->msg_inotyp = fsck_file;    /* have to use something */
      inorecptr->selected_to_rls = 1;
      inorecptr->ignore_alloc_blks = 1; /* no matter what the user
                                    * approves or disapproves, we aren't
                                    * going to even look to see which
                                    * blocks are allocated to this inode
                                    * (except for the blocks it occupies
                                    * itself)
                                    */
      agg_recptr->corrections_needed = 1;
      msgprms[0] = message_parm_0;
      msgprmidx[0] = ino_msg_info_ptr->msg_inopfx;
      sprintf( message_parm_1, "%ld", ino_msg_info_ptr->msg_inonum );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      fsck_send_msg( fsck_BADINOTYP, 0, 2 );
      }  /* end bad type field */

    else {  /* else type is recognized as valid */

      /*
       * clear the workspace area for the current inode
       */
      memset( (void *) (&(agg_recptr->this_inode)), '\0',
              sizeof( agg_recptr->this_inode ) );
      memcpy( (void *) &(agg_recptr->this_inode.eyecatcher),
              (void *) "thisinod", 8 );


      /*
       * finish filling in the inode's workspace record
       */
      if( ISDIR(inoptr->di_mode) ) {
        inorecptr->inode_type = directory_inode;
        ino_msg_info_ptr->msg_inotyp = fsck_directory;
        }
      else if( ISLNK(inoptr->di_mode) ) {
        inorecptr->inode_type = symlink_inode;
        ino_msg_info_ptr->msg_inotyp = fsck_symbolic_link;
        }
      else {  /* a regular file */
        inorecptr->inode_type = file_inode;
        ino_msg_info_ptr->msg_inotyp = fsck_file;
        }

      inorecptr->link_count -= inoptr->di_nlink;
      inorecptr->dtree_level = -1;                                                    /* @F1 */

      /*
       * validate the inode's structures
       */
      vrfi_rc = validate_EA(inoptr,inoidx,inorecptr,ino_msg_info_ptr); /*
                                   * validate the Extended Attributes if any
                                   */
      if( vrfi_rc == FSCK_OK ) { /* nothing fatal with the EA */
        vrfi_rc = validate_ACL(inoptr,inoidx,inorecptr,ino_msg_info_ptr); /*
                                   * validate the Access Control List if any
                                   */
        }
      if( vrfi_rc == FSCK_OK ) { /* nothing fatal with the EA or ACL*/
        if( inoptr->di_size > 0 ) {  /* there is data there */

          if( inorecptr->inode_type == directory_inode ) {
            vrfi_rc = validate_dir_data ( inoptr, inoidx, inorecptr,
                                          ino_msg_info_ptr ); /* validate
                                     * the data, if any,  whether inline,
                                     * a single extent, or a B+ Tree
                                     */
            }
          else {
            vrfi_rc = validate_data ( inoptr, inoidx, inorecptr,
                                      ino_msg_info_ptr ); /* validate the
                                     * data, if any,  whether inline, a
                                     * single extent, or a B+ Tree
                                     */
            }
          }  /* end there is data there */
        if( vrfi_rc == FSCK_OK ) {  /* nothing fatal with the data */
          if( inorecptr->ignore_alloc_blks ) {
            inorecptr->selected_to_rls = 1;
            agg_recptr->corrections_needed = 1;
            }

          if( !inorecptr->selected_to_rls ) {  /*
                                   * not selected to release yet
                                   */
            if( inoptr->di_nblocks != agg_recptr->this_inode.all_blks ) { /*
                                   * number of blocks is wrong.  tree must
                                   * be bad
                                   */

#ifdef _JFS_DEBUG
 printf("inode: %ld (t)   di_nblocks = %lld (t)   this_inode.all_blks = %lld (t)\n\r",
        inonum, inoptr->di_nblocks, agg_recptr->this_inode.all_blks );
#endif

              msgprms[0] = message_parm_0;
              msgprmidx[0] = ino_msg_info_ptr->msg_inotyp;
              msgprms[1] = message_parm_1;
              msgprmidx[1] = ino_msg_info_ptr->msg_inopfx;
              sprintf( message_parm_2, "%ld", ino_msg_info_ptr->msg_inonum );
              msgprms[2] = message_parm_2;
              msgprmidx[2] = 0;
              sprintf( message_parm_3, "%d", 9 );
              msgprms[3] = message_parm_3;
              msgprmidx[3] = 0;
              fsck_send_msg( fsck_BADKEYS, 0, 4 );

              inorecptr->selected_to_rls = 1;
              inorecptr->ignore_alloc_blks = 1;
              agg_recptr->corrections_needed = 1;
              bad_size = -1;
              }  /* end number of blocks is wrong */
            else {
                 /*
                  * the data size (in bytes) must not exceed the total size
                  * of the blocks allocated for it and must use at least 1
                  * byte in the last fsblock allocated for it.
                  */
              if( agg_recptr->this_inode.data_size == 0 ) {
                if( inorecptr->inode_type == directory_inode ) {
                  min_size = IDATASIZE;
                  max_size = IDATASIZE;
                  }
                else {  /* not a directory */
                  min_size = 0;
                  max_size = IDATASIZE;
                  }  /* end else not a directory */
                }
              else {  /* blocks are allocated to data */
                min_size =
                       agg_recptr->this_inode.data_size - sb_ptr->s_bsize + 1;
                max_size = agg_recptr->this_inode.data_size;
                }  /* end else blocks are allocated to data */

              if( (inoptr->di_size > max_size) ||
                  (inoptr->di_size < min_size) ) { /*
                                   * object size (in bytes) is wrong.
                                   * tree must be bad.
                                   */

#ifdef _JFS_DEBUG
printf("inode: %ld (t)   min_size = %lld (t)   max_size = %lld (t)  di_size = %lld (t)\n\r",
        inonum, min_size, max_size, inoptr->di_size );
#endif
                     msgprms[0] = message_parm_0;
                     msgprmidx[0] = ino_msg_info_ptr->msg_inotyp;
                     msgprms[1] = message_parm_1;
                     msgprmidx[1] = ino_msg_info_ptr->msg_inopfx;
                     sprintf( message_parm_2, "%ld", ino_msg_info_ptr->msg_inonum );
                     msgprms[2] = message_parm_2;
                     msgprmidx[2] = 0;
                     sprintf( message_parm_3, "%d", 10 );
                     msgprms[3] = message_parm_3;
                     msgprmidx[3] = 0;
                     fsck_send_msg( fsck_BADKEYS, 0, 4 );

                     inorecptr->selected_to_rls = 1;
                     inorecptr->ignore_alloc_blks = 1;
                     agg_recptr->corrections_needed = 1;
                     bad_size = -1;
                     }  /* end object size is wrong.  */
              }
            }  /* end not selected to release yet */

          if( !(inorecptr->ignore_alloc_blks) ) {  /* the tree looks ok */
            intermed_rc = in_inode_data_check( inorecptr, ino_msg_info_ptr );
               if( inorecptr->selected_to_rls ) {
                 msgprms[0] = message_parm_0;
                 msgprmidx[0] = ino_msg_info_ptr->msg_inotyp;
                 msgprms[1] = message_parm_1;
                 msgprmidx[1] = ino_msg_info_ptr->msg_inopfx;
                 sprintf( message_parm_2, "%ld", ino_msg_info_ptr->msg_inonum );
                 msgprms[2] = message_parm_2;
                 msgprmidx[2] = 0;
                 sprintf( message_parm_3, "%d", 39 );
                 msgprms[3] = message_parm_3;
                 msgprmidx[3] = 0;
                 fsck_send_msg( fsck_BADKEYS, 0, 4 );
                 }
            else {  /* not selected to release */
                     /* in_inode_data_check came out ok.
                      * now check to be sure the mode bit INLINEEA is set properly
                      *
                      * N.B. if not, we'll correct the mode bit.  We won't release the
                      *       inode for this.
                      */
                 dinode_sect4_avail = 0;
                 size16 = sizeof(an_inode._data3._data);  /* if in-inode data (or
                                     * description of data) overflows this then
                                     * section 4 of the disk inode is NOT available
                                     */
                 if( agg_recptr->this_inode.in_inode_data_length > size16 ) {  /*
                                     * extra long inline data
                                     */
                   if( (inoptr->di_mode & INLINEEA) == INLINEEA ) {
                     inorecptr->inlineea_off = 1;
                     }
                   }
                 else {  /* not extra long inline data */
                   if( ( !(agg_recptr->this_inode.ea_inline) ||
                         (inorecptr->clr_ea_fld)                      ) &&
                       ( !(agg_recptr->this_inode.acl_inline) ||
                         (inorecptr->clr_acl_fld)                      )     ) {  /*
                                     * if (either ea isn't inline OR ea being cleared)
                                     * AND (either acl isn't inline OR acl being cleared)
                                     */
                     dinode_sect4_avail = -1;
                     }  /* end if (either ea isn't inline OR ea being cleared) ... */
                               /*
                                * if we know section 4 is (or will be) available but
                                * the flag is off, then flag it to turn the flag on.
                                */
                   if( (dinode_sect4_avail) &&
                       ( (inoptr->di_mode & INLINEEA) != INLINEEA ) ) {
                     inorecptr->inlineea_on = 1;
                     agg_recptr->corrections_needed = 1;
                     }
                               /*
                                * if we know section 4 is (or will be) unavailable
                                * but the flag is on, then flag it to turn the flag off.
                                */
                   else if( (!dinode_sect4_avail) &&
                       ( (inoptr->di_mode & INLINEEA) == INLINEEA ) ) {
                     inorecptr->inlineea_off = 1;
                     agg_recptr->corrections_needed = 1;
                     }
                   }  /* end else not extra long inline data  */
                 }  /* end else not selected to release */
               }  /* end the tree looks ok */
          else {  /* the tree is not valid */
             /*
              * If bad_size is set then we didn't know that
              * the tree was bad until we looked at the size
              * fields.  This means that the block usage recorded
              * for this inode has not been backed out yet.
              */
            if( bad_size ) {  /* tree is bad by implication */
              if( !inorecptr->ignore_ea_blks ) {
                backout_EA( inoptr, inorecptr ); /* remove traces, in
                                   * the fsck workspace maps, of the blocks
                                   * allocated to this inode
                                   */
                }
              if( !inorecptr->ignore_acl_blks ) {
                backout_ACL( inoptr, inorecptr ); /* remove traces, in
                                   * the fsck workspace maps, of the blocks
                                   * allocated to this inode
                                   */
                }
              if( inorecptr->inode_type == directory_inode ) {
                process_valid_dir_data ( inoptr, inoidx, inorecptr,
                                         ino_msg_info_ptr, FSCK_UNRECORD ); /*
                                   * remove traces, in the fsck workspace
                                   * maps, of the blocks allocated to data
                                   * for this inode, whether a single
                                   * extent or a B+ Tree
                                   */
                }
              else {
                process_valid_data ( inoptr, inoidx, inorecptr,
                                     ino_msg_info_ptr, FSCK_UNRECORD ); /*
                                   * remove traces, in the fsck workspace
                                   * maps, of the blocks allocated to data
                                   * for this inode, whether a single
                                   * extent or a B+ Tree
                                   */
                }
              }  /* end tree is bad by implication */
            }  /* end else the tree is not valid */
          }  /* end nothing fatal with the data */
        }  /* end nothing fatal with the EA */
      }  /* end else type is recognized as valid */
    }  /* end nothing fatal yet */

  if( (vrfi_rc == FSCK_OK) && (!inorecptr->selected_to_rls) ) {  /*
                                   * looks like a keeper
                                   */
    inorecptr->cumm_blocks = agg_recptr->this_inode.all_blks;            /* @F1 */
    agg_recptr->blocks_for_eas += agg_recptr->this_inode.ea_blks;
    agg_recptr->blocks_for_acls += agg_recptr->this_inode.acl_blks;
    if( inorecptr->inode_type == directory_inode ) {  /* a directory */
      agg_recptr->blocks_for_dirs = agg_recptr->blocks_for_dirs +
                                    agg_recptr->this_inode.all_blks -
                                    agg_recptr->this_inode.ea_blks -
                                    agg_recptr->this_inode.acl_blks;
      }  /* end a directory */
    else {  /* a file */
      agg_recptr->blocks_for_files = agg_recptr->blocks_for_files +
                                     agg_recptr->this_inode.all_blks -
                                     agg_recptr->this_inode.ea_blks -
                                     agg_recptr->this_inode.acl_blks;
      }  /* end else a file */
    }  /* end looks like a keeper */

  return( vrfi_rc );
}                          /* end of validate_record_fileset_inode ()  */
