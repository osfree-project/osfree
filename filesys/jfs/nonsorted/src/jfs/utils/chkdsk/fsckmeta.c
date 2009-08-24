/* $Id: fsckmeta.c,v 1.3 2004/03/21 02:43:27 pasha Exp $ */

static char *SCCSID = "@(#)1.37  6/28/99 13:49:50 src/jfs/utils/chkdsk/fsckmeta.c, jfschk, w45.fs32, fixbld";
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
 *   MODULE_NAME:	fsckmeta.c 
 *
 *   COMPONENT_NAME: jfschk
 *
 *   FUNCTIONS:
 *              agg_clean_or_dirty
 *              fatal_dup_check
 *              first_ref_check_agg_metadata
 *              first_ref_check_fixed_metadata
 *              first_ref_check_fs_metadata
 *              record_fixed_metadata
 *              replicate_superblock
 *              validate_fs_metadata
 *              validate_repair_superblock
 *              validate_select_agg_inode_table
 *
 *              backout_ait_part1
 *              backout_valid_agg_inode
 *              first_ref_check_other_ait
 *              record_ait_part1_again
 *              record_other_ait
 *              rootdir_tree_bad
 *              validate_super
 *              validate_super_2ndaryAI
 *              verify_agg_fileset_inode
 *              verify_ait_inode
 *              verify_ait_part1
 *              verify_ait_part2
 *              verify_badblk_inode
 *              verify_bmap_inode
 *              verify_fs_super_ext
 *              verify_log_inode
 *              verify_metadata_data
 *              verify_repair_fs_rootdir
 *
 */

/*
 * defines and includes common among the xfsck modules
 */
#include "xfsckint.h"

#include <time.h>

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
extern char *terse_msg_ptr;
extern char *verbose_msg_ptr;

extern char *MsgText[];

extern char *Vol_Label;

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

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * Device information.
  *
  *      defined in xchkdsk.c
  */
extern HFILE  Dev_IOPort;
extern uint32 Dev_blksize;

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 *
 * The following are internal to this file
 *
 */

retcode_t backout_ait_part1( int );

retcode_t backout_valid_agg_inode( int, inoidx_t, fsck_msg_info_ptr );

retcode_t first_ref_check_other_ait( void );

retcode_t record_ait_part1_again( int );

retcode_t record_other_ait( void );

retcode_t rootdir_tree_bad( dinode_t *, int * );

retcode_t validate_super( int );

retcode_t validate_super_2ndaryAI( int );

retcode_t verify_agg_fileset_inode( dinode_t *, inoidx_t, int,
                                    fsck_msg_info_ptr );

retcode_t verify_ait_inode( dinode_t *, int, fsck_msg_info_ptr );

retcode_t verify_ait_part1( int );

retcode_t verify_ait_part2( int );

retcode_t verify_badblk_inode( dinode_t *, int, fsck_msg_info_ptr );

retcode_t verify_bmap_inode( dinode_t *, int, fsck_msg_info_ptr );

retcode_t verify_fs_super_ext( dinode_t *, fsck_msg_info_ptr, int * );

retcode_t verify_log_inode( dinode_t *, int, fsck_msg_info_ptr );

retcode_t verify_metadata_data( dinode_t  *, inoidx_t, fsck_inode_recptr, 
                          fsck_msg_info_ptr );

retcode_t verify_repair_fs_rootdir( dinode_t *, fsck_msg_info_ptr, int *);


/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV


/*****************************************************************************
 * NAME: agg_clean_or_dirty
 *
 * FUNCTION:  Compare the superblock state field (s_state) with fsck's
 *            conclusions about the current state (clean | dirty) of
 *            the aggregate.  If write access, attempt to update the
 *            state field if the superblock is incorrect.  If read-only, 
 *            notify the caller if the superblock is incorrect.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t agg_clean_or_dirty ()
{
  retcode_t acod_rc = FSCK_OK;
  char user_reply;

  if( !agg_recptr->ag_dirty ) {  /* aggregate is actually clean now */

    fsck_send_msg( fsck_AGGCLN, 0, 0 );  /* announce this happy news */

    if( agg_recptr->processing_readonly )  { /*
                         * don't have write access
                         */
      if( (sb_ptr->s_state & FM_DIRTY) == FM_DIRTY )  {  /* 
                         * but isn't marked clean 
                         */
        fsck_send_msg( fsck_AGGCLNNOTDRTY, 0, 0 );
        }  /* end but isn't marked clean */
      }  /* don't have write access */

    else {  /* do have write access to the aggregate */
      fsck_send_msg( fsck_ALLFXD, 0, 0 );
      sb_ptr->s_state = FM_CLEAN;
      acod_rc = replicate_superblock();
      if( acod_rc == FSCK_OK ) {
        fsck_send_msg( fsck_AGGMRKDCLN, 0, 0 );
        }
      }  /* end else do have write access to the aggregate */
    }  /* end aggregate is actually clean now */

  else {  /* aggregate is actually dirty now */

    fsck_send_msg( fsck_AGGDRTY, 0, 0 );

    if( (sb_ptr->s_state & FM_DIRTY) != FM_DIRTY)  {  /* but isn't marked dirty */

      if( agg_recptr->processing_readonly )  { /*
                         * don't have write access
                         */
        msgprms[0] = Vol_Label;
        msgprmidx[0] = 0;
        fsck_send_msg( fsck_AGGDRTYNOTCLN, 0, 1 );
        }  /* don't have write access */

      else {  /* do have write access to the aggregate */
         /*
          * in keeping with the policy of protecting the system
          * from a potential panic due to a dirty file system,
          * if we have write access we'll mark the file system
          * dirty without asking permission.
          */
        sb_ptr->s_state = FM_DIRTY;
        acod_rc = replicate_superblock();
        if( acod_rc == FSCK_OK ) {
          fsck_send_msg( fsck_AGGMRKDDRTY, 0, 0 );
          }
        }  /* end else do have write access to the aggregate */
      }  /* end but isn't marked dirty */
    }  /* end aggregate is actually dirty now */

  return( acod_rc );
}                              /* end of agg_clean_or_dirty ()  */


/*****************************************************************************
 * NAME: backout_ait_part1
 *
 * FUNCTION:  Unrecord, in the fsck workspace block map, all storage allocated
 *            to inodes in part 1 (inodes 0 through 15) of the specified 
 *            (primary or secondary) aggregate inode table.
 *
 * PARAMETERS:
 *      which_ait  - input - the Aggregate Inode Table on which to perform
 *                           the function.  { fsck_primary | fsck_secondary }
 *
 * NOTES:  o The caller to this routine must ensure that the
 *           calls made by backout_ait_part1 to inode_get()
 *           will not require device I/O.
 *           That is, the caller must ensure that the aggregate
 *           inode extent containing part1 of the target AIT
 *           resides in the fsck inode buffer before calling
 *           this routine.  (See inode_get() for more info.)
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t backout_ait_part1 ( int which_ait )
{
  retcode_t baitp1_rc = FSCK_OK;
  struct fsck_ino_msg_info  ino_msg_info;
  fsck_msg_info_ptr msg_info_ptr;

  msg_info_ptr = &ino_msg_info;
  msg_info_ptr->msg_inopfx = fsck_aggr_inode;
  msg_info_ptr->msg_inotyp = fsck_metadata;

  agg_recptr->inode_stamp = -1;

  msg_info_ptr->msg_inonum = AGGREGATE_I;
  baitp1_rc = backout_valid_agg_inode( which_ait, AGGREGATE_I, msg_info_ptr );

  msg_info_ptr->msg_inonum = BMAP_I;
  if( baitp1_rc == FSCK_OK ) {
    baitp1_rc = backout_valid_agg_inode( which_ait, BMAP_I, msg_info_ptr );

    msg_info_ptr->msg_inonum = LOG_I;
    if( baitp1_rc == FSCK_OK ) {
      baitp1_rc = backout_valid_agg_inode( which_ait, LOG_I, msg_info_ptr );

      msg_info_ptr->msg_inonum = BADBLOCK_I;
      if( baitp1_rc == FSCK_OK ) {
        baitp1_rc = backout_valid_agg_inode( which_ait, BADBLOCK_I, msg_info_ptr );
        }
      }
    }

  return( baitp1_rc );
}                          /* end of backout_ait_part1 ()  */


/*****************************************************************************
 * NAME: backout_valid_agg_inode
 *
 * FUNCTION:  Unrecord, in the fsck workspace block map, storage allocated to
 *            the specified aggregate inode, assuming that all data structures
 *            associated with the inode are consistent.  (E.g., the B+ Tree 
 *            is at least internally consistent.)
 *
 * PARAMETERS:
 *      which_ait     - input - the Aggregate Inode Table on which to perform
 *                              the function.  { fsck_primary | fsck_secondary }
 *      inoidx        - input - ordinal number of the inode (i.e., inode number
 *                              as an int32)
 *      msg_info_ptr  - input - pointer to a data area with data needed to
 *                              issue messages about the inode 
 *
 * NOTES:  o The caller to this routine must ensure that the
 *           calls made by backout_ait_part1 to inode_get()
 *           will not require device I/O.
 *           That is, the caller must ensure that the aggregate
 *           inode extent containing part1 of the target AIT
 *           resides in the fsck inode buffer before calling
 *           this routine.  (See inode_get() for more info.)
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t backout_valid_agg_inode ( int               which_ait,
                                    inoidx_t          inoidx,
                                    fsck_msg_info_ptr msg_info_ptr
                                  )
{
  retcode_t bvai_rc = FSCK_OK;
  int agg_inode = -1;
  int alloc_ifnull = 0;
  dinode_t *inoptr;
  fsck_inode_recptr inorecptr;

  bvai_rc = inode_get( agg_inode, which_ait, inoidx, &inoptr );

  if( bvai_rc != FSCK_OK )  { /* didn't get the inode  */
    bvai_rc = FSCK_FAILED_REREAD_AGGINO;  /* this is fatal */
    }  /* end didn't get the inode */
  else {  /* got the inode */
    bvai_rc = get_inorecptr( agg_inode, alloc_ifnull, inoidx, &inorecptr );
    if( (bvai_rc == FSCK_OK) && (inorecptr == NULL) ) {
      bvai_rc = FSCK_INTERNAL_ERROR_22;
      sprintf( message_parm_0, "%ld", bvai_rc );                                 /* @F1 */
      msgprms[0] = message_parm_0;                                                /* @F1 */
      msgprmidx[0] = 0;                                                                   /* @F1 */
      sprintf( message_parm_1, "%ld", 0 );                                         /* @F1 */
      msgprms[1] = message_parm_1;                                                /* @F1 */
      msgprmidx[1] = 0;                                                                   /* @F1 */
      sprintf( message_parm_2, "%ld", 0 );                                         /* @F1 */
      msgprms[2] = message_parm_2;                                                /* @F1 */
      msgprmidx[2] = 0;                                                                   /* @F1 */
      sprintf( message_parm_3, "%ld", 0 );                                         /* @F1 */
      msgprms[3] = message_parm_3;                                                /* @F1 */
      msgprmidx[3] = 0;                                                                  /* @F1 */
      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                             /* @F1 */
      }
    else if( bvai_rc == FSCK_OK ) {
      bvai_rc = unrecord_valid_inode( inoptr, inoidx, inorecptr, msg_info_ptr );
      }
    }  /* end else got the inode  */

  return( bvai_rc );
}                          /* end of backout_valid_agg_inode ()  */


/*****************************************************************************
 * NAME: fatal_dup_check
 *
 * FUNCTION:  Determine whether any blocks are allocated to more than one
 *            aggregate metadata object.  (If so, the aggregate is too
 *            far gone for fsck to correct it, or even to analyze it with
 *            any confidence.)
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *       This routine is called after all the following has been
 *       completed:
 *          - all metadata (aggregate and fileset) has been validated
 *          - all inode extents have been recorded
 *          - all fixed metadata has been recorded
 *          - the block map and inode map have been recorded
 *
 *       A similar check is done in validate_select_agg_inode_table
 *       when an apparently valid part of the table has been identified.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t fatal_dup_check ()
{
  retcode_t fdc_rc = FSCK_OK;
  dupall_blkrec_ptr dupblk_ptr;
  fsblkidx_t first_in_range, this_blknum, last_blknum;
  reg_idx_t range_size;

  if( agg_recptr->dup_alloc_lst != NULL )  {  /* duplicate allocations
                                   * detected during metadata
                                   * validation
                                   */

    dupblk_ptr = agg_recptr->dup_alloc_lst;

    first_in_range = dupblk_ptr->blk_number;
    last_blknum = first_in_range;
    dupblk_ptr = dupblk_ptr->next;
    range_size = 1;

    while( dupblk_ptr != NULL )  {  /*
                                   * for all multiply allocated blocks
                                   */
      this_blknum = dupblk_ptr->blk_number;
      if( last_blknum == (this_blknum - 1) ) {
        range_size++;
        last_blknum = this_blknum;
        }
      else if( range_size > 0 ) {
        sprintf( message_parm_0, "%ld", range_size );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%lld", first_in_range );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        fsck_send_msg( fsck_DUPBLKMDREF, 0, 2 );

        first_in_range = this_blknum;
        last_blknum = this_blknum;
        range_size = 1;
        }
      dupblk_ptr = dupblk_ptr->next;
      }  /* end for all multiply allocated blocks */
    if( range_size > 0 ) {
      sprintf( message_parm_0, "%ld", range_size );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%lld", first_in_range );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      fsck_send_msg( fsck_DUPBLKMDREF, 0, 2 );
      }

    fsck_send_msg( fsck_DUPBLKMDREFS, 0, 0 );

    fdc_rc = FSCK_DUPMDBLKREF;
    agg_recptr->ag_dirty = 1;

    }  /* end duplicate allocations detected during metadata validation */

  return( fdc_rc );
}                              /* end of fatal_dup_check ()  */


/*****************************************************************************
 * NAME: first_ref_check_agg_metadata
 *
 * FUNCTION:  Determine whether the storage allocated for aggregate metadata
 *            includes a reference to any multiply-allocated aggregate blocks
 *            for which the first reference is still unresolved.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t first_ref_check_agg_metadata ()
{
  retcode_t frcam_rc = FSCK_OK;
  inoidx_t ino_idx;
  dinode_t *ino_ptr;
  int aggregate_inode = -1;      /* going for aggregate inodes only */
  int alloc_ifnull = 0;
  int which_ait;
  fsck_inode_recptr inorec_ptr;
  struct fsck_ino_msg_info  ino_msg_info;
  fsck_msg_info_ptr msg_info_ptr;

  msg_info_ptr = &ino_msg_info;
  msg_info_ptr->msg_inopfx = fsck_aggr_inode;
  msg_info_ptr->msg_inotyp = fsck_metadata;

    /*
     * check ait part 1 inodes for first references
     */
  (agg_recptr->primary_ait_4part1) ? ( which_ait = fsck_primary )
                                   : ( which_ait = fsck_secondary );

  ino_idx = AGGREGATE_I;               /* try for the self inode */
  frcam_rc = inode_get( aggregate_inode, which_ait, ino_idx, &ino_ptr );

  if( frcam_rc == FSCK_OK )  { /* got the self inode  */
    msg_info_ptr->msg_inonum = ino_idx;
    frcam_rc = get_inorecptr( aggregate_inode, alloc_ifnull, ino_idx, &inorec_ptr );
    if( (frcam_rc == FSCK_OK) && (inorec_ptr == NULL) ) {
      frcam_rc = FSCK_INTERNAL_ERROR_25;
      sprintf( message_parm_0, "%ld", frcam_rc );                               /* @F1 */
      msgprms[0] = message_parm_0;                                                /* @F1 */
      msgprmidx[0] = 0;                                                                   /* @F1 */
      sprintf( message_parm_1, "%ld", 0 );                                         /* @F1 */
      msgprms[1] = message_parm_1;                                                /* @F1 */
      msgprmidx[1] = 0;                                                                   /* @F1 */
      sprintf( message_parm_2, "%ld", 0 );                                         /* @F1 */
      msgprms[2] = message_parm_2;                                                /* @F1 */
      msgprmidx[2] = 0;                                                                   /* @F1 */
      sprintf( message_parm_3, "%ld", 0 );                                         /* @F1 */
      msgprms[3] = message_parm_3;                                                /* @F1 */
      msgprmidx[3] = 0;                                                                  /* @F1 */
      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                             /* @F1 */
      }
    else if( frcam_rc == FSCK_OK ) {
      frcam_rc = first_ref_check_inode( ino_ptr, ino_idx,
                                        inorec_ptr, msg_info_ptr );
      }
    }  /* end got the self inode  */
  else {  /* couldn't read the inode!
           * (We read it successfully a little while ago)
           */
    frcam_rc = FSCK_FAILED_SELF_READ3;
    }  /* end else couldn't read the inode! */

  if( ( frcam_rc == FSCK_OK) &&
      (agg_recptr->unresolved_1stref_count > 0) ) {  /* no errors and
                            * still have 1st refs to resolve
                            */
    ino_idx = BMAP_I;               /* try for the blockmap inode */
    frcam_rc = inode_get( aggregate_inode, which_ait, ino_idx, &ino_ptr );

    if( frcam_rc == FSCK_OK )  { /* got the block map inode */
      msg_info_ptr->msg_inonum = ino_idx;
      frcam_rc = get_inorecptr( aggregate_inode, alloc_ifnull, ino_idx, &inorec_ptr );
      if( (frcam_rc == FSCK_OK) && (inorec_ptr == NULL) ) {
        frcam_rc = FSCK_INTERNAL_ERROR_26;
        sprintf( message_parm_0, "%ld", frcam_rc );                             /* @F1 */
        msgprms[0] = message_parm_0;                                              /* @F1 */
        msgprmidx[0] = 0;                                                                 /* @F1 */
        sprintf( message_parm_1, "%ld", 0 );                                       /* @F1 */
        msgprms[1] = message_parm_1;                                              /* @F1 */
        msgprmidx[1] = 0;                                                                 /* @F1 */
        sprintf( message_parm_2, "%ld", 0 );                                       /* @F1 */
        msgprms[2] = message_parm_2;                                              /* @F1 */
        msgprmidx[2] = 0;                                                                 /* @F1 */
        sprintf( message_parm_3, "%ld", 0 );                                       /* @F1 */
        msgprms[3] = message_parm_3;                                              /* @F1 */
        msgprmidx[3] = 0;                                                                /* @F1 */
        fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                           /* @F1 */
        }
      else if( frcam_rc == FSCK_OK ) {
        frcam_rc = first_ref_check_inode( ino_ptr, ino_idx,
                                        inorec_ptr, msg_info_ptr );
        }
      }  /* end got the block map inode  */
    else {  /* couldn't read the inode!
             * (We read it successfully a little while ago)
             */
      frcam_rc = FSCK_FAILED_BMAP_READ2;
      }  /* end else couldn't read the inode! */
    }  /* end no errors and still have 1st refs to resolve */

  if( ( frcam_rc == FSCK_OK) &&
      (agg_recptr->unresolved_1stref_count > 0) ) {  /* no errors and
                            * still have 1st refs to resolve
                            */
    ino_idx = LOG_I;               /* try for the journal inode */
    frcam_rc = inode_get( aggregate_inode, which_ait, ino_idx, &ino_ptr );

    if( frcam_rc == FSCK_OK )  { /* got the journal inode */
      msg_info_ptr->msg_inonum = ino_idx;
      frcam_rc = get_inorecptr( aggregate_inode, alloc_ifnull, ino_idx, &inorec_ptr );
      if( (frcam_rc == FSCK_OK) && (inorec_ptr == NULL) ) {
        frcam_rc = FSCK_INTERNAL_ERROR_23;
        sprintf( message_parm_0, "%ld", frcam_rc );                             /* @F1 */
        msgprms[0] = message_parm_0;                                              /* @F1 */
        msgprmidx[0] = 0;                                                                 /* @F1 */
        sprintf( message_parm_1, "%ld", 0 );                                       /* @F1 */
        msgprms[1] = message_parm_1;                                              /* @F1 */
        msgprmidx[1] = 0;                                                                 /* @F1 */
        sprintf( message_parm_2, "%ld", 0 );                                       /* @F1 */
        msgprms[2] = message_parm_2;                                              /* @F1 */
        msgprmidx[2] = 0;                                                                 /* @F1 */
        sprintf( message_parm_3, "%ld", 0 );                                       /* @F1 */
        msgprms[3] = message_parm_3;                                              /* @F1 */
        msgprmidx[3] = 0;                                                                /* @F1 */
        fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                           /* @F1 */
        }
      else if( frcam_rc == FSCK_OK ) {
        frcam_rc = first_ref_check_inode( ino_ptr, ino_idx,
                                          inorec_ptr, msg_info_ptr );
        }
      }  /* end got the journal inode  */
    else {  /* couldn't read the inode!
             * (We read it successfully a little while ago)
             */
      frcam_rc = FSCK_FAILED_LOG_READ2;
      }  /* end else couldn't read the inode! */
    }  /* end no errors and still have 1st refs to resolve */

  if( ( frcam_rc == FSCK_OK) &&
      (agg_recptr->unresolved_1stref_count > 0) ) {  /* no errors and
                            * still have 1st refs to resolve
                            */
    ino_idx = BADBLOCK_I;          /* try for the bad block inode */
    frcam_rc = inode_get( aggregate_inode, which_ait, ino_idx, &ino_ptr );

    if( frcam_rc == FSCK_OK )  { /* got the bad block inode */
      msg_info_ptr->msg_inonum = ino_idx;
      frcam_rc = get_inorecptr( aggregate_inode, alloc_ifnull, ino_idx, &inorec_ptr );
      if( (frcam_rc == FSCK_OK) && (inorec_ptr == NULL) ) {
        frcam_rc = FSCK_INTERNAL_ERROR_58;
        sprintf( message_parm_0, "%ld", frcam_rc );                             /* @F1 */
        msgprms[0] = message_parm_0;                                              /* @F1 */
        msgprmidx[0] = 0;                                                                 /* @F1 */
        sprintf( message_parm_1, "%ld", 0 );                                       /* @F1 */
        msgprms[1] = message_parm_1;                                              /* @F1 */
        msgprmidx[1] = 0;                                                                 /* @F1 */
        sprintf( message_parm_2, "%ld", 0 );                                       /* @F1 */
        msgprms[2] = message_parm_2;                                              /* @F1 */
        msgprmidx[2] = 0;                                                                 /* @F1 */
        sprintf( message_parm_3, "%ld", 0 );                                       /* @F1 */
        msgprms[3] = message_parm_3;                                              /* @F1 */
        msgprmidx[3] = 0;                                                                /* @F1 */
        fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                           /* @F1 */
        }
      else if( frcam_rc == FSCK_OK ) {
        frcam_rc = first_ref_check_inode( ino_ptr, ino_idx,
                                          inorec_ptr, msg_info_ptr );
        }
      }  /* end got the bad block inode  */
    else {  /* couldn't read the inode!
             * (We read it successfully a little while ago)
             */
      frcam_rc = FSCK_FAILED_BADBLK_READ2;
      }  /* end else couldn't read the inode! */
    }  /* end no errors and still have 1st refs to resolve */


    /*
     * check ait part 2 inodes for first references
     */

  (agg_recptr->primary_ait_4part2) ? ( which_ait = fsck_primary )
                                   : ( which_ait = fsck_secondary );

  if( ( frcam_rc == FSCK_OK) &&
      (agg_recptr->unresolved_1stref_count > 0) ) {  /* no errors and
                            * still have 1st refs to resolve
                            */
    ino_idx = FILESYSTEM_I;              /* read the aggregate inode */
    frcam_rc = inode_get( aggregate_inode, which_ait, ino_idx, &ino_ptr );

    if( frcam_rc == FSCK_OK )  { /* got the fileset inode */
      msg_info_ptr->msg_inonum = ino_idx;
      frcam_rc = get_inorecptr( aggregate_inode, alloc_ifnull, ino_idx, &inorec_ptr );
      if( (frcam_rc == FSCK_OK) && (inorec_ptr == NULL) ) {
        frcam_rc = FSCK_INTERNAL_ERROR_28;
        sprintf( message_parm_0, "%ld", frcam_rc );                            /* @F1 */
        msgprms[0] = message_parm_0;                                              /* @F1 */
        msgprmidx[0] = 0;                                                                 /* @F1 */
        sprintf( message_parm_1, "%ld", 0 );                                       /* @F1 */
        msgprms[1] = message_parm_1;                                              /* @F1 */
        msgprmidx[1] = 0;                                                                 /* @F1 */
        sprintf( message_parm_2, "%ld", 0 );                                       /* @F1 */
        msgprms[2] = message_parm_2;                                              /* @F1 */
        msgprmidx[2] = 0;                                                                 /* @F1 */
        sprintf( message_parm_3, "%ld", 0 );                                       /* @F1 */
        msgprms[3] = message_parm_3;                                              /* @F1 */
        msgprmidx[3] = 0;                                                                /* @F1 */
        fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                           /* @F1 */
        }
      else if( frcam_rc == FSCK_OK ) {
        frcam_rc = first_ref_check_inode( ino_ptr, ino_idx,
                                          inorec_ptr, msg_info_ptr );
        }
      }  /* end got the fileset inode  */
    else {  /* couldn't read the inode!
             * (We read it successfully a little while ago)
             */
      frcam_rc = FSCK_FAILED_AGFS_READ3;
      }  /* end else couldn't read the inode! */
    }  /* end no errors and still have 1st refs to resolve */

  if( ( frcam_rc == FSCK_OK) &&
      (agg_recptr->unresolved_1stref_count > 0) ) {  /* no errors and
                            * still have 1st refs to resolve
                            */
    frcam_rc = first_ref_check_other_ait( );
    }

  return( frcam_rc );
}                           /* end of first_ref_check_agg_metadata ()  */


