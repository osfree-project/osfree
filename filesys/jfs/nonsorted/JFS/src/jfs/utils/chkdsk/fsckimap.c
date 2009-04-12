/* $Id: fsckimap.c,v 1.1.1.1 2003/05/21 13:39:28 pasha Exp $ */

static char *SCCSID = "@(#)1.18  6/28/99 13:49:39 src/jfs/utils/chkdsk/fsckimap.c, jfschk, w45.fs32, fixbld";
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
 *   MODULE_NAME:		fsckimap.c
 *
 *   COMPONENT_NAME: 	jfschk
 *
 *   FUNCTIONS:
 *              AIS_redundancy_check
 *              AIS_replication
 *              first_ref_check_inode_extents
 *              rebuild_agg_iamap
 *              rebuild_fs_iamaps
 *              record_dupchk_inode_extents
 *              verify_agg_iamap
 *              verify_fs_iamaps
 *
 *              agfrext_lists_scan
 *              agfrext_lists_validation
 *              agfrino_lists_scan
 *              agfrino_lists_validation
 *              AIM_check
 *              AIM_replication
 *              AIS_inode_check
 *              AIS_inode_replication
 *              first_refchk_inoexts
 *              FSIM_add_extents
 *              FSIM_check
 *              FSIM_replication
 *              iag_alloc_rebuild
 *              iag_alloc_scan
 *              iag_alloc_ver
 *              iagfr_list_scan
 *              iagfr_list_validation
 *              iags_finish_lists
 *              iags_rebuild
 *              iags_validation
 *              iamap_rebuild
 *              iamap_validation
 *              IM_compare_leaf
 *              record_dupchk_inoexts
 *              record_imap_info
 *              xtAppend
 *              xtSplitPage
 *              xtSplitRoot
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

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 *
 * The following are internal to this file
 *
 */
typedef struct fsck_iag_info {
     iag_t                  *iagptr;
     dinomap_t              *iamctlptr;
     fsck_iamrec_t          *iamrecptr;
     fsck_iag_tbl           *iagtbl;
     fsck_ag_tbl            *agtbl;
     int8                    agg_inotbl;
     fsck_imap_msg_info_ptr  msg_info_ptr;
     } fsck_iag_info_t;
typedef fsck_iag_info_t *fsck_iag_info_ptr;


typedef struct xtree_buf
{
  struct xtree_buf        *down;  /* next rightmost child */
  struct xtree_buf        *up;    /* parent */
  xtpage_t        *page;
} xtree_buf_t;

static xtree_buf_t      *fsim_node_pages;
 
/* --------------------------------------------------------------*/

retcode_t agfrext_lists_scan( int, int, int, fsck_iag_info_ptr,
			      int *, fsck_imap_msg_info_ptr );

retcode_t agfrext_lists_validation( int, int, fsck_ag_tbl *,
				    int *, fsck_imap_msg_info_ptr );

retcode_t agfrino_lists_scan( int, int, int, fsck_iag_info_ptr,
			      int *, fsck_imap_msg_info_ptr );

retcode_t agfrino_lists_validation( int, int, fsck_ag_tbl *,
				    int *, fsck_imap_msg_info_ptr );

retcode_t AIM_check( xtpage_t *, char *, int * );

retcode_t AIM_replication( int8, xtpage_t *, char *, int8 * );

retcode_t AIS_inode_check ( dinode_t *, dinode_t *, int32, int32, int * );

retcode_t AIS_inode_replication ( int8, dinode_t *, dinode_t * );

retcode_t first_refchk_inoexts ( int, int, fsck_inode_recptr, fsck_msg_info_ptr );

retcode_t FSIM_add_extents( xtpage_t *, dinode_t *, int8 * );

retcode_t FSIM_check ( dinode_t *, dinode_t *, int * );

retcode_t FSIM_replication (  int8, dinode_t *, dinode_t *, int8 * );

retcode_t iag_alloc_rebuild( iagno_t, fsck_iag_info_ptr );

retcode_t iag_alloc_scan( int32 *, int32 *, fsck_iag_info_ptr,
			  fsck_imap_msg_info_ptr );

retcode_t iag_alloc_ver( int *, int, iagno_t, fsck_iag_info_ptr,
			 fsck_imap_msg_info_ptr );

retcode_t iagfr_list_scan( int, int, int, fsck_iag_info_ptr, int *,
			   fsck_imap_msg_info_ptr );

retcode_t iagfr_list_validation( int *, fsck_iamrec_t *,
				 fsck_imap_msg_info_ptr );

retcode_t iags_finish_lists( int, int, int, fsck_iag_info_ptr );

retcode_t iags_rebuild( int, int, int,
			fsck_iag_info_ptr, fsck_imap_msg_info_ptr );

retcode_t iags_validation( int, int, int, int *,
			   fsck_iag_info_ptr, fsck_imap_msg_info_ptr );

retcode_t iamap_rebuild( int, int, int, fsck_iag_info_ptr,
			 fsck_imap_msg_info_ptr );

retcode_t iamap_validation( int, int, int, fsck_iag_info_ptr,
			    fsck_imap_msg_info_ptr );

retcode_t IM_compare_leaf ( xtpage_t *, xtpage_t *, int * );

retcode_t record_dupchk_inoexts ( int, int, int32 *,
				  fsck_inode_recptr, fsck_msg_info_ptr );

retcode_t record_imap_info( void );

retcode_t xtAppend( dinode_t *, int64, int64, int32, xtree_buf_t * );

retcode_t xtSplitPage( dinode_t *, xtree_buf_t  *, int64, int32, int64 );

retcode_t xtSplitRoot( dinode_t *, xtree_buf_t *, int64, int32, int64 );

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV


/****************************************************************
 * NAME: agfrext_lists_scan
 *
 * FUNCTION:  Scan the Allocation Group free extent lists for the given
 *            inode allocation table.  Count the number of iags on each
 *            list.  Validate the list structure.
 *
 * PARAMETERS:
 *     is_aggregate     - input - !0 => aggregate owned
 *                                 0 => fileset owned 
 *     which_it         - input - ordinal number of the aggregate inode 
 *                                describing the inode table
 *     which_ait        - input - the aggregate inode table from which
 *                                the it inode should be read
 *                                { fsck_primary | fsck_secondary }
 *     iagiptr          - input - pointer to a data area describing the
 *                                current iag
 *     errors_detected  - input - pointer to a variable in which to return
 *                                !0 if errors are detected 
 *                                 0 if no errors are detected
 *     msg_info_ptr     - input - pointer to data needed to issue messages
 *                                about the current inode allocation map
 *
 * NOTES:  This routine does NOT attempt to determine whether the iags
 *         on the list belong on the list.  It only verifies that the
 *         list is structurally correct, i.e., that the forward and
 *         backward pointers are consistent.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t agfrext_lists_scan( int                    is_aggregate,
			      int                    which_it,
			      int                    which_ait,
			      fsck_iag_info_ptr      iagiptr,
			      int                   *errors_detected,
			      fsck_imap_msg_info_ptr msg_info_ptr
			     )
{
  retcode_t ifels_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  reg_idx_t      agidx;
  iagctl_t      *agg_agrec;
  fsck_ag_recptr wsp_agrec;
  int  end_of_list = -1;
  iagno_t        this_iagno, prev_iagno;

  for( agidx = 0;
       ( (ifels_rc == FSCK_OK) && (agidx < MAXAG) );
       agidx++ ) {

    agg_agrec = &(iagiptr->iamctlptr->in_agctl[agidx]);
    wsp_agrec = &(iagiptr->agtbl[agidx]);

    wsp_agrec->frext_list_last = end_of_list;   /* in case list is empty */
    wsp_agrec->frext_list_first = agg_agrec->extfree;
    wsp_agrec->frext_list_len = 0;
    prev_iagno = end_of_list;
    this_iagno = wsp_agrec->frext_list_first;

    while( (this_iagno != end_of_list)  &&
	   (!wsp_agrec->frext_list_bad) &&
	   (ifels_rc == FSCK_OK)    ) {
      intermed_rc = iag_get( is_aggregate, which_it, which_ait,
			     this_iagno, &(iagiptr->iagptr) );
	   /*
	    * we consider an error here to be an error in the chain.  If
	    * it's really something more serious it will come up again
	    * when we go through all allocated iag's sequentially.
	    */
      if( intermed_rc != FSCK_OK ) {
	wsp_agrec->frext_list_bad = -1;
	}
      else {  /* got the iag */
	if( iagiptr->iagptr->extfreeback != prev_iagno ) {  /* bad back chain */
	  wsp_agrec->frext_list_bad = -1;
	  }  /* end bad back chain */
	else {  /* back chain is correct */
	  prev_iagno = this_iagno;
	  this_iagno = iagiptr->iagptr->extfreefwd;
	  wsp_agrec->frext_list_len++;  /* increment the counter */
	  }  /* end else back chain is correct */
	}  /* end else got the iag */
      }  /* end while not end of list and no errors detected */

    if( wsp_agrec->frext_list_bad ) {  /* found a problem */
      agg_recptr->ag_dirty = 1;
      *errors_detected = -1;
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_mapowner;
      sprintf( message_parm_1, "%d", agidx );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      msgprms[2] = "1";
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_BADAGFELIST, 0, 3 );
      }  /* end found a problem */
    else {
      wsp_agrec->frext_list_last = prev_iagno;
      }
    }  /* end for agidx */

  return( ifels_rc );
}                              /* end of agfrext_lists_scan ()  */


/****************************************************************
 * NAME: agfrext_lists_validation
 *
 * FUNCTION:  Compare the results of the Allocation Group free extent lists
 *            scan with the results of validating the iags.  If the number
 *            of iags seen on a list during the list scan does not equal 
 *            the number of iags which appear to be on the list (i.e., which
 *            have non-initialized values for forward and back pointers) 
 *            as seen during iag validation, then the list is not structurally 
 *            consistent.
 *
 * PARAMETERS:
 *     is_aggregate     - input - !0 => aggregate owned
 *                                 0 => fileset owned
 *     which_it         - input - ordinal number of the aggregate inode 
 *                                describing the inode table
 *     agtbl            - input - pointer to the fsck workspace allocation
 *                                group table for the specified inode
 *                                table
 *     errors_detected  - input - pointer to a variable in which to return
 *                                !0 if errors are detected 
 *                                 0 if no errors are detected
 *     msg_info_ptr     - input - pointer to data needed to issue messages
 *                                about the current inode allocation map
 *
 * NOTES: o This routine is only called in the read-only path.
 *
 *        o This routine is NOT called if any structural errors have
 *          already been detected in the list.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t agfrext_lists_validation( int                    is_aggregate,
				    int                    which_it,
				    fsck_ag_tbl           *agtbl,
				    int                   *errors_detected,
				    fsck_imap_msg_info_ptr msg_info_ptr
				   )
{
  retcode_t ifelv_rc = FSCK_OK;
  reg_idx_t      agidx;
  fsck_ag_recptr wsp_agrec;

  for( agidx = 0;
       ( (ifelv_rc == FSCK_OK) && (agidx < MAXAG) );
       agidx++ ) {

    wsp_agrec = &(agtbl[agidx]);

    if( !wsp_agrec->frino_list_bad ) {  /* the list itself looked ok */

      if( wsp_agrec->frext_list_len > 0 ) { /*
		   * fsck observed fewer iag's which belong on this list
		   * than it counted when it scanned the list.
		   * (fsck has already issued messages about these iag's)
		   */
	*errors_detected = -1;
	agg_recptr->ag_dirty = 1;
	}  /* end fsck observed fewer iag's which belong ... */

      else if( wsp_agrec->frext_list_len < 0 ) { /*
		   * fsck observed more iag's which belong on this list
		   * and which appear to be on the list than it counted
		   * when it scanned the list.  So the chain has somehow
		   * lost some of its links.
		   */
	*errors_detected = -1;
	agg_recptr->ag_dirty = 1;

        msgprms[0] = message_parm_0;
        msgprmidx[0] = msg_info_ptr->msg_mapowner;
        sprintf( message_parm_1, "%d", agidx );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        fsck_send_msg( fsck_BADAGFELIST1, 0, 2 );
	}  /* end fsck observed more iag's which belong ... */
      }  /* end the list itself looked ok */
    }  /* end for agidx */

  return( ifelv_rc );
}                              /* end of agfrext_lists_validation ()  */


/****************************************************************
 * NAME: agfrino_lists_scan
 *
 * FUNCTION:  Scan the Allocation Group free inode lists for the given
 *            inode allocation table.  Count the number of iags on each
 *            list.  Validate the list structure.
 *
 * PARAMETERS:
 *     is_aggregate     - input - !0 => aggregate owned
 *                                 0 => fileset owned
 *     which_it         - input - ordinal number of the aggregate inode 
 *                                describing the inode table
 *     which_ait        - input - the aggregate inode table from which
 *                                the it inode should be read
 *                                { fsck_primary | fsck_secondary }
 *     iagiptr          - input - pointer to a data area describing the
 *                                current iag
 *     errors_detected  - input - pointer to a variable in which to return
 *                                !0 if errors are detected 
 *                                 0 if no errors are detected
 *     msg_info_ptr     - input - pointer to data needed to issue messages
 *                                about the current inode allocation map
 *
 * NOTES:  This routine does NOT attempt to determine whether the iags
 *         on the list belong on the list.  It only verifies that the
 *         list is structurally correct, i.e., that the forward and
 *         backward pointers are consistent.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t agfrino_lists_scan( int                    is_aggregate,
			      int                    which_it,
			      int                    which_ait,
			      fsck_iag_info_ptr      iagiptr,
			      int                   *errors_detected,
			      fsck_imap_msg_info_ptr msg_info_ptr
			     )
{
  retcode_t ifils_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  reg_idx_t      agidx;
  iagctl_t      *agg_agrec;
  fsck_ag_recptr wsp_agrec;
  int  end_of_list = -1;
  iagno_t        this_iagno, prev_iagno;

  for( agidx = 0;
       ( (ifils_rc == FSCK_OK) && (agidx < MAXAG) );
       agidx++ ) {

    agg_agrec = &(iagiptr->iamctlptr->in_agctl[agidx]);
    wsp_agrec = &(iagiptr->agtbl[agidx]);

    wsp_agrec->frino_list_last = end_of_list;   /* in case list is empty */
    wsp_agrec->frino_list_first = agg_agrec->inofree;
    wsp_agrec->frino_list_len = 0;
    prev_iagno = end_of_list;
    this_iagno = wsp_agrec->frino_list_first;

    while( (this_iagno != end_of_list)  &&
	   (!wsp_agrec->frino_list_bad) &&
	   (ifils_rc == FSCK_OK)    ) {

      intermed_rc = iag_get( is_aggregate, which_it, which_ait,
			     this_iagno, &(iagiptr->iagptr) );
	   /*
	    * we consider an error here to be an error in the chain.  If
	    * it's really something more serious it will come up again
	    * when we go through all allocated iag's sequentially.
	    */
      if( intermed_rc != FSCK_OK ) {
	wsp_agrec->frino_list_bad = -1;
	}
      else {  /* got the iag */
	if( iagiptr->iagptr->inofreeback != prev_iagno ) {  /* bad back chain */
	  wsp_agrec->frino_list_bad = -1;
	  }  /* end bad back chain */
	else {  /* back chain is correct */
	  prev_iagno = this_iagno;
	  this_iagno = iagiptr->iagptr->inofreefwd;
	  wsp_agrec->frino_list_len++;  /* increment the counter */
	  }  /* end else back chain is correct */
	}  /* end else got the iag */
      }  /* end free while not end of list and no errors detected */

    if( wsp_agrec->frino_list_bad ) {  /* found a problem */
      agg_recptr->ag_dirty = 1;
      *errors_detected = -1;

      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_mapowner;
      sprintf( message_parm_1, "%d", agidx );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      msgprms[2] = "1";
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_BADAGFILIST, 0, 3 );
      }  /* end found a problem */
    else {
      wsp_agrec->frino_list_last = prev_iagno;
      }
    }  /* end for agidx */

  return( ifils_rc );
}                              /* end of agfrino_lists_scan ()  */


/****************************************************************
 * NAME: agfrino_lists_validation
 *
 * FUNCTION:  Compare the results of the Allocation Group free inode lists
 *            scan with the results of validating the iags.  If the number
 *            of iags seen on a list during the list scan does not equal 
 *            the number of iags which appear to be on the list (i.e., which
 *            have non-initialized values for forward and back pointers) 
 *            as seen during iag validation, then the list is not structurally 
 *            consistent.
 *
 * PARAMETERS:
 *     is_aggregate     - input - !0 => aggregate owned
 *                                 0 => fileset owned
 *     which_it         - input - ordinal number of the aggregate inode 
 *                                describing the inode table
 *     agtbl            - input - pointer to the fsck workspace allocation
 *                                group table for the specified inode
 *                                table
 *     errors_detected  - input - pointer to a variable in which to return
 *                                !0 if errors are detected 
 *                                 0 if no errors are detected
 *     msg_info_ptr     - input - pointer to data needed to issue messages
 *                                about the current inode allocation map
 *
 * NOTES: o This routine is only called in the read-only path.
 *
 *        o This routine is NOT called if any structural errors have
 *          already been detected in the list.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t agfrino_lists_validation( int                    is_aggregate,
				    int                    which_it,
				    fsck_ag_tbl           *agtbl,
				    int                   *errors_detected,
				    fsck_imap_msg_info_ptr msg_info_ptr
				   )
{
  retcode_t ifilv_rc = FSCK_OK;
  reg_idx_t      agidx;
  fsck_ag_recptr wsp_agrec;

  for( agidx = 0;
       ( (ifilv_rc == FSCK_OK) && (agidx < MAXAG) );
       agidx++ ) {

    wsp_agrec = &(agtbl[agidx]);

    if( !wsp_agrec->frino_list_bad ) {  /* the list itself looked ok */

      if( wsp_agrec->frino_list_len > 0 ) { /*
		   * fsck observed fewer iag's which belong on this list
		   * than it counted when it scanned the list.
		   * (fsck has already issued messages about these iag's)
		    */
	*errors_detected = -1;
	agg_recptr->ag_dirty = 1;
	}  /* end fsck observed fewer iag's which belong ... */

      else if( wsp_agrec->frino_list_len < 0 ) { /*
		   * fsck observed more iag's which belong on this list
		   * and which appear to be on the list than it counted
		   * when it scanned the list.  So the chain has somehow
		   * lost some of its links.
		   */
	*errors_detected = -1;
	agg_recptr->ag_dirty = 1;

        msgprms[0] = message_parm_0;
        msgprmidx[0] = msg_info_ptr->msg_mapowner;
        sprintf( message_parm_1, "%d", agidx );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        fsck_send_msg( fsck_BADAGFILIST1, 0, 2 );
	}  /* end fsck observed more iag's which belong ... */
      }  /* end the list itself looked ok */
    }  /* end for agidx */

  return( ifilv_rc );
}                              /* end of agfrino_lists_validation ()  */