/**************************************************************************
 * NAME: first_ref_check_fixed_metadata
 *
 * FUNCTION: Certain aggregate metadata is not described by any inode.
 *           This routine determines whether any of the blocks occupied
 *           by this aggregate are multiply-allocated and still have 
 *           unresolved first reference.
 *
 * PARAMETERS:  none
 *
 * NOTES: 
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t first_ref_check_fixed_metadata ( )
{
 retcode_t frcfm_rc = FSCK_OK;
 fsblkidx_t blkidx, start_block, end_block;
 agg_byteidx_t byte_offset;
 reg_idx_t length_in_agg_blocks;
 fsblkidx_t wsp_blocks_described;
 int ref_resolved;

 /*
  * the reserved space (at the beginning of the aggregate)
  */
 start_block = 0;
 length_in_agg_blocks = AGGR_RSVD_BYTES / sb_ptr->s_bsize;
 end_block = start_block + length_in_agg_blocks - 1;
 for( blkidx = start_block;
      ((blkidx <= end_block) && 
       (frcfm_rc == FSCK_OK) &&
       (agg_recptr->unresolved_1stref_count > 0) ); 
      blkidx++ ) {
   frcfm_rc = blkall_ref_check( blkidx, &ref_resolved );
   }  /* end for */

 /*
  * the superblocks
  */
 if( frcfm_rc == FSCK_OK ) {  /* go ahead with superblocks */
   length_in_agg_blocks = SIZE_OF_SUPER / sb_ptr->s_bsize;
      /*
       * primary
       */
   start_block = SUPER1_OFF / sb_ptr->s_bsize;
   end_block = start_block + length_in_agg_blocks - 1;
   for( blkidx = start_block;
        ((blkidx <= end_block) && 
         (frcfm_rc == FSCK_OK) &&
         (agg_recptr->unresolved_1stref_count > 0) ); 
        blkidx++ ) {
     frcfm_rc = blkall_ref_check( blkidx, &ref_resolved );
     }  /* end for */
      /*
       * secondary
       */
   if( frcfm_rc == FSCK_OK ) {  /* ok to continue */
     start_block = SUPER2_OFF / sb_ptr->s_bsize;
     end_block = start_block + length_in_agg_blocks - 1;
     for( blkidx = start_block;
          ((blkidx <= end_block) && 
           (frcfm_rc == FSCK_OK) &&
           (agg_recptr->unresolved_1stref_count > 0) ); 
          blkidx++ ) {
       frcfm_rc = blkall_ref_check( blkidx, &ref_resolved );
       }  /* end for */
     }  /* end ok to continue */
   }  /* end go ahead with superblocks */
 
 /*
  * note that the fsck workspace and journal log (at the end of the
  * aggregate) are not described by the block map (neither the
  * Aggregate Block Allocation Map nor the fsck Workspace Block Map)
  */

 /*
  * the "phantom blocks" described by the last dmap page
  */
   if( frcfm_rc == FSCK_OK ) {  /* ok to continue */

     wsp_blocks_described = BITSPERBYTE *
                      (agg_recptr->ondev_wsp_byte_length - BYTESPERPAGE); /*
                               * the first page is a control page and
                               * scratch area.
                               */
     if( wsp_blocks_described > agg_recptr->sb_agg_fsblk_length ) {  /*
                               * the dmaps do describe more blocks than
                               * actually exist
                               */
       start_block = agg_recptr->sb_agg_fsblk_length;  /* since this is
                               * the number of blocks and since blocks are
                               * numbered starting at 0, this is the block
                               * number of the first phantom block;
                               */
       end_block = wsp_blocks_described - 1;
       for( blkidx = start_block;
            ((blkidx <= end_block) && 
             (frcfm_rc == FSCK_OK) &&
             (agg_recptr->unresolved_1stref_count > 0) ); 
            blkidx++ ) {
         frcfm_rc = blkall_ref_check( blkidx, &ref_resolved );
         }  /* end for */
       }  /* end the dmaps do describe more blocks than actually exist */
     }  /* end ok to continue */

 return(frcfm_rc );

}  /* end first_ref_check_fixed_metadata( ) */


/*****************************************************************************
 * NAME: first_ref_check_fs_metadata
 *
 * FUNCTION: Determine whether any blocks occupied by fileset metadata are
 *           are multiply-allocated and still have unresolved first
 *           reference.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t first_ref_check_fs_metadata ()
{
  retcode_t frcfsm_rc = FSCK_OK;
  inoidx_t ino_idx;
  dinode_t *ino_ptr;
  int aggregate_inode = 0;      /* going for fileset inodes only */
  int alloc_ifnull = 0;
  int which_fsit;               /* which fileset? */
  fsck_inode_recptr inorec_ptr;
  struct fsck_ino_msg_info  ino_msg_info;
  fsck_msg_info_ptr msg_info_ptr;

  msg_info_ptr = &ino_msg_info;
  msg_info_ptr->msg_inopfx = fsck_fset_inode;  /* all fileset owned */
  msg_info_ptr->msg_inotyp = fsck_metadata;

  /*
   * in release 1 there is exactly 1 fileset
   */
  which_fsit = 0;

  ino_idx = FILESET_EXT_I; /* try for the fileset superinode extension */
  frcfsm_rc = inode_get( aggregate_inode, which_fsit, ino_idx, &ino_ptr );

  if( frcfsm_rc == FSCK_OK )  { /* got the inode  */
    msg_info_ptr->msg_inonum = ino_idx;
    frcfsm_rc = get_inorecptr( aggregate_inode, alloc_ifnull, ino_idx, &inorec_ptr );
    if( (frcfsm_rc == FSCK_OK) && (inorec_ptr == NULL) ) {
      frcfsm_rc = FSCK_INTERNAL_ERROR_24;
      sprintf( message_parm_0, "%ld", frcfsm_rc );                            /* @F1 */
      msgprms[0] = message_parm_0;                                              /* @F1 */
      msgprmidx[0] = 0;                                                                 /* @F1 */
      sprintf( message_parm_1, "%ld", 0 );                                       /* @F1 */
      msgprms[1] = message_parm_1;                                              /* @F1 */
      msgprmidx[1] = 0;                                                                 /* @F1 */
      sprintf( message_parm_2, "%ld", 0 );                                       /* @F1 */
      msgprms[2] = message_parm_2;                                              /* @F1 */
      msgprmidx[2] = 0;                                                                 /* @F1 */
      sprintf( message_parm_3, "%ld", 0 );                                       /* @F1 */
      msgprms[3] = message_parm_3;                                              /* @F1 */
      msgprmidx[3] = 0;                                                                /* @F1 */
      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                           /* @F1 */
      }
    else if( frcfsm_rc == FSCK_OK ) {
      frcfsm_rc = first_ref_check_inode( ino_ptr, ino_idx,
                                         inorec_ptr, msg_info_ptr );
      }
    }  /* end got the inode  */
  else {  /* couldn't read the inode!
           * (We read it successfully a little while ago)
           */
    frcfsm_rc = FSCK_FAILED_FSSIEXT_READ2;
    }  /* end else couldn't read the inode! */

  if( (frcfsm_rc == FSCK_OK) &&
      (agg_recptr->unresolved_1stref_count > 0) ) {  /* no errors and
                            * still have 1st refs to resolve
                            */
    ino_idx = ROOT_I;               /* try for the root directory inode */
    frcfsm_rc = inode_get( aggregate_inode, which_fsit, ino_idx, &ino_ptr );

    if( frcfsm_rc == FSCK_OK )  { /* got the root dir inode */
      msg_info_ptr->msg_inonum = ino_idx;
      frcfsm_rc = get_inorecptr( aggregate_inode, alloc_ifnull, ino_idx, &inorec_ptr );
      if( (frcfsm_rc == FSCK_OK) && (inorec_ptr == NULL) ) {
        frcfsm_rc = FSCK_INTERNAL_ERROR_29;
        sprintf( message_parm_0, "%ld", frcfsm_rc );                            /* @F1 */
        msgprms[0] = message_parm_0;                                              /* @F1 */
        msgprmidx[0] = 0;                                                                 /* @F1 */
        sprintf( message_parm_1, "%ld", 0 );                                       /* @F1 */
        msgprms[1] = message_parm_1;                                              /* @F1 */
        msgprmidx[1] = 0;                                                                 /* @F1 */
        sprintf( message_parm_2, "%ld", 0 );                                       /* @F1 */
        msgprms[2] = message_parm_2;                                              /* @F1 */
        msgprmidx[2] = 0;                                                                 /* @F1 */
        sprintf( message_parm_3, "%ld", 0 );                                       /* @F1 */
        msgprms[3] = message_parm_3;                                              /* @F1 */
        msgprmidx[3] = 0;                                                                /* @F1 */
        fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                           /* @F1 */
        }
      else if( frcfsm_rc == FSCK_OK ) {
        frcfsm_rc = first_ref_check_inode( ino_ptr, ino_idx,
                                           inorec_ptr, msg_info_ptr );
        }
      if( (frcfsm_rc == FSCK_OK) &&
          (inorec_ptr->selected_to_rls) )  {  /* routine doesn't
                               * understand that the root directory is
                               * the special case of a directory which
                               * is also special metadata and might
                               * flag it for release.
                               */
        inorec_ptr->selected_to_rls = 0;
        }  /* end routine doesn't understand that the root directory... */
      }  /* end got the root dir inode  */
    else {  /* couldn't read the inode!
             * (We read it successfully a little while ago)
             */
      frcfsm_rc = FSCK_FAILED_FSRTDIR_READ2;
      }  /* end else couldn't read the inode! */
    }  /* end no errors and still have 1st refs to resolve */

  return( frcfsm_rc );
}                           /* end of first_ref_check_fs_metadata ()  */


/**************************************************************************
 * NAME: first_ref_check_other_ait
 *
 * FUNCTION: Determine whether any blocks occupied by the "other ait" 
 *           are multiply-allocated and still have unresolved first 
 *           reference.  To be more specific, if the primary ait is
 *           being used for fsck processing, the secondary ait is the 
 *           "other ait"...and vice versa.
 *
 * PARAMETERS:  none
 *
 * NOTES: 
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t first_ref_check_other_ait ( )
{
  retcode_t frcoa_rc = FSCK_OK;
  fsblkidx_t blkidx, start_block, end_block;
  reg_idx_t length_in_agg_blocks;
  int ref_resolved = 0;

      /*
       * first extent of the agg inode map
       */
  if( agg_recptr->primary_ait_4part1 )  {  /* primary already recorded */
    start_block = addressPXD(&(sb_ptr->s_aim2));
    length_in_agg_blocks = lengthPXD(&(sb_ptr->s_aim2));
    }  /* end primary already recorded */
  else {  /* secondary already recorded */
    start_block = AIMAP_OFF / sb_ptr->s_bsize;
    length_in_agg_blocks = (AITBL_OFF - AIMAP_OFF) / sb_ptr->s_bsize;
    }  /* end else secondary already recorded */

  end_block = start_block + length_in_agg_blocks - 1;
  for( blkidx = start_block;
       ((blkidx <= end_block) && 
        (frcoa_rc == FSCK_OK) &&
        (agg_recptr->unresolved_1stref_count > 0) );
       blkidx++ ) {
    frcoa_rc = blkall_ref_check( blkidx, &ref_resolved );
    }  /* end for */

      /*
       * first extent of the agg inode table
       */
  if( agg_recptr->primary_ait_4part1 )  {  /* primary already recorded */
    start_block = addressPXD(&(sb_ptr->s_ait2));
    length_in_agg_blocks = lengthPXD(&(sb_ptr->s_ait2));
    }  /* end primary already recorded */
  else {  /* secondary already recorded */
    start_block = AITBL_OFF / sb_ptr->s_bsize;
    length_in_agg_blocks = INODE_EXTENT_SIZE / sb_ptr->s_bsize;
    }  /* end else secondary already recorded */

  end_block = start_block + length_in_agg_blocks - 1;
  for( blkidx = start_block;
       ((blkidx <= end_block) && 
        (frcoa_rc == FSCK_OK) &&
        (agg_recptr->unresolved_1stref_count > 0) );
       blkidx++ ) {
    frcoa_rc = blkall_ref_check( blkidx, &ref_resolved );
    }  /* end for */

  return( frcoa_rc );

}  /* end first_ref_check_other_ait( ) */


/*****************************************************************************
 * NAME: record_ait_part1_again
 *
 * FUNCTION:  Record, in the fsck workspace block map, all storage allocated
 *            to inodes in part 1 (inodes 0 through 15) of the specified 
 *            (primary or secondary) aggregate inode table.  Do this with
 *            the knowledge that all these inodes have been verified 
 *            completely correct.
 *
 * PARAMETERS:
 *      which_ait  - input - the Aggregate Inode Table on which to perform
 *                           the function.  { fsck_primary | fsck_secondary }
 *
 * NOTES:  o The caller to this routine must ensure that the
 *           calls made by record_ait_part1_again to inode_get()
 *           will not require device I/O.
 *           That is, the caller must ensure that the aggregate
 *           inode extent containing part1 of the target AIT
 *           resides in the fsck inode buffer before calling
 *           this routine.  (See inode_get() for more info)
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t record_ait_part1_again ( int which_ait )
{
  retcode_t raitp1a_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  fsck_inode_recptr inorecptr;
  inoidx_t ino_idx;
  dinode_t *ino_ptr;
  int aggregate_inode = -1;      /* going for aggregate inodes only */
  int alloc_ifnull = 0;

  struct fsck_ino_msg_info msg_info;
  fsck_msg_info_ptr msg_info_ptr;

  msg_info_ptr = &msg_info;
  msg_info_ptr->msg_inopfx = fsck_aggr_inode;
  msg_info_ptr->msg_inotyp = fsck_metadata;

  ino_idx = AGGREGATE_I;               /* try for the self inode */
  msg_info_ptr->msg_inonum = ino_idx;
  raitp1a_rc = inode_get( aggregate_inode, which_ait, ino_idx, &ino_ptr );

  if( raitp1a_rc == FSCK_OK )  { /* got the self inode  */
    agg_recptr->inode_stamp = ino_ptr->di_inostamp;  

    raitp1a_rc = get_inorecptr( aggregate_inode, alloc_ifnull, ino_idx, &inorecptr );
    if( (raitp1a_rc == FSCK_OK) && (inorecptr == NULL) ) {
      raitp1a_rc = FSCK_INTERNAL_ERROR_39;
      sprintf( message_parm_0, "%ld", raitp1a_rc );                           /* @F1 */
      msgprms[0] = message_parm_0;                                              /* @F1 */
      msgprmidx[0] = 0;                                                                 /* @F1 */
      sprintf( message_parm_1, "%ld", 0 );                                       /* @F1 */
      msgprms[1] = message_parm_1;                                              /* @F1 */
      msgprmidx[1] = 0;                                                                 /* @F1 */
      sprintf( message_parm_2, "%ld", 0 );                                       /* @F1 */
      msgprms[2] = message_parm_2;                                              /* @F1 */
      msgprmidx[2] = 0;                                                                 /* @F1 */
      sprintf( message_parm_3, "%ld", 0 );                                       /* @F1 */
      msgprms[3] = message_parm_3;                                              /* @F1 */
      msgprmidx[3] = 0;                                                                /* @F1 */
      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                           /* @F1 */
      }
    else if( raitp1a_rc == FSCK_OK ) {
      inorecptr->in_use = 0;
      inorecptr->selected_to_rls = 0;
      inorecptr->crrct_link_count = 0;
      inorecptr->crrct_prnt_inonum = 0;
      inorecptr->adj_entries = 0;
      inorecptr->cant_chkea = 0;
      inorecptr->clr_ea_fld = 0;
      inorecptr->clr_acl_fld = 0;
      inorecptr->crrct_cumm_blks = 0;
      inorecptr->inlineea_on = 0;
      inorecptr->inlineea_off = 0;
      inorecptr->inline_data_err = 0;
      inorecptr->ignore_alloc_blks = 0;
      inorecptr->reconnect = 0;
      inorecptr->unxpctd_prnts = 0;
      inorecptr->badblk_inode = 0;
      inorecptr->involved_in_dups = 0;
      inorecptr->inode_type = metadata_inode;
      inorecptr->link_count = 0;
      inorecptr->parent_inonum = 0;
      inorecptr->cumm_blocks = 0;                                            /* @F1 */
      inorecptr->dtree_level = -1;                                             /* @F1 */
      inorecptr->ext_rec = NULL;
      raitp1a_rc = record_valid_inode( ino_ptr, ino_idx, inorecptr, msg_info_ptr );
      }

    if( raitp1a_rc == FSCK_OK ) {  /* recorded it successfully */
      inorecptr->in_use = 1;
      }  /* end recorded it successfully */
    }  /* end got the self inode  */

  if( raitp1a_rc == FSCK_OK )  {  /* self inode recorded ok */
    ino_idx = BMAP_I;               /* try for the blockmap inode */
    msg_info_ptr->msg_inonum = ino_idx;
    raitp1a_rc = inode_get( aggregate_inode, which_ait, ino_idx, &ino_ptr );

    if( raitp1a_rc == FSCK_OK )  { /* got the block map inode */
      raitp1a_rc = get_inorecptr( aggregate_inode, alloc_ifnull, ino_idx, &inorecptr );
      if( (raitp1a_rc == FSCK_OK) && (inorecptr == NULL) ) {
        raitp1a_rc = FSCK_INTERNAL_ERROR_43;
        sprintf( message_parm_0, "%ld", raitp1a_rc );                           /* @F1 */
        msgprms[0] = message_parm_0;                                              /* @F1 */
        msgprmidx[0] = 0;                                                                 /* @F1 */
        sprintf( message_parm_1, "%ld", 0 );                                       /* @F1 */
        msgprms[1] = message_parm_1;                                              /* @F1 */
        msgprmidx[1] = 0;                                                                 /* @F1 */
        sprintf( message_parm_2, "%ld", 0 );                                       /* @F1 */
        msgprms[2] = message_parm_2;                                              /* @F1 */
        msgprmidx[2] = 0;                                                                 /* @F1 */
        sprintf( message_parm_3, "%ld", 0 );                                       /* @F1 */
        msgprms[3] = message_parm_3;                                              /* @F1 */
        msgprmidx[3] = 0;                                                                /* @F1 */
        fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                           /* @F1 */
        }
      else if( raitp1a_rc == FSCK_OK ) {
        inorecptr->in_use = 0;
        inorecptr->selected_to_rls = 0;
        inorecptr->crrct_link_count = 0;
        inorecptr->crrct_prnt_inonum = 0;
        inorecptr->adj_entries = 0;
        inorecptr->cant_chkea = 0;
        inorecptr->clr_ea_fld = 0;
        inorecptr->clr_acl_fld = 0;
        inorecptr->crrct_cumm_blks = 0;
        inorecptr->inlineea_on = 0;
        inorecptr->inlineea_off = 0;
        inorecptr->inline_data_err = 0;
        inorecptr->ignore_alloc_blks = 0;
        inorecptr->reconnect = 0;
        inorecptr->unxpctd_prnts = 0;
        inorecptr->badblk_inode = 0;
        inorecptr->involved_in_dups = 0;
        inorecptr->inode_type = metadata_inode;
        inorecptr->link_count = 0;
        inorecptr->parent_inonum = 0;
        inorecptr->cumm_blocks = 0;                                               /* @F1 */
        inorecptr->dtree_level = -1;                                                /* @F1 */
        inorecptr->ext_rec = NULL;
        raitp1a_rc = record_valid_inode( ino_ptr, ino_idx, inorecptr, msg_info_ptr );
        }

      if( raitp1a_rc == FSCK_OK ) {  /* recorded it successfully */
        inorecptr->in_use = 1;
        }  /* end recorded it successfully */
      }  /* end got the block map inode  */
    }  /* end self inode recorded ok */

  if( raitp1a_rc == FSCK_OK )  {  /* self and bmap inodes recorded ok */
    ino_idx = LOG_I;               /* try for the journal log inode */
    msg_info_ptr->msg_inonum = ino_idx;
    raitp1a_rc = inode_get( aggregate_inode, which_ait, ino_idx, &ino_ptr );

    if( raitp1a_rc == FSCK_OK )  { /* got the journal log inode */
      raitp1a_rc = get_inorecptr( aggregate_inode, alloc_ifnull, ino_idx, &inorecptr );
      if( (raitp1a_rc == FSCK_OK) && (inorecptr == NULL) ) {
        raitp1a_rc = FSCK_INTERNAL_ERROR_44;
        sprintf( message_parm_0, "%ld", raitp1a_rc );                           /* @F1 */
        msgprms[0] = message_parm_0;                                              /* @F1 */
        msgprmidx[0] = 0;                                                                 /* @F1 */
        sprintf( message_parm_1, "%ld", 0 );                                       /* @F1 */
        msgprms[1] = message_parm_1;                                              /* @F1 */
        msgprmidx[1] = 0;                                                                 /* @F1 */
        sprintf( message_parm_2, "%ld", 0 );                                       /* @F1 */
        msgprms[2] = message_parm_2;                                              /* @F1 */
        msgprmidx[2] = 0;                                                                 /* @F1 */
        sprintf( message_parm_3, "%ld", 0 );                                       /* @F1 */
        msgprms[3] = message_parm_3;                                              /* @F1 */
        msgprmidx[3] = 0;                                                                /* @F1 */
        fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                           /* @F1 */
        }
      else if( raitp1a_rc == FSCK_OK ) {
        inorecptr->in_use = 0;
        inorecptr->selected_to_rls = 0;
        inorecptr->crrct_link_count = 0;
        inorecptr->crrct_prnt_inonum = 0;
        inorecptr->adj_entries = 0;
        inorecptr->cant_chkea = 0;
        inorecptr->clr_ea_fld = 0;
        inorecptr->clr_acl_fld = 0;
        inorecptr->crrct_cumm_blks = 0;
        inorecptr->inlineea_on = 0;
        inorecptr->inlineea_off = 0;
        inorecptr->inline_data_err = 0;
        inorecptr->ignore_alloc_blks = 0;
        inorecptr->reconnect = 0;
        inorecptr->unxpctd_prnts = 0;
        inorecptr->badblk_inode = 0;
        inorecptr->involved_in_dups = 0;
        inorecptr->inode_type = metadata_inode;
        inorecptr->link_count = 0;
        inorecptr->parent_inonum = 0;
        inorecptr->cumm_blocks = 0;                                               /* @F1 */
        inorecptr->dtree_level = -1;                                                /* @F1 */
        inorecptr->ext_rec = NULL;
        raitp1a_rc = record_valid_inode( ino_ptr, ino_idx, inorecptr, msg_info_ptr );
        }

      if( raitp1a_rc == FSCK_OK ) {  /* recorded it successfully */
        inorecptr->in_use = 1;
        }  /* end recorded it successfully */
      }  /* end got the journal log inode  */
    }  /* end self and bmap inodes recorded ok */

  if( raitp1a_rc == FSCK_OK )  {  /* self, bmap, and journal inodes recorded ok */
    ino_idx = BADBLOCK_I;          /* try for the bad block inode */
    msg_info_ptr->msg_inonum = ino_idx;
    raitp1a_rc = inode_get( aggregate_inode, which_ait, ino_idx, &ino_ptr );

    if( raitp1a_rc == FSCK_OK )  { /* got the bad block inode */
      raitp1a_rc = get_inorecptr( aggregate_inode, alloc_ifnull, ino_idx, &inorecptr );
      if( (raitp1a_rc == FSCK_OK) && (inorecptr == NULL) ) {
        raitp1a_rc = FSCK_INTERNAL_ERROR_59;
        sprintf( message_parm_0, "%ld", raitp1a_rc );                           /* @F1 */
        msgprms[0] = message_parm_0;                                              /* @F1 */
        msgprmidx[0] = 0;                                                                 /* @F1 */
        sprintf( message_parm_1, "%ld", 0 );                                       /* @F1 */
        msgprms[1] = message_parm_1;                                              /* @F1 */
        msgprmidx[1] = 0;                                                                 /* @F1 */
        sprintf( message_parm_2, "%ld", 0 );                                       /* @F1 */
        msgprms[2] = message_parm_2;                                              /* @F1 */
        msgprmidx[2] = 0;                                                                 /* @F1 */
        sprintf( message_parm_3, "%ld", 0 );                                       /* @F1 */
        msgprms[3] = message_parm_3;                                              /* @F1 */
        msgprmidx[3] = 0;                                                                /* @F1 */
        fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                           /* @F1 */
        }
      else if( raitp1a_rc == FSCK_OK ) {
        inorecptr->in_use = 0;
        inorecptr->selected_to_rls = 0;
        inorecptr->crrct_link_count = 0;
        inorecptr->crrct_prnt_inonum = 0;
        inorecptr->adj_entries = 0;
        inorecptr->cant_chkea = 0;
        inorecptr->clr_ea_fld = 0;
        inorecptr->clr_acl_fld = 0;
        inorecptr->crrct_cumm_blks = 0;
        inorecptr->inlineea_on = 0;
        inorecptr->inlineea_off = 0;
        inorecptr->inline_data_err = 0;
        inorecptr->ignore_alloc_blks = 0;
        inorecptr->reconnect = 0;
        inorecptr->unxpctd_prnts = 0;
        inorecptr->badblk_inode = 0;
        inorecptr->involved_in_dups = 0;
        inorecptr->inode_type = metadata_inode;
        inorecptr->link_count = 0;
        inorecptr->parent_inonum = 0;
        inorecptr->cumm_blocks = 0;
        inorecptr->dtree_level = -1;
        inorecptr->ext_rec = NULL;
        raitp1a_rc = record_valid_inode( ino_ptr, ino_idx, inorecptr, msg_info_ptr );
        }

      if( raitp1a_rc == FSCK_OK ) {  /* recorded it successfully */
        inorecptr->in_use = 1;
        }  /* end recorded it successfully */
      }  /* end got the bad block inode  */
    }  /* end self, bmap, and journal inodes recorded ok */

  return( raitp1a_rc );
}                          /* end of record_ait_part1_again ()  */


/**************************************************************************
 * NAME: record_fixed_metadata
 *
 * FUNCTION: Certain aggregate metadata is not described by any inode.
 *           This routine marks, in the fsck workspace block map, that
 *           the blocks occupied by this aggregate metadata are in use.
 *
 * PARAMETERS:  none
 *
 * NOTES: 
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t record_fixed_metadata ( )
{
 retcode_t rfm_rc = FSCK_OK;
 fsblkidx_t blkidx, start_block, end_block;
 agg_byteidx_t byte_offset;
 reg_idx_t length_in_agg_blocks;
 fsblkidx_t wsp_blocks_described;

 /*
  * the reserved space (at the beginning of the aggregate)
  */
 start_block = 0;
 length_in_agg_blocks = AGGR_RSVD_BYTES / sb_ptr->s_bsize;
 end_block = start_block + length_in_agg_blocks - 1;
 for( blkidx = start_block;
      ((blkidx <= end_block) && (rfm_rc == FSCK_OK));
      blkidx++ ) {
   rfm_rc = blkall_increment_owners( blkidx );
   }  /* end for */

 /*
  * the superblocks
  */
 if( rfm_rc == FSCK_OK ) {  /* go ahead with superblocks */
   length_in_agg_blocks = SIZE_OF_SUPER / sb_ptr->s_bsize;
      /*
       * primary
       */
   start_block = SUPER1_OFF / sb_ptr->s_bsize;
   end_block = start_block + length_in_agg_blocks - 1;
   for( blkidx = start_block;
        ((blkidx <= end_block) && (rfm_rc == FSCK_OK));
        blkidx++ ) {
     rfm_rc = blkall_increment_owners( blkidx );
     }  /* end for */
      /*
       * secondary
       */
   if( rfm_rc == FSCK_OK ) {  /* ok to continue */
     start_block = SUPER2_OFF / sb_ptr->s_bsize;
     end_block = start_block + length_in_agg_blocks - 1;
     for( blkidx = start_block;
          ((blkidx <= end_block) && (rfm_rc == FSCK_OK));
          blkidx++ ) {
       rfm_rc = blkall_increment_owners( blkidx );
       }  /* end for */
     }  /* end ok to continue */
   }  /* end go ahead with superblocks */
 

 /*
  * note that the fsck workspace and journal log (at the end of the
  * aggregate) are not described by the block map (neither the
  * Aggregate Block Allocation Map nor the fsck Workspace Block Map)
  */

 /*
  * the "phantom blocks" described by the last dmap page
  */
   if( rfm_rc == FSCK_OK ) {  /* ok to continue */

     wsp_blocks_described = BITSPERBYTE *
                      (agg_recptr->ondev_wsp_byte_length - BYTESPERPAGE); /*
                               * the first page is a control page and
                               * scratch area.
                               */
     if( wsp_blocks_described > agg_recptr->sb_agg_fsblk_length ) {  /*
                               * the dmaps do describe more blocks than
                               * actually exist
                               */
       start_block = agg_recptr->sb_agg_fsblk_length;  /* since this is
                               * the number of blocks and since blocks are
                               * numbered starting at 0, this is the block
                               * number of the first phantom block;
                               */
       end_block = wsp_blocks_described - 1;
       for( blkidx = start_block;
            ((blkidx <= end_block) && (rfm_rc == FSCK_OK));
            blkidx++ ) {
         rfm_rc = blkall_increment_owners( blkidx );
         }  /* end for */
       }  /* end the dmaps do describe more blocks than actually exist */
     }  /* end ok to continue */

 return( rfm_rc );

}  /* end record_fixed_metadata( ) */


/**************************************************************************
 * NAME: record_other_ait
 *
 * FUNCTION: Record the blocks occupied by the "other ait" in the fsck
 *           workspace block map.  To be more specific, if the primary
 *           ait is being used for fsck processing, the secondary ait
 *           is the "other ait"...and vice versa.
 *
 * PARAMETERS:  none
 *
 * NOTES: 
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t record_other_ait ( )
{
  retcode_t roa_rc = FSCK_OK;
  fsblkidx_t blkidx, start_block, end_block;
  reg_idx_t length_in_agg_blocks;

      /*
       * first extent of the agg inode map
       */
  if( agg_recptr->primary_ait_4part1 )  {  /* primary already recorded */
    start_block = addressPXD(&(sb_ptr->s_aim2));
    length_in_agg_blocks = lengthPXD(&(sb_ptr->s_aim2));
    }  /* end primary already recorded */
  else {  /* secondary already recorded */
    start_block = AIMAP_OFF / sb_ptr->s_bsize;
    length_in_agg_blocks = (AITBL_OFF - AIMAP_OFF) / sb_ptr->s_bsize;
    }  /* end else secondary already recorded */

  end_block = start_block + length_in_agg_blocks - 1;
  for( blkidx = start_block;
       ((blkidx <= end_block) && (roa_rc == FSCK_OK));
       blkidx++ ) {
    roa_rc = blkall_increment_owners( blkidx );
    }  /* end for */

      /*
       * first extent of the agg inode table
       */
  if( agg_recptr->primary_ait_4part1 )  {  /* primary already recorded */
    start_block = addressPXD(&(sb_ptr->s_ait2));
    length_in_agg_blocks = lengthPXD(&(sb_ptr->s_ait2));
    }  /* end primary already recorded */
  else {  /* secondary already recorded */
    start_block = AITBL_OFF / sb_ptr->s_bsize;
    length_in_agg_blocks = INODE_EXTENT_SIZE / sb_ptr->s_bsize;
    }  /* end else secondary already recorded */

  end_block = start_block + length_in_agg_blocks - 1;
  for( blkidx = start_block;
       ((blkidx <= end_block) && (roa_rc == FSCK_OK));
       blkidx++ ) {
    roa_rc = blkall_increment_owners( blkidx );
    }  /* end for */

  return( roa_rc );

}  /* end record_other_ait( ) */


/*****************************************************************************
 * NAME: replicate_superblock
 *
 * FUNCTION: Refresh both the primary and secondary superblocks in the
 *           aggregate from the correct (and possibly updated) superblock
 *           in the fsck buffer.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t replicate_superblock ()
{
  retcode_t rs_rc = FSCK_OK;

  rs_rc = ujfs_put_superblk( Dev_IOPort, sb_ptr, 1);  /* write from
                                  * the buffer to the primary superblock
                                  */
  agg_recptr->ag_modified = 1;  /* have to assume something got written */
  if( rs_rc != FSCK_OK )  {  /* not good here is really bad */
    agg_recptr->cant_write_primary_sb = 1;
    agg_recptr->ag_dirty = 1;
    sb_ptr->s_state |= FM_DIRTY; /* mark the superblock in the buffer
                                  * to show the aggregate is dirty
                                  * (in case it isn't already marked
                                  * that way)
                                  */
    fsck_send_msg( fsck_CNTWRTSUPP, 0, 0 );
    }  /* end not good here is really bad */
  else {  /* wrote to the primary superblock */
    agg_recptr->cant_write_primary_sb = 0;
    }  /* end wrote to the primary superblock */

  rs_rc = ujfs_put_superblk( Dev_IOPort, sb_ptr, 0);  /* write from
                                  * the buffer to the secondary superblock
                                  */
  agg_recptr->ag_modified = 1;  /* have to assume something got written */
  if( rs_rc == FSCK_OK )  {  /* wrote to secondary ok */
    agg_recptr->cant_write_secondary_sb = 0;
    }  /* end wrote to secondary ok */
  else {  /* not good here is pretty bad */
    agg_recptr->cant_write_secondary_sb = 1;
    fsck_send_msg( fsck_CNTWRTSUPS, 0, 0 );

    if( (sb_ptr->s_state & FM_DIRTY) != FM_DIRTY )  { /* superblk not marked dirty now */
         /*
          * This means, among other things, that we just
          * did a successful write to the primary superblock
          * and that we marked the primary to say the aggregate
          * is clean.
          */
      sb_ptr->s_state |= FM_DIRTY; /* mark it dirty in the buffer */
      rs_rc = ujfs_put_superblk( Dev_IOPort, sb_ptr, 1);  /* write from
                                  * the buffer to the primary superblock
                                  */
      agg_recptr->ag_modified = 1; /* have to assume something got written */
      if( rs_rc != FSCK_OK )  {  /* not good here is a disaster */
           /*
            * We may have just taken an aggregate marked dirty and
            * changed it to clean, but now we discover that it really
            * does have a serious problem.  And all we can do about
            * it is issue the strongest warning we can think up.
            */
        agg_recptr->cant_write_primary_sb = 1;

        fsck_send_msg( fsck_CNTWRTSUPP, 0, 0 );

        msgprms[0] = Vol_Label;
        msgprmidx[0] = 0;
        fsck_send_msg( fsck_AGGDRTYNOTCLN, 0, 1 );
        }  /* end not good here is a disaster */
      }  /* superblock not marked dirty now */
    }  /* end not good here is pretty bad */

  if( (agg_recptr->cant_write_primary_sb) &&
      (agg_recptr->cant_write_secondary_sb) ) {  /* both bad */
    rs_rc = FSCK_FAILED_BTHSBLK_WRITE;
    }  /* end both bad */
  else if( agg_recptr->cant_write_primary_sb )  {  /* primary bad */
    rs_rc = FSCK_FAILED_PSBLK_WRITE;
    }  /* end primary bad */
  else if( agg_recptr->cant_write_secondary_sb ) { /* secondary bad */
    rs_rc = FSCK_FAILED_SSBLK_WRITE;
    }  /* end secondary bad */

  return( rs_rc );
}                              /* end of replicate_superblock ()  */


/*****************************************************************************
 * NAME: rootdir_tree_bad
 *
 * FUNCTION:  This routine is called if the B+ Tree rooted in the fileset
 *            root directory (aggregate inode FILESET_I) is found to be
 *            corrupt.  If the user approves the repair, it makes the
 *            root directory B+ tree a correct, empty tree.
 *
 * PARAMETERS:
 *      inoptr         - input - pointer to the inode in an fsck buffer
 *      inode_updated  - input - pointer to a variable in which to return
 *                               !0 if the inode (in the buffer) has been
 *                                  modified by this routine
 *                                0 if the inode (in the buffer) has not been
 *                                  modified by this routine
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t rootdir_tree_bad ( dinode_t *inoptr, int *inode_updated )
{
  retcode_t rtb_rc = FSCK_OK;
  char user_reply;

  *inode_updated = 0;

  fsck_send_msg( fsck_RIBADTREE, 0, 0 );
  init_dir_tree( (dtroot_t *) &(inoptr->di_btroot) );
  inoptr->di_nblocks = 0;
  inoptr->di_nlink = 2;
  inoptr->di_size = sizeof( struct dinode ) - offsetof( struct dinode, _data3._data );
  *inode_updated = 1;
  agg_recptr->rootdir_rebuilt = 1;
  fsck_send_msg( fsck_RICRETREE, 0, 0 );

  return( rtb_rc );
}                              /* end of rootdir_tree_bad ()  */


/*****************************************************************************
 * NAME: validate_fs_metadata
 *
 * FUNCTION: Verify the metadata inodes for all filesets in the
 *           aggregate.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t validate_fs_metadata ( )
{
  retcode_t vfm_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;

  struct fsck_ino_msg_info  ino_msg_info;
  fsck_msg_info_ptr msg_info_ptr;

  int which_fsit, which_ait;
  inoidx_t ino_idx;
  dinode_t *ino_ptr;
  int aggregate_inode = 0;            /* going for fileset inodes only */
  int alloc_ifnull = -1;
  int inode_updated;

  fsck_inode_recptr inorecptr;

  msg_info_ptr = &ino_msg_info;
  msg_info_ptr->msg_inopfx = fsck_fset_inode;  /* all fileset owned */
  msg_info_ptr->msg_inotyp = fsck_metadata;

  if( agg_recptr->primary_ait_4part2 ) {
    which_ait = fsck_primary;
    }
  else {
    which_ait = fsck_secondary;
    }

  vfm_rc = ait_special_read_ext1( which_ait );

  if( vfm_rc != FSCK_OK ) {  /* read failed */
    report_readait_error( vfm_rc, 
                          FSCK_FAILED_CANTREADAITEXTE, 
                          which_ait );
    vfm_rc = FSCK_FAILED_CANTREADAITEXTE;
    }  /* end read failed */

  else {  /* read successful */

      /*
       * In release 1 there's exactly 1 fileset
       */
    which_fsit = FILESYSTEM_I;

    ino_idx = FILESET_EXT_I;       /* read the fileset superinode
                                    * extension
                                    */
    intermed_rc = inode_get( aggregate_inode, which_fsit, ino_idx, &ino_ptr );
    if( intermed_rc != FSCK_OK )  { /* can't get the inode */
      vfm_rc = FSCK_CANTREADFSEXT;
      }  /* end can't get inode  */

    else {  /* else got superinode extension inode  */
      msg_info_ptr->msg_inonum = FILESET_EXT_I;
      intermed_rc = verify_fs_super_ext ( ino_ptr, msg_info_ptr, &inode_updated );

      if( intermed_rc < 0 ) {  /* something really really bad happened   @F1 */
        vfm_rc = intermed_rc;                                                   /*     @F1 */
        }                                                                                /*     @F1 */
      else if( intermed_rc != FSCK_OK )  {  /* inode is bad.                   @F1 */
        vfm_rc = FSCK_FSETEXTBAD;
        }  /* end inode is bad. */
      else {  /* else superinode extension inode is ok */

        if( inode_updated )  {  /* need to write the superinode extension */
          vfm_rc = inode_put( ino_ptr );
          }  /* end need to write the superinode extension */

        if( vfm_rc == FSCK_OK ) {  /* still on track */
 
          ino_idx = ROOT_I;               /* read the root directory inode */

          intermed_rc = inode_get( aggregate_inode, which_fsit,
                                   ino_idx, &ino_ptr );

          if( intermed_rc < 0 ) {  /* something really really bad happened  @F1 */
            vfm_rc = intermed_rc;                                                /*       @F1 */
            }                                                                             /*       @F1 */

          else if( intermed_rc != FSCK_OK )  { /* can't get the inode          @F1 */
            vfm_rc = FSCK_CANTREADFSRTDR;
            }  /* end can't get inode  */

          else {  /* else got root directory inode  */

            msg_info_ptr->msg_inonum = ROOT_I;
            msg_info_ptr->msg_inotyp = fsck_directory;
            intermed_rc = verify_repair_fs_rootdir ( ino_ptr, msg_info_ptr,
                                                     &inode_updated );

            if( intermed_rc != FSCK_OK )  {  /* inode is bad. Couldn't
                                  * (or wasn't allowed to) repair it
                                  */
              vfm_rc = FSCK_FSRTDRBAD;
              }  /* end inode is bad Couldn't repair it */
            else {  /* root directory is good */

              if( inode_updated )  {  /* need to write the root directory */
                vfm_rc = inode_put( ino_ptr );
                }  /* end need to write the root directory */

                  /*
                   * now get records as placeholders for the 2 reserved
                   * fileset inodes
                   */

              vfm_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                                      FILESET_RSVD_I, &inorecptr );
              if( (vfm_rc == FSCK_OK) && (inorecptr == NULL) ) {
                vfm_rc = FSCK_INTERNAL_ERROR_34;                            /* @F1 */
                msgprms[0] = message_parm_0;                                   /* @F1 */
                msgprmidx[0] = 0;                                                      /* @F1 */
                sprintf( message_parm_1, "%ld", 0 );                             /* @F1 */
                msgprms[1] = message_parm_1;                                    /* @F1 */
                msgprmidx[1] = 0;                                                       /* @F1 */
                sprintf( message_parm_2, "%ld", 0 );                              /* @F1 */
                msgprms[2] = message_parm_2;                                     /* @F1 */
                msgprmidx[2] = 0;                                                        /* @F1 */
                sprintf( message_parm_3, "%ld", 0 );                              /* @F1 */
                msgprms[3] = message_parm_3;                                     /* @F1 */
                msgprmidx[3] = 0;                                                        /* @F1 */
                fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                   /* @F1 */
                }
              else if( vfm_rc == FSCK_OK ) {  /* got first record */
                inorecptr->inode_type = metadata_inode;
                inorecptr->in_use = 1;

                vfm_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                                        ACL_I, &inorecptr );
                if( (vfm_rc == FSCK_OK) && (inorecptr == NULL) ) {
                  vfm_rc = FSCK_INTERNAL_ERROR_35;
                  sprintf( message_parm_0, "%ld", vfm_rc );                   /* @F1 */
                  msgprms[0] = message_parm_0;                                 /* @F1 */
                  msgprmidx[0] = 0;                                                    /* @F1 */
                  sprintf( message_parm_1, "%ld", 0 );                           /* @F1 */
                  msgprms[1] = message_parm_1;                                  /* @F1 */
                  msgprmidx[1] = 0;                                                     /* @F1 */
                  sprintf( message_parm_2, "%ld", 0 );                            /* @F1 */
                  msgprms[2] = message_parm_2;                                   /* @F1 */
                  msgprmidx[2] = 0;                                                      /* @F1 */
                  sprintf( message_parm_3, "%ld", 0 );                            /* @F1 */
                  msgprms[3] = message_parm_3;                                   /* @F1 */
                  msgprmidx[3] = 0;                                                      /* @F1 */
                  fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                 /* @F1 */
                  }

                else if( vfm_rc == FSCK_OK ) {  /* got second record */
                  inorecptr->inode_type = metadata_inode;
                  inorecptr->in_use = 1;
                  }  /* end got second record */
                }  /* end got first record */
              }  /* end root directory is good */
            }  /* end else got root directory inode */
          }  /* end still on track */
        }  /* end else superinode extension inode is ok */
      }  /* end else got superinode extension inode */
    }  /* end else read successful */

  return( vfm_rc );
}                              /* end of validate_fs_metadata ( )  */