/****************************************************************
 * NAME: AIM_check
 *
 * FUNCTION:  Verify that the Secondary Aggregate Inode Map is correct
 *            and that it is a logical equivalent to the Primary Aggregate
 *            Inode Map.
 *
 * PARAMETERS:
 *     secondary_btroot        - input - pointer to the root of the B+ Tree
 *                                       root in of the secondary aggregate
 *                                       inode table inode in an fsck buffer.
 *     temp_bufptr             - input - pointer to a temporary I/O buffer,
 *                                       large enough for an inode extent.
 *     inconsistency_detected  - input - pointer to a variable in which 
 *                                       to return !0 if errors are detected 
 *                                                  0 if no errors are detected
 *
 *
 * NOTES:  o This routine reads the Primary Aggregate Inode Map into the
 *           the first page of the fsck temporary inode buffer and reads
 *           the Secondary Aggregate Inode Map into the 3rd page of the
 *           fsck temporary inode buffer.
 *
 *         o In release 1, there is always exactly 1 iag in the Aggregate
 *           Inode Map and so the inode which represents the map always has
 *           a root-leaf B+ tree rooted in it.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t AIM_check ( xtpage_t *secondary_btroot,
		      char     *temp_bufptr,
		      int      *inconsistency_detected
		    )
{
  retcode_t aimc_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  char  *primary_bufptr, *secondary_bufptr;
  dinomap_t *primary_ctlptr, *secondary_ctlptr;
  iag_t *primary_iagptr, *secondary_iagptr;
  uint8 expected_flag;
  fsblkidx_t sb_aim2_addr, btrt_aim2_addr;
  reg_idx_t sb_aim2_length, btrt_aim2_length;
  agg_byteidx_t byte_offset;
  uint32 bytes_wanted, bytes_read;

  expected_flag = (BT_ROOT | BT_LEAF | DXD_INDEX);
  sb_aim2_addr = addressPXD( &(sb_ptr->s_aim2) );
  sb_aim2_length = lengthPXD( &(sb_ptr->s_aim2) );
  btrt_aim2_addr = addressXAD( &(secondary_btroot->xad[XTENTRYSTART]) );
  btrt_aim2_length = lengthXAD( &(secondary_btroot->xad[XTENTRYSTART]) );

  *inconsistency_detected = (
	    (secondary_btroot->header.flag != expected_flag)           ||
	    (secondary_btroot->header.nextindex != (XTENTRYSTART + 1)) ||
	    (sb_aim2_addr != btrt_aim2_addr)                         ||
	    (sb_aim2_length != btrt_aim2_length)
			    );

  if( !(*inconsistency_detected) ) {  /* descriptors match */
    primary_bufptr = temp_bufptr;
    secondary_bufptr = (char *) (((int32) temp_bufptr) + (2 * BYTESPERPAGE));

    byte_offset = AIMAP_OFF;
    bytes_wanted = sb_aim2_length * sb_ptr->s_bsize;
    intermed_rc = readwrite_device( byte_offset, bytes_wanted, &bytes_read,
				    (void *) primary_bufptr, fsck_READ );
    if( (intermed_rc != FSCK_OK) || (bytes_wanted != bytes_read) ) {
      *inconsistency_detected = -1;
      }
    else {  /* primary AIM is in the buffer */
      byte_offset = sb_aim2_addr * sb_ptr->s_bsize;
      bytes_wanted = sb_aim2_length * sb_ptr->s_bsize;
      intermed_rc = readwrite_device( byte_offset, bytes_wanted, &bytes_read,
				      (void *) secondary_bufptr, fsck_READ );
      if( (intermed_rc != FSCK_OK) || (bytes_wanted != bytes_read) ) {
	*inconsistency_detected = -1;
	}
      }  /* end else primary AIM is in the buffer */
    }  /* end descriptors match */

  if( (aimc_rc == FSCK_OK) && (!(*inconsistency_detected)) ) {  /*
				 * the AIMaps are in the temp buffer
				 */
    primary_ctlptr = (dinomap_t *) primary_bufptr;
    secondary_ctlptr = (dinomap_t *) secondary_bufptr;

    *inconsistency_detected = (
	   (primary_ctlptr->in_freeiag != secondary_ctlptr->in_freeiag) ||
	   (primary_ctlptr->in_nextiag != secondary_ctlptr->in_nextiag) ||
	   (primary_ctlptr->in_numinos != secondary_ctlptr->in_numinos) ||
	   (primary_ctlptr->in_numfree != secondary_ctlptr->in_numfree) ||
	   (primary_ctlptr->in_nbperiext != secondary_ctlptr->in_nbperiext) ||
	   (primary_ctlptr->in_l2nbperiext != secondary_ctlptr->in_l2nbperiext)
			      );
    if( !(*inconsistency_detected) ) {
      *inconsistency_detected =
			     memcmp( (void *) &(primary_ctlptr->in_agctl[0]),
				     (void *) &(secondary_ctlptr->in_agctl[0]),
				     (sizeof(iagctl_t) * MAXAG) );
      }
    }  /* end the AIMaps are in the temp buffer */

  if( (aimc_rc == FSCK_OK) && (!(*inconsistency_detected)) ) {  /*
				 * the AIMaps are in the temp buffer and
				 * the control pages are consistent
				 */
    primary_iagptr = (iag_t *) (((int32) primary_bufptr) + BYTESPERPAGE);
    secondary_iagptr = (iag_t *) (((int32) secondary_bufptr) + BYTESPERPAGE);

    *inconsistency_detected = (
	   (primary_iagptr->agstart != secondary_iagptr->agstart) ||
	   (primary_iagptr->iagnum != secondary_iagptr->iagnum) ||
	   (primary_iagptr->inofreefwd != secondary_iagptr->inofreefwd) ||
	   (primary_iagptr->inofreeback != secondary_iagptr->inofreeback) ||
	   (primary_iagptr->extfreefwd != secondary_iagptr->extfreefwd) ||
	   (primary_iagptr->extfreeback != secondary_iagptr->extfreeback) ||
	   (primary_iagptr->iagfree != secondary_iagptr->iagfree) ||
	   (primary_iagptr->nfreeinos != secondary_iagptr->nfreeinos) ||
	   (primary_iagptr->nfreeexts != secondary_iagptr->nfreeexts)
			      );
    if( !(*inconsistency_detected) ) {
      *inconsistency_detected =
			     memcmp( (void *) &(primary_iagptr->inosmap[0]),
				     (void *) &(secondary_iagptr->inosmap[0]),
				     (sizeof(int32) * SMAPSZ) );
      }
    if( !(*inconsistency_detected) ) {
      *inconsistency_detected =
			     memcmp( (void *) &(primary_iagptr->extsmap[0]),
				     (void *) &(secondary_iagptr->extsmap[0]),
				     (sizeof(int32) * SMAPSZ) );
      }
    if( !(*inconsistency_detected) ) {
      *inconsistency_detected =
			     memcmp( (void *) &(primary_iagptr->pmap[0]),
				     (void *) &(secondary_iagptr->pmap[0]),
				     (sizeof(int32) * EXTSPERIAG) );
      }
    if( !(*inconsistency_detected) ) {
      *inconsistency_detected =
			     memcmp( (void *) &(secondary_iagptr->inoext[0]),
				     (void *) &(sb_ptr->s_ait2),
				     sizeof(pxd_t) );
      }
    }  /* end the AIMaps are in the temp buffer and the control pages ... */

  if( (aimc_rc == FSCK_OK) && (*inconsistency_detected) ) {  /*
				 * future recover capability is compromised
				 */
      /*
       * Note that we're in read-only mode or we wouldn't be checking this
       * (because when we have write access we always rebuild it)
       */
    msgprms[0] = "1";
    msgprmidx[0] = 0;
    fsck_send_msg( fsck_INCONSIST2NDRY, 0, 1 );
    }  /* end future recover capability is compromised */

  return( aimc_rc );
}                          /* end of AIM_check ()  */


/****************************************************************
 * NAME: AIM_replication
 *
 * FUNCTION:  Rebuild specified target Aggregate Inode Map so that it is
 *            a logical equivalent to the specified source Aggregate Inode
 *            Map.
 *
 * PARAMETERS:
 *     source_is_primary    - input - !0 => replicate from primary to secondary
 *                                     0 => replicate from secondary to primary
 *     target_btroot        - input - pointer to the B+ Tree root in the 
 *                                    aggregate inode allocation table (either
 *                                    primary or secondary) inode which is
 *                                    the target of the replication.
 *     temp_bufptr          - input - pointer to a temporary I/O buffer,
 *                                    large enough for an inode extent.
 *     replication_failure  - input - pointer to a variable in which to return
 *                                   !0 if errors occurred during replication
 *                                    0 if the replication was successful
 *
 * NOTES:  o This routine considers the buffer for the Primary Aggregate
 *           Inode Map to begin at the beginning of the fsck temporary
 *           inode buffer.  This routine considers the buffer for the
 *           Secondary Aggregate Inode Map to begin at the 3rd page
 *           of the fsck temporary inode buffer.
 *
 *         o This routine reads the source Aggregate Inode Map (either 
 *           primary or secondary) into its buffer (see note above) and
 *           rebuilds the target Aggregate Inode Map (either secondary
 *           or primary) in its buffer.
 *
 *         o In release 1, there is always exactly 1 iag in the Aggregate
 *           Inode Map and so the inode which represents the map always has
 *           a root-leaf B+ tree rooted in it.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t AIM_replication ( int8      source_is_primary,
			    xtpage_t *target_btroot,
			    char     *temp_bufptr,
			    int8      *replication_failure
			  )
{
  retcode_t aimr_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  char  *target_bufptr, *source_bufptr;
  iag_t *target_iagptr;
  agg_byteidx_t source_byte_offset, target_byte_offset;
  uint32 bytes_wanted, bytes_transferred;

  target_btroot->header.flag = (BT_ROOT | BT_LEAF | DXD_INDEX);
  target_btroot->header.nextindex = XTENTRYSTART + 1;

  source_bufptr = temp_bufptr;
  target_bufptr = (char *) (((int32) temp_bufptr) + (2 * BYTESPERPAGE));
  target_iagptr = (iag_t *) (((int32) target_bufptr) + BYTESPERPAGE);

  if( source_is_primary ) {
    source_byte_offset = AIMAP_OFF;
    target_byte_offset = addressPXD(&(sb_ptr->s_aim2)) * sb_ptr->s_bsize;

    XADaddress( &(target_btroot->xad[XTENTRYSTART]),
		addressPXD( &(sb_ptr->s_aim2) ) );
    XADlength( &(target_btroot->xad[XTENTRYSTART]),
	       lengthPXD( &(sb_ptr->s_aim2) ) );
    }
  else {
    source_byte_offset = addressPXD(&(sb_ptr->s_aim2)) * sb_ptr->s_bsize;
    target_byte_offset = AIMAP_OFF;

    XADaddress( &(target_btroot->xad[XTENTRYSTART]), (AIMAP_OFF / sb_ptr->s_bsize) );
    XADlength( &(target_btroot->xad[XTENTRYSTART]), lengthPXD(&sb_ptr->s_aim2) );
    }

  bytes_wanted = lengthPXD(&(sb_ptr->s_aim2)) * sb_ptr->s_bsize;

  intermed_rc = readwrite_device( source_byte_offset,
				  bytes_wanted, &bytes_transferred,
				  (void *) source_bufptr, fsck_READ );
  if( (intermed_rc != FSCK_OK) || (bytes_wanted != bytes_transferred) ) {
    aimr_rc = FSCK_FAILED_CANTREADAIMNOW;
         /*
          * message to the user 
          */
    msgprms[0] = message_parm_0;
    msgprmidx[0] = fsck_metadata;
    msgprms[1] = Vol_Label;
    msgprmidx[1] = 0;
    fsck_send_msg( fsck_URCVREAD, 0, 2 );
         /*
          * message to the debugger
          */
    sprintf( message_parm_0, "%d", FSCK_FAILED_CANTREADAIMNOW );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    sprintf( message_parm_1, "%d", intermed_rc );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    sprintf( message_parm_2, "%d", fsck_READ );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    sprintf( message_parm_3, "%lld", source_byte_offset );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    sprintf( message_parm_4, "%ld", bytes_wanted );
    msgprms[4] = message_parm_4;
    msgprmidx[4] = 0;
    sprintf( message_parm_5, "%ld", bytes_transferred );
    msgprms[5] = message_parm_5;
    msgprmidx[5] = 0;
    fsck_send_msg( fsck_ERRONAGG, 0, 6 );
    }

  if( aimr_rc == FSCK_OK ) {  /* the source AIMap is in the temp buffer */

     /*
      * copy the source over the target, then adjust the field(s) which
      * should be different.
      */

    memcpy( (void *) target_bufptr, (void *) source_bufptr, bytes_wanted );

    memcpy( (void *) &(target_iagptr->inoext[0]),
	    (void *) &(sb_ptr->s_ait2),
	    sizeof(pxd_t) );

    if( !source_is_primary ) {
      XADaddress( &(target_iagptr->inoext[0]), (AITBL_OFF / sb_ptr->s_bsize) );
      }

     /*
      * now write the redundant AIMap to the device
      */
    aimr_rc = readwrite_device( target_byte_offset,
				bytes_wanted, &bytes_transferred,
				(void *) target_bufptr, fsck_WRITE );
    agg_recptr->ag_modified = 1;
    if( (intermed_rc != FSCK_OK) || (bytes_wanted != bytes_transferred) ) {  /*
				 * some or all didn't make it to the device.
				 */
      agg_recptr->ait_aim_update_failed = 1;
      if( !source_is_primary ) {  /* we're trying to repair the primary
				 * table and can't -- that makes this a
				 * dirty file system.
				 */
	agg_recptr->ag_dirty = 1;
        msgprms[0] = "1";
        msgprmidx[0] = 0;
        fsck_send_msg( fsck_CANTREPAIRAIS, 0, 1 );
	}
      else {  /* trying to repair the secondary table and can't */
	  /*
	   * We won't stop fsck and we won't mark the file system dirty
	   * on this condition, but we'll issue a warning and mark the
	   * superblock to prevent future attempts to maintain the flawed
	   * table.
	   */
	*replication_failure = -1;
        msgprms[0] = "1";
        msgprmidx[0] = 0;
        fsck_send_msg( fsck_INCONSIST2NDRY1, 0, 1 );
	}  /* end else trying to repair the secondary table and can't */
      }  /* end some or all didn't make it to the device. */
    }  /* end the AIMaps are in the temp buffer  */

  return( aimr_rc );
}                          /* end of AIM_replication ()  */


/****************************************************************
 * NAME: AIS_inode_check
 *
 * FUNCTION: Compare the specified Primary Inode Allocation Table inode
 *           to its (specified) Secondary Inode Allocation Table counterpart
 *           to ensure that they are logically equivalent.
 *
 * PARAMETERS:
 *     primary_inoptr          - input - pointer to an inode in the primary
 *                                       aggregate inode allocation table
 *                                       in an fsck buffer
 *     secondary_inoptr        - input - pointer to the equivalent inode in
 *                                       the secondary aggregate inode 
 *                                       allocation table in an fsck buffer
 *     tree_offset             - input - the offset of the B+ Tree rooted
 *                                       in the inode
 *     tree_size               - input - the number of inodes bytes occupied
 *                                       by the B+ tree rooted in the inode
 *     inconsistency_detected  - input - pointer to a variable in which 
 *                                       to return !0 if errors are detected 
 *                                                  0 if no errors are detected
 *
 * NOTES:  
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t AIS_inode_check ( dinode_t *primary_inoptr,
			    dinode_t *secondary_inoptr,
			    int32    tree_offset,
			    int32    tree_size,
			    int      *inconsistency_detected
			   )
{
  retcode_t aisic_rc = FSCK_OK;
  xtpage_t *primary_root, *secondary_root;
  int32 result1, result2, result3, result4, result5, result6;

  *inconsistency_detected = (
	    (primary_inoptr->di_inostamp != secondary_inoptr->di_inostamp) ||
	    (primary_inoptr->di_fileset != secondary_inoptr->di_fileset)   ||
	    (primary_inoptr->di_number != secondary_inoptr->di_number)     ||
	    (primary_inoptr->di_gen != secondary_inoptr->di_gen)           ||
	    (primary_inoptr->di_size != secondary_inoptr->di_size)         ||
	    (primary_inoptr->di_nblocks != secondary_inoptr->di_nblocks)   ||
	    (primary_inoptr->di_nlink != secondary_inoptr->di_nlink)       ||
	    (primary_inoptr->di_uid != secondary_inoptr->di_uid)           ||
	    (primary_inoptr->di_gid != secondary_inoptr->di_gid)           ||
	    (primary_inoptr->di_mode != secondary_inoptr->di_mode)         ||
	    (primary_inoptr->di_compress != secondary_inoptr->di_compress)
			    );

  if( !(*inconsistency_detected) ) {  /* all ok so far */

    if( tree_offset == 0 )  {
      result1 = 0;
      }
    else {  /* tree compare needed */
      primary_root = (xtpage_t *) (((int32) primary_inoptr) + tree_offset);
      secondary_root = (xtpage_t *) (((int32) primary_inoptr) + tree_offset);
      result1 = memcmp( (void *) primary_root,
			(void *) secondary_root,
			tree_size );
      }  /* tree compare needed */

    result2 = memcmp( (void *) &(primary_inoptr->di_atime),
		      (void *) &(secondary_inoptr->di_atime),
		      sizeof(primary_inoptr->di_atime) );
    result3 = memcmp( (void *) &(primary_inoptr->di_ctime),
		      (void *) &(secondary_inoptr->di_ctime),
		      sizeof(primary_inoptr->di_ctime) );
    result4 = memcmp( (void *) &(primary_inoptr->di_mtime),
		      (void *) &(secondary_inoptr->di_mtime),
		      sizeof(primary_inoptr->di_mtime) );
    result5 = memcmp( (void *) &(primary_inoptr->di_otime),
		      (void *) &(secondary_inoptr->di_otime),
		      sizeof(primary_inoptr->di_otime) );
    result6 = memcmp( (void *) &(primary_inoptr->di_ea),
		      (void *) &(secondary_inoptr->di_ea),
		      sizeof(primary_inoptr->di_ea) );

    *inconsistency_detected = (result1 || result2 || result3 ||
			       result4 || result5 || result6   );
    }  /* end all ok so far */

  if( !(*inconsistency_detected) ) {  /* no problems yet */
    *inconsistency_detected = memcmp( (void *) &(sb_ptr->s_ait2),
				      (void *) &(secondary_inoptr->di_ixpxd),
				      sizeof(pxd_t) );
    }  /* end no problems yet */

  if( (*inconsistency_detected) ) {  /* future recover capability is compromised */
      /*
       * Note that we're in read-only mode or we wouldn't be checking this
       * (because when we have write access we always rebuild it)
       */
    msgprms[0] = "2";
    msgprmidx[0] = 0;
    fsck_send_msg( fsck_INCONSIST2NDRY, 0, 1 );
    }  /* end future recover capability is compromised */

  return( aisic_rc );
}                          /* end of AIS_inode_check ()  */


/****************************************************************
 * NAME: AIS_inode_replication
 *
 * FUNCTION: Rebuild the specified target inode by making it the logical
 *           equivalent of the specified source inode.
 *
 * PARAMETERS:
 *     source_is_primary  - input - !0 => replicate from primary to secondary
 *                                   0 => replicate from secondary to primary
 *     target_inoptr      - input - address of an inode in the aggregate
 *                                  inode allocation table (primary or
 *                                  secondary) which is the target of the
 *                                  replication
 *     source_inoptr      - input - address of the equivalent inode in the 
 *                                  aggregate inode allocation table (secondary
 *                                  or primary) which is the source of the
 *                                  secondary) which is the target of the
 *                                  replication
 *
 * NOTES:  o The caller to this routine must ensure that the first extent
 *           of the primary aggregate inode allocation table is at the
 *           beginning of the fsck temporary inode buffer and that the
 *           first extent of the secondary aggregate inode allocation map
 *           immediately follows it in the fsck temporary inode buffer.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t AIS_inode_replication ( int8      source_is_primary,
				  dinode_t *target_inoptr,
				  dinode_t *source_inoptr
				 )
{
  retcode_t aisir_rc = FSCK_OK;
  fsblkidx_t ext_offset;

     /*
      * copy the source over the target, then adjust the field(s) which
      * should be different.
      */
  memcpy( (void *) target_inoptr, (void *) source_inoptr, sizeof(dinode_t) );

  if( source_is_primary ) {
    memcpy( (void *) &(target_inoptr->di_ixpxd),
	    (void *) &(sb_ptr->s_ait2),
	    sizeof(pxd_t) );
    }
  else {
    ext_offset = AITBL_OFF / sb_ptr->s_bsize;
    PXDaddress( &(target_inoptr->di_ixpxd), ext_offset );
    PXDlength( &(target_inoptr->di_ixpxd), lengthPXD(&sb_ptr->s_ait2) );
    }

  return( aisir_rc );
}                          /* end of AIS_inode_replication ()  */


/****************************************************************
 * NAME: AIS_redundancy_check
 *
 * FUNCTION:  Verify that the Secondary Aggregate Inode structures are
 *            logically equivalent to the Primary Aggregate Inode structures.
 *
 * PARAMETERS:  none
 *
 * NOTES:  o The "aggregate inode structures" are the Aggregate Inode Map
 *           and the Aggregate Inode Table.
 *
 *         o This routine reads the first extent of the Primary Aggregate 
 *           Inode Table into the first page of the fsck temporary inode 
 *           buffer and reads the first extent of the Secondary Aggregate 
 *           Inode table into the regular fsck inode buffer.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t AIS_redundancy_check ( )
{
  retcode_t aisrc_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  dinode_t *primary_inoptr, *secondary_inoptr;
  xtpage_t *secondary_ait_root;
  char *primary_bufptr, *secondary_bufptr;
  int32 inode_offset;
  int outcome;
  int inconsistency_detected = 0;
  int32 tree_offset, treesize;
  struct fsck_inode_record temp_inorec;
  fsck_inode_recptr temp_inorecptr = &temp_inorec;
  struct fsck_ino_msg_info temp_msg_info;
  fsck_msg_info_ptr temp_msg_infoptr = &temp_msg_info;
	 
  aisrc_rc = temp_inode_buf_alloc( &primary_bufptr );
  secondary_bufptr = agg_recptr->ino_buf_ptr;
  tree_offset = ((int32) &(primary_inoptr->di_btroot)) -
		((int32) primary_inoptr);
  treesize = sizeof(primary_inoptr->di_DASD) +
	     sizeof(primary_inoptr->di_dxd) + 
                sizeof(primary_inoptr->di_inlinedata);                         /* @F1 */

  if( aisrc_rc == FSCK_OK ) {  /* allocated the temp buffer */

    aisrc_rc = ait_special_read_ext1( fsck_primary );
    if( aisrc_rc != FSCK_OK ) {  /* read failed */
      report_readait_error( aisrc_rc, 
                            FSCK_FAILED_CANTREADAITEXT1, 
                            fsck_primary );
      aisrc_rc = FSCK_FAILED_CANTREADAITEXT1;
      }  /* end read failed */
    else {  /* primary is in the inode buffer */
      memcpy( (void *) primary_bufptr,
	      (void *) agg_recptr->ino_buf_ptr,
	      INODE_IO_BUFSIZE );

      aisrc_rc = ait_special_read_ext1( fsck_secondary );
      if( aisrc_rc != FSCK_OK ) {  /* read failed */
        report_readait_error( aisrc_rc, 
                              FSCK_FAILED_CANTREADAITEXT2, 
                              fsck_secondary );
        aisrc_rc = FSCK_FAILED_CANTREADAITEXT2;
        }  /* end read failed */
      }  /* end else primary is in the inode buffer */

    if( aisrc_rc == FSCK_OK ) {  /* the 2 inode extents are in memory */

	/*
	 * the self inode
	 */
      inode_offset = AGGREGATE_I * sizeof(dinode_t);
      primary_inoptr = (dinode_t *) (((int32) primary_bufptr) + inode_offset );
      secondary_inoptr = (dinode_t *) (((int32) secondary_bufptr) + inode_offset );

      aisrc_rc = AIS_inode_check ( primary_inoptr,
				 secondary_inoptr,
				 0, 0,            /* don't check the tree */
				 &inconsistency_detected
				);
      if( (aisrc_rc == FSCK_OK) && (!inconsistency_detected) ) {
	secondary_ait_root = (xtpage_t *) &( secondary_inoptr->di_btroot );
	}
      }  /* end the 2 inode extents are in memory */

    if( (aisrc_rc == FSCK_OK) && (!inconsistency_detected) ) {  /*
			      * no problems detected so far
			      */
	/*
	 * the block map inode
	 */
      inode_offset = BMAP_I * sizeof(dinode_t);
      primary_inoptr = (dinode_t *) (((int32) primary_bufptr) + inode_offset );
      secondary_inoptr = (dinode_t *) (((int32) secondary_bufptr) + inode_offset );
      aisrc_rc = AIS_inode_check ( primary_inoptr,
				 secondary_inoptr,
				 tree_offset,
				 treesize,
				 &inconsistency_detected
				);
      }  /* end no problems detected so far */

    if( (aisrc_rc == FSCK_OK) && (!inconsistency_detected) ) {  /*
			      * still in business
			      */
	/*
	 * the journal log inode
	 */
      inode_offset = LOG_I * sizeof(dinode_t);
      primary_inoptr = (dinode_t *) (((int32) primary_bufptr) + inode_offset );
      secondary_inoptr = (dinode_t *) (((int32) secondary_bufptr) + inode_offset );
      aisrc_rc = AIS_inode_check ( primary_inoptr,
				 secondary_inoptr,
				 tree_offset,
				 treesize,
				 &inconsistency_detected
				);
      }  /* end still in business */

    if( (aisrc_rc == FSCK_OK) && (!inconsistency_detected) ) {  /*
			      * no show stopper yet
			      */
	/*
	 * the bad block inode
	 */
      inode_offset = BADBLOCK_I * sizeof(dinode_t);
      primary_inoptr = (dinode_t *) (((int32) primary_bufptr) + inode_offset );
      secondary_inoptr = (dinode_t *) (((int32) secondary_bufptr) + inode_offset );
      aisrc_rc = AIS_inode_check ( primary_inoptr,
				 secondary_inoptr,
				 tree_offset,
				 treesize,
				 &inconsistency_detected
				);
      }  /* end no show stopper yet */

    if( (aisrc_rc == FSCK_OK) && (!inconsistency_detected) ) {  /*
			      * and finally the filesystem inode
			      */
      inode_offset = FILESYSTEM_I * sizeof(dinode_t);
      primary_inoptr = (dinode_t *) (((int32) primary_bufptr) + inode_offset );
      secondary_inoptr = (dinode_t *) (((int32) secondary_bufptr) + inode_offset );
      aisrc_rc = AIS_inode_check ( primary_inoptr,
				 secondary_inoptr,
				 0, 0,            /* don't check the tree yet */
				 &inconsistency_detected
				);
      if( (aisrc_rc == FSCK_OK) && (!inconsistency_detected) ) { /*
			      * no problems detected yet
			      */
        temp_inorecptr->inonum = FILESYSTEM_I;
                                                        /* 6 lines deleted                     @F1 */
        temp_inorecptr->in_use = 0;
        temp_inorecptr->selected_to_rls = 0;  /*                                      @F1 */
        temp_inorecptr->crrct_link_count = 0;
        temp_inorecptr->crrct_prnt_inonum = 0;
        temp_inorecptr->adj_entries = 0;
        temp_inorecptr->cant_chkea = 0;
        temp_inorecptr->clr_ea_fld = 0;
        temp_inorecptr->clr_acl_fld = 0;
        temp_inorecptr->crrct_cumm_blks = 0;
                                                        /* 1 line deleted                      @F1 */
        temp_inorecptr->inlineea_on = 0;
        temp_inorecptr->inlineea_off = 0;
        temp_inorecptr->inline_data_err = 0;
        temp_inorecptr->ignore_alloc_blks = 0;
        temp_inorecptr->reconnect = 0;
        temp_inorecptr->unxpctd_prnts = 0;
        temp_inorecptr->badblk_inode = 0;
        temp_inorecptr->involved_in_dups = 0;
        temp_inorecptr->inode_type = metadata_inode;
        temp_inorecptr->link_count = 0;
        temp_inorecptr->parent_inonum = 0;
        temp_inorecptr->cumm_blocks = 0;                                         /* @F1 */
        temp_inorecptr->dtree_level = -1;                                          /* @F1 */
        temp_inorecptr->ext_rec = NULL;
	 
        temp_msg_infoptr->msg_inonum = FILESYSTEM_I;
        temp_msg_infoptr->msg_inopfx = fsck_aggr_inode;
        temp_msg_infoptr->msg_inotyp = fsck_metadata;
        temp_msg_infoptr->msg_dxdtyp = 0;
           /*
            * need to verify the tree structure in the secondary 
            * aggregate fileset inode and record/dupcheck the 
            * tree nodes (but not the data extents described by
            * the leaf node(s)).
            */
        aisrc_rc = xTree_processing( secondary_inoptr, FILESYSTEM_I,
                                                temp_inorecptr, temp_msg_infoptr,
                                                FSCK_FSIM_RECORD_DUPCHECK );
        if( temp_inorecptr->involved_in_dups || 
            temp_inorecptr->selected_to_rls    ||
            temp_inorecptr->ignore_alloc_blks    )  {  /* N.F.G. */
          inconsistency_detected = -1;
          aisrc_rc = xTree_processing( secondary_inoptr, FILESYSTEM_I,
                                                  temp_inorecptr, temp_msg_infoptr,
                                                  FSCK_FSIM_UNRECORD );
              /*
               * Note that we're in read-only mode or we wouldn't be 
               * checking this (because when we have write access we 
               * always rebuild it)
               */
          msgprms[0] = "4";
          msgprmidx[0] = 0;
          fsck_send_msg( fsck_INCONSIST2NDRY, 0, 1 );
          }  /* end N.F.G. */
        else {  /* the tree is good and its nodes have been recorded */
          aisrc_rc = FSIM_check( primary_inoptr, secondary_inoptr,
                                          &inconsistency_detected
                                          );
          }  /* end else the tree is good and its nodes have been recorded */
        }  /* end no problems detected yet */
      }  /* end and finally the filesystem inode */

    if( (aisrc_rc == FSCK_OK) && (!inconsistency_detected) ) { /*
			      * no problems detected in the AIT --
			      * now verify the secondary AIM
			      */
      aisrc_rc = AIM_check( secondary_ait_root,
			  primary_bufptr,
			  &inconsistency_detected );
      }  /* end no problems detected in the AIT -- now verify ... */

    intermed_rc = temp_inode_buf_release( primary_bufptr );
    }  /* end allocated the temp buffer */

  return( aisrc_rc );
}                          /* end of AIS_redundancy_check ()  */


/****************************************************************
 * NAME: AIS_replication
 *
 * FUNCTION:  Rebuild the aggregate inode structures so that the Secondary
 *            Aggregate Inode structures are logically equivalent to the
 *            Primary Aggregate Inode structures.
 *
 * PARAMETERS:  none
 *
 * NOTES:  o Since the roles of source and target AIT can change
 *           during this routine, we read both source and target
 *           so that when we write we preserve the portion of the
 *           target extent which is really a source.
 *           (This is simpler than attempting to write individual
 *           inodes and dealing with the question of device block
 *           size larger than an inode.)
 *
 *         o The "aggregate inode structures" are the Aggregate Inode Map
 *           and the Aggregate Inode Table.
 *
 *         o This routine reads the first extent of one Aggregate Inode
 *           Table into the fsck temporary inode buffer and reads the 
 *           first extent of the other Aggregate Inode Table into the
 *           regular fsck inode buffer.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t AIS_replication ( )
{
  retcode_t aisr_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  dinode_t *source_inoptr, *target_inoptr;
  fsblkidx_t target_byte_offset;
  xtpage_t *target_ait_root;
  char *source_bufptr, *target_bufptr;
  int32 inode_offset;
  int8 source_is_primary, extents_in_memory;
  int inconsistency_detected = 0;
  int8 replication_failed = 0;
  int32 tree_offset, treesize;
  uint32 bytes_transferred;

  aisr_rc = temp_inode_buf_alloc( &source_bufptr );

  target_bufptr = agg_recptr->ino_buf_ptr;

  if( aisr_rc == FSCK_OK ) {  /* allocated the temp buffer */
      /*
       * the filesystem inode could have different source-target than the
       *  rest of the table, so deal with it first.
       */
    extents_in_memory = 0;
    if( agg_recptr->primary_ait_4part2 ) {  /*
			   * primary for fileset inode source
			   */
      source_is_primary = -1;
      aisr_rc = ait_special_read_ext1( fsck_primary );
      if( aisr_rc != FSCK_OK ) {  /* read failed */
        report_readait_error( aisr_rc, 
                              FSCK_FAILED_CANTREADAITEXT3, 
                              fsck_primary );
        aisr_rc = FSCK_FAILED_CANTREADAITEXT3;
        }  /* end read failed */
      }  /* end primary for fileset inode source */
    else {  /* secondary for fileset inode source */
      source_is_primary = 0;
      aisr_rc = ait_special_read_ext1( fsck_secondary );
      if( aisr_rc != FSCK_OK ) {  /* read failed */
        report_readait_error( aisr_rc, 
                              FSCK_FAILED_CANTREADAITEXT4, 
                              fsck_secondary );
        aisr_rc = FSCK_FAILED_CANTREADAITEXT4;
        }  /* end read failed */
      }  /* end else secondary for fileset inode source */

    if( aisr_rc == FSCK_OK ) {  /* source is in the inode buffer */
      memcpy( (void *) source_bufptr,
	      (void *) agg_recptr->ino_buf_ptr,
	      INODE_IO_BUFSIZE );

      if( agg_recptr->primary_ait_4part2 ) {  /*
			   * secondary for fileset inode target
			   */
	aisr_rc = ait_special_read_ext1( fsck_secondary );
        if( aisr_rc == FSCK_OK ) {  /* read successful */
          target_byte_offset = 
                addressPXD( &(sb_ptr->s_ait2) ) * sb_ptr->s_bsize;
          }  /* end read successful */
        else {  /* read failed */
          report_readait_error( aisr_rc, 
                                FSCK_FAILED_CANTREADAITEXT5, 
                                fsck_secondary );
          aisr_rc = FSCK_FAILED_CANTREADAITEXT5;
          }  /* end else read failed */
	}  /* end secondary for fileset inode target */
      else {  /* primary for fileset inode target */
	aisr_rc = ait_special_read_ext1( fsck_primary );
        if( aisr_rc == FSCK_OK ) {  /* read successful */
 	  target_byte_offset = AITBL_OFF;
          }  /* end read successful */
        else {  /* read failed */
          report_readait_error( aisr_rc, 
                                FSCK_FAILED_CANTREADAITEXT6, 
                                fsck_primary );
          aisr_rc = FSCK_FAILED_CANTREADAITEXT6;
          }  /* end else read failed */
	}  /* end else primary for fileset inode target */
      }  /* end else source is in the inode buffer */

    if( aisr_rc == FSCK_OK ) {  /* the 2 inode extents for the fileset inode
			   * are in memory
			   */
      extents_in_memory = -1;
      inode_offset = FILESYSTEM_I * sizeof(dinode_t);
      source_inoptr = (dinode_t *) (((int32) source_bufptr) + inode_offset );
      target_inoptr = (dinode_t *) (((int32) target_bufptr) + inode_offset );

      aisr_rc = AIS_inode_replication ( source_is_primary,
					target_inoptr, source_inoptr );
      if( aisr_rc == FSCK_OK ) {
        aisr_rc = init_xtree_root( target_inoptr );
        }
      if( aisr_rc == FSCK_OK ) {  
        aisr_rc = FSIM_replication( source_is_primary, 
                                             target_inoptr, source_inoptr,
                                             &replication_failed );
        if( (aisr_rc == FSCK_OK) && (replication_failed) ) {
          if( !source_is_primary ) {  /* we're trying to repair the primary
				 * table and can't -- that makes this a
				 * dirty file system.
				 */
            agg_recptr->ag_dirty = 1;
            msgprms[0] = "2";
            msgprmidx[0] = 0;
            fsck_send_msg( fsck_CANTREPAIRAIS, 0, 1 );
            }
          else {  /* trying to repair the secondary table and can't */
		/*
		 * We won't stop fsck and we won't  the file system dirty
		 * on this condition, but we'll issue a warning and  the
		 * superblock to prevent future attempts to maintain the
		 * flawed table.
		 */
            msgprms[0] = "2";
            msgprmidx[0] = 0;
            fsck_send_msg( fsck_INCONSIST2NDRY1, 0, 1 );
            }  /* end else trying to repair the secondary table and can't */
          }
        }
      }  /* end the 2 inode extents for the fileset inode are in memory */

    if( (aisr_rc == FSCK_OK) && (!replication_failed) &&
	(agg_recptr->primary_ait_4part1 != agg_recptr->primary_ait_4part2) ) { /*
			   * we need to switch source and target
			   */
      intermed_rc = readwrite_device( target_byte_offset,
				      INODE_EXTENT_SIZE, &bytes_transferred,
				      (void *) target_bufptr, fsck_WRITE );
      agg_recptr->ag_modified = 1;
      if( (intermed_rc != FSCK_OK) || (bytes_transferred != INODE_EXTENT_SIZE) ) {
	  /*
	   * some or all didn't make it to the device.
	   *
	   */
	agg_recptr->ait_aim_update_failed = 1;

	if( !source_is_primary ) {  /* we're trying to repair the primary
				 * table and can't -- that makes this a
				 * dirty file system.
				 */
             agg_recptr->ag_dirty = 1;
             msgprms[0] = "3";
             msgprmidx[0] = 0;
             fsck_send_msg( fsck_CANTREPAIRAIS, 0, 1 );
             }
	else {  /* trying to repair the secondary table and can't */
	    /*
	     * We won't stop fsck and we won't  the file system dirty
	     * on this condition, but we'll issue a warning and  the
	     * superblock to prevent future attempts to maintain the flawed
	     * table.
	     */
	  replication_failed = -1;
          msgprms[0] = "3";
          msgprmidx[0] = 0;
          fsck_send_msg( fsck_INCONSIST2NDRY1, 0, 1 );
	  }  /* end else trying to repair the secondary table and can't */
	}  /* write to target failed */

      extents_in_memory = 0;
      if( agg_recptr->primary_ait_4part1 ) {  /*
			   * primary for remaining inodes source
			   */
	source_is_primary = -1;
	aisr_rc = ait_special_read_ext1( fsck_primary );
        if( aisr_rc != FSCK_OK ) {  /* read failed */
          report_readait_error( aisr_rc, 
                                FSCK_FAILED_CANTREADAITEXT7, 
                                fsck_primary );
          aisr_rc = FSCK_FAILED_CANTREADAITEXT7;
          }  /* end read failed */
	}  /* end primary for remaining inodes source */
      else {  /* secondary for remaining inodes source */
           source_is_primary = 0;
           aisr_rc = ait_special_read_ext1( fsck_secondary );
           if( aisr_rc != FSCK_OK ) {  /* read failed */
             report_readait_error( aisr_rc, 
                                            FSCK_FAILED_CANTREADAITEXT8, 
                                            fsck_secondary );
             aisr_rc = FSCK_FAILED_CANTREADAITEXT8;
             }  /* end read failed */
	}  /* end else secondary for remaining inodes source */

      if( aisr_rc == FSCK_OK ) {  /* source is in the inode buffer */
	memcpy( (void *) source_bufptr,
		(void *) agg_recptr->ino_buf_ptr,
		INODE_IO_BUFSIZE );

	if( agg_recptr->primary_ait_4part1 ) {  /*
			   * secondary for remaining inodes target
			   */
	  intermed_rc = ait_special_read_ext1( fsck_secondary );
          target_byte_offset = 
                   addressPXD( &(sb_ptr->s_ait2) ) * sb_ptr->s_bsize;
          if( intermed_rc != FSCK_OK ) {  /* this isn't fatal */
                     /*
                      * message to debugger
                      */
            sprintf( message_parm_0, "%d", FSCK_CANTREADAITEXT3 );
            msgprms[0] = message_parm_0;
            msgprmidx[0] = 0;
            sprintf( message_parm_1, "%d", intermed_rc );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            sprintf( message_parm_2, "%d", fsck_secondary );
            msgprms[2] = message_parm_2;
            msgprmidx[2] = 0;
            fsck_send_msg( fsck_ERRONAITRD, 0, 3 );
            }  /* end this isn't fatal */
	  }  /* end secondary for remaining inodes target */

	else {  /* primary for remaining inodes target */
	  intermed_rc = ait_special_read_ext1( fsck_primary );
          target_byte_offset = AITBL_OFF;
          if( intermed_rc != FSCK_OK ) {  /* this isn't fatal */
                     /*
                      * message to debugger
                      */
            sprintf( message_parm_0, "%d", FSCK_CANTREADAITEXT4 );
            msgprms[0] = message_parm_0;
            msgprmidx[0] = 0;
            sprintf( message_parm_1, "%d", intermed_rc );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            sprintf( message_parm_2, "%d", fsck_primary );
            msgprms[2] = message_parm_2;
            msgprmidx[2] = 0;
            fsck_send_msg( fsck_ERRONAITRD, 0, 3 );
            }  /* end this isn't fatal */
	  }  /* end else primary for remaining inodes target */

	if( intermed_rc == FSCK_OK ) {  /* the 2 inode extents for the remaining
			   * inodes are in memory
			   */
	  extents_in_memory = -1;
	  }  /* end the 2 inode extents for the remaining inodes are ... */
	}  /* end source is in the inode buffer */
      }

    if( (aisr_rc == FSCK_OK) && (!replication_failed) && 
        (extents_in_memory) ) {  /* we have the extents for the remaining 
                                  * AIT inodes
                                  */
	/*
	 * the self inode
	 */
      inode_offset = AGGREGATE_I * sizeof(dinode_t);
      source_inoptr = (dinode_t *) (((int32) source_bufptr) + inode_offset );
      target_inoptr = (dinode_t *) (((int32) target_bufptr) + inode_offset );

      aisr_rc = AIS_inode_replication ( source_is_primary,
					target_inoptr, source_inoptr );
      if( aisr_rc == FSCK_OK ) {  /* aggregate inode replicated in buffer */
	target_ait_root = (xtpage_t *) &( target_inoptr->di_btroot );

	/*
	 * the bmap inode
	 */
	inode_offset = BMAP_I * sizeof(dinode_t);
	source_inoptr = (dinode_t *) (((int32) source_bufptr) + inode_offset );
	target_inoptr = (dinode_t *) (((int32) target_bufptr) + inode_offset );

	aisr_rc = AIS_inode_replication ( source_is_primary,
					  target_inoptr, source_inoptr );
	}  /* end aggregate inode replicated in buffer */

      if( aisr_rc == FSCK_OK ) {  /* no problems so far */

	/*
	 * the journal log inode
	 */
	inode_offset = LOG_I * sizeof(dinode_t);
	source_inoptr = (dinode_t *) (((int32) source_bufptr) + inode_offset );
	target_inoptr = (dinode_t *) (((int32) target_bufptr) + inode_offset );

	aisr_rc = AIS_inode_replication ( source_is_primary,
					  target_inoptr, source_inoptr );
	}  /* end no problems so far */

      if( aisr_rc == FSCK_OK ) {  /* no show stopper yet */

	/*
	 * the bad block inode
	 */
	inode_offset = BADBLOCK_I * sizeof(dinode_t);
	source_inoptr = (dinode_t *) (((int32) source_bufptr) + inode_offset );
	target_inoptr = (dinode_t *) (((int32) target_bufptr) + inode_offset );

	aisr_rc = AIS_inode_replication ( source_is_primary,
					  target_inoptr, source_inoptr );
	}  /* end no show stopper yet */

      if( aisr_rc == FSCK_OK ) {  /* the ait is replicated in the buffer */

	/*
	 * now finish up the self inode by replicating the aggregate inode
	 * map which it describes.
	 */
	aisr_rc = AIM_replication( source_is_primary,
				   target_ait_root,
				   source_bufptr,
				   &replication_failed );

	if( aisr_rc == FSCK_OK ) {  /* ait replication in buffer now complete */

	  intermed_rc = readwrite_device( target_byte_offset,
					  INODE_EXTENT_SIZE, &bytes_transferred,
					  (void *) target_bufptr, fsck_WRITE );
	  agg_recptr->ag_modified = 1;
	  if( (intermed_rc != FSCK_OK) || (bytes_transferred != INODE_EXTENT_SIZE) ) {
	      /*
	       * some or all didn't make it to the device.
	       */
	    agg_recptr->ait_aim_update_failed = 1;
	    if( !source_is_primary ) {  /* we're trying to repair the primary
				 * table and can't -- that makes this a
				 * dirty file system.
				 */
	      agg_recptr->ag_dirty = 1;
              msgprms[0] = "4";
              msgprmidx[0] = 0;
              fsck_send_msg( fsck_CANTREPAIRAIS, 0, 1 );
	      }
	    else {  /* trying to repair the secondary table and can't */
		/*
		 * We won't stop fsck and we won't  the file system dirty
		 * on this condition, but we'll issue a warning and  the
		 * superblock to prevent future attempts to maintain the
		 * flawed table.
		 */
	      replication_failed = -1;
              msgprms[0] = "4";
              msgprmidx[0] = 0;
              fsck_send_msg( fsck_INCONSIST2NDRY1, 0, 1 );
	      }  /* end else trying to repair the secondary table and can't */
	    }  /* write to target failed */
	  }  /* end ait replication in buffer now complete */
	}  /* end the ait is replicated in the buffer */
      }  /* end we have the extents for the remaining AIT inodes */

    intermed_rc = temp_inode_buf_release( source_bufptr );

    if( aisr_rc == FSCK_OK ) {
      if( replication_failed ) {
	sb_ptr->s_flag |= JFS_BAD_SAIT;
	}
      else {
	sb_ptr->s_flag &= (~JFS_BAD_SAIT);
	}
      }

    }  /* end allocated the temp buffer */

  return( aisr_rc );
}                          /* end of AIS_replication ()  */