/*****************************************************************************
 * NAME: validate_repair_superblock
 *
 * FUNCTION:  Verify that the primary superblock is valid.  If it is,
 *            the secondary superblock will be refreshed later in
 *            processing.  If the primary superblock is not valid, 
 *            verify that the secondary superblock is valid.  If the
 *            secondary superblock is found to be valid, copy it
 *            over the primary superblock on the device so that 
 *            logredo will find a valid primary superblock.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t validate_repair_superblock ( )
{
  retcode_t vrsb_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  UniChar *uni_result_ptr;
  int primary_sb_bad = 1;           /* assume a problem with primary */
  int secondary_sb_bad = 1;         /* assume a problem with secondary */
  int which_sb;

  vrsb_rc = ujfs_get_superblk(Dev_IOPort, sb_ptr, 1); /* get primary  */

  if( vrsb_rc != FSCK_OK ) { /* if read primary fails */
    fsck_send_msg( fsck_CNTRESUPP, 0, 0 );
    }
  else {   /* got primary superblock */
    which_sb = fsck_primary;
    primary_sb_bad = validate_super( fsck_primary );
    }

  if( primary_sb_bad ) {  /* can't use the primary superblock */
    fsck_send_msg( fsck_SBBADP, 0, 0 );

    vrsb_rc = ujfs_get_superblk(Dev_IOPort, sb_ptr, 0); /* get 2ndary */

    if( vrsb_rc != FSCK_OK ) {
      fsck_send_msg( fsck_CNTRESUPS, 0, 0 );
      }
    else {    /* got secondary superblock */
      which_sb = fsck_secondary;
      secondary_sb_bad = validate_super( fsck_secondary );
      }

    if( !secondary_sb_bad ) {  /* secondary is ok */

      if( agg_recptr->processing_readonly ) {
        agg_recptr->ag_dirty = 1;
        agg_recptr->cant_write_primary_sb = 1;
        fsck_send_msg( fsck_BDSBNWRTACC, 0, 0 );
        }  /* end if processing read only */

      else { /* else processing read/write */
        sb_ptr->s_state = (sb_ptr->s_state | FM_DIRTY);
        intermed_rc = ujfs_put_superblk(Dev_IOPort, sb_ptr, 1); /*
                                   * correct the primary superblock
                                   */
        agg_recptr->ag_modified = 1;  /* must assume something got written */
        if( intermed_rc == FSCK_OK ) {
          agg_recptr->ag_modified = 1;
          }
        else {  /* write primary superblock failed */
           /*
            * we won't bail out on this condition (so we don't want
            * to pass back the return code), but it does leave
            * the aggregate dirty
            */
          agg_recptr->ag_dirty = 1;
          agg_recptr->cant_write_primary_sb = 1;

          fsck_send_msg( fsck_CNTWRTSUPP, 0, 0 );
          }  /* end write primary superblock failed */
        }  /* end else processing read/write */
      }  /* end secondary is ok */
    else { /* can't use the secondary superblock either */
      agg_recptr->ag_dirty = 1;
      vrsb_rc = FSCK_FAILED_BTHSBLK_BAD;

      fsck_send_msg( fsck_SBBADS, 0, 0 );

      fsck_send_msg( fsck_BDSBBTHCRRPT, 0, 0 );
      }  /* end can't use the secondary superblock either */
    }  /* end can't use the primary superblock */

  if( (!primary_sb_bad) || (!secondary_sb_bad) )  {   /*
                                      * the buffer holds a valid superblock
                                      */   
    agg_recptr->ag_blk_size = sb_ptr->s_bsize; /* aggregate block size */

    if( which_sb == fsck_primary ) {
      fsck_send_msg( fsck_SBOKP, 0, 0 );
      }
    else {
      fsck_send_msg( fsck_SBOKS, 0, 0 );
      }
    if( (sb_ptr->s_flag&JFS_SPARSE) == JFS_SPARSE ) {
      fsck_send_msg( fsck_SPARSEFILSYS, 0, 0 );
      }
    }

  return( vrsb_rc );
}                              /* end of validate_repair_superblock ()  */