/****************************************************************
 * NAME: first_ref_check_inode_extents
 *
 * FUNCTION:  Determine whether any inode extent in the aggregate (i.e., 
 *            in either the Aggregate Inode Table or in the Fileset Inode
 *            Table) contains a reference to any multiply-allocated 
 *            aggregate block whose first reference has not yet been 
 *            resolved.
 *
 * PARAMETERS:  none
 *
 * NOTES:  
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t first_ref_check_inode_extents ()
{
  retcode_t rdie_rc = FSCK_OK;
  int      is_aggregate;
  int      which_it;
  fsck_inode_recptr inorecptr;

  struct fsck_ino_msg_info msg_info;
  fsck_msg_info_ptr msg_info_ptr;

  msg_info_ptr = &msg_info;
  msg_info_ptr->msg_inonum = AGGREGATE_I;
  msg_info_ptr->msg_inopfx = fsck_aggr_inode;
  msg_info_ptr->msg_inotyp = fsck_metaIAG;

      /*
       * check the inode extents in the Aggregate Inode Allocation Map
       * for first references to multiply allocated blocks.
       */
  is_aggregate = -1;          /* aggregate IAGs */
  if( agg_recptr->primary_ait_4part1 ) {
    which_it = fsck_primary;
    }
  else {
    which_it = fsck_secondary;
    }
  rdie_rc = get_inorecptr( is_aggregate, 0, AGGREGATE_I, &inorecptr );

  if( (rdie_rc == FSCK_OK) && (inorecptr == NULL) ) {
    rdie_rc = FSCK_INTERNAL_ERROR_18;
    }
  else if( rdie_rc == FSCK_OK ) {

    rdie_rc = first_refchk_inoexts( is_aggregate, which_it,
				    inorecptr, msg_info_ptr );

    if( rdie_rc == FSCK_OK ) {  /* things are going as expected */
	/*
	 * check the inode extents in the Fileset Inode Allocation Maps
	 * for first references to multiply allocated blocks.
	 *
	 * (In release 1 there is exactly 1 fileset)
	 */
      which_it = FILESYSTEM_I;
      is_aggregate = -1;         /* aggregate inode */
      rdie_rc = get_inorecptr( is_aggregate, 0, FILESYSTEM_I, &inorecptr );
      if( (rdie_rc == FSCK_OK) && (inorecptr == NULL) ) {
	rdie_rc = FSCK_INTERNAL_ERROR_19;
	}
      else if( rdie_rc == FSCK_OK ) {
	is_aggregate = 0;         /* fileset IAGs */
	rdie_rc = first_refchk_inoexts( is_aggregate, which_it,
					inorecptr, msg_info_ptr );
	}
      }  /* end things are going as expected */
    }
  return( rdie_rc );
}                              /* end of first_ref_check_inode_extents ()  */

/****************************************************************
 * NAME: first_refchk_inoexts
 *
 * FUNCTION:  Check all inode extents described by IAGs in the specified
 *            inode table to determine whether they contain a reference
 *            to any multiply-allocated aggregate block whose first reference
 *            has not yet been resolved.
 *
 * PARAMETERS:
 *     is_aggregate  - input - !0 => aggregate owned
 *                              0 => fileset owned
 *     which_it      - input - ordinal number of the aggregate inode 
 *                             describing the inode table
 *     inorecptr     - input - pointer to an fsck inode record which
 *                             describes the current inode allocation
 *                             table inode
 *     msg_info_ptr  - input - pointer to data needed to issue messages
 *                             about the current inode 
 *
 * NOTES:  o Since this routine completes the fsck workspace
 *           initialization needed by inode_get() (et al), this
 *           routine ensures fsck I/O buffers contain the inode
 *           it needs before invoking inode_get().
 *           (inode_get() is invoked to locate the inode within
 *           the fsck I/O buffer.)
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t first_refchk_inoexts ( int                 is_aggregate,
				 int                 which_it,
				 fsck_inode_recptr   inorecptr,
				 fsck_msg_info_ptr   msg_info_ptr
				)
{
  retcode_t ifrie_rc = FSCK_OK;
  iag_t    *iagptr;
  reg_idx_t  ext_idx;
  reg_idx_t  ext_idx_max;
  fsblkidx_t ext_addr;
  reg_idx_t  ext_len;
  int        which_ait;
  int8       is_EA = 0;
  int8       is_ACL = 0;
  int8       extent_is_valid;
  reg_idx_t  adjusted_length;
  ext_idx_max = EXTSPERIAG - 1;
  if( is_aggregate ) {
    if( agg_recptr->primary_ait_4part1 ) {
      which_ait = fsck_primary;
      }
    else {
      which_ait = fsck_secondary;
      }
    }
  else {
    if( agg_recptr->primary_ait_4part2 ) {
      which_ait = fsck_primary;
      }
    else {
      which_ait = fsck_secondary;
      }
    }
  ifrie_rc = iag_get_first( is_aggregate, which_it, which_ait, &iagptr );
  while( (iagptr != NULL) &&
	 (agg_recptr->unresolved_1stref_count > 0) &&
	 (ifrie_rc == FSCK_OK) ) {
    for( ext_idx = 0;
	 ( (ext_idx <= ext_idx_max) &&
	   (agg_recptr->unresolved_1stref_count > 0) &&
	   (ifrie_rc == FSCK_OK) );
	 ext_idx++ ) {
	
      ext_addr = addressPXD(&(iagptr->inoext[ext_idx]));
      if( ext_addr != 0 ) {  /* the extent is allocated */
	ext_len = lengthPXD(&(iagptr->inoext[ext_idx]));
	ifrie_rc = process_extent( inorecptr, ext_len, ext_addr, is_EA, 
				   is_ACL, msg_info_ptr, &adjusted_length, 
				   &extent_is_valid, FSCK_QUERY );
	}  /* end the extent is allocated */
      }  /* end for ext_idx */
    if( ifrie_rc == FSCK_OK ) {
      ifrie_rc = iag_get_next( &iagptr );
      }
    }  /* end while iagptr != NULL */
  return( ifrie_rc );
}                            /* end of first_refchk_inoexts ()  */


/*--------------------------------------------------------------------
 * NAME: FSIM_add_extents
 *
 * FUNCTION: Add an extent of <thisblk> to the <bb_inode> inode
 *
 * NOTES:
 *	o It is not necessary to mark the extent in the block map
 *           since it is an extent already owned by the source 
 *           FileSet Inode Map.
 *
 * PARAMETERS:
 *      thisblk - block number of bad block to add
 *      bb_inode        - Inode to add bad block to
 *
 * RETURNS: 0 for success; Other indicates failure
 */
retcode_t FSIM_add_extents( xtpage_t   *src_leaf_ptr,
                                         dinode_t    *target_inode,
                                         int8           *replication_failed
                                        )
{
  int32      fsimae_rc = FSCK_OK;
  int32      xad_idx, ext_length, ext_bytes;
  int64      ext_addr;
	 
  *replication_failed = 0;
	 
  for( xad_idx=XTENTRYSTART; 
        ( (fsimae_rc == FSCK_OK) &&(!*replication_failed) &&
          (xad_idx < src_leaf_ptr->header.nextindex)         );
        xad_idx++ ) {
    ext_addr = addressXAD( &(src_leaf_ptr->xad[xad_idx]) );
    ext_length = lengthXAD( &(src_leaf_ptr->xad[xad_idx]) );
    ext_bytes = ext_length << agg_recptr->log2_blksize;

    fsimae_rc = xtAppend( target_inode, target_inode->di_size / sb_ptr->s_bsize, 
                                    ext_addr, ext_length, fsim_node_pages );

    target_inode->di_size += ext_bytes;
    target_inode->di_nblocks+= ext_length;
    }  /* end for xad_idx */
	 
  if( fsimae_rc > 0 )  {  /* an error, but not fatal */
    *replication_failed = -1;
    fsimae_rc = FSCK_OK;
    }
	 
  return( fsimae_rc );
}                                                   /* end FSIM_add_extents() */
	 
	 
/****************************************************************
 * NAME: FSIM_check
 *
 * FUNCTION: Compare the specified Primary Fileset Inode Map to its 
 *                (specified) Secondary Fileset Inode Map counterpart to
 *                ensure that they are logically equivalent.
 *
 * PARAMETERS:
 *     primary_inoptr          - input - pointer to an inode in the primary
 *                                       aggregate inode allocation table
 *                                       in an fsck buffer
 *     secondary_inoptr        - input - pointer to the equivalent inode in
 *                                       the secondary aggregate inode 
 *                                       allocation table in an fsck buffer
 *     inconsistency_detected  - input - pointer to a variable in which 
 *                                       to return !0 if errors are detected 
 *                                                  0 if no errors are detected
 *
 * NOTES:  
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t FSIM_check ( dinode_t *primary_inoptr,
                                 dinode_t *secondary_inoptr,
                                 int         *inconsistency_detected
                                )
{
  retcode_t fsimc_rc = FSCK_OK;
  xtpage_t *primary_nodeptr, *secondary_nodeptr, *leaf_node;
  int8 primary_is_inline, primary_is_rootleaf;
  int8 secondary_is_inline, secondary_is_rootleaf;
  int leaf_length;
  fsblkidx_t node_agg_offset;
	 
             /*
              * allocate 2 buffers to contain the nodes
              */
  fsimc_rc = temp_node_buf_alloc ( (char **) (&agg_recptr->prim_nodeptr) );
  if( fsimc_rc == FSCK_OK ) {  /* first node buffer allocated */
    primary_nodeptr = agg_recptr->prim_nodeptr;
    fsimc_rc = temp_node_buf_alloc ( (char **) (&agg_recptr->second_nodeptr) );
    if( fsimc_rc == FSCK_OK ) {  /* second node buffer allocated */
      secondary_nodeptr = agg_recptr->second_nodeptr;
      fsimc_rc = find_first_leaf( primary_inoptr, 
                                          &leaf_node, &node_agg_offset,
                                          &primary_is_inline, &primary_is_rootleaf );
      if( fsimc_rc != FSCK_OK ) {
        *inconsistency_detected = -1;
        fsimc_rc = FSCK_OK;
        }
      else {  /* got first leaf of primary fs inode map */
              /*
               * copy it into the temp primary node buffer
               */
        if( primary_is_rootleaf) { 
          leaf_length = XTROOTMAXSLOT * sizeof(xad_t);
          memcpy( (void *) primary_nodeptr, 
                        (void *) &(primary_inoptr->di_btroot), leaf_length );
          }
        else {
           leaf_length = XTPAGE_SIZE;
           memcpy( (void *) primary_nodeptr, 
                        (void *) leaf_node, leaf_length );
           }
	 
        fsimc_rc = find_first_leaf( secondary_inoptr, 
                                            &leaf_node, &node_agg_offset,
                                            &secondary_is_inline, &secondary_is_rootleaf );
        if( fsimc_rc != FSCK_OK ) {
          *inconsistency_detected = -1;
          fsimc_rc = FSCK_OK;
          }
        else {  /* got first leaf of secondary fs inode map */
              /*
               * copy it into the temp secondary node buffer
               */
          if( secondary_is_rootleaf) { 
            leaf_length = XTROOTMAXSLOT * sizeof(xad_t);
            memcpy( (void *) secondary_nodeptr, 
                          (void *) &(secondary_inoptr->di_btroot), leaf_length );
            }
          else {
             leaf_length = XTPAGE_SIZE;
             memcpy( (void *) secondary_nodeptr, 
                           (void *) leaf_node, leaf_length );
             }
           }  /* end else got first leaf of secondary fs inode map */
        }  /* end else got first leaf of primary fs inode map */
      if( !(*inconsistency_detected) && (fsimc_rc == FSCK_OK) ) {
        if( (primary_is_rootleaf != secondary_is_rootleaf) ||
            (primary_is_inline != secondary_is_inline)            ) {
          *inconsistency_detected = -1;
          }
        else {  /* either both or neither is a rootleaf */
          if( primary_is_rootleaf ) {  /* they're root leaf nodes */
             fsimc_rc = IM_compare_leaf ( primary_nodeptr, 
                                                      secondary_nodeptr, 
                                                      inconsistency_detected );
             }  /* end they're root leaf nodes */
          else {  /* they're not root leaf nodes */
            while( (primary_nodeptr != NULL) && (secondary_nodeptr != NULL) &&
                      (!(*inconsistency_detected)) && (fsimc_rc == FSCK_OK)  ) {
              fsimc_rc = IM_compare_leaf ( primary_nodeptr, 
                                                       secondary_nodeptr, 
                                                       inconsistency_detected );
              if( (fsimc_rc == FSCK_OK) && !(*inconsistency_detected) ) {   /*
                                          * leafs compare as equal 
                                          */
                if( primary_nodeptr->header.next == 0 ) {
                  primary_nodeptr = NULL;
                  }
                else {  /* primary leaf has a next */
                  node_agg_offset = primary_nodeptr->header.next;
                  fsimc_rc = ait_node_get( node_agg_offset, primary_nodeptr );
                  if( fsimc_rc != FSCK_OK ) {
                    if( fsimc_rc > 0 ) {  /* error but not fatal */
                      *inconsistency_detected = -1;
                      fsimc_rc = FSCK_OK;
                      }  /* end error but not fatal */
                    }
                  }  /* end else primary leaf has a next */
                if( (fsimc_rc == FSCK_OK) && (!(*inconsistency_detected)) ) {
                  if( secondary_nodeptr->header.next == 0 ) {
                    secondary_nodeptr = NULL;
                    }
                  else {  /* secondary leaf has a next */
                    node_agg_offset = secondary_nodeptr->header.next; 
                    fsimc_rc = ait_node_get( node_agg_offset, secondary_nodeptr );
                    if( fsimc_rc > 0 ) {  /* error but not fatal */
                      *inconsistency_detected = -1;
                      fsimc_rc = FSCK_OK;
                      }  /* end error but not fatal */
                    }  /* end else secondary leaf has a next */
                  }
                }  /* end leafs compare as equal */
              }  /* end while */
            if( (primary_nodeptr != NULL) || (secondary_nodeptr != NULL) ) {  /*
                                               * on exit these should both be NULL
                                               */
              *inconsistency_detected = -1;
              }  /* end on exit these should both be NULL */
            }  /* end else they're not root leaf nodes */
          }  /* end either both or neither is a rootleaf */
        }
      }  /* end second node buffer allocated */
    }  /* end first node buffer allocated */
	 
  if( (*inconsistency_detected) ) {  /* future recovery capability is compromised */
      /*
       * Note that we're in read-only mode or we wouldn't be checking this
       * (because when we have write access we always rebuild it)
       */
    msgprms[0] = "3";
    msgprmidx[0] = 0;
    fsck_send_msg( fsck_INCONSIST2NDRY, 0, 1 );
    }  /* end future recover capability is compromised */

  return( fsimc_rc );
}                          /* end of FSIM_check ()  */
	 
	 
/****************************************************************
 * NAME: FSIM_replication
 *
 * FUNCTION: Replicate the Fileset Inode Map from the given source Aggregate 
 *                Map to the given target Aggregate Inode Map so that they are
 *                logically equivalent.  That is, so that they have independent
 *                B+Trees, but the leaf level of the 2 trees point to the same "data"
 *                extents (control page and IAGs).
 *
 * PARAMETERS:
 *     source_is_primary - input - 0 if secondary FSIM is being 
 *                                              replicated into primary FSIM
 *                                           !0 if primary FSIM is being 
 *                                              replicated into secondary FSIM
 *     source_inoptr   - input - pointer to an inode in the primary
 *                                       aggregate inode allocation table
 *                                       in an fsck buffer
 *     target_inoptr   - input - pointer to the equivalent inode in
 *                                       the secondary aggregate inode 
 *                                       allocation table in an fsck buffer
 *     replication_failed - input - ptr to a variable in which to return
 *                                          0  if the FSIM is successfully replicated and
 *                                          !0 if the FSIM is not successfully replicated
 *
 * NOTES:  
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t FSIM_replication ( int8         source_is_primary,
                                       dinode_t *target_inoptr,
                                       dinode_t *source_inoptr,
                                       int8        *replication_failed
                                       )
{
  retcode_t fsimr_rc = FSCK_OK;
  xtpage_t *source_nodeptr, *leaf_node;
  int8 source_is_inline, source_is_rootleaf;
  int leaf_length = 0;
  fsblkidx_t node_agg_offset;
  xtree_buf_t *curpage;
  int I_am_logredo = 0;
	 
             /*
              * allocate a buffer to contain the source leaf nodes
              */
  fsimr_rc = temp_node_buf_alloc ( (char **) (&agg_recptr->prim_nodeptr) );
  if( fsimr_rc == FSCK_OK ) {  /* node buffer allocated */
    source_nodeptr = agg_recptr->prim_nodeptr;
    fsimr_rc = find_first_leaf( source_inoptr, 
                                        &leaf_node, &node_agg_offset,
                                        &source_is_inline, &source_is_rootleaf );
    if( fsimr_rc != FSCK_OK ) {
      *replication_failed = -1;
      fsimr_rc = FSCK_OK;
      }
    else {  /* got first leaf of primary fs inode map */
              /*
               * copy it into the temp primary node buffer
               */
      if( source_is_rootleaf) { 
        leaf_length = XTROOTMAXSLOT * sizeof(xad_t);
        memcpy( (void *) source_nodeptr, 
                      (void *) &(source_inoptr->di_btroot), leaf_length );
        }
      else {  /* root is internal */
         leaf_length = XTPAGE_SIZE;
         memcpy( (void *) source_nodeptr, 
                       (void *) leaf_node, leaf_length );
                  /*
                   * Initialize list of xtree append buffers
                   */
         fsimr_rc = alloc_wrksp( sizeof(xtree_buf_t), 
                                          dynstg_xtreebuf, I_am_logredo,
                                          (void **) &fsim_node_pages );
         if( fsimr_rc == FSCK_OK ) {
           fsim_node_pages->down = fsim_node_pages->up = NULL;
           fsim_node_pages->page = (xtpage_t *)&target_inoptr->di_btroot;
           }
         }  /* end else root is internal */
      }  /* end else got first leaf of source fs inode map */
	   
      if( !(*replication_failed) && (fsimr_rc == FSCK_OK) ) {
	 
        if( source_is_rootleaf ) { 
          target_inoptr->di_size = source_inoptr->di_size;
          target_inoptr->di_nblocks = source_inoptr->di_nblocks;
          memcpy( (void *) &(target_inoptr->di_btroot), 
                        (void *) &(source_inoptr->di_btroot), 
                        leaf_length );
          }
        else {  /* source is not a rootleaf */
          target_inoptr->di_size = 0;
          target_inoptr->di_nblocks = 0;
          while( (source_nodeptr != NULL) && (!(*replication_failed)) && 
                    (fsimr_rc == FSCK_OK)  ) {
                  /*
                   * add the extents described in this xtpage to the
                   * target inode
                   */
            fsimr_rc = FSIM_add_extents( source_nodeptr, 
                                                     target_inoptr,
                                                     replication_failed );
            if( (fsimr_rc == FSCK_OK) && !(*replication_failed) ) {   /*
                                          * leaf replicated  
                                          */
              if( source_nodeptr->header.next == 0 ) {
                source_nodeptr = NULL;
                }
              else {  /* source leaf has a next */
                node_agg_offset = source_nodeptr->header.next;
                fsimr_rc = ait_node_get( node_agg_offset, source_nodeptr );
                if( fsimr_rc != FSCK_OK ) {
                  if( fsimr_rc > 0 ) {  /* error but not fatal */
                    *replication_failed = -1;
                    fsimr_rc = FSCK_OK;
                    }  /* end error but not fatal */
                  }  
                }  /* end else source leaf has a next */
              }  /* end leaf replicated */
            }  /* end while */
                /*
                 * If any pages were allocated for the xtree rooted
                 * in the FileSet Inode Map (FSIM) then we need to 
                 * write them to the device
                 */
          curpage = fsim_node_pages;
          while ( (!(curpage->page->header.flag & BT_ROOT))  &&
               (!(*replication_failed))  && (fsimr_rc==FSCK_OK) )  {
            node_agg_offset = addressPXD(&(curpage->page->header.self));
            fsimr_rc = ait_node_put ( node_agg_offset, curpage->page );
             if ( fsimr_rc != 0) {
              *replication_failed = -1;
              }
            else {
              curpage = curpage->up;
              }
            }  /* end while */
          }  /* end else source is not a rootleaf */
        }
    }  /* end first node buffer allocated */
	 
  return( fsimr_rc );
}                          /* end of FSIM_replication ()  */
	 
	 
/****************************************************************
 * NAME: iag_alloc_rebuild
 *
 * FUNCTION: Rebuild the specified Inode Allocation Group (iag) using data
 *           based on fsck's observations of the aggregate.
 *
 * PARAMETERS:
 *     iagnum   - input - ordinal number of the current iag
 *     iagiptr  - input - pointer to a data area describing the
 *                        current iag
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iag_alloc_rebuild (iagno_t                iagnum,
			     fsck_iag_info_ptr      iagiptr
			    )
{
  retcode_t iar_rc = FSCK_OK;
  reg_idx_t agnum;
      /*
       * rebuild IAG's pmap and wmap
       */
  memcpy( (void *) &(iagiptr->iagptr->pmap[0]),      /* to */
	  (void *) &(agg_recptr->amap[0]),           /* from */
	  AMAPSIZE );                                /* number of bytes */
  memcpy( (void *) &(iagiptr->iagptr->wmap[0]),      /* to */
	  (void *) &(agg_recptr->amap[0]),           /* from */
	  AMAPSIZE );                                /* number of bytes */
      /*
       * rebuild the IAG's summary maps
       */
  memcpy( (void *) &(iagiptr->iagptr->extsmap[0]),     /* to */
	  (void *) &(agg_recptr->fextsumm[0]),         /* from */
	  SMAPSIZE );                                  /* number of bytes */
  memcpy( (void *) &(iagiptr->iagptr->inosmap[0]),     /* to */
	  (void *) &(agg_recptr->finosumm[0]),         /* from */
	  SMAPSIZE );                                  /* number of bytes */
      /*
       * reset the IAG's "free counts"
       */
  iagiptr->iagptr->nfreeinos = iagiptr->iagtbl[iagnum].unused_backed_inodes;
  iagiptr->iagptr->nfreeexts = iagiptr->iagtbl[iagnum].unbacked_extents;
      /*
       * add it to the various lists, as appropriate
       *
       * (In the case of doubly linked list, only the forward pointers
       *  are set here.  Resetting back pointers involves I/O and will be
       *  done later.)
       */
  if( iagiptr->iagtbl[iagnum].unbacked_extents == EXTSPERIAG ) {  /*
				 * this iag belongs on the imap free IAG list
				 */
    iagiptr->iagptr->iagfree = iagiptr->iamctlptr->in_freeiag;
    iagiptr->iamctlptr->in_freeiag = iagnum;
    iagiptr->iagptr->extfreefwd = -1;
    iagiptr->iagptr->extfreeback = -1;
    iagiptr->iagptr->inofreefwd = -1;
    iagiptr->iagptr->inofreeback = -1;
    }
  else {  /* this iag does not belong on the imap free IAG list */
    iagiptr->iagptr->iagfree = -1;
    agnum = iagiptr->iagtbl[iagnum].AG_num;
    if( (iagiptr->iagtbl[iagnum].unbacked_extents > 0) &&
	(iagiptr->iagtbl[iagnum].unbacked_extents < EXTSPERIAG) ) {  /*
				   * this iag belongs on the AG free extent list
				   */
      iagiptr->iagptr->extfreefwd = iagiptr->iamctlptr->in_agctl[agnum].extfree;
      iagiptr->iamctlptr->in_agctl[agnum].extfree = iagnum;
      }  /* end this iag belongs on the AG free extent list */
    else {  /* this iag does not belong on the AG free extent list */
      iagiptr->iagptr->extfreefwd = -1;
      iagiptr->iagptr->extfreeback = -1;
      }  /* end else this iag does not belong on the AG free extent list */
    if( iagiptr->iagtbl[iagnum].unused_backed_inodes > 0 ) {  /*
				   * this iag belongs on the AG free inode list
				   */
      iagiptr->iagptr->inofreefwd = iagiptr->iamctlptr->in_agctl[agnum].inofree;
      iagiptr->iamctlptr->in_agctl[agnum].inofree = iagnum;
      }  /* end this iag belongs on the AG free extent list */
    else {  /* this iag does not belong on the AG free inode list */
      iagiptr->iagptr->inofreefwd = -1;
      iagiptr->iagptr->inofreeback = -1;
      }  /* end else this iag does not belong on the AG free inode list */
    }  /* end else this iag does not belong on the imap free IAG list */
  return( iar_rc );
}                              /* end of iag_alloc_rebuild ()  */
	 
	 
/****************************************************************
 * NAME: iag_alloc_scan
 *
 * FUNCTION: Scan data collected by fsck which describes the inodes in 
 *           the reange described by the current iag.  Summarize this
 *           data in a form which will be convenient for use when verifying 
 *           or rebuiding the iag.
 *
 * PARAMETERS:
 *     iag_alloc_inodes  - input - pointer to a variable in which to return
 *                                 a count of inodes which are described
 *                                 by the current iag and are both backed and 
 *                                 in use in the filesystem
 *     iag_alloc_exts    - input - pointer to a variable in which to return
 *                                 a count of the inode extents which are
 *                                 backed and described by the current iag
 *     iagiptr           - input - pointer to a data area describing the
 *                                 current iag
 *     msg_info_ptr      - input - pointer to data needed to issue messages
 *                                 about the current inode allocation map
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iag_alloc_scan (int32                 *iag_alloc_inodes,
			  int32                 *iag_alloc_exts,
			  fsck_iag_info_ptr      iagiptr,
			  fsck_imap_msg_info_ptr msg_info_ptr
			 )
{
  retcode_t ias_rc = FSCK_OK;
  int32    iag_smapidx;
  int32    iag_extidx, smap_extidx;
  int32    inoidx, agg_inoidx;
  int32    ext_alloc_inodes;
  fsck_inode_recptr this_inorecptr;
  fsblkidx_t ext_addr;
  reg_idx_t  ext_length;
  uint32   map_mask = 0x80000000;
  int      alloc_ifnull = 0;
	/*
	 * build maps for this iag
	 */
    iag_extidx = 0;
    *iag_alloc_inodes = 0;
    *iag_alloc_exts = 0;
    agg_inoidx = (iagiptr->iamrecptr->num_iags - 1) * INOSPERIAG;
    for( iag_smapidx = 0;
	 ( (iag_smapidx < SMAPSZ) && (ias_rc == FSCK_OK) );
	 iag_smapidx++ ) {
      for( smap_extidx = 0;
	   ( (smap_extidx < EXTSPERSUM) && (ias_rc == FSCK_OK) );
	   smap_extidx++ ) {
	ext_addr = addressPXD( &(iagiptr->iagptr->inoext[iag_extidx]) );
	if( ext_addr == 0 ) {  /* extent not allocated */
	  agg_recptr->amap[iag_extidx] = 0;    /* no inodes allocated here */
	  agg_recptr->fextsumm[iag_smapidx] &= ~(map_mask >> smap_extidx); /*
		       * turn off the bit
		       */
	  agg_recptr->finosumm[iag_smapidx] |= (map_mask >> smap_extidx); /*
		       * turn on the bit
		       */
	  agg_inoidx += INOSPEREXT;
	  }  /* end extent not allocated */
	else {  /* extent is allocated */
	  (*iag_alloc_exts)++;
	  agg_recptr->fextsumm[iag_smapidx] |= (map_mask >> smap_extidx); /*
		       * turn on the bit
		       */
	  ext_alloc_inodes = 0;
	  for( inoidx = 0;
	       ( (inoidx < INOSPEREXT) && (ias_rc == FSCK_OK) );
	       inoidx++ ) {
	    ias_rc = get_inorecptr( iagiptr->agg_inotbl, alloc_ifnull,
				    agg_inoidx, &this_inorecptr );
	    if( ias_rc == FSCK_OK ) {
	      if( this_inorecptr == NULL ) {  /* backed but not referenced */
		agg_recptr->amap[iag_extidx] &= ~(map_mask >> inoidx); /*
			* turn off the bit
			*/
		}  /* end backed but not referenced */
	      else {  /* either in use or referenced */
		if( (this_inorecptr->in_use)               && 
		    ( (agg_recptr->processing_readonly) ||    
		      (!this_inorecptr->selected_to_rls)  )  ){  /*
		       * inode allocated and won't be released (this session)
		       */
		  agg_recptr->amap[iag_extidx] |= (map_mask >> inoidx); /*
		       * turn on the bit
		       */
		  ext_alloc_inodes++;
		  if( this_inorecptr->inode_type == file_inode ) {
		    agg_recptr->files_in_aggregate ++;
		    }
		  else if( this_inorecptr->inode_type == directory_inode ) {
		    agg_recptr->dirs_in_aggregate ++;
		    }
		  }  /* end inode allocated */
		else {  /* inode backed but not allocated */
		  agg_recptr->amap[iag_extidx] &= ~(map_mask >> inoidx); /*
		       * turn off the bit
		       */
		  }  /* end else inode backed but not allocated */
		}  /* end else either in use or referenced */
	      }
	    agg_inoidx++;
	    }  /* end for inoidx */
	  if( ias_rc == FSCK_OK ) {  /* not bailing out */
	    if( (ext_alloc_inodes == 0) &&
		(agg_recptr->processing_readwrite) ) { /*
		       * we have write access (so we'll be rebuilding
		       * the IAGs, not just verifying them) and none
		       * of the inodes in this extent are allocated.
		       *
		       * Release this inode extent.
		       */
	      agg_recptr->fextsumm[iag_smapidx] &= ~(map_mask >> smap_extidx); /*
		       * turn off the bit because extent slot available
		       */
	      agg_recptr->finosumm[iag_smapidx] |= (map_mask >> smap_extidx); /*
		       * turn on the bit because no inodes available
		       */
	      ext_length = lengthPXD( &(iagiptr->iagptr->inoext[iag_extidx]) );
	      ias_rc = extent_unrecord( ext_addr, (ext_addr + ext_length - 1) );
	      *iag_alloc_exts -= 1;
	      PXDaddress( &(iagiptr->iagptr->inoext[iag_extidx]), 0 );
	      PXDlength( &(iagiptr->iagptr->inoext[iag_extidx]), 0 );
	      }  /* end we have write access and none of the inodes ... */
	    else {  /* read only or at least one inode is allocated */
	      *iag_alloc_inodes += ext_alloc_inodes;
	      if( ext_alloc_inodes == INOSPEREXT ) {  /* all in use */
		agg_recptr->finosumm[iag_smapidx] |= (map_mask >> smap_extidx); /*
		       * turn on the bit
		       */
		}  /* end all in use */
	      else {  /* some backed but not in use */
	       agg_recptr->finosumm[iag_smapidx] &= ~(map_mask >> smap_extidx); /*
		       * turn off the bit
		       */
		}  /* end some backed but not in use */
	      }  /* end else read only or at least one inode is allocated */
	    }  /* end not bailing out */
	  }  /* end else extent is allocated */
	iag_extidx++;
	}  /* end for smap_extidx */
      }  /* end for iag_smapidx */
  return( ias_rc );
}                              /* end of iag_alloc_scan ()  */
	 
	 
/****************************************************************
 * NAME: iag_alloc_ver
 *
 * FUNCTION: Verify that the specified Inode Allocation Group (iag) is 
 *           correct according to fsck's observations of the aggregate.
 *
 * PARAMETERS:
 *     errors_detected  - input - pointer to a variable in which to return
 *                                !0 if errors are detected 
 *                                 0 if no errors are detected
 *     agstrt_is_bad    - input - !0 => the iag cannot be associated with
 *                                      a particular allocation group
 *                                 0 => the iag appears to be associated
 *                                      with a valid allocation group
 *     iagnum           - input - ordinal number of the current iag
 *     iagiptr          - input - pointer to a data area describing the
 *                                current iag
 *     msg_info_ptr     - input - pointer to data needed to issue messages
 *                                about the current inode allocation map 
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iag_alloc_ver (int                   *errors_detected,
			 int                    agstrt_is_bad,
			 iagno_t                iagnum,
			 fsck_iag_info_ptr      iagiptr,
			 fsck_imap_msg_info_ptr msg_info_ptr
			)
{
  retcode_t iav_rc = FSCK_OK;
  int outcome;
  reg_idx_t agnum;
      /*
       * check out the IAG's pmap
       */
  outcome = memcmp( (void *) &(agg_recptr->amap[0]),
		    (void *) &(iagiptr->iagptr->pmap[0]),
		    AMAPSIZE );
  if( outcome != 0 ) {  /* pmaps don't match! */
    *errors_detected = -1;
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_mapowner;
    sprintf( message_parm_1, "%ld", iagnum );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    fsck_send_msg( fsck_BADIAGPMAP, 0, 2 );
    }  /* end pmaps don't match */
      /*
       * check out the IAG's free extent summary map
       */
  outcome = memcmp( (void *) &(agg_recptr->fextsumm[0]),
		    (void *) &(iagiptr->iagptr->extsmap[0]),
		    SMAPSIZE );
  if( outcome != 0 ) {  /* free extent maps don't match! */
    *errors_detected = -1;
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_mapowner;
    sprintf( message_parm_1, "%ld", iagnum );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    fsck_send_msg( fsck_BADIAGFIES, 0, 2 );
    }  /* end free extent maps don't match */
      /*
       * check out the IAG's (extents with) free inodes summary map
       */
  outcome = memcmp( (void *) &(agg_recptr->finosumm[0]),
		    (void *) &(iagiptr->iagptr->inosmap[0]),
		    SMAPSIZE );
  if( outcome != 0 ) {  /* free inode maps don't match! */
    *errors_detected = -1;
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_mapowner;
    sprintf( message_parm_1, "%ld", iagnum );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    fsck_send_msg( fsck_BADIAGFIS, 0, 2 );
    }  /* end free inode maps don't match */
      /*
       * check out the IAG's "free counts"
       */
  if( iagiptr->iagptr->nfreeinos !=
			  iagiptr->iagtbl[iagnum].unused_backed_inodes )  { /*
				 * free inode count is wrong
				 */
    *errors_detected = -1;
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_mapowner;
    sprintf( message_parm_1, "%ld", iagnum );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    fsck_send_msg( fsck_BADIAGNFINO, 0, 2 );
    }  /* end free inode count is wrong */
  if( iagiptr->iagptr->nfreeexts !=
			  iagiptr->iagtbl[iagnum].unbacked_extents )  { /*
				 * free extent count is wrong
				 */
    *errors_detected = -1;
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_mapowner;
    sprintf( message_parm_1, "%ld", iagnum );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    fsck_send_msg( fsck_BADIAGNFEXT, 0, 2 );
    }  /* end free extent count is wrong */
      /*
       * verify *inclusion in* / *exclusion from* the various IAG lists
       */
  if( !iagiptr->iamrecptr->friag_list_bad ) { /*
				 * the list looked good when we scanned it
				 */
    if( iagiptr->iagtbl[iagnum].unbacked_extents == EXTSPERIAG ) {  /*
				 * this iag belongs on the imap free IAG list
				 */
      if( (iagiptr->iagptr->iagfree == -1) &&
	  (iagiptr->iamrecptr->friag_list_last != iagnum) ) { /* not on the list! */
	*errors_detected = -1;
        msgprms[0] = message_parm_0;
        msgprmidx[0] = msg_info_ptr->msg_mapowner;
        msgprms[1] = "2";
        msgprmidx[1] = 0;
        fsck_send_msg( fsck_BADIAGFLIST, 0, 2 );
	}  /* end not on the list! */
      else {  /* it is on the list */
	iagiptr->iamrecptr->friag_list_len--;  /* we've seen one of them */
	}  /* end else it is on the list */
      }  /* end this iag belongs on the imap free IAG list */
    else {  /* this iag does not belong on the imap free IAG list */
      if( (iagiptr->iagptr->iagfree != -1) ||
	  (iagiptr->iamrecptr->friag_list_last == iagnum) ) {  /* on the list! */
	iagiptr->iamrecptr->friag_list_len--;  /* we've seen one of them */
	*errors_detected = -1;
        msgprms[0] = message_parm_0;
        msgprmidx[0] = msg_info_ptr->msg_mapowner;
        msgprms[1] = "3";
        msgprmidx[1] = 0;
        fsck_send_msg( fsck_BADIAGFLIST, 0, 2 );
	}  /* end on the list! */
      }  /* end else this iag does not belong on the imap free IAG list */
    }  /* end the list looked good when we scanned it */
  if( !agstrt_is_bad ) {  /* we can associate an AG with this IAG */
    agnum = iagiptr->iagtbl[iagnum].AG_num;
    if( !iagiptr->agtbl[agnum].frext_list_bad ) { /*
				   * the list looked valid when we scanned it
				   */
      if( (iagiptr->iagtbl[iagnum].unbacked_extents > 0) &&
	  (iagiptr->iagtbl[iagnum].unbacked_extents < EXTSPERIAG) ) {  /*
				   * this iag belongs on the AG free extent list
				   */
	if( (iagiptr->iagptr->extfreefwd == -1)                     &&
	    (iagiptr->iagptr->extfreeback == -1)                    &&
	    ((iagiptr->agtbl[agnum].frext_list_first != iagnum) ||
	     (iagiptr->agtbl[agnum].frext_list_last != iagnum)    )    ) { /*
				   * not on the list!
				   */
	  *errors_detected = -1;
          msgprms[0] = message_parm_0;
          msgprmidx[0] = msg_info_ptr->msg_mapowner;
          sprintf( message_parm_1, "%ld", iagnum );
          msgprms[1] = message_parm_1;
          msgprmidx[1] = 0;
          msgprms[2] = "2";
          msgprmidx[2] = 0;
          fsck_send_msg( fsck_BADAGFELIST, 0, 3 );
	  }  /* end not on the list! */
	else {  /* it is on the list */
	  iagiptr->agtbl[agnum].frext_list_len--;  /* we've seen one of them */
	  }  /* end else it is on the list */
	}  /* end this iag belongs on the AG free extent list */
      else {  /* this iag does not belong on the AG free extent list */
	if( (iagiptr->iagptr->extfreefwd != -1)                     ||
	    (iagiptr->iagptr->extfreeback != -1)                    ||
	    ((iagiptr->agtbl[agnum].frext_list_first == iagnum) &&
	     (iagiptr->agtbl[agnum].frext_list_last == iagnum)    )    ) { /*
				   * on the list!
				   */
	  iagiptr->agtbl[agnum].frext_list_len--;  /* we've seen one of them */
	  *errors_detected = -1;
          msgprms[0] = message_parm_0;
          msgprmidx[0] = msg_info_ptr->msg_mapowner;
          sprintf( message_parm_1, "%d", iagnum );
          msgprms[1] = message_parm_1;
          msgprmidx[1] = 0;
          msgprms[2] = "3";
          msgprmidx[2] = 0;
          fsck_send_msg( fsck_BADAGFELIST, 0, 3 );
	  }  /* end on the list! */
	}  /* end this iag belongs on the AG free extent list */
      }  /* end the list looked valid when we scanned it */
    if( !iagiptr->agtbl[agnum].frino_list_bad ) { /*
				   * the list looked valid when we scanned it
				   */
      if( iagiptr->iagtbl[iagnum].unused_backed_inodes > 0 ) {  /*
				   * this iag belongs on the AG free inode list
				   */
	if( (iagiptr->iagptr->inofreefwd == -1)                     &&
	    (iagiptr->iagptr->inofreeback == -1)                    &&
	    ((iagiptr->agtbl[agnum].frino_list_first != iagnum) ||
	     (iagiptr->agtbl[agnum].frino_list_last != iagnum)    )    ) { /*
				   * not on the list!
				   */
	  *errors_detected = -1;
          msgprms[0] = message_parm_0;
          msgprmidx[0] = msg_info_ptr->msg_mapowner; 
          sprintf( message_parm_1, "%d", iagnum );
          msgprms[1] = message_parm_1;
          msgprmidx[1] = 0;
          msgprms[2] = "3";
          msgprmidx[2] = 0;
          fsck_send_msg( fsck_BADAGFILIST, 0, 3 );
	  }  /* end not on the list! */
	else {  /* it is on the list */
	  iagiptr->agtbl[agnum].frino_list_len--;  /* we've seen one of them */
	  }  /* end else it is on the list */
	}  /* end this iag belongs on the AG free inode list */
      else {  /* this iag does not belong on the AG free inode list */
	if( (iagiptr->iagptr->inofreefwd != -1)                     ||
	    (iagiptr->iagptr->inofreeback != -1)                    ||
	    ((iagiptr->agtbl[agnum].frino_list_first == iagnum) &&
	     (iagiptr->agtbl[agnum].frino_list_last == iagnum)    )    ) { /*
				   * on the list!
				   */
	  iagiptr->agtbl[agnum].frino_list_len--;  /* we've seen one of them */
	  *errors_detected = -1;
          msgprms[0] = message_parm_0;
          msgprmidx[0] = msg_info_ptr->msg_mapowner;
          sprintf( message_parm_1, "%ld", iagnum );
          msgprms[1] = message_parm_1;
          msgprmidx[1] = 0;
          msgprms[2] = "3";
          msgprmidx[2] = 0;
          fsck_send_msg( fsck_BADAGFILIST, 0, 3 );
	  }  /* end on the list! */
	}  /* end this iag belongs on the AG free extent list */
      }  /* end the list looked valid when we scanned it */
    }  /* end we can associate an AG with this IAG */
  return( iav_rc );
}                              /* end of iag_alloc_ver ()  */
	 
	 
/****************************************************************
 * NAME: iagfr_list_scan
 *
 * FUNCTION:  Scan the iag list for the specified Inode Allocation Map.
 *            Count the number of iags on the list.  Validate the list
 *            structure.
 *
 * PARAMETERS:
 *     is_aggregate     - input - !0 => aggregate owned
 *                                 0 => fileset owned
 *     which_it         - input - ordinal number of the aggregate inode 
 *                                describing the inode table
 *     which_ait        - input - the aggregate inode table from which
 *                                the it inode should be read
 *                                { fsck_primary | fsck_secondary }
 *     iagiptr          - input - pointer to a data area describing the
 *                                current iag
 *     errors_detected  - input - pointer to a variable in which to return
 *                                !0 if errors are detected 
 *                                 0 if no errors are detected
 *     msg_info_ptr     - input - pointer to data needed to issue messages
 *                                about the current inode allocation map
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iagfr_list_scan( int                    is_aggregate,
			   int                    which_it,
			   int                    which_ait,
			   fsck_iag_info_ptr      iagiptr,
			   int                   *errors_detected,
			   fsck_imap_msg_info_ptr msg_info_ptr
			  )
{
  retcode_t ifls_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  iagno_t this_iagno;
  int32   end_of_list = -1;
  iagiptr->iamrecptr->friag_list_last = end_of_list;
  iagiptr->iamrecptr->friag_list_len = 0;
  this_iagno = iagiptr->iamctlptr->in_freeiag;
  while( (this_iagno != end_of_list)  &&
	 (!iagiptr->iamrecptr->friag_list_bad) &&
	 (ifls_rc == FSCK_OK)   ) {
    intermed_rc = iag_get( is_aggregate, which_it, which_ait,
			   this_iagno, &(iagiptr->iagptr) );
	   /*
	    * we consider an error here to be an error in the chain.  If
	    * it's really something more serious it will come up again
	    * when we go through all allocated iag's sequentially.
	    */
    if( intermed_rc != FSCK_OK ) {
      iagiptr->iamrecptr->friag_list_bad = -1;
      }
    else {  /* got the iag */
      iagiptr->iamrecptr->friag_list_last = this_iagno;  /* in case it's last */
      iagiptr->iamrecptr->friag_list_len++;       /* increment the counter */
      this_iagno = iagiptr->iagptr->iagfree;
      }  /* end else got the iag */
    }  /* end free while not end of list and no errors detected */
  if( iagiptr->iamrecptr->friag_list_bad ) {  /* found a problem */
    agg_recptr->ag_dirty = 1;
    *errors_detected = -1;
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_mapowner;
    msgprms[1] = "1";
    msgprmidx[1] = 0;
    fsck_send_msg( fsck_BADIAGFLIST, 0, 2 );
    }
  return( ifls_rc );
}                              /* end of iagfr_list_scan ()  */
	 
	 
/****************************************************************
 * NAME: iagfr_list_validation
 *
 * FUNCTION:  Compare the results of the Inode Allocation Map free iag 
 *            list scan with the results of validating the iags.  If
 *            the number of iags seen on the list during list scan does
 *            not match the number of iags which appear to be on the 
 *            list (i.e., which have a non-initialized value in their
 *            forward pointer) as seen during iag validation, then the 
 *            list is not structurally consistent.
 *
 * PARAMETERS:
 *     errors_detected  - input - pointer to a variable in which to return
 *                                !0 if errors are detected 
 *                                 0 if no errors are detected
 *     iamrecptr        - input - pointer to a data area which describes the
 *                                current inode allocation map
 *     msg_info_ptr     - input - pointer to data needed to issue messages
 *                                about the current inode allocation map
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iagfr_list_validation( int                   *errors_detected,
				 fsck_iamrec_t         *iamrecptr,
				 fsck_imap_msg_info_ptr msg_info_ptr
				)
{
  retcode_t iflv_rc = FSCK_OK;
  if( iamrecptr->friag_list_len > 0 ) { /*
		   * fsck observed fewer iag's which belong on the free iag
		   * list than it counted when it scanned the free iag list.
		   * (fsck has already issued messages about these iag's)
		   */
    *errors_detected = -1;
    agg_recptr->ag_dirty = 1;
    }  /* end fsck observed fewer iag's which belong ... */
  else if( iamrecptr->friag_list_len < 0 ) { /*
		   * fsck observed more iag's which belong on the free iag
		   * list and which appear to be on the list than it counted
		   * when it scanned the free iag list.  So the chain has
		   * somehow lost some of its links.
		   */
    *errors_detected = -1;
    agg_recptr->ag_dirty = 1;
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_mapowner;
    fsck_send_msg( fsck_BADIAGFL1, 0, 1 );
    }  /* end fsck observed more iag's which belong ... */
  return( iflv_rc );
}                              /* end of iagfr_list_validation ()  */
	 
	 
/****************************************************************
 * NAME: iags_finish_lists
 *
 * FUNCTION:  Complete the rebuild process for the Allocation Group (ag)
 *            free extent lists and free inode lists.  Specifically,
 *            for each ag, set the back pointer in each iag on its
 *            free extent list and in each iag on its free inode list.
 *
 * PARAMETERS:
 *     is_aggregate  - input - !0 => aggregate owned
 *                              0 => fileset owned
 *     which_it      - input - ordinal number of the aggregate inode 
 *                             describing the inode table
 *     which_ait     - input - the aggregate inode table from which
 *                             the it inode should be read
 *                             { fsck_primary | fsck_secondary }
 *     iagiptr       - input - pointer to a data area describing the
 *                             current iag
 *
 * NOTES: o When rebuilding the ag free extent lists and the free inode
 *          lists, it is very inconvenient to establish both forward and
 *          backward pointers.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iags_finish_lists( int                    is_aggregate,
			     int                    which_it,
			     int                    which_ait,
			     fsck_iag_info_ptr      iagiptr
			    )
{
  retcode_t iifl_rc = FSCK_OK;
  iagno_t    prev_iag, this_iag;
  reg_idx_t  agidx;
   /*
    * first set the back pointers in the free inode lists
    */
  for( agidx= 0;
       ( (agidx < MAXAG) && (iifl_rc == FSCK_OK) );
       agidx++ ) {
    prev_iag = -1;
    this_iag = iagiptr->iamctlptr->in_agctl[agidx].inofree;
    while( (this_iag != -1) && (iifl_rc == FSCK_OK) ) {
      iifl_rc = iag_get( is_aggregate, which_it, which_ait,
			 this_iag, &(iagiptr->iagptr) );
      if( iifl_rc == FSCK_OK ) {  /* got the iag */
	iagiptr->iagptr->inofreeback = prev_iag;
	prev_iag = this_iag;
	this_iag = iagiptr->iagptr->inofreefwd;
	iifl_rc = iag_put( iagiptr->iagptr );
	}  /* end got the iag */
      }  /* end while */
    }  /* end for agidx */
   /*
    * now set the back pointers in the free extent lists
    */
  for( agidx= 0;
       ( (agidx < MAXAG) && (iifl_rc == FSCK_OK) );
       agidx++ ) {
    prev_iag = -1;
    this_iag = iagiptr->iamctlptr->in_agctl[agidx].extfree;
    while( (this_iag != -1) && (iifl_rc == FSCK_OK) ) {
      iifl_rc = iag_get( is_aggregate, which_it, which_ait,
			 this_iag, &(iagiptr->iagptr) );
      if( iifl_rc == FSCK_OK ) {  /* got the iag */
	iagiptr->iagptr->extfreeback = prev_iag;
	prev_iag = this_iag;
	this_iag = iagiptr->iagptr->extfreefwd;
	iifl_rc = iag_put( iagiptr->iagptr );
	}  /* end got the iag */
      }  /* end while */
    }  /* end for agidx */
  return( iifl_rc );
}                              /* end of iags_finish_lists ()  */
	 
	 
/****************************************************************
 * NAME: iags_rebuild
 *
 * FUNCTION: Rebuild each Inode Allocation Group (iag) in the specified 
 *           Inode Allocation Map using data based on fsck's observations
 *           of the aggregate.
 *
 * PARAMETERS:
 *     is_aggregate  - input - !0 => aggregate owned
 *                              0 => fileset owned
 *     which_it      - input - ordinal number of the aggregate inode 
 *                             describing the inode table
 *     which_ait     - input - the aggregate inode table from which
 *                             the it inode should be read
 *                             { fsck_primary | fsck_secondary }
 *     iagiptr       - input - pointer to a data area describing the
 *                             current iag
 *     msg_info_ptr  - input - pointer to data needed to issue messages
 *                             about the current inode allocation map
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iags_rebuild( int                    is_aggregate,
			int                    which_it,
			int                    which_ait,
			fsck_iag_info_ptr      iagiptr,
			fsck_imap_msg_info_ptr msg_info_ptr
		       )
{
  retcode_t iir_rc = FSCK_OK;
  iagno_t  iag_idx = 0;
  int64    this_ag;
  int32    iag_alloc_inodes, iag_alloc_exts;
  iir_rc = iag_get_first( is_aggregate, which_it, which_ait, &(iagiptr->iagptr) );
  iagiptr->iamrecptr->num_iags = 0;
  iagiptr->iamrecptr->unused_bkd_inodes = 0;
  iagiptr->iamrecptr->bkd_inodes = 0;
  while( (iagiptr->iagptr != NULL) && (iir_rc == FSCK_OK) ) {
	
    iagiptr->iamrecptr->num_iags++;  /* increment the iag count for this imap */
    iagiptr->iagptr->iagnum = iag_idx;
    this_ag = iagiptr->iagptr->agstart / ((int64) sb_ptr->s_agsize);
    if( iagiptr->iagptr->agstart != (this_ag * sb_ptr->s_agsize) ) { /*
		       * not a valid starting block for an AG
		       */
      this_ag = 0;                     /* default it to AG 0 */
      iagiptr->iagptr->agstart = 0;
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_mapowner;
      sprintf( message_parm_1, "%ld", iag_idx );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      fsck_send_msg( fsck_BADIAGAGSTRTCRCTD, 0, 2 );
      }
    else if( this_ag > MAXAG ) {  /* not a valid starting block for an AG */
      this_ag = 0;                     /* default it to AG 0 */
      iagiptr->iagptr->agstart = 0;
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_mapowner;
      sprintf( message_parm_1, "%ld", iag_idx );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", this_ag );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_BADIAGAGCRCTD, 0, 3 );
      }  /* end not a valid starting block for an AG */
	/*
	 * count allocations and build maps for this iag
	 */
    iir_rc = iag_alloc_scan( &iag_alloc_inodes, &iag_alloc_exts,
			       iagiptr, msg_info_ptr );
    if( iir_rc == FSCK_OK ) {  /* nothing fatal so far */
	  /*
	   * record the info in the iag record
	   */
      iagiptr->iagtbl[iag_idx].backed_inodes = iag_alloc_exts * INOSPEREXT;
      iagiptr->iagtbl[iag_idx].unused_backed_inodes =
			      (iag_alloc_exts * INOSPEREXT) - iag_alloc_inodes;
      iagiptr->iagtbl[iag_idx].unbacked_extents = EXTSPERIAG - iag_alloc_exts;
	  /*
	   * update the workspace with the AG-related info.
	   */
      iagiptr->iagtbl[iag_idx].AG_num = this_ag;
      iagiptr->agtbl[this_ag].backed_inodes +=
				 iagiptr->iagtbl[iag_idx].backed_inodes;
      iagiptr->agtbl[this_ag].unused_backed_inodes +=
				 iagiptr->iagtbl[iag_idx].unused_backed_inodes;
	  /*
	   * add info for this iag to imap totals
	   */
      iagiptr->iamrecptr->unused_bkd_inodes +=
				 iagiptr->iagtbl[iag_idx].unused_backed_inodes;
      iagiptr->iamrecptr->bkd_inodes += iagiptr->iagtbl[iag_idx].backed_inodes;
	/*
	 * rebuild the allocation maps in the iag
	 */
      iir_rc = iag_alloc_rebuild( iag_idx, iagiptr );
      if( iir_rc == FSCK_OK ) {  /* nothing unexpected */
	  /*
	   * put this iag and get the next one
	   */
	iir_rc = iag_put( iagiptr->iagptr );
	if( iir_rc == FSCK_OK ) {  /* put was successful */
	  iir_rc = iag_get_next( &(iagiptr->iagptr) );
	  iag_idx++;                        /* increment for next iag */
	  }  /* end put was successful */
	}  /* end nothing unexpected */
      }  /* end nothing fatal so far */
    }  /* end while iagptr != NULL */
  return( iir_rc );
}                              /* end of iags_rebuild ()  */
	 
 	 
/****************************************************************
 * NAME: iags_validation
 *
 * FUNCTION: Validate each Inode Allocation Group (iag) in the specified 
 *           Inode Allocation Map based on fsck's observations of the 
 *           aggregate.
 *
 * PARAMETERS:
 *     is_aggregate     - input - !0 => aggregate owned
 *                                 0 => fileset owned
 *     which_it         - input - ordinal number of the aggregate inode 
 *                                describing the inode table
 *     which_ait        - input - the aggregate inode table from which
 *                                the it inode should be read
 *                                { fsck_primary | fsck_secondary }
 *     errors_detected  - input - pointer to a variable in which to return
 *                                !0 if errors are detected 
 *                                 0 if no errors are detected
 *     iagiptr          - input - pointer to a data area describing the
 *                                current iag
 *     msg_info_ptr     - input - pointer to data needed to issue messages
 *                                about the current inode allocation map
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iags_validation( int                    is_aggregate,
			   int                    which_it,
			   int                    which_ait,
			   int                   *errors_detected,
			   fsck_iag_info_ptr      iagiptr,
			   fsck_imap_msg_info_ptr msg_info_ptr
			  )
{
  retcode_t iiv_rc = FSCK_OK;
  iagno_t  iag_idx = 0;
  int64    this_ag;
  int      errors_in_iag = 0;
  int      bad_agstrt_in_iag = 0;
  int32    iag_alloc_inodes, iag_alloc_exts;
  reg_idx_t old_iam_bkd_inodes;

  iiv_rc = iag_get_first( is_aggregate, which_it, which_ait, &(iagiptr->iagptr) );
  iagiptr->iamrecptr->num_iags = 0;
  iagiptr->iamrecptr->unused_bkd_inodes = 0;
  old_iam_bkd_inodes = iagiptr->iamrecptr->bkd_inodes;
  iagiptr->iamrecptr->bkd_inodes = 0;
  while( (iagiptr->iagptr != NULL) && (iiv_rc == FSCK_OK) ) {
	
    iagiptr->iamrecptr->num_iags++;  /* increment the iag count for this imap */
    if( iagiptr->iagptr->iagnum != iag_idx ) {  /* iag number is wrong */
      errors_in_iag = -1;
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_mapowner;
      sprintf( message_parm_1, "%ld", iag_idx );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      fsck_send_msg( fsck_BADIAGIAGNUM, 0, 2 );
      }  /* end iag number is wrong */
    this_ag = iagiptr->iagptr->agstart / ((int64) sb_ptr->s_agsize);
    if( iagiptr->iagptr->agstart != (this_ag * sb_ptr->s_agsize) ) { /*
		       * not a valid starting block for an AG
		       */
      bad_agstrt_in_iag = -1;
      errors_in_iag = -1;
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_mapowner;
      sprintf( message_parm_1, "%ld", iag_idx );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      fsck_send_msg( fsck_BADIAGAGSTRT, 0, 2 );
      }
    else if( this_ag > MAXAG ) {  /* not a valid starting block for an AG */
      bad_agstrt_in_iag = -1;
      errors_in_iag = -1;
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_mapowner;
      sprintf( message_parm_1, "%ld", iag_idx );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", this_ag );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_BADIAGAG, 0, 3 );
      }  /* end not a valid starting block for an AG */
	/*
	 * count allocations and build maps for this iag
	 */
    if( iiv_rc == FSCK_OK ) {
      iiv_rc = iag_alloc_scan( &iag_alloc_inodes, &iag_alloc_exts,
			       iagiptr, msg_info_ptr );
      }
    if( iiv_rc == FSCK_OK ) {  /* nothing fatal so far */
	  /*
	   * record the info in the iag record
	   */
      iagiptr->iagtbl[iag_idx].backed_inodes = iag_alloc_exts * INOSPEREXT;
      iagiptr->iagtbl[iag_idx].unused_backed_inodes =
			      (iag_alloc_exts * INOSPEREXT) - iag_alloc_inodes;
      iagiptr->iagtbl[iag_idx].unbacked_extents = EXTSPERIAG - iag_alloc_exts;
	  /*
	   * if we can associate an AG with the IAG then update the workspace
	   * with the AG-related info.
	   */
      if( !bad_agstrt_in_iag ) {  /* we have an AG */
	iagiptr->iagtbl[iag_idx].AG_num = this_ag;
	iagiptr->agtbl[this_ag].backed_inodes +=
				 iagiptr->iagtbl[iag_idx].backed_inodes;
	iagiptr->agtbl[this_ag].unused_backed_inodes +=
				 iagiptr->iagtbl[iag_idx].unused_backed_inodes;
	}  /* end we have an AG */
	  /*
	   * add info for this iag to imap totals
	   */
      iagiptr->iamrecptr->unused_bkd_inodes +=
				 iagiptr->iagtbl[iag_idx].unused_backed_inodes;
      iagiptr->iamrecptr->bkd_inodes += iagiptr->iagtbl[iag_idx].backed_inodes;
	/*
	 * verify the allocation maps in this iag
	 */
      iiv_rc = iag_alloc_ver( &errors_in_iag, bad_agstrt_in_iag,
			      iag_idx, iagiptr, msg_info_ptr );
      if( iiv_rc == FSCK_OK ) {  /* nothing unexpected */
	  /*
	   * sum it up for this iag and get the next one
	   */
	if( errors_in_iag ) {  /* found some thing(s) wrong! */
	  agg_recptr->ag_dirty = 1;
	  *errors_detected = -1;
          msgprms[0] = message_parm_0;
          msgprmidx[0] = msg_info_ptr->msg_mapowner;
          sprintf( message_parm_1, "%d", iag_idx );
          msgprms[1] = message_parm_1;
          msgprmidx[1] = 0;
          fsck_send_msg( fsck_BADIAG, 0, 2 );
	  }  /* end found some thing(s) wrong! */
	iiv_rc = iag_get_next( &(iagiptr->iagptr) );
	iag_idx++;                        /* increment for next iag */
	errors_in_iag = 0;                /* reset for next iag */
	}  /* end nothing unexpected */
      }  /* end nothing fatal so far */
    }  /* end while iagptr != NULL */
  if( (old_iam_bkd_inodes != iagiptr->iamrecptr->bkd_inodes) &&
      (iiv_rc == FSCK_OK) ) {  /*
		       * we got no fatal errors but we came up with a
		       * different count of backed inodes on this pass than
		       * when we went through recording and dupchecking the
		       * extents.
		       */
    iiv_rc = FSCK_INTERNAL_ERROR_12;
    }  /* end we got no fatal errors but we came up with a different ... */
  return( iiv_rc );
}                              /* end of iags_validation ()  */
	 
	 
/****************************************************************
 * NAME: iamap_rebuild
 *
 * FUNCTION:  Rebuild the Inode Allocation Map which describes the specified
 *            Inode Allocation Table using data based on fsck's observations
 *            of the aggregate.
 *
 * PARAMETERS:
 *     is_aggregate  - input - !0 => aggregate owned
 *                              0 => fileset owned
 *     which_it      - input - ordinal number of the aggregate inode 
 *                             describing the inode table
 *     which_ait     - input - the aggregate inode table from which
 *                             the it inode should be read
 *                             { fsck_primary | fsck_secondary }
 *     iagiptr       - input - pointer to a data area describing the
 *                             current iag
 *     msg_info_ptr  - input - pointer to data needed to issue messages
 *                             about the current inode allocation map
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iamap_rebuild ( int                     is_aggregate,
			  int                     which_it,
			  int                     which_ait,
			  fsck_iag_info_ptr       iagiptr,
			  fsck_imap_msg_info_ptr  msg_info_ptr
			 )
{
  retcode_t iamr_rc = FSCK_OK;
  int agidx;
     /*
      * initialize the imap lists and counters
      */
  iagiptr->iamctlptr->in_freeiag = -1;
  for (agidx = 0; (agidx < MAXAG); agidx++ ) {
    iagiptr->iamctlptr->in_agctl[agidx].inofree = -1;
    iagiptr->iamctlptr->in_agctl[agidx].extfree = -1;
    iagiptr->iamctlptr->in_agctl[agidx].numinos = 0;
    iagiptr->iamctlptr->in_agctl[agidx].numfree = 0;
    }  /* end for agidx */
     /*
      * set blocks per inode extent fields
      */
  iagiptr->iamctlptr->in_nbperiext = INODE_EXTENT_SIZE / sb_ptr->s_bsize;
  iagiptr->iamctlptr->in_l2nbperiext =
				   log2shift(iagiptr->iamctlptr->in_nbperiext);
     /*
      * rebuild the IAGs and collect info to finish the iamap
      */
  iamr_rc = iags_rebuild( is_aggregate, which_it, which_ait, iagiptr,
			  msg_info_ptr );
  if( iamr_rc == FSCK_OK ) {  /* things are going well */
       /*
	* fill in those pesky IAG list back pointers
	*/
    iamr_rc = iags_finish_lists( is_aggregate, which_it, which_ait, iagiptr );
    if( iamr_rc == FSCK_OK ) { /* not rushing to exit fsck */
	 /*
	  * finish up the control page info and put the control page
	  */
      iagiptr->iamctlptr->in_nextiag = iagiptr->iamrecptr->num_iags;
      iagiptr->iamctlptr->in_numinos = iagiptr->iamrecptr->bkd_inodes;
      iagiptr->iamctlptr->in_numfree = iagiptr->iamrecptr->unused_bkd_inodes;
      for (agidx = 0; (agidx < MAXAG); agidx++ ) {
	iagiptr->iamctlptr->in_agctl[agidx].numinos =
				    iagiptr->agtbl[agidx].backed_inodes;
	iagiptr->iamctlptr->in_agctl[agidx].numfree =
				    iagiptr->agtbl[agidx].unused_backed_inodes;
	}  /* end for agidx */
      iamr_rc = inotbl_put_ctl_page( is_aggregate, iagiptr->iamctlptr );
      }  /* end not rushing to exit fsck */
    }  /* end things are going well */
  return( iamr_rc );
}                              /* end of iamap_rebuild ()  */
	 
	 
/****************************************************************
 * NAME: iamap_validation
 *
 * FUNCTION:  Validate the Inode Allocation Map which describes the specified
 *            Inode Allocation Table based on fsck's observations of the 
 *            aggregate.
 *
 * PARAMETERS:
 *     is_aggregate  - input - !0 => aggregate owned
 *                              0 => fileset owned
 *     which_it      - input - ordinal number of the aggregate inode 
 *                             describing the inode table
 *     which_ait     - input - the aggregate inode table from which
 *                             the it inode should be read
 *                             { fsck_primary | fsck_secondary }
 *     iagiptr       - input - pointer to a data area describing the
 *                             current iag
 *     msg_info_ptr  - input - pointer to data needed to issue messages
 *                             about the current inode allocation map
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iamap_validation ( int                     is_aggregate,
			     int                     which_it,
			     int                     which_ait,
			     fsck_iag_info_ptr       iagiptr,
			     fsck_imap_msg_info_ptr  msg_info_ptr
			    )
{
  retcode_t iamv_rc = FSCK_OK;
  int imapctl_errors_detected = 0;
  int imap_errors_detected = 0;
  int32 blksperinoext;
  int agidx;

  blksperinoext = INODE_EXTENT_SIZE / sb_ptr->s_bsize;
  if( iagiptr->iamctlptr->in_nbperiext != blksperinoext ) {
    imapctl_errors_detected = -1;
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_mapowner;
    fsck_send_msg( fsck_BADIAMBPIE, 0, 1 );
    }
  if( iagiptr->iamctlptr->in_l2nbperiext != (log2shift(blksperinoext)) ) {
    imapctl_errors_detected = -1;
    msgprms[0] = message_parm_0;
    msgprmidx[0] = msg_info_ptr->msg_mapowner;
    fsck_send_msg( fsck_BADIAML2BPIE, 0, 1 );
    }
  iamv_rc = iagfr_list_scan( is_aggregate, which_it, which_ait, iagiptr,
			     &imapctl_errors_detected, msg_info_ptr );
  if( iamv_rc == FSCK_OK ) {
    iamv_rc = agfrino_lists_scan( is_aggregate, which_it, which_ait, iagiptr,
				  &imap_errors_detected, msg_info_ptr );
    if( iamv_rc == FSCK_OK ) {
      iamv_rc = agfrext_lists_scan( is_aggregate, which_it, which_ait, iagiptr,
				    &imap_errors_detected, msg_info_ptr );
      }
    }
  if( iamv_rc == FSCK_OK ) {
    iamv_rc = iags_validation ( is_aggregate, which_it, which_ait,
				&imap_errors_detected, iagiptr,
				msg_info_ptr );
    if( iamv_rc == FSCK_OK ) {
      if( !iagiptr->iamrecptr->friag_list_bad ) {  /* not already judged invalid */
	iamv_rc = iagfr_list_validation( &imapctl_errors_detected,
					 iagiptr->iamrecptr, msg_info_ptr );
	}  /* end not already judged invalid */
      if( iamv_rc == FSCK_OK ) {
	iamv_rc = agfrino_lists_validation( is_aggregate, which_it,
					    iagiptr->agtbl,
					    &imap_errors_detected,
					    msg_info_ptr );
	if( iamv_rc == FSCK_OK ) {
	  iamv_rc = agfrext_lists_validation( is_aggregate, which_it,
					      iagiptr->agtbl,
					      &imap_errors_detected,
					      msg_info_ptr );
	  }
	}
      }
    }
  if( iamv_rc == FSCK_OK ) {  /* not rushing to exit fsck */
	/*
	 * finish up verifying the control page info
	 */
    if( iagiptr->iamctlptr->in_nextiag != iagiptr->iamrecptr->num_iags ) {
      imapctl_errors_detected = -1;
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_mapowner;
      fsck_send_msg( fsck_BADIAMNXTIAG, 0, 1 );
      }
    if( iagiptr->iamctlptr->in_numinos != iagiptr->iamrecptr->bkd_inodes ) {
      imapctl_errors_detected = -1;
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_mapowner;
      fsck_send_msg( fsck_BADIAMNBI, 0, 1 );
      }
    if( iagiptr->iamctlptr->in_numfree !=
				     iagiptr->iamrecptr->unused_bkd_inodes ) {
      imapctl_errors_detected = -1;
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_mapowner;
      fsck_send_msg( fsck_BADIAMNFI, 0, 1 );
      }
    for (agidx = 0; (agidx < MAXAG); agidx++ ) {
      if( iagiptr->iamctlptr->in_agctl[agidx].numinos !=
				  iagiptr->agtbl[agidx].backed_inodes ) {
	imapctl_errors_detected = -1;
        msgprms[0] = message_parm_0;
        msgprmidx[0] = msg_info_ptr->msg_mapowner;
        sprintf( message_parm_1, "%ld", agidx );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        fsck_send_msg( fsck_BADIAMAGNBI, 0, 2 );
	}
      if( iagiptr->iamctlptr->in_agctl[agidx].numfree !=
				  iagiptr->agtbl[agidx].unused_backed_inodes ) {
	imapctl_errors_detected = -1;
        msgprms[0] = message_parm_0;
        msgprmidx[0] = msg_info_ptr->msg_mapowner;
        sprintf( message_parm_1, "%ld", agidx );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        fsck_send_msg( fsck_BADIAMAGNFI, 0, 2 );
	}
      }  /* end for agidx */
	/*
	 * if errors have been detected, issue summary message(s) in
	 * case we're not processing verbose.
	 */
    if( imapctl_errors_detected ) {
      agg_recptr->ag_dirty = 1;
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_mapowner;
      fsck_send_msg( fsck_BADIAMCTL, 0, 1 );
      }
    if( imap_errors_detected ) {
      agg_recptr->ag_dirty = 1;
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_mapowner;
      fsck_send_msg( fsck_BADIAM, 0, 1 );
      }
    }  /* end not rushing to exit fsck */

  return( iamv_rc );
}                              /* end of iamap_validation ()  */
	 	 
	 
/****************************************************************
 * NAME: IM_compare_leaf
 *
 * FUNCTION: Compare the 2 specified Inode Map B+Tree leaf  
 *                nodes to ensure that they are logically equivalent.
 *
 * PARAMETERS:
 *     primary_leafptr          - input - pointer to a leaf node in an inode 
 *                                                 in the primary aggregate inode 
 *                                                 allocation table in an fsck buffer
 *     secondary_leafptr        - input - pointer to a leaf node in an inode 
 *                                                   in the secondary aggregate inode 
 *                                                   allocation table in an fsck buffer
 *     inconsistency_detected  - input - pointer to a variable in which 
 *                                                    to return !0 if errors are detected 
 *                                                                  0 if no errors are detected
 *
 * NOTES:  
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t IM_compare_leaf ( xtpage_t *leaf1ptr,
                                        xtpage_t *leaf2ptr,
                                        int      *inconsistency_detected
                                       )
{
  retcode_t imcl_rc = FSCK_OK;
  int16 entry_idx;
  int64 lf1_offset, lf2_offset;
  int64 lf1_addr, lf2_addr;
  int32 lf1_len, lf2_len;
  int32 result;
	 
  if( (leaf1ptr->header.flag != leaf2ptr->header.flag)                 ||
      (leaf1ptr->header.nextindex != leaf2ptr->header.nextindex) ||
      (leaf1ptr->header.maxentry != leaf2ptr->header.maxentry )    ) {
    *inconsistency_detected = -1;
    }
	 
  entry_idx = XTENTRYSTART;
  while( (imcl_rc == FSCK_OK) && (!(*inconsistency_detected)) &&
            (entry_idx < leaf1ptr->header.nextindex)                       ) {
    lf1_offset = offsetXAD( &(leaf1ptr->xad[entry_idx]) );
    lf2_offset = offsetXAD( &(leaf2ptr->xad[entry_idx]) );
    if( lf1_offset != lf2_offset ) {
      *inconsistency_detected = -1;
      }
    else {  /* offsets match */
      lf1_addr = addressXAD( &(leaf1ptr->xad[entry_idx]) );
      lf2_addr = addressXAD( &(leaf2ptr->xad[entry_idx]) );
      if( lf1_addr != lf2_addr ) {
        *inconsistency_detected = -1;
        }
      else {  /* addresses match */
        lf1_len = lengthXAD( &(leaf1ptr->xad[entry_idx]) );
        lf2_len = lengthXAD( &(leaf2ptr->xad[entry_idx]) );
        if( lf1_len != lf2_len ) {
          *inconsistency_detected = -1;
          }
        else {  /* lengths match */
          entry_idx++;
          }  /* end else lengths match */
        }  /* end else addresses match */
      }  /* end else offsets match */
    }  /* end while */
	 
  return( imcl_rc );
}                          /* end of IM_compare_leaf()  */
	 
	 
/****************************************************************
 * NAME: rebuild_agg_iamap
 *
 * FUNCTION:  Rebuild the Aggregate Inode Allocation Map using data based
 *                 on fsck's observations of the aggregate.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t rebuild_agg_iamap ()
{
  retcode_t raiam_rc = FSCK_OK;
  int is_aggregate = -1;
  int which_it = 0;
  struct fsck_imap_msg_info imap_msg_info;
  fsck_imap_msg_info_ptr msg_info_ptr;
  struct fsck_iag_info iag_info;
  fsck_iag_info_ptr iag_info_ptr;
  if( agg_recptr->primary_ait_4part1 ) {
    which_it = fsck_primary;
    }
  else {
    which_it = fsck_secondary;
    }
  iag_info_ptr = &iag_info;
  iag_info_ptr->iamrecptr = &(agg_recptr->agg_imap);
  iag_info_ptr->iagtbl = agg_recptr->agg_imap.iag_tbl;
  iag_info_ptr->agtbl = agg_recptr->agg_imap.ag_tbl;
  iag_info_ptr->agg_inotbl = is_aggregate;
  msg_info_ptr = &(imap_msg_info);
  msg_info_ptr->msg_mapowner = fsck_aggregate;
  raiam_rc = inotbl_get_ctl_page( is_aggregate, &(iag_info_ptr->iamctlptr) );
  if( raiam_rc == FSCK_OK )  {  /* got the imap control page */
    raiam_rc = iamap_rebuild( is_aggregate, which_it, which_it,
			      iag_info_ptr, msg_info_ptr );
    }  /* end got the imap control page */
  return( raiam_rc );
}                              /* end of rebuild_agg_iamap ()  */
	 
	 
/****************************************************************
 * NAME: rebuild_fs_iamaps
 *
 * FUNCTION:  Rebuild the Fileset Inode Allocation Map using data based
 *            on fsck's observations of the aggregate.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t rebuild_fs_iamaps ()
{
  retcode_t rfsiam_rc = FSCK_OK;
  int is_aggregate = 0;
  int which_it = FILESYSTEM_I;     /* release 1 there is exactly 1 fileset */
  int which_ait;
  struct fsck_imap_msg_info imap_msg_info;
  fsck_imap_msg_info_ptr    msg_info_ptr;
  struct fsck_iag_info iag_info;
  fsck_iag_info_ptr iag_info_ptr;
  if( agg_recptr->primary_ait_4part2 ) {
    which_ait = fsck_primary;
    }
  else {
    which_ait = fsck_secondary;
    }
  iag_info_ptr = &iag_info;
  iag_info_ptr->iamrecptr = &(agg_recptr->fset_imap);
  iag_info_ptr->iagtbl = agg_recptr->fset_imap.iag_tbl;
  iag_info_ptr->agtbl = agg_recptr->fset_imap.ag_tbl;
  iag_info_ptr->agg_inotbl = is_aggregate;
  msg_info_ptr = &(imap_msg_info);
  msg_info_ptr->msg_mapowner = fsck_fileset;
  rfsiam_rc = inotbl_get_ctl_page( is_aggregate, &(iag_info_ptr->iamctlptr) );
  if( rfsiam_rc == FSCK_OK )  {  /* got the imap control page */
    rfsiam_rc = iamap_rebuild( is_aggregate, which_it, which_ait,
			       iag_info_ptr, msg_info_ptr );
    }  /* end got the imap control page */
  return( rfsiam_rc );
}                              /* end of rebuild_fs_iamaps ()  */
	 
	 
/****************************************************************
 * NAME: record_dupchk_inode_extents
 *
 * FUNCTION:  For each inode extent in the aggregate (i.e., describing
 *            either the Aggregate Inode Table or in the Fileset Inode
 *            Table), for each block in the extent, record (in the 
 *            fsck workspace block map) that the block is allocated
 *            and check to see if this is a duplicate allocation.  (That
 *            is, check to see if any other file system object has 
 *            claimed to own that block.)
 *
 * PARAMETERS:  none
 *
 * NOTES:  o Since this routine completes the fsck workspace
 *           initialization needed by inode_get() (et al), this
 *           routine ensures fsck I/O buffers contain the inode
 *           it needs before invoking inode_get().
 *           (inode_get() is invoked to locate the inode within
 *           the fsck I/O buffer.)
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t record_dupchk_inode_extents ()
{
  retcode_t rdie_rc = FSCK_OK;
  int      aggregate_IAGs;
  int      aggregate_IA_Inode = -1;   /* AIT inodes are both aggregate owned */
  int      which_it;
  fsck_inode_recptr inorecptr;
  struct fsck_ino_msg_info ino_msg_info;
  fsck_msg_info_ptr msg_info_ptr;
  msg_info_ptr = &(ino_msg_info);
  msg_info_ptr->msg_inonum = AGGREGATE_I;
  msg_info_ptr->msg_inotyp = fsck_metaIAG;
  msg_info_ptr->msg_dxdtyp = fsck_inoext;
  msg_info_ptr->msg_inopfx = fsck_aggr_inode;    /*
				      * the inodes representing the Inode
				      * Allocation Maps are aggregate inodes
				      */
  rdie_rc = record_imap_info();
  if( rdie_rc == FSCK_OK ) {  /* nothing unexpected */
      /*
       * record (and check for duplicate allocations) the
       * inode extents described by aggregate IAGs
       */
    aggregate_IAGs = -1;        /* aggregate IAGs */
    if( agg_recptr->primary_ait_4part1 ) {
      which_it = fsck_primary;
      }
    else {
      which_it = fsck_secondary;
      }
    msg_info_ptr->msg_inonum = AGGREGATE_I;
    rdie_rc = get_inorecptr( aggregate_IA_Inode, 0, AGGREGATE_I, &inorecptr );
    if( (rdie_rc == FSCK_OK) && (inorecptr == NULL) ) {
      rdie_rc = FSCK_INTERNAL_ERROR_45;
      }
    else if( rdie_rc == FSCK_OK ) {
      rdie_rc = record_dupchk_inoexts( aggregate_IAGs, which_it,
				       &(agg_recptr->agg_imap.bkd_inodes),
				       inorecptr, msg_info_ptr );
      }
    if( rdie_rc == FSCK_OK ) {  /* things are going as expected */
      agg_recptr->inodes_in_aggregate = agg_recptr->agg_imap.bkd_inodes;
      agg_recptr->blocks_for_inodes =
			 (agg_recptr->agg_imap.bkd_inodes * sizeof(dinode_t)) /
			 sb_ptr->s_bsize;
	/*
	 * record (and check for duplicate allocations) the
	 * inode extents described by fileset IAGs
	 *
	 * (In release 1 there is exactly 1 fileset)
	 */
      aggregate_IAGs = 0;          /* fileset IAGs */
      which_it = FILESYSTEM_I;
      msg_info_ptr->msg_inonum = FILESYSTEM_I;
      rdie_rc = get_inorecptr( aggregate_IA_Inode, 0, FILESYSTEM_I, &inorecptr );
      if( (rdie_rc == FSCK_OK) && (inorecptr == NULL) ) {
	rdie_rc = FSCK_INTERNAL_ERROR_20;
	}
      else if( rdie_rc == FSCK_OK ) {
	rdie_rc = record_dupchk_inoexts( aggregate_IAGs, which_it,
					 &(agg_recptr->fset_imap.bkd_inodes),
					 inorecptr, msg_info_ptr );
	}
      if( rdie_rc == FSCK_OK ) {
	agg_recptr->inodes_in_aggregate += agg_recptr->fset_imap.bkd_inodes;
	agg_recptr->blocks_for_inodes +=
		       (agg_recptr->fset_imap.bkd_inodes * sizeof(dinode_t)) /
		       sb_ptr->s_bsize;
	}
      }  /* end things are going as expected */
    }  /* end nothing unexpected */
  return( rdie_rc );
}                              /* end of record_dupchk_inode_extents ()  */
	 
	 
/****************************************************************
 * NAME: record_dupchk_inoexts
 *
 * FUNCTION:  For each inode extent in the specified Inode Table (either
 *            the Aggregate Inode Table or in the Fileset Inode Table),
 *            for each block in the extent, record (in the fsck workspace
 *            block map) that the block is allocated and check to see if
 *            this is a duplicate allocation.  (That is, check to see if
 *            any other file system object has claimed to own that block.)
 *
 * PARAMETERS:
 *     is_aggregate        - input - !0 => aggregate owned
 *                                    0 => fileset owned
 *     which_it            - input - ordinal number of the aggregate inode 
 *                                   describing the inode table
 *     backed_inode_count  - input - pointer to a variable in which to return
 *                                   the count of backed inodes described
 *                                   by inode extents described by iags in
 *                                   the current inode allocation table.
 *     inorecptr           - input - pointer to an fsck inode record which
 *                                   describes the current inode allocation
 *                                   table inode  
 *     msginfoptr          - input - pointer to data needed to issue messages
 *                                   about the current inode 
 *
 * NOTES:  o Since this routine completes the fsck workspace
 *           initialization needed by inode_get() (et al), this
 *           routine ensures fsck I/O buffers contain the inode
 *           it needs before invoking inode_get().
 *           (inode_get() is invoked to locate the inode within
 *           the fsck I/O buffer.)
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t record_dupchk_inoexts ( int                 is_aggregate,
				  int                 which_it,
				  int32              *backed_inode_count,
				  fsck_inode_recptr   inorecptr,
				  fsck_msg_info_ptr   msginfoptr
				 )
{
  retcode_t irdie_rc = FSCK_OK;
  int       which_ait;
  iag_t    *iagptr;
  reg_idx_t ext_idx;
  reg_idx_t ext_idx_max;
  fsblkidx_t ext_addr;
  reg_idx_t  ext_len;
  int8       is_EA = 0;
  int8       is_ACL = 0;
  int8       extent_is_valid;
  reg_idx_t  adjusted_length;
  ino_t      first_inonum, last_inonum;
  char       user_reply;
  ext_idx_max = EXTSPERIAG - 1;
  *backed_inode_count = 0;
  if( is_aggregate ) {
    if( agg_recptr->primary_ait_4part1 ) {
      which_ait = fsck_primary;
      }
    else {
      which_ait = fsck_secondary;
      }
    }
  else {
    if( agg_recptr->primary_ait_4part2 ) {
      which_ait = fsck_primary;
      }
    else {
      which_ait = fsck_secondary;
      }
    }
  irdie_rc = ait_special_read_ext1( which_ait );
  if( irdie_rc == FSCK_OK ) {
    irdie_rc = iag_get_first( is_aggregate, which_it, which_ait, &iagptr );
    }
  else {
    report_readait_error( irdie_rc, 
                          FSCK_FAILED_CANTREADAITEXT9, 
                          which_ait );
    irdie_rc = FSCK_FAILED_CANTREADAITEXT9;
    }
  while( (iagptr != NULL) && (irdie_rc == FSCK_OK) ) {
    for( ext_idx = 0;
	 ( (ext_idx <= ext_idx_max) && (irdie_rc == FSCK_OK) );
	 ext_idx++ ) {
	
      ext_addr = addressPXD(&(iagptr->inoext[ext_idx]));
      if( ext_addr != 0 ) {  /* the extent is allocated */
	*backed_inode_count += INOSPEREXT;
	ext_len = lengthPXD(&(iagptr->inoext[ext_idx]));
	irdie_rc = process_extent( inorecptr, ext_len, ext_addr, is_EA, 
				   is_ACL, msginfoptr, &adjusted_length, 
				   &extent_is_valid, FSCK_RECORD_DUPCHECK );
	if( !extent_is_valid ) {  /* yuck! */
	  irdie_rc = process_extent( inorecptr, ext_len, ext_addr, is_EA, 
				     is_ACL, msginfoptr, &adjusted_length, 
				     &extent_is_valid, FSCK_UNRECORD );
	  first_inonum = (iagptr->iagnum * INOSPERIAG) + 
                         (ext_idx * INOSPEREXT);
	  last_inonum = first_inonum + INOSPEREXT - 1;
             /*
              * notify the user
              */
          sprintf( message_parm_0, "%ld", first_inonum );
          msgprms[0] = message_parm_0;
          msgprmidx[0] = 0;
          sprintf( message_parm_1, "%ld", last_inonum );
          msgprms[1] = message_parm_1;
          msgprmidx[1] = 0;
          fsck_send_msg( fsck_BADIAMIAGPXDU, 0, 2 );
             /*
              * log more details
              */
          msgprms[0] = message_parm_0;
          msgprms[1] = message_parm_1;
          msgprms[2] = message_parm_2;
          msgprmidx[2] = msginfoptr->msg_inopfx;
          sprintf( message_parm_3, "%ld", msginfoptr->msg_inonum );
          msgprms[3] = message_parm_3;
          msgprmidx[3] = 0;
          sprintf( message_parm_4, "%ld", iagptr->iagnum );
          msgprms[4] = message_parm_4;
          msgprmidx[4] = 0;
          sprintf( message_parm_5, "%ld", ext_idx );
          msgprms[5] = message_parm_5;
          msgprmidx[5] = 0;
          fsck_send_msg( fsck_BADIAMIAGPXDL, 0, 6 );
             /*
              * fix it if we can
              */
          if( agg_recptr->processing_readwrite ) {  /* we can fix this */
            PXDaddress( &(iagptr->inoext[ext_idx]), 0 );
            PXDlength( &(iagptr->inoext[ext_idx]), 0 );
            irdie_rc = iag_put( iagptr );
            *backed_inode_count -= INOSPEREXT;
            msgprms[0] = message_parm_0;
            msgprms[1] = message_parm_1;
            fsck_send_msg( fsck_WILLRELEASEINOS, 0, 2 );
            }  /* end we can fix this */
          else {       /* we don't have write access */
            agg_recptr->ag_dirty = 1;          /* boy, is it ever dirty! */
            irdie_rc = FSCK_FAILED_IAG_CORRUPT_PXD;  /* bail out of fsck */
            msgprms[0] = message_parm_0;
            msgprms[1] = message_parm_1;
            fsck_send_msg( fsck_CANTRECOVERINOS, 0, 2 );
            }  /* end we don't have write access */
	  }  /* end yuck! */
	}  /* end the extent is allocated */
      }  /* end for ext_idx */
    if( irdie_rc == FSCK_OK ) {
      irdie_rc = iag_get_next( &iagptr );
      }
    }  /* end while iagptr != NULL */
  return( irdie_rc );
}                            /* end of record_dupchk_inoexts ()  */