/*****************************************************************************
 * NAME: validate_select_agg_inode_table
 *
 * FUNCTION:  Verify the inodes in the Aggregate Inode Table.  If all
 *            inodes in the Primary Aggregate Inode Table are valid,
 *            select it.  Otherwise, if all inodes in the Secondary
 *            Aggregate Inode Table are valid, select it.  Otherwise,
 *            if inodes 0 through 15 are valid in one table and inodes
 *            16 through 31 are valid in the other, select the valid
 *            0 through 15 and the valid 16 through 31.  
 *
 * PARAMETERS:  none
 *
 * NOTES:  o Aggregate inodes 0 through 15 describe the aggregate and
 *           aggregate metadata.  Aggregate inodes 16 through 31 each
 *           describe a fileset in the aggregate (In release 1, there
 *           is only 1 fileset in each aggregate and it is described
 *           by aggregate inode 16).  When neither Aggregate Inode
 *           Table is completely valid, this suggests the division:
 *                 "part 1" is aggregate inodes 0 through 15 
 *                 "part 2" is aggregate inodes 16 through 31
 *
 *         o While we naturally prefer to use only the Primary Aggregate
 *           Inode Table and, failing that, to use only the Secondary
 *           Aggregate Inode Table, in the interests of avoiding loss
 *           of user data, fsck will continue if it can find a valid
 *           part 1 and a valid part 2.
 *
 *         o Since this routine is invoked before the fsck workspace
 *           has been completely initialized, this routine ensures
 *           that the fsck I/O buffers contain the data needed by
 *           any routine which it invokes.
 *
 *           That is, since the workspace does not contain all the
 *           information for inode_get (et al) to calculate device
 *           offsets (needed to perform I/O), this routine ensures
 *           that any invocations of inode_get by routines invoked
 *           here will find the target data already in the fsck
 *           inode buffer.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t validate_select_agg_inode_table ( )
{
  retcode_t vsait_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;

  int aggregate_inode = -1;
  int alloc_ifnull = -1;
  fsck_inode_recptr inorecptr;
  int record_secondary = 0;
  int cant_read_primary = 0;
  int cant_read_secondary = 0;
  int primary_part1_good = 0;
  int primary_part2_good = 0;
  int primary_part1_bad = 0;
  int primary_part2_bad = 0;
  int secondary_part1_good = 0;
  int secondary_part2_good = 0;
  int secondary_part1_bad = 0;
  int secondary_part2_bad = 0;
  uint32 primary_inode_stamp;
  uint32 secondary_inode_stamp;
  uint32 unknown_stamp = (uint32) (-1);
  /*
   * try for part 1 and part 2 both from the primary aggregate inode table
   */
  intermed_rc = ait_special_read_ext1( fsck_primary );
  if( intermed_rc != FSCK_OK )  {  /* don't have it in the buffer */
    primary_part1_bad = -1;
    primary_part2_bad = -1;
    cant_read_primary = -1;
    }  /* end don't have it in the buffer */
  else {  /* got the 1st extent of primary AIT into the inode buffer */
    agg_recptr->inode_stamp = unknown_stamp;
    intermed_rc = verify_ait_part1( fsck_primary );
    primary_inode_stamp = agg_recptr->inode_stamp;
    if( intermed_rc < 0 )  {  /* something fatal */
      vsait_rc = intermed_rc;     /* we'll exit fsck on this */
      }
    else if( intermed_rc > 0 ) {  /* primary table part 1 is bad */
      primary_part1_bad = -1;
      }
    else {  /* primary table, part 1 is good */
      primary_part1_good = -1;
      intermed_rc = verify_ait_part2( fsck_primary );
      if( intermed_rc < 0 )  {  /* something fatal */
        vsait_rc = intermed_rc;     /* we'll exit fsck on this */
        }
      else if( intermed_rc > 0 ) {  /* primary table part 2 is bad */
        primary_part2_bad = -1;
        vsait_rc = backout_ait_part1( fsck_primary );
        }
      else {  /* primary table, part 2 is good */
        primary_part2_good = -1;
        }
      }  /* end else primary table, part 1 is good */
    } /* end else got the 1st extent of primary AIT into the inode buffer */
  /*
   * if can't have both part 1 and part 2 from the primary, try for
   * part 1 and part 2 both from the secondary aggregate inode table
   */
  if((vsait_rc == FSCK_OK) &&
     (primary_part1_bad || primary_part2_bad) ) { /* go for secondary ait */
    intermed_rc = ait_special_read_ext1( fsck_secondary );
    if( intermed_rc != FSCK_OK )  {  /* don't have it in the buffer */
      secondary_part1_bad = -1;
      secondary_part2_bad = -1;
      cant_read_secondary = -1;
      }  /* end don't have it in the buffer */
    else {  /* got the 1st extent of secondary AIT into the inode buffer */
      agg_recptr->inode_stamp = unknown_stamp;
      intermed_rc = verify_ait_part1( fsck_secondary );
      secondary_inode_stamp = agg_recptr->inode_stamp;
      if( intermed_rc < 0 )  {  /* something fatal */
        vsait_rc = intermed_rc;     /* we'll exit fsck on this */
        }
      else if( intermed_rc > 0 ) {  /* secondary table part 1 is bad */
        secondary_part1_bad = 1;
        }
      else {  /* secondary table, part 1 is good */
        secondary_part1_good = 1;
        intermed_rc = verify_ait_part2( fsck_secondary );
        if( intermed_rc < 0 )  {  /* something fatal */
          vsait_rc = intermed_rc;   /* we'll exit fsck on this */
           }
        else if( intermed_rc > 0 ) {  /* secondary table part 2 is bad */
          secondary_part2_bad = 1;
          vsait_rc = backout_ait_part1( fsck_secondary );
          }
        else {  /* secondary table, part 2 is good */
          secondary_part2_good = 1;
          }
        }  /* end else secondary table, part 1 is good */
      }  /* got the 1st extent of secondary AIT into the inode buffer */
    }  /* end go for secondary table */
	  
  if((vsait_rc == FSCK_OK) &&
     (primary_part1_good && primary_part2_good) ) {  /*
                            * normal case, nothing amiss
                            */
    agg_recptr->primary_ait_4part1 = 1;
    agg_recptr->primary_ait_4part2 = 1;
    agg_recptr->inode_stamp =  primary_inode_stamp;
    }  /* end normal case, nothing amiss */
  else if( (vsait_rc == FSCK_OK) &&
           (secondary_part1_good && secondary_part2_good) )  {  /*
                            * first safety net held up
                            */
    agg_recptr->primary_ait_4part1 = 0;
    agg_recptr->primary_ait_4part2 = 0;
    agg_recptr->inode_stamp =  secondary_inode_stamp;
    }  /* end else first safety net held up */
  else {  /* multiple points of failure. */
    /*
     * try to go on by using part1 from one table and part 2 from the other
     */
    if( vsait_rc == FSCK_OK ) { /* nothing fatal */
      if( primary_part1_good &&
               (!secondary_part2_good) && (!secondary_part2_bad) ) { /*
                               * primary part 1 is good and haven't checked
                               * secondary part 2 yet
                               */
        agg_recptr->inode_stamp =  primary_inode_stamp;
        intermed_rc = ait_special_read_ext1( fsck_primary );
        if( intermed_rc == FSCK_OK ) {
          vsait_rc = record_ait_part1_again( fsck_primary );
          }
        else {
          vsait_rc = FSCK_FAILED_CANTREADAITEXT4;
          }
        if( vsait_rc == FSCK_OK ) {  /* primary part1 re-recorded ok */
          intermed_rc = ait_special_read_ext1( fsck_secondary );
          if( intermed_rc != FSCK_OK )  {  /* didn't get it */
            secondary_part2_bad = 1;
            cant_read_secondary = -1;
            }  /* end didn't get it */
          else {  /* got the 1st extent of secondary AIT into the buffer */
            intermed_rc = verify_ait_part2( fsck_secondary );
            if( intermed_rc < 0 )  {  /* something fatal */
              vsait_rc = intermed_rc;
              }
            else if( intermed_rc > 0 ) {  /* secondary table part 2 is bad */
              secondary_part2_bad = 1;
              }
            else {  /* secondary table, part 2 is good */
              secondary_part2_good = -1;
              }
            }  /* got the 1st extent of secondary AIT into the buffer */
          }  /* end primary part1 re-recorded ok */
        }  /* end primary part 1 is good and haven't checked secondary... */
      else if( secondary_part1_good &&
               (!primary_part2_good) && (!primary_part2_bad) ) { /*
                               * secondary part 1 is good and haven't
                               * checked primary part 2 yet
                               */
        agg_recptr->inode_stamp =  secondary_inode_stamp;
        intermed_rc = ait_special_read_ext1( fsck_primary );
        if( intermed_rc != FSCK_OK )  {  /* didn't get it */
          primary_part2_bad = -1;
          cant_read_primary = -1;
          }  /* end didn't get it */
        else {  /* got the 1st extent of primary AIT into the buffer */
          intermed_rc = verify_ait_part2( fsck_primary );
          if( intermed_rc < 0 )  {  /* something fatal */
            vsait_rc = intermed_rc;
            }
          else if( intermed_rc > 0 ) {  /* primary table part 2 is bad */
            primary_part2_bad = 1;
            }
          else {  /* primary table, part 2 is good */
            primary_part2_good = 1;
            intermed_rc = ait_special_read_ext1( fsck_secondary );
            if( intermed_rc == FSCK_OK ) {
              vsait_rc = record_ait_part1_again( fsck_secondary );
              }
            else {
              vsait_rc = FSCK_FAILED_CANTREADAITEXT5;
              }
            }
          }  /* got the 1st extent of primary AIT into the buffer */
        }  /* end secondary part 1 is good and haven't checked primary... */
      }  /* end nothing fatal */
    if( vsait_rc == FSCK_OK ) { /* still nothing fatal */
      if( primary_part1_good && secondary_part2_good ) {
        agg_recptr->primary_ait_4part1 = 1;
        agg_recptr->primary_ait_4part2 = 0;
        }
      else if( secondary_part1_good && primary_part2_good ) {
        agg_recptr->primary_ait_4part1 = 0;
        agg_recptr->primary_ait_4part2 = 1;
        }
      else { /* either both have bad part 1 or both have bad part 2 */
        vsait_rc = FSCK_FAILED_BOTHAITBAD;
        }
      }  /* end else multiple points of failure */
    }  /* end still nothing fatal */
  if( vsait_rc == FSCK_OK ) { /* we have enough to go on */
        /*
         * get a record as placeholder for the reserved
         * aggregate inode
         */
    vsait_rc = get_inorecptr( aggregate_inode, alloc_ifnull, 0, &inorecptr );
    if( (vsait_rc == FSCK_OK) && (inorecptr == NULL) ) {
      vsait_rc = FSCK_INTERNAL_ERROR_36;
      sprintf( message_parm_0, "%ld", vsait_rc );                 /* @F1 */
      msgprms[0] = message_parm_0;                                 /* @F1 */
      msgprmidx[0] = 0;                                                    /* @F1 */
      sprintf( message_parm_1, "%ld", 0 );                           /* @F1 */
      msgprms[1] = message_parm_1;                                  /* @F1 */
      msgprmidx[1] = 0;                                                     /* @F1 */
      sprintf( message_parm_2, "%ld", 0 );                            /* @F1 */
      msgprms[2] = message_parm_2;                                   /* @F1 */
      msgprmidx[2] = 0;                                                      /* @F1 */
      sprintf( message_parm_3, "%ld", 0 );                            /* @F1 */
      msgprms[3] = message_parm_3;                                   /* @F1 */
      msgprmidx[3] = 0;                                                      /* @F1 */
      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                 /* @F1 */
      }
    else if( vsait_rc == FSCK_OK ) {  /* got the record */
      inorecptr->inode_type = metadata_inode;
      inorecptr->in_use = 1;
      }  /* end got the record */
    }  /* end we have enough to go on */
  else {
    if( cant_read_primary && cant_read_secondary ) {  /* this is fatal */
      vsait_rc = FSCK_FAILED_CANTREADAITS;
      }  /* end this is fatal */
    }
        /*
         * Deal with the Aggregate Inode Map (and Table) not chosen
         *
         * If we're processing read-only and the primary versions are
         * ok, we need to verify that the secondary versions are
         * correctly redundant to the primary versions.
         */
  if( vsait_rc == FSCK_OK ) {  /*  a table is chosen */
    if( agg_recptr->processing_readwrite )  {  /*
                               * have write access so we'll be refreshing
                               * the redundant version later on -- for now
                               * just reserve the blocks for it.
                               */
      vsait_rc = record_other_ait( );
      }  /* end have write access */
    else {  /* processing read-only */
      if( primary_part1_good && primary_part2_good ) {  /*
                               * need to verify that the secondary table and
                               * map are correct redundant copies of the
                               * primary table and map.
                               */
        vsait_rc = AIS_redundancy_check( );
        }  /* end need to verify that the secondary table and map are ... */
      else {  /* either part1 or part2 of primary are invalid */
        agg_recptr->ag_dirty = 1;
        }
      if( vsait_rc == FSCK_OK ) {  /* if it isn't correct a message has
                               * been issued.  Record the blocks they
                               * way that we were unable to record the
                               * occupy to avoid misleading error messages
                               * later when we verify the block allocation
                               * map.
                               */
        vsait_rc = record_other_ait( );
        }
      }  /* end else processing read-only */
    }  /* end a table is chosen */
  /*
   * report problems detected (if any) 
   */
  if( cant_read_primary ) {
    fsck_send_msg( fsck_CANTREADAITP, 0, 0 );
    }
  else if( primary_part1_bad || primary_part2_bad ) {
    fsck_send_msg( fsck_ERRORSINAITP, 0, 0 );
    }
  if( cant_read_secondary ) {
    fsck_send_msg( fsck_CANTREADAITS, 0, 0 );
    }
  else if( secondary_part1_bad || secondary_part2_bad ) {
    fsck_send_msg( fsck_ERRORSINAITS, 0, 0 );
    }
  if( cant_read_primary && cant_read_secondary ) {
    agg_recptr->ag_dirty = 1;
    fsck_send_msg( fsck_CANTCONTINUE, 0, 0 );
    }
  else if( primary_part1_bad && secondary_part1_bad ) {
    agg_recptr->ag_dirty = 1;
    fsck_send_msg( fsck_CANTCONTINUE, 0, 0 );
    }
  else if( primary_part2_bad && secondary_part2_bad ) {
    agg_recptr->ag_dirty = 1;
    fsck_send_msg( fsck_CANTCONTINUE, 0, 0 );
    }
  return( vsait_rc );
}                          /* end of validate_select_agg_inode_table ()  */
 	
 	
/*****************************************************************************
 * NAME: validate_super
 *
 * FUNCTION:  This routine validates the JFS superblock currently in the
 *            buffer.  If any problem is detected, the which_superblock
 *            input parm is used to tailor the message issued to notify
 *            the user.
 *
 * PARAMETERS:
 *      which_super  - input - specifies the superblock which is in the
 *                             buffer { fsck_primary | fsck_secondary }
 *
 * NOTES: 
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t validate_super ( int which_super )
{
  retcode_t vs_rc = 0;               /* assume the superblock is ok */
  retcode_t ujfs_rc = 0;
  agg_byteidx_t bytes_on_device, hidden_bytes_on_device;
  agg_byteidx_t s_size_inbytes, sum_inbytes;
  fsblkidx_t agg_blks_in_aggreg, agg_blks_on_device, dev_blks_on_device;
  fsblkidx_t fsck_start_from_pxd, fsck_blkmap_start_blks;
  reg_idx_t fsck_length_from_pxd, fsck_blkmap_size_blks, fsck_blkmap_size_pages;
  fsblkidx_t jlog_start_from_pxd;
  reg_idx_t jlog_length_from_pxd;
  int32 agl2size;
  uint32 expected_flag = JFS_OS2|JFS_GROUPCOMMIT|JFS_INLINELOG;
  uint32 agsize;
  int bad_bsize = 0;

  if( memcmp(sb_ptr->s_magic, JFS_MAGIC,
               sizeof(sb_ptr->s_magic)) != 0 ) {
    vs_rc = FSCK_BADSBMGC;           /* problem detected */
    msgprms[0] = message_parm_0;
    msgprmidx[0] = which_super;
    fsck_send_msg( fsck_BADSBMGC, 0, 1 );
    }  /* end if bad magic number */
  else if( sb_ptr->s_version != JFS_VERSION ) {
    vs_rc = FSCK_BADSBVRSN;          /* problem detected */
    msgprms[0] = message_parm_0;
    msgprmidx[0] = which_super;
    fsck_send_msg( fsck_BADSBVRSN, 0, 1 );
    }  /* end if bad version number */
  else  {  /* the magic number and version number are correct so it
            * probably is a JFS superblock with the format we are expecting
            */
    ujfs_rc = ujfs_get_dev_size( Dev_IOPort, 
                                 &bytes_on_device,
                                 &hidden_bytes_on_device );   /*
                                       * get physical device size
                                       */
    dev_blks_on_device = bytes_on_device / Dev_blksize;
    if( sb_ptr->s_pbsize != Dev_blksize ) {
      vs_rc = FSCK_BADSBOTHR1;       /* problem detected */
      msgprms[0] = "1";
      msgprmidx[0] = 0;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = which_super;
      fsck_send_msg( fsck_BADSBOTHR, 0, 2 );
      }  /* end if physical blocksize is incorrect */
	
    if( sb_ptr->s_l2pbsize != log2shift( Dev_blksize ) ) {
      vs_rc = FSCK_BADSBOTHR2;       /* problem detected */
      msgprms[0] = "2";
      msgprmidx[0] = 0;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = which_super;
      fsck_send_msg( fsck_BADSBOTHR, 0, 2 );
      }  /* end if log 2 of physical blocksize is incorrect */
	
    if( !inrange( sb_ptr->s_bsize, 512, 4096 ) ) {
      bad_bsize = -1;
      vs_rc = FSCK_BADSBOTHR3;       /* problem detected */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = which_super;
      fsck_send_msg( fsck_BADSBBLSIZ, 0, 1 );
      }  /* end if the filesystem block size is not an integral power of 2
            * and between 512 and 4096
            */
    else {  /* else the filesystem block size is a legal value */
      if( sb_ptr->s_l2bsize != log2shift( sb_ptr->s_bsize ) ) {
        vs_rc = FSCK_BADSBOTHR4;     /* problem detected */
        msgprms[0] = "4";
        msgprmidx[0] = 0;
        msgprms[1] = message_parm_1;
        msgprmidx[1] = which_super;
        fsck_send_msg( fsck_BADSBOTHR, 0, 2 );
        }  /* end if log2 of block size is incorrect */
	
      if( sb_ptr->s_l2bfactor !=
          log2shift( sb_ptr->s_bsize / Dev_blksize ) ) {
        vs_rc = FSCK_BADSBOTHR5;     /* problem detected */
        msgprms[0] = "5";
        msgprmidx[0] = 0;
        msgprms[1] = message_parm_1;
        msgprmidx[1] = which_super;
        fsck_send_msg( fsck_BADSBOTHR, 0, 2 );
        }  /* end if log2 of the ratio between logical and physical
            * blocks is incorrect
            */
	
      if( sb_ptr->s_bsize < Dev_blksize ) {
        bad_bsize = -1;
        vs_rc = FSCK_BLSIZLTLVBLSIZ; /* problem detected */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = which_super;
        fsck_send_msg( fsck_BLSIZLTLVBLSIZ, 0, 1 );
        }  /* end if filesystem block size is less than device block size */
      }  /* end else the filesystem block size is an acceptable value */
	
    if( !bad_bsize ) {  /* the blocksize looks ok */
      agg_blks_on_device = bytes_on_device / sb_ptr->s_bsize;
	
      if( sb_ptr->s_size > dev_blks_on_device ) {
        vs_rc = FSCK_BADSBFSSIZ;       /* problem detected */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = which_super;
        fsck_send_msg( fsck_BADSBFSSIZ, 0, 1 );
        }  /* end if size in phys blocks is bigger than whole device */
#ifdef	_JFS_DFS_LFS
	
      s_size_inbytes = sb_ptr->s_size * Dev_blksize;
      sum_inbytes = (int64)(sb_ptr->totalUsable * 1024) +
                    (int64)(sb_ptr->minFree * 1024);
      if( (sum_inbytes > s_size_inbytes) ||
          ( (s_size_inbytes - sum_inbytes) >= 1024 ) 
        ) { /* the sum is greater or the difference is at least 1K */
        vs_rc = FSCK_BADBLKCTTTL;      /* problem detected */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = which_super;
        fsck_send_msg( fsck_BADBLKCTTTL, 0, 1 );
        }  /* end the sum is greater or the difference is at least 1 K */
#endif	/* _JFS_DFS_LFS */
	
      if( (sb_ptr->s_flag&expected_flag)!= expected_flag ) {
        vs_rc = FSCK_BADSBOTHR6;     /* problem detected */
        msgprms[0] = "6";
        msgprmidx[0] = 0;
        msgprms[1] = message_parm_1;
        msgprmidx[1] = which_super;
        fsck_send_msg( fsck_BADSBOTHR, 0, 2 );
        }  /* end if allocation group is too small (must be at least 32M) */
	
      if( sb_ptr->s_agsize < (1 << L2BPERDMAP) ) {
        vs_rc = FSCK_BADSBAGSIZ;       /* problem detected */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = which_super;
        fsck_send_msg( fsck_BADSBAGSIZ, 0, 1 );
        }  /* end if allocation group is too small (must be at least 32M) */
      else {  /* else the alloc group size is possibly correct */
        agg_blks_in_aggreg = sb_ptr->s_size * sb_ptr->s_pbsize / sb_ptr->s_bsize;
        agl2size = ujfs_getagl2size( agg_blks_in_aggreg, sb_ptr->s_bsize );
        agsize = (int64) 1 << agl2size; /* get the allocation group size */
        if( sb_ptr->s_agsize != agsize ) {
          vs_rc = FSCK_BADAGFSSIZ;     /* problem detected */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = which_super;
          fsck_send_msg( fsck_BADSBAGSIZ, 0, 1 );
          }  /* end if size in phys blocks is bigger than whole device */
        }  /* end else the alloc group size is possibly correct */
      }   /* end the blocksize looks ok */

    if( !vs_rc ) {  /* no problems detected in this superblock yet */
        /*
         * check out the fsck in-aggregate workspace 
         */
      fsck_length_from_pxd = lengthPXD(&(sb_ptr->s_fsckpxd));
      fsck_start_from_pxd = addressPXD(&(sb_ptr->s_fsckpxd));
      agg_blks_in_aggreg =  fsck_length_from_pxd +
                     (sb_ptr->s_size * sb_ptr->s_pbsize / sb_ptr->s_bsize);
      if( agg_blks_in_aggreg > agg_blks_on_device ) {  /* wsp length is bad */
        vs_rc = FSCK_BADSBFWSL1;      /* problem detected */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = which_super;
        fsck_send_msg( fsck_BADSBFWSL1, 0, 1 );
        }  /* end wsp length is bad */
      else {  /* wsp length is plausible */
        fsck_blkmap_size_pages =
              ((agg_blks_in_aggreg + (BITSPERPAGE - 1)) / BITSPERPAGE) + 1 + 50;
        fsck_blkmap_size_blks =
                  (fsck_blkmap_size_pages << L2PSIZE) / sb_ptr->s_bsize; /*
                                      * size in aggregate blocks
                                      */
        fsck_blkmap_start_blks = agg_blks_in_aggreg - fsck_blkmap_size_blks; /*
                                      * aggregate block offset of the fsck
                                      * workspace in the aggregate.
                                      */
        if( fsck_length_from_pxd != fsck_blkmap_size_blks ) { /*
                                      * length of fsck in-aggregate workspace
                                      * is incorrect
                                      */
          vs_rc = FSCK_BADSBFWSL;      /* problem detected */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = which_super;
          fsck_send_msg( fsck_BADSBFWSL, 0, 1 );
          }  /* end length of fsck in-aggregate workspace is incorrect */
        if( fsck_start_from_pxd != fsck_blkmap_start_blks ) { /*
                                      * address of fsck in-aggregate workspace
                                      * is incorrect
                                      */
          vs_rc = FSCK_BADSBFWSA;      /* problem detected */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = which_super;
          fsck_send_msg( fsck_BADSBFWSA, 0, 1 );
          }  /* end address of fsck in-aggregate workspace is incorrect */
        }  /* end else wsp length is plausible */
      }  /* end no problems detected in this superblock yet */
	 
    if( !vs_rc ) {  /* no problems detected in this superblock yet */
        /*
         * check out the in-aggregate journal log
         *
         * if there is one it starts at the end of the fsck
         * in-aggregate workspace.
         */
      jlog_length_from_pxd = lengthPXD(&(sb_ptr->s_logpxd));
      jlog_start_from_pxd = addressPXD(&(sb_ptr->s_logpxd));
      if( jlog_start_from_pxd != 0 ) {  /* there's one in there */
        if( jlog_start_from_pxd != agg_blks_in_aggreg ) { /*
                                      * address of in-aggregate journal log
                                      * is incorrect
                                      */
          vs_rc = FSCK_BADSBFJLA;      /* problem detected */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = which_super;
          fsck_send_msg( fsck_BADSBFJLA, 0, 1 );
          }  /* end address of in-aggregate journal log is incorrect */
        agg_blks_in_aggreg +=  jlog_length_from_pxd;
        if( agg_blks_in_aggreg > agg_blks_on_device ) {  /* log length is bad */
          vs_rc = FSCK_BADSBFJLL;      /* problem detected */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = which_super;
          fsck_send_msg( fsck_BADSBFJLL, 0, 1 );
          }  /* end log length is bad */
        }  /* end there's one in there */
      }  /* end no problems detected in this superblock yet */

    if( !vs_rc ) {  /* no problems detected in this superblock yet */
        /*
         * check out the descriptors for 
         * the Secondary Agg Inode Table and the Secondary Agg Inode Map
         */
      vs_rc = validate_super_2ndaryAI( which_super );
      }  /* end no problems detected in this superblock yet */
    }  /* end else the magic number and version number are correct so it
        * probably is a JFS superblock with the format we are expecting
        */
  return( vs_rc );
}                              /* end of validate_super ()  */
 	
 	
/*****************************************************************************
 * NAME: validate_super_2ndaryAI
 *
 * FUNCTION:  This routine validates, in the current superblock, the
 *            descriptors for the Secondary Aggregate Inode Table and
 *            the Secondary Aggregate Inode Map.
 *
 *            If any problem is detected, the which_superblock input parm
 *            is used to tailor the message issued to notify the user.
 *
 * PARAMETERS:
 *      which_super  - input - specifies the superblock which is in the
 *                             buffer { fsck_primary | fsck_secondary }
 *
 * NOTES: 
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t validate_super_2ndaryAI( int which_super )
{
  retcode_t vs2AI_rc = 0;               /* assume the superblock is ok */
  retcode_t intermed_rc = 0;
  int32 AIM_bytesize, AIT_bytesize, selfIno_bytesize;
  int32 expected_AIM_bytesize, expected_AIT_bytesize;
  int64 AIM_byte_addr, AIT_byte_addr, fsckwsp_addr;
  int64 selfIno_addr, other_sb_AIM_byte_addr, other_sb_AIT_byte_addr;
  int64 byte_addr_diff, offset_other_super;
  dinode_t *AggInodes = NULL;
  uint32  bufsize, datasize;
  xtpage_t *selfIno_xtree;
  xad_t *selfIno_xad;
  struct superblock *other_sb_ptr;

  bufsize = PGSPERIEXT* BYTESPERPAGE;
  expected_AIM_bytesize = 2 * BYTESPERPAGE;
  AIM_bytesize = lengthPXD(&(sb_ptr->s_aim2)) * sb_ptr->s_bsize;
  if( AIM_bytesize != expected_AIM_bytesize ) {
    vs2AI_rc = FSCK_BADSBOTHR7;     /* problem detected */
    msgprms[0] = "7";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = which_super;
    fsck_send_msg( fsck_BADSBOTHR, 0, 2 );
    }
  else {  /* AIM size ok */
    expected_AIT_bytesize = 4 * BYTESPERPAGE;
    AIT_bytesize = lengthPXD(&(sb_ptr->s_ait2)) * sb_ptr->s_bsize;
    if( AIT_bytesize != expected_AIT_bytesize ) {
      vs2AI_rc = FSCK_BADSBOTHR8;     /* problem detected */
      msgprms[0] = "8";
      msgprmidx[0] = 0;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = which_super;
      fsck_send_msg( fsck_BADSBOTHR, 0, 2 );
       }
    else {  /* AIT size ok */
      AIM_byte_addr = addressPXD(&(sb_ptr->s_aim2)) * sb_ptr->s_bsize;
      AIT_byte_addr = addressPXD(&(sb_ptr->s_ait2)) * sb_ptr->s_bsize;
      byte_addr_diff = AIT_byte_addr - AIM_byte_addr;
      if( byte_addr_diff != AIM_bytesize ) {
        vs2AI_rc = FSCK_BADSBOTHR9;     /* problem detected */
        msgprms[0] = "9";
        msgprmidx[0] = 0;
        msgprms[1] = message_parm_1;
        msgprmidx[1] = which_super;
        fsck_send_msg( fsck_BADSBOTHR, 0, 2 );
        }
      else {  /* relative addrs of AIT and AIM are ok */
        fsckwsp_addr = addressPXD(&(sb_ptr->s_fsckpxd)) * sb_ptr->s_bsize;
        byte_addr_diff = fsckwsp_addr - AIT_byte_addr;
        if( byte_addr_diff <= AIT_bytesize ) {
          vs2AI_rc = FSCK_BADSBOTHR10;     /* problem detected */
          msgprms[0] = "10";
          msgprmidx[0] = 0;
          msgprms[1] = message_parm_1;
          msgprmidx[1] = which_super;
          fsck_send_msg( fsck_BADSBOTHR, 0, 2 );
          }
        else {  /* relative addrs of fsck workspace and AIT are possible */
		/*
		 * Allocate a buffer then read in the alleged secondary AIT.
		 * The self inode should describe the AIM.
		 */
          agg_recptr->vlarge_current_use = USED_FOR_SUPER_VALIDATION;
          AggInodes = (dinode_t *) agg_recptr->vlarge_buf_ptr;

          intermed_rc = readwrite_device( AIT_byte_addr, bufsize, &datasize, 
                                                        (void *) AggInodes, fsck_READ );
          if( (intermed_rc != FSCK_OK) || (datasize != bufsize) ) {
            vs2AI_rc = FSCK_BADSBOTHR11;     /* problem detected */
            msgprms[0] = "11";
            msgprmidx[0] = 0;
            msgprms[1] = message_parm_1;
            msgprmidx[1] = which_super;
            fsck_send_msg( fsck_BADSBOTHR, 0, 2 );
            }
          else {  /* alleged secondary AIT is in the buffer */
		/*
		 * Check the "data" extent in the self inode in the
		 * alleged Secondary Aggregate Inode Table.
		 *
		 * If it should describes the AIM, all is well.
		 * 
		 * If it does NOT describe the AIM, then it might be a bad
		 * superblock, and it might be a bad AIT.  Read the other
		 * superblock and compare the AIM and AIT descriptors.
		 * If they match, assume the superblock is ok but the AIT
		 * is bad.  
		 * N.B. we can fix a bad AIM and a bad AIT.  we cannot do
		 *       even continue chkdsk without a good superblock.
		 */
            selfIno_xtree = (xtpage_t *) &(AggInodes[AGGREGATE_I].di_btroot);
            selfIno_xad = &(selfIno_xtree->xad[XTENTRYSTART]);
            selfIno_bytesize = lengthXAD( selfIno_xad ) * sb_ptr->s_bsize;
            selfIno_addr = addressXAD( selfIno_xad ) * sb_ptr->s_bsize;
            if( (selfIno_bytesize != AIM_bytesize) ||
                 (selfIno_addr != AIM_byte_addr) ) {  /* inode doesn't describe AIM */
              if( which_super == fsck_primary ) {
                offset_other_super = SUPER2_OFF;
                }
              else {
                offset_other_super = SUPER1_OFF;
                }
              other_sb_ptr = (struct superblock *)  AggInodes;
              intermed_rc = readwrite_device( offset_other_super, 
                                                            SIZE_OF_SUPER, &datasize, 
                                                            (void *) other_sb_ptr, fsck_READ );
              if( (intermed_rc != FSCK_OK) || (datasize != SIZE_OF_SUPER) ) {
                vs2AI_rc = FSCK_BADSBOTHR12;     /* problem detected */
                msgprms[0] = "12";
                msgprmidx[0] = 0;
                msgprms[1] = message_parm_1;
                msgprmidx[1] = which_super;
                fsck_send_msg( fsck_BADSBOTHR, 0, 2 );
                }
              else {  /* other superblock has been read */
                other_sb_AIM_byte_addr = 
                                addressPXD(&(other_sb_ptr->s_aim2)) * sb_ptr->s_bsize;
                other_sb_AIT_byte_addr = 
                                addressPXD(&(other_sb_ptr->s_ait2)) * sb_ptr->s_bsize;
                if( (AIM_byte_addr != other_sb_AIM_byte_addr) || 
                    (AIT_byte_addr != other_sb_AIT_byte_addr)    ) {  /* no joy */
                  vs2AI_rc = FSCK_BADSBOTHR13;     /* problem detected */
                  msgprms[0] = "13";
                  msgprmidx[0] = 0;
                  msgprms[1] = message_parm_1;
                  msgprmidx[1] = which_super;
                  fsck_send_msg( fsck_BADSBOTHR, 0, 2 );
                  }  /* end no joy */
                }  /* end else other superblock has been read */
              }  /* end inode doesn't describe AIM */
            }  /* end else alleged secondary AIT is in the buffer */
          }  /* end else relative addrs of fsck workspace and AIT are possible */
        }  /* end else relative addrs of AIT and AIM are ok */
      }  /* end else AIT size ok */
    }  /* end else AIM size ok */
  
  agg_recptr->vlarge_current_use = NOT_CURRENTLY_USED;	

  return( vs2AI_rc );
}                              /* end of validate_super_2ndaryAI ()  */
 	
 	
/*****************************************************************************
 * NAME: verify_agg_fileset_inode
 *
 * FUNCTION:  Verify the structures associated with and the content of 
 *            the aggregate fileset inode, aggregate inode 16,  whose 
 *            data is the Fileset Inode Map.
 *
 * PARAMETERS:
 *      inoptr        - input - pointer to the inode in an fsck buffer
 *      inoidx        - input - ordinal number of the inode (i.e., inode number
 *                              as an int32)
 *      which_ait     - input - the Aggregate Inode Table on which to perform
 *                              the function.  { fsck_primary | fsck_secondary }
 *      msg_info_ptr  - input - pointer to a data area with data needed to
 *                              issue messages about the inode
 *
 * NOTES:  o Inode number and inode index are input parameters to facilitate
 *           multiple filesets per aggregate in a later release.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t verify_agg_fileset_inode ( dinode_t          *inoptr,
                                     inoidx_t           inoidx,
                                     int                which_ait,
                                     fsck_msg_info_ptr  msg_info_ptr
                                   )
{
  retcode_t vafsi_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  int inode_invalid = 0;
  int ixpxd_unequal = 0;
  fsck_inode_recptr inorecptr;
  int aggregate_inode = -1;
  int alloc_ifnull = -1;
  uint32 unknown_stamp = (uint32) (-1);
  if( (agg_recptr->inode_stamp != unknown_stamp) &&
      (inoptr->di_inostamp != agg_recptr->inode_stamp) ) { /* 
                             * we got a key from the corresponding AIT but
                             * the one in this inode doesn't match -- so 
                             * this is either trashed or residual
                             */
    inode_invalid = 1;
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_inotyp;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    fsck_send_msg( fsck_BADINOSTAMP, 0, 3 );
    }  /* end doesn't match the key -- so this is either trashed or residual */
    
  if( inoptr->di_fileset != AGGREGATE_I ) {  /* unexpected fileset # */
    inode_invalid = 1;
    msgprms[0] = "1";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end unexpected fileset # */
  if( inoptr->di_number != ((ino_t) inoidx) ) {  /* unexpected inode # */
    inode_invalid = 1;
    msgprms[0] = "2";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end unexpected inode # */
  if( inoptr->di_gen != 1 ) {  /* incorrect generation # */
    inode_invalid = 1;
    msgprms[0] = "2a";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect generation # */
 ixpxd_unequal = memcmp( (void *) &(inoptr->di_ixpxd),
                          (void *) &(agg_recptr->ino_ixpxd),
                          sizeof(pxd_t) );       /* returns 0 if equal */
  if( ixpxd_unequal ) {  /* incorrect extent descriptor */
    inode_invalid = 1;
    msgprms[0] = "3";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect extent descriptor */
  if( (inoptr->di_mode & (IFJOURNAL|IFREG)) != (IFJOURNAL|IFREG) ) {  /* 
                                      * incorrect mode 
                                      */
    inode_invalid = 1;
    msgprms[0] = "4";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect mode */
  if( inoptr->di_nlink != 1 ) {  /* incorrect # of links */
    inode_invalid = 1;
    msgprms[0] = "5";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect # of links */
  if( !(inoptr->di_dxd.flag & BT_ROOT) ) {  /* not flagged as B+ Tree root */
    inode_invalid = 1;
    msgprms[0] = "6";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end not flagged as B+ Tree root *
  /*
   * If any problems detected so far, don't bother trying to validate
   * the B+ Tree
   */
  if( !inode_invalid )  {  /* self inode looks ok so far */
    vafsi_rc = get_inorecptr( aggregate_inode, alloc_ifnull, inoidx, &inorecptr );
    if( (vafsi_rc == FSCK_OK) && (inorecptr == NULL) ) {
      vafsi_rc = FSCK_INTERNAL_ERROR_37;
      sprintf( message_parm_0, "%ld", vafsi_rc );                 /* @F1 */
      msgprms[0] = message_parm_0;                                 /* @F1 */
      msgprmidx[0] = 0;                                                    /* @F1 */
      sprintf( message_parm_1, "%ld", 0 );                           /* @F1 */
      msgprms[1] = message_parm_1;                                  /* @F1 */
      msgprmidx[1] = 0;                                                     /* @F1 */
      sprintf( message_parm_2, "%ld", 0 );                            /* @F1 */
      msgprms[2] = message_parm_2;                                   /* @F1 */
      msgprmidx[2] = 0;                                                      /* @F1 */
      sprintf( message_parm_3, "%ld", 0 );                            /* @F1 */
      msgprms[3] = message_parm_3;                                   /* @F1 */
      msgprmidx[3] = 0;                                                      /* @F1 */
      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                 /* @F1 */
      }
    else if( vafsi_rc == FSCK_OK ) {  /* no problems so far */
      vafsi_rc = verify_metadata_data( inoptr, inoidx, inorecptr, msg_info_ptr ); 
      if( inorecptr->ignore_alloc_blks || (vafsi_rc != FSCK_OK) ) {
        inode_invalid = -1;
        }
      }  /* end no problems so far */
    }  /* end self inode looks ok so far */
 /*
  * wrap it all up for this inode
  */
  if( (inode_invalid ) && (vafsi_rc == FSCK_OK) )  {
    vafsi_rc = FSCK_AGGFSINOBAD;
    }
  if( inode_invalid ) {  /* this one's corrupt */
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    if( which_ait == fsck_primary ) {
      fsck_send_msg( fsck_BADMETAINOP, 0, 2 );
      }
    else {
      fsck_send_msg( fsck_BADMETAINOS, 0, 2 );
      }
    }  /* end this one's corrupt */
  return( vafsi_rc );
}                          /* end of verify_agg_fileset_inode ()  */
 	
 	
/*****************************************************************************
 * NAME: verify_ait_inode
 *
 * FUNCTION:  Verify the structures associated with and the content of 
 *            the aggregate "self" inode, aggregate inode 1, whose
 *            data is the Fileset Inode Map.
 *
 * PARAMETERS:
 *      inoptr        - input - pointer to the inode in an fsck buffer
 *      which_ait     - input - the Aggregate Inode Table on which to perform
 *                              the function.  { fsck_primary | fsck_secondary }
 *      msg_info_ptr  - input - pointer to a data area with data needed to
 *                              issue messages about the inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t verify_ait_inode ( dinode_t          *inoptr,
                             int                which_ait,
                             fsck_msg_info_ptr  msg_info_ptr
                            )
{
  retcode_t vai_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  int inode_invalid = 0;
  int ixpxd_unequal = 0;
  fsck_inode_recptr inorecptr;
  int aggregate_inode = -1;
  int alloc_ifnull = -1;
  if( inoptr->di_fileset != AGGREGATE_I ) {  /* unexpected fileset # */
    inode_invalid = 1;
    msgprms[0] = "7";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end unexpected fileset # */
  if( inoptr->di_fileset != AGGREGATE_I ) {  /* unexpected fileset # */
    inode_invalid = 1;
    msgprms[0] = "8";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end unexpected fileset # */
  if( inoptr->di_number != AGGREGATE_I ) {  /* unexpected inode # */
    inode_invalid = 1;
    msgprms[0] = "9";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end unexpected inode # */
  if( inoptr->di_gen != 1 ) {  /* incorrect generation # */
    inode_invalid = 1;
    msgprms[0] = "10";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect generation # */
  ixpxd_unequal = memcmp( (void *) &(inoptr->di_ixpxd),
                          (void *) &(agg_recptr->ino_ixpxd),
                          (sizeof(pxd_t)) );       /* returns 0 if equal */
  if( ixpxd_unequal ) {  /* incorrect extent descriptor */
    inode_invalid = 1;
    msgprms[0] = "11";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect extent descriptor */
  if( (inoptr->di_mode & (IFJOURNAL|IFREG)) != (IFJOURNAL|IFREG) ) {  /* 
                                 * incorrect mode 
                                 */
    inode_invalid = 1;
    msgprms[0] = "12";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect mode */
  if( inoptr->di_nlink != 1 ) {  /* incorrect # of links */
    inode_invalid = 1;
    msgprms[0] = "13";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect # of links */
  if( !(inoptr->di_dxd.flag & BT_ROOT) ) {  /* not flagged as B+ Tree root */
    inode_invalid = 1;
    msgprms[0] = "14";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end not flagged as B+ Tree root *
  /*
   * If any problems detected so far, don't bother trying to validate
   * the B+ Tree
   */
  if( !inode_invalid )  {  /* self inode looks ok so far */
    vai_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                            AGGREGATE_I, &inorecptr );
    if( (vai_rc == FSCK_OK) && (inorecptr == NULL) ) {
      vai_rc = FSCK_INTERNAL_ERROR_38;
      sprintf( message_parm_0, "%ld", vai_rc );                    /* @F1 */
      msgprms[0] = message_parm_0;                                 /* @F1 */
      msgprmidx[0] = 0;                                                    /* @F1 */
      sprintf( message_parm_1, "%ld", 0 );                           /* @F1 */
      msgprms[1] = message_parm_1;                                  /* @F1 */
      msgprmidx[1] = 0;                                                     /* @F1 */
      sprintf( message_parm_2, "%ld", 0 );                            /* @F1 */
      msgprms[2] = message_parm_2;                                   /* @F1 */
      msgprmidx[2] = 0;                                                      /* @F1 */
      sprintf( message_parm_3, "%ld", 0 );                            /* @F1 */
      msgprms[3] = message_parm_3;                                   /* @F1 */
      msgprmidx[3] = 0;                                                      /* @F1 */
      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                 /* @F1 */
      }
    else if( vai_rc == FSCK_OK ) {  /* no problems so far */
      vai_rc = verify_metadata_data( inoptr, AGGREGATE_I,inorecptr, msg_info_ptr ); 
      if( inorecptr->ignore_alloc_blks || (vai_rc != FSCK_OK) ) {
        inode_invalid = -1;
        }
      }  /* end no problems so far */
    }  /* end self inode looks ok so far */
  if( (!inode_invalid) && (vai_rc == FSCK_OK) ) {
    agg_recptr->inode_stamp = inoptr->di_inostamp;  
    }
 /*
  * wrap it all up for this inode
  */
  if( (inode_invalid ) && (vai_rc == FSCK_OK) )  {
    vai_rc = FSCK_AGGAITINOBAD;
    }
  if( inode_invalid ) {  /* this one's corrupt */
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    if( which_ait == fsck_primary ) {
      fsck_send_msg( fsck_BADMETAINOP, 0, 2 );
      }
    else {
      fsck_send_msg( fsck_BADMETAINOS, 0, 2 );
      }
    }  /* end this one's corrupt */
  return( vai_rc );
}                          /* end of verify_ait_inode ()  */
 	
 	
/*****************************************************************************
 * NAME: verify_ait_part1
 *
 * FUNCTION:  Validate the inodes in "part 1" (inodes 0 through 15) of 
 *            the specified Aggregate Inode Table.
 *
 * PARAMETERS:
 *      which_ait  - input - the Aggregate Inode Table on which to perform
 *                           the function.  { fsck_primary | fsck_secondary }
 *
 * NOTES:  o The caller to this routine must ensure that the
 *           calls made by verify_ait_part1 to inode_get()
 *           will not require device I/O.
 *           That is, the caller must ensure that the aggregate
 *           inode extent containing part1 of the target AIT
 *           resides in the fsck inode buffer before calling
 *           this routine.  (See inode_get() for more info.)
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t verify_ait_part1 ( int which_ait )
{
  retcode_t vaitp1_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  struct fsck_ino_msg_info  ino_msg_info;
  fsck_msg_info_ptr msg_info_ptr;
  fsck_inode_recptr inorecptr;
  inoidx_t ino_idx;
  dinode_t *ino_ptr;
  int aggregate_inode = -1;      /* going for aggregate inodes only */
  int alloc_ifnull = 0;
  msg_info_ptr = &ino_msg_info;
  msg_info_ptr->msg_inopfx = fsck_aggr_inode;
  msg_info_ptr->msg_inotyp = fsck_metadata;
	 
  ino_idx = AGGREGATE_I;               /* try for the self inode */
  intermed_rc = inode_get( aggregate_inode, which_ait, ino_idx, &ino_ptr );
  if( intermed_rc != FSCK_OK )  { /* can't get inode  */
    vaitp1_rc = FSCK_CANTREADSELFINO;
    }  /* end can't get inode  */
  else {  /* else got aggregate inode  */
    msg_info_ptr->msg_inonum = AGGREGATE_I;
    intermed_rc = verify_ait_inode( ino_ptr, which_ait, msg_info_ptr );
    if( intermed_rc == FSCK_OK ) {
      vaitp1_rc = get_inorecptr( aggregate_inode, alloc_ifnull, ino_idx, &inorecptr );
      if( (vaitp1_rc == FSCK_OK) && (inorecptr == NULL) ) {
        vaitp1_rc = FSCK_INTERNAL_ERROR_27;                       /* @F1 */
        msgprms[0] = message_parm_0;                                 /* @F1 */
        msgprmidx[0] = 0;                                                    /* @F1 */
        sprintf( message_parm_1, "%ld", 0 );                           /* @F1 */
        msgprms[1] = message_parm_1;                                  /* @F1 */
        msgprmidx[1] = 0;                                                     /* @F1 */
        sprintf( message_parm_2, "%ld", 0 );                            /* @F1 */
        msgprms[2] = message_parm_2;                                   /* @F1 */
        msgprmidx[2] = 0;                                                      /* @F1 */
        sprintf( message_parm_3, "%ld", 0 );                            /* @F1 */
        msgprms[3] = message_parm_3;                                   /* @F1 */
        msgprmidx[3] = 0;                                                      /* @F1 */
        fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                 /* @F1 */
        }
      else if( (vaitp1_rc == FSCK_OK) && (inorecptr->involved_in_dups) )  {  /*
                            * duplicate allocation(s) detected
                            * while validating the inode
                            */
        vaitp1_rc = unrecord_valid_inode( ino_ptr, AGGREGATE_I, 
                                          inorecptr, msg_info_ptr );
        if( vaitp1_rc == FSCK_OK ) {
          vaitp1_rc = FSCK_DUPMDBLKREF;
          }
        }  /* end duplicate allocation(s) detected ... */
      }
    if( vaitp1_rc == FSCK_OK ) {
      if( intermed_rc != FSCK_OK )  {  /* self inode is bad */
        vaitp1_rc = FSCK_SELFINOBAD;
        }  /* end self inode is bad */
      else {  /* else self inode is good */
        ino_idx = BMAP_I;               /* try for the blockmap inode */
        intermed_rc = inode_get( aggregate_inode, which_ait,
                               ino_idx, &ino_ptr );
        if( intermed_rc != FSCK_OK )  { /* can't get block map inode */
          vaitp1_rc = backout_valid_agg_inode( which_ait, AGGREGATE_I, msg_info_ptr );
          if( vaitp1_rc == FSCK_OK ) {
            vaitp1_rc = FSCK_CANTREADBMINO;   /* not fatal */
            }
          }  /* end can't get block map inode */
        else {  /* else got block map inode */
          msg_info_ptr->msg_inonum = BMAP_I;
          intermed_rc = verify_bmap_inode( ino_ptr, which_ait, msg_info_ptr );
          if( intermed_rc == FSCK_OK ) {
            vaitp1_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                                       BMAP_I, &inorecptr );
            if( (vaitp1_rc == FSCK_OK) && (inorecptr == NULL) ) {
              vaitp1_rc = FSCK_INTERNAL_ERROR_30;
              sprintf( message_parm_0, "%ld", vaitp1_rc );               /* @F1 */
              msgprms[0] = message_parm_0;                                 /* @F1 */
              msgprmidx[0] = 0;                                                    /* @F1 */
              sprintf( message_parm_1, "%ld", 0 );                           /* @F1 */
              msgprms[1] = message_parm_1;                                  /* @F1 */
              msgprmidx[1] = 0;                                                     /* @F1 */
              sprintf( message_parm_2, "%ld", 0 );                            /* @F1 */
              msgprms[2] = message_parm_2;                                   /* @F1 */
              msgprmidx[2] = 0;                                                      /* @F1 */
              sprintf( message_parm_3, "%ld", 0 );                            /* @F1 */
              msgprms[3] = message_parm_3;                                   /* @F1 */
              msgprmidx[3] = 0;                                                      /* @F1 */
              fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                 /* @F1 */
              }
            else if( (vaitp1_rc == FSCK_OK) && (inorecptr->involved_in_dups) ) {  /*
                            * duplicate allocation(s) detected
                            * while validating the inode
                            */
              vaitp1_rc = unrecord_valid_inode( ino_ptr, BMAP_I, 
                                                inorecptr, msg_info_ptr ); 
              if( vaitp1_rc == FSCK_OK ) {
                vaitp1_rc = FSCK_DUPMDBLKREF;
                }
              }  /* end duplicate allocation(s) detected ... */
            }
          if( (intermed_rc != FSCK_OK) && (vaitp1_rc == FSCK_OK) )  {  /* 
                            * block map inode is bad 
                            */
            msg_info_ptr->msg_inonum = AGGREGATE_I;
            vaitp1_rc = backout_valid_agg_inode( which_ait, 
                                                 AGGREGATE_I, 
                                                 msg_info_ptr );
            if( vaitp1_rc == FSCK_OK ) {
              vaitp1_rc = FSCK_BMINOBAD;  /* not fatal */
              }
            }  /* end block map is bad */
          else if( vaitp1_rc == FSCK_OK ) {  /* block map inode is good */
            ino_idx = LOG_I;               /* try for the journal inode */
            intermed_rc = inode_get( aggregate_inode, which_ait,
                                     ino_idx, &ino_ptr );
            if( intermed_rc != FSCK_OK )  { /* can't get journal inode */
              if( vaitp1_rc == FSCK_OK ) {
                msg_info_ptr->msg_inonum = AGGREGATE_I;
                vaitp1_rc = backout_valid_agg_inode( which_ait, AGGREGATE_I, msg_info_ptr );
                if( vaitp1_rc == FSCK_OK ) {
                  msg_info_ptr->msg_inonum = BMAP_I;
                  vaitp1_rc = backout_valid_agg_inode( which_ait, BMAP_I, msg_info_ptr );
                  if( vaitp1_rc == FSCK_OK ) {
                    vaitp1_rc = FSCK_CANTREADLOGINO;  /* not fatal */
                    }
                  }
                }
              }  /* end can't get journal inode */
            else {  /* else got journal inode */
              msg_info_ptr->msg_inonum = LOG_I;
              intermed_rc = verify_log_inode( ino_ptr, which_ait, msg_info_ptr );
              if( intermed_rc == FSCK_OK ) {
                vaitp1_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                                           LOG_I, &inorecptr );
                if( (vaitp1_rc == FSCK_OK) && (inorecptr == NULL) ) {
                  vaitp1_rc = FSCK_INTERNAL_ERROR_31;
                  sprintf( message_parm_0, "%ld", vaitp1_rc );               /* @F1 */
                  msgprms[0] = message_parm_0;                                 /* @F1 */
                  msgprmidx[0] = 0;                                                    /* @F1 */
                  sprintf( message_parm_1, "%ld", 0 );                           /* @F1 */
                  msgprms[1] = message_parm_1;                                  /* @F1 */
                  msgprmidx[1] = 0;                                                     /* @F1 */
                  sprintf( message_parm_2, "%ld", 0 );                            /* @F1 */
                  msgprms[2] = message_parm_2;                                   /* @F1 */
                  msgprmidx[2] = 0;                                                      /* @F1 */
                  sprintf( message_parm_3, "%ld", 0 );                            /* @F1 */
                  msgprms[3] = message_parm_3;                                   /* @F1 */
                  msgprmidx[3] = 0;                                                      /* @F1 */
                  fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                 /* @F1 */
                  }
                else if( (vaitp1_rc == FSCK_OK) && (inorecptr->involved_in_dups) ) {  /*
                            * duplicate allocation(s) detected
                            * while validating the inode
                            */
                  vaitp1_rc = unrecord_valid_inode( ino_ptr, LOG_I, 
                                                    inorecptr, msg_info_ptr );
                  if( vaitp1_rc == FSCK_OK ) {
                    vaitp1_rc = FSCK_DUPMDBLKREF;
                    }
                  }  /* end duplicate allocation(s) detected ... */
                }
              if( (vaitp1_rc == FSCK_OK) && (intermed_rc != FSCK_OK) )  {  /*
                            * journal inode is bad
                            */
                msg_info_ptr->msg_inonum = AGGREGATE_I;
                vaitp1_rc = backout_valid_agg_inode( which_ait, AGGREGATE_I, 
                                                     msg_info_ptr );
                if( vaitp1_rc == FSCK_OK ) {
                  msg_info_ptr->msg_inonum = BMAP_I;
                  vaitp1_rc = backout_valid_agg_inode( which_ait, BMAP_I, 
                                                       msg_info_ptr );
                  }
                if( vaitp1_rc == FSCK_OK ) {
                  vaitp1_rc = FSCK_LOGINOBAD;  /* not fatal */
                  }
                }  /* end journal inode is bad */
              else {  /* journal inode is good */
                ino_idx = BADBLOCK_I;          /* try for the bad block inode */
                intermed_rc = inode_get( aggregate_inode, which_ait,
                                         ino_idx, &ino_ptr );
                if( intermed_rc != FSCK_OK )  { /* can't get bad block inode */
                  if( vaitp1_rc == FSCK_OK ) {
                    msg_info_ptr->msg_inonum = AGGREGATE_I;
                    vaitp1_rc = backout_valid_agg_inode( which_ait, 
                                                         AGGREGATE_I, 
                                                         msg_info_ptr );
                    if( vaitp1_rc == FSCK_OK ) {
                      msg_info_ptr->msg_inonum = BMAP_I;
                      vaitp1_rc = backout_valid_agg_inode( which_ait, 
                                                           BMAP_I, 
                                                           msg_info_ptr );
                      if( vaitp1_rc == FSCK_OK ) {
                        msg_info_ptr->msg_inonum = LOG_I;
                        vaitp1_rc = backout_valid_agg_inode( which_ait, 
                                                             LOG_I, 
                                                             msg_info_ptr );
                        if( vaitp1_rc == FSCK_OK ) {
                          vaitp1_rc = FSCK_CANTREADBBINO;  /* not fatal */
                          }
                        }
                      }
                    }
                  }  /* end can't get bad block inode */
                else {  /* else got bad block inode */
                  msg_info_ptr->msg_inonum = BADBLOCK_I;
                  intermed_rc = verify_badblk_inode( ino_ptr, which_ait, msg_info_ptr );
                  if( intermed_rc == FSCK_OK ) {
                    vaitp1_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                                               BADBLOCK_I, &inorecptr );
                    if( (vaitp1_rc == FSCK_OK) && (inorecptr == NULL) ) {
                      vaitp1_rc = FSCK_INTERNAL_ERROR_56;
                      sprintf( message_parm_0, "%ld", vaitp1_rc );           /* @F1 */
                      msgprms[0] = message_parm_0;                             /* @F1 */
                      msgprmidx[0] = 0;                                                /* @F1 */
                      sprintf( message_parm_1, "%ld", 0 );                       /* @F1 */
                      msgprms[1] = message_parm_1;                              /* @F1 */
                      msgprmidx[1] = 0;                                                 /* @F1 */
                      sprintf( message_parm_2, "%ld", 0 );                        /* @F1 */
                      msgprms[2] = message_parm_2;                               /* @F1 */
                      msgprmidx[2] = 0;                                                  /* @F1 */
                      sprintf( message_parm_3, "%ld", 0 );                        /* @F1 */
                      msgprms[3] = message_parm_3;                               /* @F1 */
                      msgprmidx[3] = 0;                                                  /* @F1 */
                      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );             /* @F1 */
                      }
                    else if( (vaitp1_rc == FSCK_OK) && (inorecptr->involved_in_dups) ) {  /*
                            * duplicate allocation(s) detected
                            * while validating the inode
                            */
                      vaitp1_rc = unrecord_valid_inode( ino_ptr, BADBLOCK_I, 
                                                      inorecptr, msg_info_ptr );
                      if( vaitp1_rc == FSCK_OK ) {
                        vaitp1_rc = FSCK_DUPMDBLKREF;
                        }
                      }  /* end duplicate allocation(s) detected ... */
                    }
                  if( (vaitp1_rc == FSCK_OK) && (intermed_rc != FSCK_OK) )  {  /*
                            * bad block inode is bad
                            */
                    msg_info_ptr->msg_inonum = AGGREGATE_I;
                    vaitp1_rc = backout_valid_agg_inode( which_ait, 
                                                         AGGREGATE_I, 
                                                         msg_info_ptr );
                    if( vaitp1_rc == FSCK_OK ) {
                      msg_info_ptr->msg_inonum = BMAP_I;
                      vaitp1_rc = backout_valid_agg_inode( which_ait, 
                                                           BMAP_I, 
                                                           msg_info_ptr );
                      if( vaitp1_rc == FSCK_OK ) {
                        msg_info_ptr->msg_inonum = LOG_I;
                        vaitp1_rc = backout_valid_agg_inode( which_ait, 
                                                             LOG_I, 
                                                             msg_info_ptr );
                        if( vaitp1_rc == FSCK_OK ) {
                          vaitp1_rc = FSCK_BBINOBAD;  /* not fatal */
                          }
                        }
                      }
                    }  /* end bad block inode is bad */
                  }  /* end else got bad block inode */
                }  /* end else journal inode is good */
              }  /* end else got journal inode */
            }  /* end else block map inode is good */
          }  /* end else got block map inode */
        }  /* end else self inode is good */
      }
    }  /* end else got aggregate inode */
  return( vaitp1_rc );
}                          /* end of verify_ait_part1 ()  */
 	
 	
/*****************************************************************************
 * NAME: verify_ait_part2
 *
 * FUNCTION:  Validate the inodes in "part 2" (inodes 16 through 31) of 
 *            the specified Aggregate Inode Table.
 *
 * PARAMETERS:
 *      which_ait  - input - the Aggregate Inode Table on which to perform
 *                           the function.  { fsck_primary | fsck_secondary }
 *
 * NOTES:  o The caller to this routine must ensure that the
 *           calls made by verify_ait_part2 to inode_get()
 *           will not require device I/O.
 *           That is, the caller must ensure that the aggregate
 *           inode extent containing part1 of the target AIT
 *           resides in the fsck inode buffer before calling
 *           this routine.  (See inode_get() for more info.)
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t verify_ait_part2 ( int which_ait )
{
  retcode_t vaitp2_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  struct fsck_ino_msg_info  ino_msg_info;
  fsck_msg_info_ptr msg_info_ptr;
  fsck_inode_recptr inorecptr;
  inoidx_t ino_idx;
  dinode_t *ino_ptr;
  int aggregate_inode = -1;            /* going for aggregate inodes only */
  int alloc_ifnull = 0;
  msg_info_ptr = &ino_msg_info;
  msg_info_ptr->msg_inopfx = fsck_aggr_inode;
  msg_info_ptr->msg_inotyp = fsck_metadata;
  /*
   * In release 1 there is always exactly 1 fileset, described
   * by aggregate inode FILESYSTEM_I
   */
  ino_idx = FILESYSTEM_I;              /* read the aggregate inode */
  intermed_rc = inode_get( aggregate_inode, which_ait, ino_idx, &ino_ptr );
  if( intermed_rc != FSCK_OK )  { /* can't get the inode */
    vaitp2_rc = FSCK_CANTREADAGGFSINO;
    }  /* end can't get inode  */
  else {  /* else got aggregate inode  */
    msg_info_ptr->msg_inonum = FILESYSTEM_I;
    vaitp2_rc = verify_agg_fileset_inode( ino_ptr, ino_idx, which_ait, msg_info_ptr );
    if( vaitp2_rc == FSCK_OK ) {
      vaitp2_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                                 FILESYSTEM_I, &inorecptr );
      if( (vaitp2_rc == FSCK_OK) && (inorecptr == NULL) ) {
        vaitp2_rc = FSCK_INTERNAL_ERROR_32;
        sprintf( message_parm_0, "%ld", vaitp2_rc );                /* @F1 */
        msgprms[0] = message_parm_0;                                 /* @F1 */
        msgprmidx[0] = 0;                                                    /* @F1 */
        sprintf( message_parm_1, "%ld", 0 );                           /* @F1 */
        msgprms[1] = message_parm_1;                                  /* @F1 */
        msgprmidx[1] = 0;                                                     /* @F1 */
        sprintf( message_parm_2, "%ld", 0 );                            /* @F1 */
        msgprms[2] = message_parm_2;                                   /* @F1 */
        msgprmidx[2] = 0;                                                      /* @F1 */
        sprintf( message_parm_3, "%ld", 0 );                            /* @F1 */
        msgprms[3] = message_parm_3;                                   /* @F1 */
        msgprmidx[3] = 0;                                                      /* @F1 */
        fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                 /* @F1 */
        }
      else if( (vaitp2_rc == FSCK_OK) && (inorecptr->involved_in_dups) ) {  /*
                            * duplicate allocation(s) detected
                            * while validating the inode
                            */
        vaitp2_rc = unrecord_valid_inode( ino_ptr, FILESYSTEM_I, 
                                          inorecptr, msg_info_ptr );
        if( vaitp2_rc == FSCK_OK ) {
          vaitp2_rc = FSCK_DUPMDBLKREF;
          }
        }  /* end duplicate allocation(s) detected ... */
      }
    }  /* end else got aggregate inode  */
  return( vaitp2_rc );
}                          /* end of verify_ait_part2 ()  */
 	
 	
/*****************************************************************************
 * NAME: verify_badblk_inode
 *
 * FUNCTION:  Verify the structures associated with and the content of 
 *            the aggregate bad block inode, aggregate inode 4, whose
 *            data is the collection of bad blocks detected in the
 *            aggregate during mkfs processing.
 *
 * PARAMETERS:
 *      inoptr        - input - pointer to the inode in an fsck buffer
 *      which_ait     - input - the Aggregate Inode Table on which to perform
 *                              the function.  { fsck_primary | fsck_secondary }
 *      msg_info_ptr  - input - pointer to a data area with data needed to
 *                              issue messages about the inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t verify_badblk_inode ( dinode_t         *inoptr,
                                int                which_ait,
                                fsck_msg_info_ptr  msg_info_ptr
                              )
{
  retcode_t vbbi_rc = FSCK_OK;
  retcode_t intermed_rc;
  int inode_invalid = 0;
  int ixpxd_unequal = 0;
  fsck_inode_recptr inorecptr;
  int aggregate_inode = -1;
  int alloc_ifnull = -1;
  if( inoptr->di_inostamp != agg_recptr->inode_stamp ) { /* 
                             * doesn't match the key -- so 
                             * this is either trashed or residual
                             */
    inode_invalid = 1;
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_inotyp;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    fsck_send_msg( fsck_BADINOSTAMP, 0, 3 );
    }  /* end doesn't match the key -- so this is either trashed or residual */
    
  if( inoptr->di_fileset != AGGREGATE_I ) {  /* unexpected fileset # */
    inode_invalid = 1;
    msgprms[0] = "15";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end unexpected fileset # */
  if( inoptr->di_number != BADBLOCK_I ) {  /* unexpected inode # */
    inode_invalid = 1;
    msgprms[0] = "16";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end unexpected inode # */
  if( inoptr->di_gen != 1 ) {  /* incorrect generation # */
    inode_invalid = 1;
    msgprms[0] = "17";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect generation # */
  ixpxd_unequal = memcmp( (void *) &(inoptr->di_ixpxd),
                          (void *) &(agg_recptr->ino_ixpxd),
                          (sizeof(pxd_t)) );      /* returns 0 if equal */
  if( ixpxd_unequal ) {  /* incorrect extent descriptor */
    inode_invalid = 1;
    msgprms[0] = "18";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect extent descriptor */
  if( (inoptr->di_mode & (IFJOURNAL|IFREG|ISPARSE)) != (IFJOURNAL|IFREG|ISPARSE) ) { 
    inode_invalid = 1;
    msgprms[0] = "19";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect mode */
  if( inoptr->di_nlink != 1 ) {  /* incorrect # of links */
    inode_invalid = 1;
    msgprms[0] = "20";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect # of links */
  if( !(inoptr->di_dxd.flag & BT_ROOT) ) {  /* not flagged as B+ Tree root */
    inode_invalid = 1;
    msgprms[0] = "21";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end not flagged as B+ Tree root *
  /*
   * If any problems detected so far, don't bother trying to validate
   * the B+ Tree
   */
  if( !inode_invalid )  {  /* self inode looks ok so far */
    vbbi_rc = get_inorecptr( aggregate_inode, alloc_ifnull, BADBLOCK_I, &inorecptr );
    if( (vbbi_rc == FSCK_OK) && (inorecptr == NULL) ) {
      vbbi_rc = FSCK_INTERNAL_ERROR_57;
      sprintf( message_parm_0, "%ld", vbbi_rc );                  /* @F1 */
      msgprms[0] = message_parm_0;                                 /* @F1 */
      msgprmidx[0] = 0;                                                    /* @F1 */
      sprintf( message_parm_1, "%ld", 0 );                           /* @F1 */
      msgprms[1] = message_parm_1;                                  /* @F1 */
      msgprmidx[1] = 0;                                                     /* @F1 */
      sprintf( message_parm_2, "%ld", 0 );                            /* @F1 */
      msgprms[2] = message_parm_2;                                   /* @F1 */
      msgprmidx[2] = 0;                                                      /* @F1 */
      sprintf( message_parm_3, "%ld", 0 );                            /* @F1 */
      msgprms[3] = message_parm_3;                                   /* @F1 */
      msgprmidx[3] = 0;                                                      /* @F1 */
      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                 /* @F1 */
      }
    else if( vbbi_rc == FSCK_OK ) {  /* no problems so far */
      vbbi_rc = verify_metadata_data( inoptr, BADBLOCK_I, inorecptr, msg_info_ptr ); 
      if( inorecptr->ignore_alloc_blks || (vbbi_rc != FSCK_OK) ) {
        inode_invalid = -1;
        }
      }  /* end no problems so far */
    }  /* end self inode looks ok so far */
 /*
  * wrap it all up for this inode
  */
  if( (inode_invalid ) && (vbbi_rc == FSCK_OK) )  {
    vbbi_rc = FSCK_BMINOBAD;
    }
  if( inode_invalid ) {  /* this one's corrupt */
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    if( which_ait == fsck_primary ) {
      fsck_send_msg( fsck_BADMETAINOP, 0, 2 );
      }
    else {
      fsck_send_msg( fsck_BADMETAINOS, 0, 2 );
      }
    }  /* end this one's corrupt */
  return( vbbi_rc );
}                          /* end of verify_badblk_inode ()  */
 	
 	
/*****************************************************************************
 * NAME: verify_bmap_inode
 *
 * FUNCTION:  Verify the structures associated with and the content of 
 *            the aggregate block map inode, aggregate inode 2, whose
 *            data is the Aggregate Block Allocation Map.
 *
 * PARAMETERS:
 *      inoptr        - input - pointer to the inode in an fsck buffer
 *      which_ait     - input - the Aggregate Inode Table on which to perform
 *                              the function.  { fsck_primary | fsck_secondary }
 *      msg_info_ptr  - input - pointer to a data area with data needed to
 *                              issue messages about the inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t verify_bmap_inode ( dinode_t         *inoptr,
                              int                which_ait,
                              fsck_msg_info_ptr  msg_info_ptr
                            )
{
  retcode_t vbi_rc = FSCK_OK;
  retcode_t intermed_rc;
  int inode_invalid = 0;
  int ixpxd_unequal = 0;
  fsck_inode_recptr inorecptr;
  int aggregate_inode = -1;
  int alloc_ifnull = -1;
  if( inoptr->di_inostamp != agg_recptr->inode_stamp ) { /* 
                             * doesn't match the key -- so 
                             * this is either trashed or residual
                             */
    inode_invalid = 1;
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_inotyp;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    fsck_send_msg( fsck_BADINOSTAMP, 0, 3 );
    }  /* end doesn't match the key -- so this is either trashed or residual */
    
  if( inoptr->di_fileset != AGGREGATE_I ) {  /* unexpected fileset # */
    inode_invalid = 1;
    msgprms[0] = "22";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end unexpected fileset # */
  if( inoptr->di_number != BMAP_I ) {  /* unexpected inode # */
    inode_invalid = 1;
    msgprms[0] = "23";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end unexpected inode # */
  if( inoptr->di_gen != 1 ) {  /* incorrect generation # */
    inode_invalid = 1;
    msgprms[0] = "24";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect generation # */
  ixpxd_unequal = memcmp( (void *) &(inoptr->di_ixpxd),
                          (void *) &(agg_recptr->ino_ixpxd),
                          (sizeof(pxd_t)) );      /* returns 0 if equal */
  if( ixpxd_unequal ) {  /* incorrect extent descriptor */
    inode_invalid = 1;
    msgprms[0] = "25";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect extent descriptor */
  if( (inoptr->di_mode & (IFJOURNAL|IFREG)) != (IFJOURNAL|IFREG) ) { 
    inode_invalid = 1;
    msgprms[0] = "26";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect mode */
  if( inoptr->di_nlink != 1 ) {  /* incorrect # of links */
    inode_invalid = 1;
    msgprms[0] = "27";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect # of links */
  if( !(inoptr->di_dxd.flag & BT_ROOT) ) {  /* not flagged as B+ Tree root */
    inode_invalid = 1;
    msgprms[0] = "28";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end not flagged as B+ Tree root *
  /*
   * If any problems detected so far, don't bother trying to validate
   * the B+ Tree
   */
  if( !inode_invalid )  {  /* self inode looks ok so far */
    vbi_rc = get_inorecptr( aggregate_inode, alloc_ifnull, BMAP_I, &inorecptr );
    if( (vbi_rc == FSCK_OK) && (inorecptr == NULL) ) {
      vbi_rc = FSCK_INTERNAL_ERROR_40;
      sprintf( message_parm_0, "%ld", vbi_rc );                    /* @F1 */
      msgprms[0] = message_parm_0;                                 /* @F1 */
      msgprmidx[0] = 0;                                                    /* @F1 */
      sprintf( message_parm_1, "%ld", 0 );                           /* @F1 */
      msgprms[1] = message_parm_1;                                  /* @F1 */
      msgprmidx[1] = 0;                                                     /* @F1 */
      sprintf( message_parm_2, "%ld", 0 );                            /* @F1 */
      msgprms[2] = message_parm_2;                                   /* @F1 */
      msgprmidx[2] = 0;                                                      /* @F1 */
      sprintf( message_parm_3, "%ld", 0 );                            /* @F1 */
      msgprms[3] = message_parm_3;                                   /* @F1 */
      msgprmidx[3] = 0;                                                      /* @F1 */
      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                 /* @F1 */
      }
    else if( vbi_rc == FSCK_OK ) {  /* no problems so far */
      vbi_rc = verify_metadata_data( inoptr, BMAP_I, inorecptr, msg_info_ptr ); 

      if( inorecptr->ignore_alloc_blks || (vbi_rc != FSCK_OK) ) {
        inode_invalid = -1;
        }
      }  /* end no problems so far */
    }  /* end self inode looks ok so far */
 /*
  * wrap it all up for this inode
  */
  if( (inode_invalid ) && (vbi_rc == FSCK_OK) )  {
    vbi_rc = FSCK_BMINOBAD;
    }
  if( inode_invalid ) {  /* this one's corrupt */
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    if( which_ait == fsck_primary ) {
      fsck_send_msg( fsck_BADMETAINOP, 0, 2 );
      }
    else {
      fsck_send_msg( fsck_BADMETAINOS, 0, 2 );
      }
    }  /* end this one's corrupt */
  return( vbi_rc );
}                          /* end of verify_bmap_inode ()  */
 	
 	
/*****************************************************************************
 * NAME: verify_fs_super_ext
 *
 * FUNCTION:  Verify the structures associated with and the content of 
 *            the fileset super extension inode, fileset inode 1, whose
 *            data is a logical extension of the aggregate fileset inode.
 *
 * PARAMETERS:
 *      inoptr         - input - pointer to the inode in an fsck buffer
 *      msg_info_ptr   - input - pointer to a data area with data needed to
 *                               issue messages about the inode
 *      inode_changed  - input - pointer to a variable in which to return
 *                               !0 if the inode (in the buffer) has been
 *                                  modified by this routine
 *                                0 if the inode (in the buffer) has not been
 *                                  modified by this routine
 *
 * NOTES:  o In release 1 this inode is allocated and initialized but is
 *           not used.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t verify_fs_super_ext ( dinode_t         *inoptr,
                                fsck_msg_info_ptr msg_info_ptr,
                                int              *inode_changed
                               )
{
  retcode_t vfse_rc = FSCK_OK;
  retcode_t intermed_rc;
  int inode_invalid = 0;
  int ixpxd_unequal = 0;
  fsck_inode_recptr inorecptr;
  inoidx_t inoidx = FILESET_EXT_I;
  int aggregate_inode = 0;              /* this is a fileset inode */
  int alloc_ifnull = -1;
  *inode_changed = 0;            /* assume no changes */
  if( inoptr->di_inostamp != agg_recptr->inode_stamp ) { /* 
                             * doesn't match the key -- so 
                             * this is either trashed or residual
                             */
    inode_invalid = 1;
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_inotyp;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    fsck_send_msg( fsck_BADINOSTAMP, 0, 3 );
    }  /* end doesn't match the key -- so this is either trashed or residual */
    
  if( inoptr->di_fileset != FILESYSTEM_I ) {  /* unexpected fileset # */
    inode_invalid = 1;
    msgprms[0] = "29";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end unexpected fileset # */
  if( inoptr->di_number != FILESET_EXT_I ) {  /* unexpected inode # */
    inode_invalid = 1;
    msgprms[0] = "30";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end unexpected inode # */
  if( inoptr->di_gen != 1 ) {  /* incorrect generation # */
    inode_invalid = 1;
    msgprms[0] = "30a";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect generation # */
  ixpxd_unequal = memcmp( (void *) &(inoptr->di_ixpxd),
                          (void *) &(agg_recptr->ino_ixpxd),
                          sizeof(pxd_t) );       /* returns 0 if equal */
  if( ixpxd_unequal ) {  /* incorrect extent descriptor */
    inode_invalid = 1;
    msgprms[0] = "31";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect extent descriptor */
  if( (inoptr->di_mode & (IFJOURNAL|IFREG)) != (IFJOURNAL|IFREG) ) { 
    inode_invalid = 1;
    msgprms[0] = "32";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect mode */
  if( inoptr->di_nlink != 1 ) {  /* incorrect # of links */
    inode_invalid = 1;
    msgprms[0] = "33";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect # of links */
  if( !(inoptr->di_dxd.flag & BT_ROOT) ) {  /* not flagged as B+ Tree root */
    inode_invalid = 1;
    msgprms[0] = "34";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end not flagged as B+ Tree root *
  /*
   * If any problems detected so far, don't bother trying to validate
   * the B+ Tree
   */
  if( !inode_invalid )  {  /* self inode looks ok so far */
    vfse_rc = get_inorecptr( aggregate_inode, alloc_ifnull, inoidx, &inorecptr );
    if( (vfse_rc == FSCK_OK) && (inorecptr == NULL) ) {
      vfse_rc = FSCK_INTERNAL_ERROR_41;
      sprintf( message_parm_0, "%ld", vfse_rc );                 /* @F1 */
      msgprms[0] = message_parm_0;                                 /* @F1 */
      msgprmidx[0] = 0;                                                    /* @F1 */
      sprintf( message_parm_1, "%ld", 0 );                           /* @F1 */
      msgprms[1] = message_parm_1;                                  /* @F1 */
      msgprmidx[1] = 0;                                                     /* @F1 */
      sprintf( message_parm_2, "%ld", 0 );                            /* @F1 */
      msgprms[2] = message_parm_2;                                   /* @F1 */
      msgprmidx[2] = 0;                                                      /* @F1 */
      sprintf( message_parm_3, "%ld", 0 );                            /* @F1 */
      msgprms[3] = message_parm_3;                                   /* @F1 */
      msgprmidx[3] = 0;                                                      /* @F1 */
      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                 /* @F1 */
      }
    else if( vfse_rc == FSCK_OK ) {  /* no problems so far */
      vfse_rc = verify_metadata_data( inoptr, inoidx, inorecptr, msg_info_ptr ); 
      if( inorecptr->ignore_alloc_blks || (vfse_rc != FSCK_OK) ) {
        inode_invalid = -1;
        }
      }  /* end no problems so far */
    }  /* end self inode looks ok so far */
  inode_invalid = 0;     /* at the moment it really doesn't matter
                                * what's in this inode...it's here and
                                * reserved for fileset superinode extension,
                                * but isn't being used and so the data
                                * it contains is irrelevant
                                */
 /*
  * wrap it all up for this inode
  */
  if( (inode_invalid ) && (vfse_rc == FSCK_OK) )  {
    vfse_rc = FSCK_FSETEXTBAD;
    }
  if( inode_invalid ) {  /* this one's corrupt */
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    fsck_send_msg( fsck_BADMETAINOF, 0, 2 );
    }  /* end this one's corrupt */
  return( vfse_rc );
}                              /* end of verify_fs_super_ext
 	
 	
/*****************************************************************************
 * NAME: verify_log_inode
 *
 * FUNCTION:  Verify the structures associated with and the content of 
 *            the aggregate journal inode, aggregate inode 3, whose
 *            data is (or describes) the aggregate's journal, or log,
 *            of transactions.
 *
 * PARAMETERS:
 *      inoptr        - input - pointer to the inode in an fsck buffer
 *      which_ait     - input - the Aggregate Inode Table on which to perform
 *                              the function.  { fsck_primary | fsck_secondary }
 *      msg_info_ptr  - input - pointer to a data area with data needed to
 *                              issue messages about the inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t verify_log_inode ( 	dinode_t          *inoptr,
                             		int                which_ait,
                             		fsck_msg_info_ptr  msg_info_ptr
                           		)
{
  retcode_t vli_rc = FSCK_OK;
  retcode_t intermed_rc;
  int inode_invalid = 0;
  int ixpxd_unequal = 0;
  fsck_inode_recptr inorecptr;
  int aggregate_inode = -1;
  int alloc_ifnull = -1;
  if( inoptr->di_inostamp != agg_recptr->inode_stamp ) { /* 
                             * doesn't match the key -- so 
                             * this is either trashed or residual
                             */
    inode_invalid = 1;
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_inotyp;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    fsck_send_msg( fsck_BADINOSTAMP, 0, 3 );
    }  /* end doesn't match the key -- so this is either trashed or residual */
    
  if( inoptr->di_fileset != AGGREGATE_I ) {  /* unexpected fileset # */
    inode_invalid = 1;
    msgprms[0] = "35";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end unexpected fileset # */
  if( inoptr->di_number != LOG_I ) {  /* unexpected inode # */
    inode_invalid = 1;
    msgprms[0] = "36";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end unexpected inode # */
  if( inoptr->di_gen != 1 ) {  /* incorrect generation # */
    inode_invalid = 1;
    msgprms[0] = "37";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect generation # */
  ixpxd_unequal = memcmp( (void *) &(inoptr->di_ixpxd),
                          (void *) &(agg_recptr->ino_ixpxd),
                          (sizeof(pxd_t)) );      /* returns 0 if equal */
  if( ixpxd_unequal ) {  /* incorrect extent descriptor */
    inode_invalid = 1;
    msgprms[0] = "38";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect extent descriptor */
  if( (inoptr->di_mode & (IFJOURNAL|IFREG)) != (IFJOURNAL|IFREG) ) { 
    inode_invalid = 1;
    msgprms[0] = "39";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect mode */
  if( inoptr->di_nlink != 1 ) {  /* incorrect # of links */
    inode_invalid = 1;
    msgprms[0] = "40";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end incorrect # of links */
  if( !(inoptr->di_dxd.flag & BT_ROOT) ) {  /* not flagged as B+ Tree root */
    inode_invalid = 1;
    msgprms[0] = "41";
    msgprmidx[0] = 0;
    msgprms[1] = message_parm_1;
    msgprmidx[1] = msg_info_ptr->msg_inotyp;
    msgprms[2] = message_parm_2;
    msgprmidx[2] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_BADINOOTHR, 0, 4 );
    }  /* end not flagged as B+ Tree root *
  /*
   * If any problems detected so far, don't bother trying to validate
   * the B+ Tree
   */
  if( !inode_invalid )  {  /* log inode looks ok so far */
    vli_rc = get_inorecptr( aggregate_inode, alloc_ifnull, LOG_I, &inorecptr );
    if( (vli_rc == FSCK_OK) && (inorecptr == NULL) ) {
      vli_rc = FSCK_INTERNAL_ERROR_42;
      sprintf( message_parm_0, "%ld", vli_rc );                    /* @F1 */
      msgprms[0] = message_parm_0;                                 /* @F1 */
      msgprmidx[0] = 0;                                                    /* @F1 */
      sprintf( message_parm_1, "%ld", 0 );                           /* @F1 */
      msgprms[1] = message_parm_1;                                  /* @F1 */
      msgprmidx[1] = 0;                                                     /* @F1 */
      sprintf( message_parm_2, "%ld", 0 );                            /* @F1 */
      msgprms[2] = message_parm_2;                                   /* @F1 */
      msgprmidx[2] = 0;                                                      /* @F1 */
      sprintf( message_parm_3, "%ld", 0 );                            /* @F1 */
      msgprms[3] = message_parm_3;                                   /* @F1 */
      msgprmidx[3] = 0;                                                      /* @F1 */
      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                 /* @F1 */
      }
    else if( vli_rc == FSCK_OK ) {  /* no problems so far */
      vli_rc = verify_metadata_data( 	inoptr, LOG_I,
		                                inorecptr, msg_info_ptr
					);
      if( inorecptr->ignore_alloc_blks || (vli_rc != FSCK_OK) ) {
        inode_invalid = -1;
        }
      }  /* end no problems so far */
    }  /* end log inode looks ok so far */
 /*
  * wrap it all up for this inode
  */
  if( (inode_invalid ) && (vli_rc == FSCK_OK) )  {
    vli_rc = FSCK_LOGINOBAD;
    }
  if( inode_invalid ) {  /* this one's corrupt */
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_inopfx;
    sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    if( which_ait == fsck_primary ) {
      fsck_send_msg( fsck_BADMETAINOP, 0, 2 );
      }
    else {
      fsck_send_msg( fsck_BADMETAINOS, 0, 2 );
      }
    }  /* end this one's corrupt */
	
  return( vli_rc );
}                          /* end of verify_log_inode ()  */
 	
 	
/*****************************************************************************
 * NAME: verify_metadata_data
 *
 * FUNCTION:  Initialize the inode record for and verify the data structures 
 *		allocated to a JFS metadata inode. 
 *
 * PARAMETERS:
 *      inoptr        - input - pointer to the inode in an fsck buffer
 *      inoidx	   - input - inode number of the inode in the buffer
 *      inorecptr   - input - pointer to record allocated to describe the inode
 *      msg_info_ptr  - input - pointer to a data area with data needed to
 *                              issue messages about the inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t verify_metadata_data (	dinode_t  		*inoptr, 
					inoidx_t		inoidx, 
					fsck_inode_recptr 	inorecptr, 
		    			fsck_msg_info_ptr	msg_info_ptr
					)
{
  retcode_t vmd_rc = FSCK_OK;
  int8 bad_size = 0;
  agg_byteidx_t min_size, max_size;

	/*
	 * clear the workspace area for the current inode
	 */
    memset( (void *) (&(agg_recptr->this_inode)), '\0',
	      sizeof( agg_recptr->this_inode ) );
    memcpy( (void *) &(agg_recptr->this_inode.eyecatcher),
	      (void *) "thisinod", 8 );
	/*
	 * initialize the inode record for this inode
	 */
    inorecptr->in_use = 1;
    inorecptr->selected_to_rls = 0;
    inorecptr->crrct_link_count = 0;
    inorecptr->crrct_prnt_inonum = 0;
    inorecptr->adj_entries = 0;
    inorecptr->cant_chkea = 0;
    inorecptr->clr_ea_fld = 0;
    inorecptr->clr_acl_fld = 0;
    inorecptr->crrct_cumm_blks = 0;
                                                                         /* 1 line deleted    @F1 */
    inorecptr->inlineea_on = 0;
    inorecptr->inlineea_off = 0;
    inorecptr->inline_data_err = 0;
    inorecptr->ignore_alloc_blks = 0;
    inorecptr->reconnect = 0;
    inorecptr->unxpctd_prnts = 0;
    if( inoidx == BADBLOCK_I ) {
      inorecptr->badblk_inode = 1;
      }
    else {
      inorecptr->badblk_inode = 0;
      }
    inorecptr->involved_in_dups = 0;
    inorecptr->inode_type = metadata_inode;
    inorecptr->link_count = 0;
    inorecptr->parent_inonum = 0;
    inorecptr->cumm_blocks = 0;                                        /* @F1 */
    inorecptr->dtree_level = -1;                                         /* @F1 */
    inorecptr->ext_rec = NULL;
	/*
	 * verify the B+ Tree and record the blocks it occupies and 
	 * also the blocks it describes.
	 *
	 * (If the tree is corrupt any recorded blocks will be unrecorded 
	 * before control is returned.)
	 */
    vmd_rc = validate_data( inoptr, inoidx, inorecptr, msg_info_ptr ); 
	
    if( (!inorecptr->selected_to_rls) && (!inorecptr->ignore_alloc_blks) ) {  /*
				   * no problems found in the tree yet
				   */
      if( inoptr->di_nblocks != agg_recptr->this_inode.all_blks ) { /*
				   * number of blocks is wrong.  tree must
				   * be bad
				   */
#ifdef _JFS_DEBUG
 printf("bad num blocks: agg ino: %ld(t)  di_nblocks = %lld(t)  this_inode.all_blks = %lld(t)\n\r",
	inoidx, inoptr->di_nblocks, agg_recptr->this_inode.all_blks );
#endif
        inorecptr->selected_to_rls = 1;
        inorecptr->ignore_alloc_blks = 1;
        agg_recptr->corrections_needed = 1;
        bad_size = -1;
        }  /* end number of blocks is wrong */
      else {  /* the data size (in bytes) must not exceed the total size
		* of the blocks allocated for it and must use at least 1 
		* byte in the last fsblock allocated for it.
		*/
        if( agg_recptr->this_inode.data_size == 0 ) {
          min_size = 0;
          max_size = IDATASIZE;
          }
        else {  /* blocks are allocated to data */
          min_size = agg_recptr->this_inode.data_size - sb_ptr->s_bsize + 1;
          max_size = agg_recptr->this_inode.data_size;
          }  /* end else blocks are allocated to data */

        if( (inoptr->di_size > max_size) || (inoptr->di_size < min_size) ) { /*
				   * object size (in bytes) is wrong.
				   * tree must be bad.
				   */
#ifdef _JFS_DEBUG
printf("bad object size: agg ino: %ld(t)  minsize = %lld(t)  maxsize = %lld(t)  di_size = %lld(t)\n\r",
	inoidx, min_size, max_size, inoptr->di_size );
#endif
          inorecptr->selected_to_rls = 1;
          inorecptr->ignore_alloc_blks = 1;
          agg_recptr->corrections_needed = 1;
          bad_size = -1;
          }  /* end object size is wrong.  */
        }  /* end else data size (in bytes) must not exceed the ... */
      }  /* end no problems found in the tree yet */
	/*
	 * If bad_size is set then we didn't know that
	 * the tree was bad until we looked at the size
	 * fields.  This means that the block usage recorded
	 * for this inode has not been backed out yet.
	 */
    if( bad_size ) {  /* tree is bad by implication */
      process_valid_data ( inoptr, inoidx, inorecptr, 
			   msg_info_ptr, FSCK_UNRECORD ); /*

				   * remove traces, in the fsck workspace
				   * maps, of the blocks allocated to data
				   * for this inode, whether a single
				   * extent or a B+ Tree
				   */
      }  /* end tree is bad by implication */

    return( vmd_rc );
}                 		        /* end of verify_metadata_data ()  */
	
 	
/*****************************************************************************
 * NAME: verify_repair_fs_rootdir
 *
 * FUNCTION:  Verify the structures associated with and the content of 
 *            the fileset root directory inode, fileset inode 2, whose
 *            leaves contain the entries in the root directory.  If any
 *            problems are detected then, with the caller's permission,
 *            correct (or reinitialize) the inode.
 *
 * PARAMETERS:
 *      inoptr         - input - pointer to the inode in an fsck buffer
 *      msg_info_ptr   - input - pointer to a data area with data needed to
 *                               issue messages about the inode
 *      inode_changed  - input - pointer to a variable in which to return
 *                               !0 if the inode (in the buffer) has been
 *                                  modified by this routine
 *                                0 if the inode (in the buffer) has not been
 *                                  modified by this routine
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t verify_repair_fs_rootdir ( dinode_t         *inoptr,
                                     fsck_msg_info_ptr msg_info_ptr,
                                     int              *inode_changed
                                    )
{
  retcode_t vrfr_rc = FSCK_OK;
  retcode_t intermed_rc;
  int bad_root_format = 0;
  int bad_root_data_format = 0;
  int ixpxd_unequal = 0;
  int inode_invalid = 0;
  int inode_tree_changed = 0;
  mode_t expected_mode;
  fsck_inode_recptr inorecptr;
  inoidx_t inoidx = ROOT_I;
  char usr_reply;
  DATETIME now_today;
  int aggregate_inode = 0;
  int alloc_ifnull = -1;
  int8 bad_size = 0;
  agg_byteidx_t min_size, max_size;
			
  *inode_changed = 0;            /* assume no changes */
  expected_mode = IDIRECTORY|IFJOURNAL|IFDIR|IREAD|IWRITE|IEXEC;
  ixpxd_unequal = memcmp( (void *) &(inoptr->di_ixpxd),
                          (void *) &(agg_recptr->ino_ixpxd),
                          (sizeof(pxd_t)) );       /* returns 0 if equal */
  vrfr_rc = get_inorecptr( aggregate_inode, alloc_ifnull, ROOT_I, &inorecptr );
  if( vrfr_rc == FSCK_OK ) {  /* got an inode record */
    
    if( (inoptr->di_inostamp != agg_recptr->inode_stamp) || /* bad stamp or */
        (inoptr->di_fileset != FILESYSTEM_I) || /* unexpected fileset # or */
        (inoptr->di_number != ROOT_I) ||   /* unexpected inode # or */
        (inoptr->di_gen != 1) ||   /* unexpected generation # or */
        (ixpxd_unequal)                    /* incorrect extent descriptor */
      )  {  /* inode is not allocated */
#ifdef _JFS_DEBUG
if( (inoptr->di_inostamp != agg_recptr->inode_stamp) ) /* bad stamp or */
 printf("verify_repair_fs_rootdir \tstamp inoptr= %u agg=%u\n\r",inoptr->di_inostamp,agg_recptr->inode_stamp);
if(inoptr->di_fileset != FILESYSTEM_I) /* unexpected fileset # or */
 printf("verify_repair_fs_rootdir \t fileset %u FILESYSTEM %u\n\r",inoptr->di_fileset,FILESYSTEM_I);
if(inoptr->di_number != ROOT_I)    /* unexpected inode # or */
 printf("verify_repair_fs_rootdir \t di_number=%u ROOT_I=%u\n\r",inoptr->di_number,ROOT_I);
if(inoptr->di_gen != 1)    /* unexpected generation # or */
 printf("verify_repair_fs_rootdir \t di_gen=%u\n\r",inoptr->di_gen);
if(ixpxd_unequal)                    /* incorrect extent descriptor */
 printf("verify_repair_fs_rootdir \t ixpxd_unequal=%u\n\r",ixpxd_unequal);
 
#endif
      bad_root_format  = -1;
      fsck_send_msg( fsck_RIUNALLOC, 0, 0 );
      if( agg_recptr->processing_readwrite )  {  /* we can fix this */
         /*
          * If we get this far, the inode is allocated physically,
          * just isn't in use
          */
        memset ( inoptr, 0, sizeof(dinode_t) );
        inoptr->di_inostamp = agg_recptr->inode_stamp;
        inoptr->di_fileset = FILESYSTEM_I;
        inoptr->di_number = ROOT_I;
        inoptr->di_gen = 1;
        memcpy( (void *) &(inoptr->di_ixpxd),
                (void *) &(agg_recptr->ino_ixpxd),
                sizeof(pxd_t) );
        inoptr->di_mode = expected_mode;
        inoptr->di_nlink = 2;   /* one from itself to itself as
                                 * parent to child and one from 
			 * itself to itself as child to parent  
                                 */
        inoptr->di_atime.tv_sec = (uint32) time( NULL );
        inoptr->di_ctime.tv_sec = inoptr->di_atime.tv_sec;
        inoptr->di_mtime.tv_sec = inoptr->di_atime.tv_sec;
        inoptr->di_otime.tv_sec = inoptr->di_atime.tv_sec;
          /*
           * initialize the d-tree
           */
        init_dir_tree( (dtroot_t *) &(inoptr->di_btroot) );
        *inode_changed = 1;
        fsck_send_msg( fsck_ROOTALLOC, 0, 0 );
        }  /* end we can fix this */
      else {  /* don't have write access */ 
        vrfr_rc = FSCK_RIUNALLOC;            /* done with the fileset */
        }  /* end don't have write access */
      }  /* end inode is not allocated */
	
    else {  /* inode is allocated */
      if( (inoptr->di_mode & expected_mode) != expected_mode ) {  /* wrong type */
        bad_root_format  = -1;
        fsck_send_msg( fsck_RINOTDIR, 0, 0 );
        if( agg_recptr->processing_readwrite )  {  /* we can fix this */
            /*
             * If this is the root directory with a bad value
             *      in the mode field,
             * then below we'll find the B+ Tree is a valid directory
             *      tree and keep it.
             *
             * Else if it really was commandeered for something else,
             *
             *    If it was taken for a directory,
             *    then we'll find a valid directory tree and keep it as
             *         the root directory.  /lost+found/ will be created
             *         here and probably get very full.
             *
             *    If it was taken for something else
             *    then we won't find a valid directory tree so we'll
             *         either initialize the tree or else mark the file
             *         system dirty.
             *
             * Else (it was trashed or was never initialized)
             *
             *     we won't find a valid directory tree so we'll either
             *     initialize the tree or else mark the file system dirty.
             */
          inoptr->di_mode = expected_mode;
          *inode_changed = 1;
          fsck_send_msg( fsck_ROOTNOWDIR, 0, 0 );
          }  /* end we can fix this */
        else {  /* don't have write access */
          vrfr_rc = FSCK_RINOTDIR;            /* done with the fileset */
          }  /* end don't have write access */
        }  /* end wrong type */
	
      if( vrfr_rc == FSCK_OK )  {  /* we've corrected every
                        * problem we've seen to this point
                        */
        if( inoptr->di_parent != ROOT_I ) {  /* doesn't link
                        * back to itself correctly
                        */
          bad_root_format  = -1;
          fsck_send_msg( fsck_RIINCINOREF, 0, 0 );
          if( agg_recptr->processing_readwrite )  {  /* we can fix this */
            inoptr->di_parent = ROOT_I;
            *inode_changed = 1;
            fsck_send_msg( fsck_RICRRCTDREF, 0, 0 );
            }  /* end we can fix this */
          else {  /* don't have write access */
            agg_recptr->ag_dirty = 1;
            }  /* end don't have write access */
          }  /* end doesn't link back to itself correctly */
        }  /* end we've corrected every problem we've seen to this point */
	
          /*
           * clear the workspace area for the current inode
           */
          memset( (void *) (&(agg_recptr->this_inode)), '\0',
	          sizeof( agg_recptr->this_inode ) );
          memcpy( (void *) &(agg_recptr->this_inode.eyecatcher),
	          (void *) "thisinod", 8 );
          /*
           * verify the root inode's extended attributes (if any)
           *
           * If a problem is found, the user is notified and EA cleared.
           */
          intermed_rc = validate_EA( inoptr, inoidx, inorecptr, msg_info_ptr );
          if( inorecptr->clr_ea_fld ) {  /* the ea isn't valid */
            clear_EA_field( inorecptr, inoptr );
            *inode_changed = 1;
            }  /* end the ea isn't valid */

          /*
           * verify the root inode's access control list (if any)
           *
           * If a problem is found, the user is notified and ACL cleared.
           */
          intermed_rc = validate_ACL( inoptr, inoidx, inorecptr, msg_info_ptr );
          if( inorecptr->clr_acl_fld ) {  /* the ea isn't valid */
            clear_ACL_field( inorecptr, inoptr );
            *inode_changed = 1;
            }  /* end the acl isn't valid */

      if( vrfr_rc == FSCK_OK )  {  /* still in business */
        if( !(inoptr->di_dxd.flag & BT_ROOT) ) {  /* not a B+ Tree root */
          bad_root_data_format  = -1;
          if( agg_recptr->processing_readwrite )  {  /* we can fix this */
            vrfr_rc = rootdir_tree_bad( inoptr, &inode_tree_changed );
            if( inode_tree_changed ) {
              *inode_changed = 1;
              }
            }  /* end we can fix this */
          else {  /* don't have write access */
            vrfr_rc = FSCK_RIDATAERROR;
            }  /* end don't have write access */
          }  /* end not a B+ Tree root */
        else {  /* else the tree looks ok from here... */
	
          /*
           * check the dtree rooted in the inode
           */
          intermed_rc = validate_dir_data( inoptr, inoidx, inorecptr,
                                         msg_info_ptr ); /*
           		             * verify the B+ Tree and the directory entries
		                        * contained in its leaf nodes.  record the blocks
		                        * it occupies.
		                        */
	
          if( (!inorecptr->selected_to_rls) 	&& 
              (!inorecptr->ignore_alloc_blks) 	&&
              (intermed_rc == FSCK_OK)             	     ) {  /*
				   * no problems found in the tree yet
				   */
            if( inoptr->di_nblocks != agg_recptr->this_inode.all_blks ) { /*
				   * number of blocks is wrong.  tree must
				   * be bad
				   */
#ifdef _JFS_DEBUG
 printf("bad num blocks: fs ino: %ld(t)  di_nblocks = %lld(t)  this_inode.all_blks = %lld(t)\n\r",
	inoidx, inoptr->di_nblocks, agg_recptr->this_inode.all_blks );
#endif
              bad_size = -1;
              }  /* end number of blocks is wrong */
            else {  /* the data size (in bytes) must not exceed the total size
		* of the blocks allocated for it and must use at least 1 
		* byte in the last fsblock allocated for it.
		*/
              if( agg_recptr->this_inode.data_size == 0 ) {
                min_size = 0;
                max_size = IDATASIZE;
                }
              else {  /* blocks are allocated to data */
                min_size = agg_recptr->this_inode.data_size - sb_ptr->s_bsize + 1;
                max_size = agg_recptr->this_inode.data_size;
                }  /* end else blocks are allocated to data */
	
              if( (inoptr->di_size > max_size) || (inoptr->di_size < min_size) ) { /*
				   * object size (in bytes) is wrong.
				   * tree must be bad.
				   */
#ifdef _JFS_DEBUG
printf("bad obj size: fs ino: %ld(t)  minsize = %lld(t)  maxsize = %lld(t)  di_size = %lld(t)\n\r",
	inoidx, min_size, max_size, inoptr->di_size );
#endif
                bad_size = -1;
                }  /* end object size is wrong.  */
              }  /* end else data size (in bytes) must not exceed the ... */
	
	/*
	 * If bad_size is set then we didn't know that
	 * the tree was bad until we looked at the size
	 * fields.  This means that the block usage recorded
	 * for this inode has not been backed out yet.
	 */
            if( bad_size ) {  /* tree is bad by implication */
              process_valid_dir_data ( inoptr, inoidx, inorecptr, 
                                                msg_info_ptr, FSCK_UNRECORD ); /*
				   * remove traces, in the fsck workspace
				   * maps, of the blocks allocated to data
				   * for this inode, whether a single
				   * extent or a B+ Tree
				   */
#ifdef _JFS_DEBUG
printf("bad size:  = %u\n\r",bad_size );
#endif
             bad_root_data_format  = -1;
              if( agg_recptr->processing_readwrite )  {  /* we can fix this */
                vrfr_rc = rootdir_tree_bad( inoptr, &inode_tree_changed );
                if( inode_tree_changed ) {
                  *inode_changed = 1;
                  }
                }  /* end we can fix this */
              else {  /* don't have write access */
                vrfr_rc = FSCK_RIBADTREE;
                }  /* end don't have write access */
              }  /* end tree is bad by implication */
             else {  /* things still look ok */
               intermed_rc = in_inode_data_check( inorecptr, msg_info_ptr );
               if( inorecptr->selected_to_rls ) {  /* nope, it isn't right */
                 inorecptr->selected_to_rls  = 0;
                 process_valid_dir_data ( inoptr, inoidx, inorecptr, 
                                                     msg_info_ptr, FSCK_UNRECORD ); /*
				   * remove traces, in the fsck workspace
				   * maps, of the blocks allocated to data
				   * for this inode, whether a single
				   * extent or a B+ Tree
				   */
                 bad_root_data_format  = -1;
                 if( agg_recptr->processing_readwrite )  {  /* we can fix this */
                   vrfr_rc = rootdir_tree_bad( inoptr, &inode_tree_changed );
                   if( inode_tree_changed ) {
                     *inode_changed = 1;
                     }
                   }  /* end we can fix this */
                 else {  /* don't have write access */
                   vrfr_rc = FSCK_RIBADTREE;
                   }  /* end else don't have write access */
                 }  /* end nope, it isn't right */
               if( inorecptr->clr_ea_fld ) {
                 clear_EA_field( inorecptr, inoptr );
                 *inode_changed = 1;
                 }              
               if( inorecptr->clr_acl_fld ) {
                 clear_ACL_field( inorecptr, inoptr );
                 *inode_changed = 1;
                 }              
               }  /* end else things still look ok */
             }  /* end no problems found in the tree yet */
          else {  /* not a good tree */
            if( vrfr_rc == FSCK_OK )  {  /* but nothing fatal */
#ifdef _JFS_DEBUG
/*+++ PS */
printf("Not good tree\n\r");
#endif
              bad_root_data_format  = -1;
              if( agg_recptr->processing_readwrite )  {  /* we can fix this */
                vrfr_rc = rootdir_tree_bad( inoptr, &inode_tree_changed );
                if( inode_tree_changed ) {
                  *inode_changed = 1;
                  }
                }  /* end we can fix this */
              else {  /* don't have write access */
                vrfr_rc = FSCK_RIBADTREE;
                }  /* end don't have write access */
              }  /* end but nothing fatal */
            }  /* end else not a good tree  */
          }  /* end else the tree looks ok from here */
        }  /* end still in business */
      }  /* end inode is allocated */
    }  /* end got an inode record */
	
    if( bad_root_format ) {
#ifdef _JFS_DEBUG
 printf("bad_root format %u\n\r",bad_root_format);
#endif
      if( agg_recptr->processing_readwrite )  {  /* we have fixed this */
        fsck_send_msg( fsck_WILLFIXRIBADFMT, 0, 0 );
        }  /* end we have fixed this */
      else {  /* no write access */
        fsck_send_msg( fsck_RIBADFMT, 0, 0 );
        if( vrfr_rc != FSCK_OK ) {
          agg_recptr->ag_dirty = 1;
          fsck_send_msg( fsck_CANTCONTINUE, 0, 0 );
          }
        }  /* end no write access */
      }  
    if( bad_root_data_format ) {
#ifdef _JFS_DEBUG
 printf("bad root data format %u\n\r",bad_root_data_format);
#endif
      if( agg_recptr->processing_readwrite )  {  /* we have fixed this */
        fsck_send_msg( fsck_WILLFIXRIBADDATFMT, 0, 0 );
        }  /* end we have fixed this */
      else {  /* no write access */
        fsck_send_msg( fsck_RIBADDATFMT, 0, 0 );
        if( vrfr_rc != FSCK_OK ) {
          agg_recptr->ag_dirty = 1;
          fsck_send_msg( fsck_CANTCONTINUE, 0, 0 );
          }
        }  /* end no write access */
      }
	
  if( vrfr_rc == FSCK_OK ) {  /* no problems or all fixed */
    vrfr_rc = get_inorecptr( aggregate_inode, alloc_ifnull, inoidx, &inorecptr );
   if( (vrfr_rc == FSCK_OK) && (inorecptr == NULL) ) {
      vrfr_rc = FSCK_INTERNAL_ERROR_33;
      sprintf( message_parm_0, "%ld", vrfr_rc );                  /* @F1 */
      msgprms[0] = message_parm_0;                                 /* @F1 */
      msgprmidx[0] = 0;                                                    /* @F1 */
      sprintf( message_parm_1, "%ld", 0 );                           /* @F1 */
      msgprms[1] = message_parm_1;                                  /* @F1 */
      msgprmidx[1] = 0;                                                     /* @F1 */
      sprintf( message_parm_2, "%ld", 0 );                            /* @F1 */
      msgprms[2] = message_parm_2;                                   /* @F1 */
      msgprmidx[2] = 0;                                                      /* @F1 */
      sprintf( message_parm_3, "%ld", 0 );                            /* @F1 */
      msgprms[3] = message_parm_3;                                   /* @F1 */
      msgprmidx[3] = 0;                                                      /* @F1 */
      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                 /* @F1 */
      }
   else if( vrfr_rc == FSCK_OK ) {  /* got a record to describe it */
      inorecptr->in_use = 1;
      inorecptr->inode_type = directory_inode;
      inorecptr->link_count -= inoptr->di_nlink;
      inorecptr->parent_inonum = ROOT_I;
      inorecptr->cumm_blocks = inoptr->di_nblocks;                 /* @F1 */
      inorecptr->dtree_level = 0;                                           /* @F1 */
      inorecptr->ignore_alloc_blks = 0;	/* if there was a problem, it's now solved */
      }  /* end got a record to describe it */
    }  /* end no problems or all fixed */
	
  return( vrfr_rc );
}                              /* end of verify_repair_fs_rootdir ()  */