/****************************************************************
 * NAME: record_imap_info
 *
 * FUNCTION:  Find and then record, in the fsck global aggregate record,
 *            information describing the Inode Allocation Maps (i.e.,
 *            the Aggregate Inode Allocation Map and the Fileset Inode
 *            Allocation Map) which will be used to expedite subsequent
 *            access to the maps.
 *
 * PARAMETERS:  none
 *
 * NOTES:  o Since this routine completes the fsck workspace
 *           initialization needed by inode_get() (et al), this
 *           routine ensures fsck I/O buffers contain the inode
 *           it needs before invoking inode_get().
 *           (inode_get() is invoked to locate the inode within
 *           the fsck I/O buffer.)
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t record_imap_info ()
{
  retcode_t rii_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  int   aggregate_inode = 1;          /* aggregate owned inodes */
  int   which_ait;
  inoidx_t ino_idx;
  fsblkidx_t  leaf_fsblk_offset;
  dinode_t *ino_ptr;
  xtpage_t *dummy_page;
  int8  inline_data;
  if( agg_recptr->primary_ait_4part1 ) {
    which_ait = fsck_primary;
    }
  else {
    which_ait = fsck_secondary;
    }
  intermed_rc = ait_special_read_ext1( which_ait );
  if( intermed_rc != FSCK_OK )  {  /* this is fatal */
    report_readait_error( intermed_rc, 
                          FSCK_FAILED_CANTREADAITEXTA, 
                          which_ait );
    rii_rc = FSCK_FAILED_CANTREADAITEXTA;
    }  /* end this is fatal */
  else {  /* got the inode extension needed for part 1 */
    ino_idx = AGGREGATE_I;        /* the aggregate inode map inode */
    rii_rc = inode_get( aggregate_inode, which_ait, ino_idx, &ino_ptr );
    if( rii_rc != FSCK_OK ) {  /* but we read this before! */
      rii_rc = FSCK_FAILED_SELF_READ2;
      }  /* end but we read this before! */
    else {  /* successful read on aggregate inode map inode */
      rii_rc = find_first_leaf( ino_ptr, &dummy_page,
				&leaf_fsblk_offset,
				&inline_data,
				&(agg_recptr->agg_imap.imap_is_rootleaf) );
      if( rii_rc != FSCK_OK )  {  /* but it had a valid tree before */
	rii_rc = FSCK_FAILED_SELF_NOWBAD;
	}  /* end but it had a valid tree before */
      else {  /* got the info for the aggregate imap */
	agg_recptr->agg_imap.first_leaf_offset = leaf_fsblk_offset * sb_ptr->s_bsize;
	}  /* end else got the info for the aggregate imap */
      }  /* end else successful read on aggregate inode map inode */
    }  /* end else got the inode extension needed for part 1 */
  if( rii_rc == FSCK_OK )  {  /* still in business */
    if( (agg_recptr->primary_ait_4part1 &&
	 !agg_recptr->primary_ait_4part2   ) ||
	(!agg_recptr->primary_ait_4part1 &&
	 agg_recptr->primary_ait_4part2    )    )  {  /*
			    * part1 and part2 of the AIT are not being
			    * used from the same table
			    */
      if( agg_recptr->primary_ait_4part2 ) {
	which_ait = fsck_primary;
	}
      else {
	which_ait = fsck_secondary;
	}
      intermed_rc = ait_special_read_ext1( which_ait );
      if( intermed_rc != FSCK_OK )  {  /* this is fatal */
        report_readait_error( intermed_rc, 
                              FSCK_FAILED_CANTREADAITEXTB, 
                              which_ait );
	rii_rc = FSCK_FAILED_CANTREADAITEXTB;
	}  /* end this is fatal */
      }  /* part1 and part2 of the AIT are not being used from ... */
    if( rii_rc == FSCK_OK )  {  /* inode extent is in the buffer */
      ino_idx = FILESYSTEM_I;        /* the aggregate inode map inode */
      rii_rc = inode_get( aggregate_inode, which_ait, ino_idx, &ino_ptr );
      if( rii_rc != FSCK_OK ) {  /* but we read this before! */
	rii_rc = FSCK_FAILED_AGFS_READ2;
	}  /* end but we read this before! */
      else {  /* successful read on aggregate inode map inode */
	rii_rc = find_first_leaf( ino_ptr, &dummy_page,
				  &leaf_fsblk_offset,
				  &inline_data,
				  &(agg_recptr->fset_imap.imap_is_rootleaf) );
	if( rii_rc != FSCK_OK )  {  /* but it had a valid tree before */
	  rii_rc = FSCK_FAILED_AGFS_NOWBAD;
	  }  /* end but it had a valid tree before */
	else {  /* got the info for the fileset imap */
	  agg_recptr->fset_imap.first_leaf_offset =
					      leaf_fsblk_offset * sb_ptr->s_bsize;
	  }  /* end else got the info for the fileset imap */
	}  /* end else successful read on fileset inode map inode */
      }  /* end inode extent is in the buffer */
    }  /* end still in business */
  return( rii_rc );
}                              /* end of record_imap_info ()  */
	 
	 
/****************************************************************
 * NAME: verify_agg_iamap
 *
 * FUNCTION:  Verify that the Aggregate Inode Allocation Map is correct 
 *            according to fsck's observations of the aggregate.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t verify_agg_iamap ()
{
  retcode_t vaiam_rc = FSCK_OK;
  int is_aggregate = -1;
  int which_it = 0;
  struct fsck_imap_msg_info imap_msg_info;
  fsck_imap_msg_info_ptr msg_info_ptr;
  struct fsck_iag_info iag_info;
  fsck_iag_info_ptr iag_info_ptr;
  if( agg_recptr->primary_ait_4part1 ) {
    which_it = fsck_primary;
    }
  else {
    which_it = fsck_secondary;
    }
  iag_info_ptr = &iag_info;
  iag_info_ptr->iamrecptr = &(agg_recptr->agg_imap);
  iag_info_ptr->iagtbl = agg_recptr->agg_imap.iag_tbl;
  iag_info_ptr->agtbl = agg_recptr->agg_imap.ag_tbl;
  iag_info_ptr->agg_inotbl = is_aggregate;
  msg_info_ptr = &(imap_msg_info);
  msg_info_ptr->msg_mapowner = fsck_aggregate;
  vaiam_rc = inotbl_get_ctl_page( is_aggregate, &(iag_info_ptr->iamctlptr) );
  if( vaiam_rc == FSCK_OK )  {  /* got the imap control page */
    vaiam_rc = iamap_validation ( is_aggregate, which_it, which_it,
				  iag_info_ptr, msg_info_ptr );
    }  /* end got the imap control page */
  return( vaiam_rc );
}                              /* end of verify_agg_iamap ()  */
	 
	 
/****************************************************************
 * NAME: verify_fs_iamaps
 *
 * FUNCTION:  Verify that the Fileset Inode Allocation Map is correct 
 *            according to fsck's observations of the aggregate.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t verify_fs_iamaps ()
{
  retcode_t vfsiam_rc = FSCK_OK;
  int is_aggregate = 0;
  int which_it = FILESYSTEM_I;     /* release 1 there is exactly 1 fileset */
  int which_ait;
  struct fsck_imap_msg_info imap_msg_info;
  fsck_imap_msg_info_ptr    msg_info_ptr;
  struct fsck_iag_info iag_info;
  fsck_iag_info_ptr iag_info_ptr;
  if( agg_recptr->primary_ait_4part2 ) {
    which_ait = fsck_primary;
    }
  else {
    which_ait = fsck_secondary;
    }
  iag_info_ptr = &iag_info;
  iag_info_ptr->iamrecptr = &(agg_recptr->fset_imap);
  iag_info_ptr->iagtbl = agg_recptr->fset_imap.iag_tbl;
  iag_info_ptr->agtbl = agg_recptr->fset_imap.ag_tbl;
  iag_info_ptr->agg_inotbl = is_aggregate;
  msg_info_ptr = &(imap_msg_info);
  msg_info_ptr->msg_mapowner = fsck_fileset;
  vfsiam_rc = inotbl_get_ctl_page( is_aggregate, &(iag_info_ptr->iamctlptr) );
  if( vfsiam_rc == FSCK_OK )  {  /* got the imap control page */
    vfsiam_rc = iamap_validation ( is_aggregate, which_it, which_ait,
				   iag_info_ptr, msg_info_ptr );
    }  /* end got the imap control page */
  return( vfsiam_rc );
}                              /* end of verify_fs_iamaps ()  */
	 
	 
/*--------------------------------------------------------------------
 * NAME: xtAppend
 *
 * FUNCTION: Append an extent to the specified file
 *
 * PARAMETERS:
 *      di      - Inode to add extent to
 *      offset  - offset of extent to add
 *      blkno   - block number of start of extent to add
 *      nblocks - number of blocks in extent to add
 *      xpage   - xtree page to add extent to
 *
 * NOTES: xpage points to its parent in the xtree and its rightmost child (if it
 *      has one).  It also points to the buffer for the page.
 *
 * RETURNS: 0 for success; Other indicates failure
 */
retcode_t xtAppend( dinode_t        *di,
                              int64             offset,
                              int64             blkno,
                              int32             nblocks,
                              xtree_buf_t    *xpage )
{
  int32       rc = 0;
  int32       index;
  xad_t       *xad;
  xtpage_t    *cur_page;

  cur_page = xpage->page;
  index = cur_page->header.nextindex;

  /* insert entry for new extent */
  if ( index == cur_page->header.maxentry )
  {
    /*
     * There is not room in this page to add the entry; Need to create
     * a new page
     */
    if ( cur_page->header.flag & BT_ROOT )
    {
      /* This is the root of the xtree; need to split root */
      rc = xtSplitRoot( di, xpage, offset, nblocks, blkno );
    }
    else
    {
      /*
       * Non-root page: add new page at this level, xtSplitPage() calls
       * xtAppend again to propogate up the new page entry
       */
      rc = xtSplitPage( di, xpage, offset, nblocks, blkno );
    }
  }
  else
  {
    /* There is room to add the entry to this page */
    xad = &cur_page->xad[index];
    XADoffset(xad, offset);
    XADlength(xad, nblocks);
    XADaddress(xad, blkno);

    /* advance next available entry index */
    ++cur_page->header.nextindex;

    rc = 0;
  }

  return rc;
}
	 
	 
/*--------------------------------------------------------------------
 * NAME: xtSplitPage
 *
 * FUNCTION: Split non-root page of xtree
 *
 * PARAMETERS:
 *      ip      - Inode of xtree splitting
 *      xpage   - page to split
 *      offset  - offset of new extent to add
 *      nblocks - number of blocks of new extent to add
 *      blkno   - starting block number of new extent to add
 *
 * RETURNS: 0 for success; Other indicates failure
 */
retcode_t xtSplitPage( dinode_t      *ip,
                                xtree_buf_t  *xpage,
                                int64             offset,
                                int32             nblocks,
                                int64             blkno )
{
  int32       rc = 0;
  int64       xaddr;  /* new right page block number */
  xad_t       *xad;
  int32       xlen;
  xtpage_t    *lastpage, *newpage;
  int64       leftbn;

  /* Allocate disk space for the new xtree page */
  xlen = 1 << agg_recptr->log2_blksperpg;
  rc = fsck_alloc_fsblks( xlen, &xaddr);
  if( rc )
    return rc;

  /*
   * Modify xpage's next entry to point to the new disk space,
   * write the xpage to disk since we won't be needing it anymore.
   */
  lastpage = xpage->page;
  lastpage->header.next = xaddr;

  leftbn = addressPXD(&(lastpage->header.self));
  rc = ujfs_rw_diskblocks(Dev_IOPort, leftbn * sb_ptr->s_bsize,
                                    PSIZE, lastpage, PUT);
  if ( rc != 0) return rc;

  /*
   * We are now done with the xpage as-is.  We can now re-use this buffer for
   * our new buffer.
   */
  newpage = xpage->page;

  PXDlength(&(newpage->header.self), xlen);
  PXDaddress(&(newpage->header.self), xaddr);
  newpage->header.flag = newpage->header.flag & BT_TYPE;

  /* initialize sibling pointers of newpage */
  newpage->header.next = 0;
  newpage->header.prev = leftbn;

  /* insert entry at the first entry of the new right page */
  xad = &newpage->xad[XTENTRYSTART];
  XADoffset(xad, offset);
  XADlength(xad, nblocks);
  XADaddress(xad, blkno);

  newpage->header.nextindex = XTENTRYSTART + 1;

  /* Now append new page to parent page */
  rc = xtAppend( ip, offset, xaddr, xlen, xpage->up );

  /* Update inode to account for new page */
  ip->di_nblocks += xlen;

  return rc;
}
	 	
	 
/*--------------------------------------------------------------------
 * NAME: xtSplitRoot
 *
 * FUNCTION: Split full root of xtree
 *
 * PARAMETERS:
 *      ip      - Inode of xtree
 *      xroot   - Root of xtree
 *      offset  - Offset of extent to add
 *      nblocks - number of blocks for extent to add
 *      blkno   - starting block of extent to add
 *
 * RETURNS: 0 for success; Other indicates failure
 */
retcode_t xtSplitRoot( dinode_t      *ip,
                                xtree_buf_t *xroot,
                                int64           offset,
                                int32           nblocks,
                                int64           blkno)
{
  xtpage_t    *rootpage;
  xtpage_t    *newpage;
  int64       xaddr;
  int32       nextindex;
  xad_t       *xad;
  int32       rc;
  xtree_buf_t *newbuf;
  int32       xlen;
  int I_am_logredo = 0;

  /* Allocate and initialize buffer for new page to accomodate the split */
  rc = alloc_wrksp( sizeof(xtree_buf_t), 
                           dynstg_xtreebuf, I_am_logredo,
                           (void **) &newbuf );
  if( rc ) 
    return( rc );
	
  newbuf->up = xroot;
  if ( xroot->down == NULL ) {
    fsim_node_pages = newbuf;
    }
  else  {
    xroot->down->up = newbuf;
    }
  newbuf->down = xroot->down;
  xroot->down = newbuf;
  rc = alloc_wrksp( PSIZE, dynstg_xtreepagebuf, 
                           I_am_logredo, (void **) &newpage );
  if( rc ) 
    return( rc );
  newbuf->page = newpage;
	
  /* Allocate disk blocks for new page */
  xlen = 1 << agg_recptr->log2_blksperpg;
  rc = fsck_alloc_fsblks( xlen, &xaddr);
  if ( rc )
    return rc;

  rootpage = xroot->page;

    /* Initialize new page */
  if( (rootpage->header.flag & BT_LEAF) == BT_LEAF ) {
    newpage->header.flag = BT_LEAF;
    }
  else {
    newpage->header.flag = BT_INTERNAL;
    }
  PXDlength(&(newpage->header.self), xlen);
  PXDaddress(&(newpage->header.self), xaddr);
  newpage->header.nextindex = XTENTRYSTART;
  newpage->header.maxentry = XTPAGEMAXSLOT;

    /* initialize sibling pointers */
  newpage->header.next = 0;
  newpage->header.prev = 0;

    /* copy the in-line root page into new right page extent */
  nextindex = rootpage->header.maxentry;
  memcpy(&newpage->xad[XTENTRYSTART], &rootpage->xad[XTENTRYSTART],
           (nextindex - XTENTRYSTART) << L2XTSLOTSIZE);

    /* insert the new entry into the new right/child page */
  xad = &newpage->xad[nextindex];
  XADoffset(xad, offset);
  XADlength(xad, nblocks);
  XADaddress(xad, blkno);

  /* update page header */
  newpage->header.nextindex = nextindex + 1;

  /* init root with the single entry for the new right page */
  xad = &rootpage->xad[XTENTRYSTART];
  XADoffset(xad, 0);
  XADlength(xad, xlen);
  XADaddress(xad, xaddr);

  /* update page header of root */
  rootpage->header.flag &= ~BT_LEAF;
  rootpage->header.flag |= BT_INTERNAL;

  rootpage->header.nextindex = XTENTRYSTART + 1;

  /* Update nblocks for inode to account for new page */
  ip->di_nblocks += xlen;

  return 0;
}


