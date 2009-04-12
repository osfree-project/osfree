/* $Id: fsckdtre.c,v 1.4 2004/03/21 02:43:29 pasha Exp $ */

static char *SCCSID = "@(#)1.27.1.2  12/2/99 11:13:44 src/jfs/utils/chkdsk/fsckdtre.c, jfschk, w45.fs32, fixbld";
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
 *                   CHANGE
 *
 *    MODULE_NAME:		fsckdtre.c
 *
 *   COMPONENT_NAME: 	jfschk
 *
 *   FUNCTIONS:
 *              direntry_add
 *              direntry_get_inonum
 *              direntry_get_objnam
 *              direntry_remove
 *              dTree_processing
 *              dTree_search
 *              find_first_dir_leaf
 *              init_dir_tree
 *              process_valid_dir_data
 *              reconnect_fs_inodes
 *
 *              direntry_get_objnam_node
 *              dTree_binsrch_internal_page
 *              dTree_binsrch_leaf
 *              dTree_key_compare
 *              dTree_key_compare_leaflvl
 *              dTree_key_compare_prntchld
 *              dTree_key_compare_samelvl
 *              dTree_key_extract
 *              dTree_key_extract_cautiously
 *              dTree_key_extract_record
 *              dTree_key_to_upper
 *              dTree_node_first_key
 *              dTree_node_first_in_level
 *              dTree_node_last_in_level
 *              dTree_node_not_first_in_level
 *              dTree_node_not_last_in_level
 *              dTree_node_size_check
 *              dTree_process_internal_slots
 *              dTree_process_leaf_slots
 *              dTree_verify_slot_freelist
 *              process_valid_dir_node
 *
*/

/*
 * defines and includes common among the xfsck modules
 */
/* History
 * PS21032004 - insert IBM fixes
 */
#include "xfsckint.h"

/*
 * for inline unicode functions
 */
#define _ULS_UNIDEFK
#include <uni_inln.h>

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
  * For directory entry processing
  *
  *      defined in xchkdsk.c
  */
extern reg_idx_t key_len[2];
extern UniChar key[2][JFS_NAME_MAX];
extern UniChar ukey[2][JFS_NAME_MAX];

extern int32   Uni_Name_len;
extern UniChar Uni_Name[JFS_NAME_MAX];

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

#define LEFT_KEY_LOWER -1
#define KEYS_MATCH      0
#define LEFT_KEY_HIGHER 1

typedef struct fsck_Dtree_info {
     dtroot_t       *dtr_ptr;
     dtpage_t       *dtp_ptr;
     pxd_t          *pxd_ptr;
     int8           *dtstbl;
     int16           last_dtidx;
     int16           freelist_first_dtidx;
     int16           freelist_count;
     dtslot_t       *slots;
     int8            slot_map[DTPAGEMAXSLOT];
     fsblkidx_t      ext_addr;
     reg_idx_t       ext_length;
     dtreeQ_elem_ptr this_Qel;
     dtreeQ_elem_ptr next_Qel;
     fsblkidx_t      last_node_addr;
     int8            last_level;
     int8            leaf_level;
     int8            leaf_seen;
     int16           max_slotidx;
     int16           this_key_idx;
     int16           last_key_idx;
     reg_idx_t       key_len[2];
     UniChar         key[2][JFS_NAME_MAX];
     } fsck_Dtree_info_t;
typedef fsck_Dtree_info_t *fsck_Dtree_info_ptr;

retcode_t direntry_get_objnam_node( ino_t, int8 *, dtslot_t *,
				    int, int *, UniChar *, int8 * );

retcode_t dTree_binsrch_internal_page( fsck_Dtree_info_ptr, UniChar *,
				       reg_idx_t, int8, int8, int8 *,
				       int8 *, int8 *, fsck_inode_recptr );

retcode_t dTree_binsrch_leaf( fsck_Dtree_info_ptr, UniChar *, reg_idx_t,
			      int8, int8, int8 *, int8 *, int8 *, int8 *,
			      fsck_inode_recptr );

retcode_t dTree_key_compare ( UniChar *, uint8, UniChar *, uint8, int * );

retcode_t dTree_key_compare_leaflvl( UniChar *, uint8, UniChar *,
				     uint8, int8 * );

retcode_t dTree_key_compare_prntchld( fsck_Dtree_info_ptr, UniChar *, uint8,
				      UniChar *, uint8, int8 * );

retcode_t dTree_key_compare_samelvl( UniChar *, uint8, UniChar *,
				      uint8, int8 * );

retcode_t dTree_key_extract ( fsck_Dtree_info_ptr, int, UniChar *,
			      reg_idx_t *, int8, int8, fsck_inode_recptr );

retcode_t dTree_key_extract_cautiously ( fsck_Dtree_info_ptr, int, UniChar *,
					 reg_idx_t *, int8, int8,
					 fsck_inode_recptr );

retcode_t dTree_key_extract_record ( fsck_Dtree_info_ptr, int, UniChar *,
				     reg_idx_t *, int8, int8,
				     fsck_inode_recptr );

retcode_t dTree_key_to_upper( UniChar *, UniChar *, int32 );

retcode_t dTree_node_first_key( fsck_Dtree_info_ptr, fsck_inode_recptr,
				fsck_msg_info_ptr, int );

retcode_t dTree_node_first_in_level( fsck_Dtree_info_ptr,
					 fsck_inode_recptr, fsck_msg_info_ptr,
					 int );

retcode_t dTree_node_last_in_level( fsck_Dtree_info_ptr,
					fsck_inode_recptr, fsck_msg_info_ptr,
					int );
retcode_t dTree_node_not_first_in_level( fsck_Dtree_info_ptr,
					 fsck_inode_recptr, fsck_msg_info_ptr,
					 int );

retcode_t dTree_node_not_last_in_level( fsck_Dtree_info_ptr,
					fsck_inode_recptr, fsck_msg_info_ptr,
					int );

retcode_t dTree_node_size_check( fsck_Dtree_info_ptr, int8, int8, int8,
				 fsck_inode_recptr,fsck_msg_info_ptr,
				 int );

retcode_t dTree_process_internal_slots( fsck_Dtree_info_ptr,
					fsck_inode_recptr, fsck_msg_info_ptr,
					int );

retcode_t dTree_process_leaf_slots( fsck_Dtree_info_ptr,
				    fsck_inode_recptr, ino_t,
				    fsck_msg_info_ptr,
				    int );

retcode_t dTree_verify_slot_freelist( fsck_Dtree_info_ptr,
				      fsck_inode_recptr, fsck_msg_info_ptr,
				      int);

retcode_t process_valid_dir_node ( int8 *, dtslot_t *, int,
				   fsck_inode_recptr, fsck_msg_info_ptr, int );

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */


/*****************************************************************************
 * NAME: direntry_add
 *
 * FUNCTION: Add an entry to a directory.
 *
 * PARAMETERS:
 *      parent_inoptr  - input - pointer to the directory inode, in an
 *                               fsck buffer, to which the entry should
 *                               be added.
 *      child_inonum   - input - inode number to put in the new directory
 *                               entry.
 *      child_name     - input - pointer to the file name to put in the
 *                               new directory entry.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t direntry_add( dinode_t *parent_inoptr,
			ino_t     child_inonum,
			UniChar  *child_name )
{
  retcode_t ad_rc = FSCK_OK;
  component_t uniname_struct;

  uniname_struct.namlen = UniStrlen(child_name);
  uniname_struct.name = child_name;

  ad_rc = fsck_dtInsert( parent_inoptr, &uniname_struct, &child_inonum );
  if( ad_rc == FSCK_OK ) {
    ad_rc = inode_put( parent_inoptr );
    }
  else {
    if( ad_rc < 0 ) {  /* it's fatal */
      sprintf( message_parm_0, "%ld", FSCK_INTERNAL_ERROR_60 );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%ld", ad_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%ld", child_inonum );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%ld", parent_inoptr->di_number );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );
      ad_rc = FSCK_INTERNAL_ERROR_60;
      }  /* end it's fatal */
    else {
      if( ad_rc == FSCK_BLKSNOTAVAILABLE ) {
        sprintf( message_parm_0, "%ld", 3 );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( fsck_CANTRECONINSUFSTG, 0, 1 );
        }
      }
    }  /* end else */

  return( ad_rc );
}                              /* end of direntry_add ()  */


/*****************************************************************************
 * NAME: direntry_get_inonum
 *
 * FUNCTION:  Get the inode number for the file whose name is given.
 *
 * PARAMETERS:
 *      parent_inonum   - input - the inode number of a directory containing
 *                                an entry for the inode whose name is 
 *                                desired.
 *      obj_name_length - input - the length of the file name in mixed case
 *      obj_name        - input - pointer to the file name in mixed case
 *      obj_NAME_length - input - the length of the file name in upper case
 *      obj_NAME        - input - pointer to the file name in upper case
 *      found_inonum    - input - pointer to a variable in which to return
 *                                the inode number stored in the (found) entry
 *
 * NOTES:	A case insensitive search is conducted.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t direntry_get_inonum( ino_t     parent_inonum,
			       int       obj_name_length,
			       UniChar  *obj_name,
			       int       obj_NAME_length,
			       UniChar  *obj_NAME,
			       ino_t    *found_inonum
			      )
{
  retcode_t gdi_rc = FSCK_OK;
  dinode_t *root_inoptr;
  int is_aggregate = 0;
  int alloc_ifnull = 0;
  int which_table = FILESYSTEM_I;
  dtslot_t *slot_ptr;
  int8      entry_found;
  fsck_inode_recptr root_inorecptr;

  gdi_rc = inode_get( is_aggregate, which_table, ROOT_I, &root_inoptr );

  if( gdi_rc == FSCK_OK ) {  /* got the root inode */

    gdi_rc = get_inorecptr( is_aggregate, alloc_ifnull, ROOT_I, &root_inorecptr );

    if( (gdi_rc == FSCK_OK) && (root_inorecptr == NULL) ) {
      gdi_rc = FSCK_INTERNAL_ERROR_46;
      }
    else if( gdi_rc == FSCK_OK ) {
      gdi_rc = dTree_search( root_inoptr, obj_name, obj_name_length,
			     obj_NAME, obj_NAME_length, &slot_ptr,
			     &entry_found, root_inorecptr );
      }

    if( (gdi_rc == FSCK_OK) && (entry_found) ) {
      *found_inonum = ((ldtentry_t *) slot_ptr)->inumber;
      }
    }  /* end got the root inode */

  return( gdi_rc );
}                              /* end of direntry_get_inonum ()  */


/*****************************************************************************
 * NAME: direntry_get_objnam
 *
 * FUNCTION: Find the file name for the given inode number in the given
 *           directory inode.
 *
 * PARAMETERS:
 *      parent_inonum     - input - the inode number of the directory 
 *                                  containing an entry for the object
 *      obj_inonum        - input - the inode number of the object for
 *                                  which the file name is desired
 *      found_name_length - input - pointer to a variable in which to return
 *                                  the length of the object name
 *      found_name        - input - pointer to a buffer in which to return
 *                                  the object name
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t direntry_get_objnam( ino_t    parent_inonum,
			       ino_t    obj_inonum,
			       int     *found_name_length,
			       UniChar *found_name
			     )
{
  retcode_t dgon_rc = FSCK_OK;
  int is_aggregate = 0;
  int which_it = FILESYSTEM_I;  /* in release 1 there's exactly 1 fileset */
  inoidx_t parent_inoidx;
  dinode_t  *parent_inoptr;
  dtpage_t  *leaf_ptr;
  fsblkidx_t leaf_offset;
  int8       dir_inline;
  int8       dir_rootleaf;
  dtroot_t  *dtroot_ptr;
  int8      *dtstbl;
  dtslot_t  *slots;
  int        last_dtidx;
  int8       entry_found;

  *found_name_length = 0;      /* assume no match will be found */
  parent_inoidx = (inoidx_t) parent_inonum;

  dgon_rc = inode_get( is_aggregate, which_it, parent_inoidx, &parent_inoptr );

  if( dgon_rc != FSCK_OK )  {  /* but we read it before!  */
    dgon_rc = FSCK_FAILED_CANTREAD_DIRNOW;  /* this is fatal */
    }  /* end but we read it before! */
  else {  /* got the parent inode */
    dgon_rc = find_first_dir_leaf( parent_inoptr, &leaf_ptr, &leaf_offset,
				   &dir_inline, &dir_rootleaf );
    if( dgon_rc != FSCK_OK ) {  /* we already verified the dir! */
      dgon_rc = FSCK_FAILED_DIRGONEBAD;
      }  /*  end we already verified the dir! */
    else {  /* we found and read the first leaf */
      if( dir_rootleaf ) {  /* rootleaf directory tree */
	dtroot_ptr = (dtroot_t *) &(parent_inoptr->di_btroot);
	dtstbl = (int8 *) &(dtroot_ptr->header.stbl[0]);
	slots = &(dtroot_ptr->slot[0]);
	last_dtidx = dtroot_ptr->header.nextindex - 1;
	dgon_rc = direntry_get_objnam_node( obj_inonum, dtstbl, slots,
					    last_dtidx, found_name_length,
					    found_name, &entry_found );
	}  /* end rootleaf directory tree */
      else {  /* it's a separate node and probably first in a chain */
	    /*
	     * try the first leaf
	     */
	dtstbl = (int8 *) &(leaf_ptr->slot[ leaf_ptr->header.stblindex ]);
	slots = &(leaf_ptr->slot[0]);
	last_dtidx = leaf_ptr->header.nextindex - 1;
	dgon_rc = direntry_get_objnam_node( obj_inonum, dtstbl, slots,
					    last_dtidx, found_name_length,
					    found_name, &entry_found );
	    /*
	     * try the remaining leaves
	     */
	while( (dgon_rc == FSCK_OK)   &&
	       (!entry_found)                &&
	       (leaf_ptr != NULL)               ) {
	  dgon_rc = dnode_get( leaf_ptr->header.next, BYTESPERPAGE, &leaf_ptr );
	  if( dgon_rc != FSCK_OK ) {  /* this is fatal */
	    dgon_rc = FSCK_FAILED_READ_NODE4;
	    }  /* end this is fatal */
	  else {  /* got the sibling leaf node */
	    dtstbl = (int8 *)
			 &(leaf_ptr->slot[ leaf_ptr->header.stblindex ]);
	    slots = &(leaf_ptr->slot[0]);
	    last_dtidx = leaf_ptr->header.nextindex - 1;
	    dgon_rc = direntry_get_objnam_node( obj_inonum, dtstbl,
						slots, last_dtidx,
						found_name_length,
						found_name, &entry_found );
	    }  /* end else got the sibling leaf node */
	  }  /* end while */
	}  /* end it's a separate node and probably first in a chain */
      }  /* end else we found and read the first leaf */
    }  /* end else got the parent inode */

  if( (dgon_rc == FSCK_OK) && (!entry_found) )  { /*
			   * but we saw this entry earlier!
			   */
    dgon_rc = FSCK_FAILED_DIRENTRYGONE;
    }  /* end but we saw this entry earlier! */

  return( dgon_rc );
}                              /* end of direntry_get_objnam ()  */


/*****************************************************************************
 * NAME: direntry_get_objnam_node
 *
 * FUNCTION: Find the file name, in the given directory leaf node, of the
 *           object whose inode number is given.
 *
 * PARAMETERS:
 *      child_inonum       - input - the inode number in the directory entry
 *      dtstbl             - input - pointer to the sorted entry index table
 *                                   in the directory node
 *      slots              - input - pointer to slot[0] in the directory node
 *      last_dtidx         - input - last valid entry in the directory
 *                                   node's sorted entry index table
 *      found_name_length  - input - pointer to a variable in which to return
 *                                   the length of the filename being returned
 *                                   in *found_name
 *      found_name         - input - pointer to a buffer in which to return
 *                                   the filename extracted from the node
 *      entry_found        - input - pointer to a variable in which to return
 *                                   !0 if an entry is found with inode number
 *                                      child_inonum
 *                                    0 if no entry is found with inode number
 *                                      child_inonum
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t direntry_get_objnam_node( ino_t     child_inonum,
				    int8     *dtstbl,
				    dtslot_t *slots,
				    int       last_dtidx,
				    int     *found_name_length,
				    UniChar *found_name,
				    int8    *entry_found
				  )
{
  retcode_t dgonn_rc = FSCK_OK;
  int dtidx;
  ldtentry_t *entry_ptr;
  dtslot_t   *contin_entry_ptr;
  int seg_length, UniChars_left, seg_max;

  seg_max = DTLHDRDATALEN;
  *entry_found = 0;           /* assume it's not here */

   /*
    * see if this leaf has the entry for the requested child
    */
  for( dtidx = 0;
       ( (dgonn_rc == FSCK_OK) &&
	 (dtidx <= last_dtidx)        &&
	 (!(*entry_found))               );
       dtidx++
      )  {  /* for each entry in the dtstbl index to slots */
    entry_ptr = (ldtentry_t *) &(slots[ dtstbl[dtidx] ]);
    if( entry_ptr->inumber == child_inonum ) {
      *entry_found = -1;
      }
    }  /* end for */

      /*
       * if the child's entry was found, construct its name
       */
  if( *entry_found )  {  /* the child's entry was found */

    UniChars_left = entry_ptr->namlen;
    if( UniChars_left > seg_max ) {
      seg_length = seg_max;
      UniChars_left -= seg_max;
      }
    else {
      seg_length = UniChars_left;
      UniChars_left = 0;
      }
    memcpy( (void *) &(found_name[*found_name_length]),
	    (void *) &(entry_ptr->name[0]),
	    (size_t) (seg_length * sizeof(UniChar))
	  );
    *found_name_length = seg_length;

    if( entry_ptr->next != -1 )  {  /* name is continued */
      contin_entry_ptr = (dtslot_t *) &(slots[ entry_ptr->next ]);
      }
    else {
      contin_entry_ptr = NULL;
      }

    seg_max = DTSLOTDATALEN;
    while( (contin_entry_ptr != NULL) &&
	   ((*found_name_length) <= JFS_NAME_MAX) ) {  /* name is continued */

      if( UniChars_left > seg_max ) {
	seg_length = seg_max;
	UniChars_left -= seg_max;
	}
      else {
	seg_length = UniChars_left;
	UniChars_left = 0;
	}

      memcpy((void *) &(found_name[*found_name_length]),
	     (void *) &(contin_entry_ptr->name[0]),
	     (size_t) (seg_length * sizeof(UniChar))
	    );
      *found_name_length += seg_length;
      if( contin_entry_ptr->next != -1 )  {  /* still more */
	contin_entry_ptr = (dtslot_t *) &(slots[ contin_entry_ptr->next ]);
	}  /* end still more */
      else {
	contin_entry_ptr = NULL;
	}
      }  /* end while name is continued */

    if( contin_entry_ptr != NULL )  {  /* we ran amok! */
      dgonn_rc = FSCK_FAILED_DIRGONEBAD2;
      }  /* end we ran amok */
    }  /* end the child's entry was found */

  return( dgonn_rc );
}                              /* end of direntry_get_objnam_node ()  */


/*****************************************************************************
 * NAME: direntry_remove
 *
 * FUNCTION: Issue an fsck message, depending on the message's protocol
 *           according to the fsck message arrays (above).  Log the
 *           message to fscklog if logging is in effect.
 *
 * PARAMETERS:
 *      parent_inoptr  - input - pointer to the directory inode in an fsck 
 *                               buffer
 *      child_inonum   - input - the inode number in the directory entry
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t direntry_remove( dinode_t *parent_inoptr,
			   ino_t     child_inonum
			  )
{
  retcode_t rd_rc = FSCK_OK;
  UniChar uniname[JFS_NAME_MAX];
  int     uniname_length;
  component_t uniname_struct;

  rd_rc = direntry_get_objnam( parent_inoptr->di_number, child_inonum,
			       &uniname_length, &(uniname[0]) );

  if( rd_rc == FSCK_OK ) {  /* got the name */
    uniname_struct.namlen = uniname_length;
    uniname_struct.name = &(uniname[0]);

    rd_rc = fsck_dtDelete( parent_inoptr, &uniname_struct, &child_inonum );
    if( rd_rc == FSCK_OK ) {
      rd_rc = inode_put( parent_inoptr );
      }
    else {
      sprintf( message_parm_0, "%ld", FSCK_INTERNAL_ERROR_61 );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%ld", rd_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%ld", child_inonum );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%ld", parent_inoptr->di_number );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      fsck_send_msg( fsck_INTERNALERROR, 0, 4 );
      rd_rc = FSCK_INTERNAL_ERROR_61;
      }
    }  /* end got the name */
  else {
    sprintf( message_parm_0, "%ld", FSCK_INTERNAL_ERROR_62 );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    sprintf( message_parm_1, "%ld", rd_rc );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    sprintf( message_parm_2, "%ld", child_inonum );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    sprintf( message_parm_3, "%ld", parent_inoptr->di_number );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    fsck_send_msg( fsck_INTERNALERROR, 0, 4 );
    rd_rc = FSCK_INTERNAL_ERROR_62;
    }

  return( rd_rc );
}                              /* end of direntry_remove ()  */


/*****************************************************************************
 * NAME: dTree_binsrch_internal_page
 *
 * FUNCTION: Perform a binary search, on the given dTree internal node, for
 *           the entry which is parent/grandparent/... to the leaf entry 
 *           containing the given filename.
 *
 * PARAMETERS:
 *      dtiptr            - input - pointer to an fsck record describing the
 *                                  directory tree
 *      given_name        - input - pointer to the name to search for 
 *      given_name_len    - input - number of characters in given_name
 *      case_insensitive  - input - !0 => do a case insensitive search 
 *                                   0 => do a case sensitive search
 *      is_root           - input - !0 => specified node is a B+ Tree root
 *                                   0 => specified node is not a B+ Tree root
 *      no_key_match      - input - pointer to a variable in which to return
 *                                  an indication of whether the search has
 *                                  been completed because it has been 
 *                                  determined that no entry in the directory
 *                                  matches given_name
 *                                  !0 if the search should be ended, No 
 *                                     match found 
 *                                   0 if either the search should continue or
 *                                     a match has been found
 *      slot_selected     - input - pointer to a variable in which to return
 *                                  an indication of whether the search has
 *                                  been completed at this level by finding
 *                                  a match, a prefix match, or reason to
 *                                  believe we may still find a match.
 *                                  !0 if a slot has been selected
 *                                   0 if either the search should continue or
 *                                     it has been determined that there is
 *                                     no match in the directory
 *      selected_slotidx  - input - pointer to a variable in which to return
 *                                  the number n such that slot[n] contains
 *                                  (or begins) the key which is a match or
 *                                  a prefix match for given_name
 *      inorecptr         - input - pointer to an fsck inode record describing
 *                                  the inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_binsrch_internal_page( fsck_Dtree_info_ptr dtiptr,
				       UniChar            *given_name,
				       reg_idx_t           given_name_len,
				       int8                case_insensitive,
				       int8                is_root,
				       int8               *no_key_match,
				       int8               *slot_selected,
				       int8               *selected_slotidx,
				       fsck_inode_recptr   inorecptr
				      )
{
  retcode_t   dbip_rc = FSCK_OK;
  UniChar    *this_name;
  reg_idx_t  *this_name_len;
  int         lowidx, mididx, highidx;
  int         this_idx, prev_idx, next_idx;
  int8        is_leaf = 0;
  int         outcome;

  this_name = &(key[1][0]);
  this_name_len = &(key_len[1]);

  *no_key_match = 0;
  *slot_selected = 0;
  *selected_slotidx = 0;

  lowidx = 0;
  highidx = dtiptr->last_dtidx;

  dbip_rc = dTree_key_extract( dtiptr, lowidx, this_name, this_name_len,
			       is_root, is_leaf, inorecptr ); /*
			    * note that we can proceed with (some)
			    * confidence since we never search a directory
			    * until after we've verified it's structure
			    */
  if( dbip_rc == FSCK_OK ) {  /* got the first key */
    dbip_rc = dTree_key_compare( given_name, given_name_len, this_name,
				 (*this_name_len), &outcome );
    if( outcome == LEFT_KEY_LOWER ) {  /* given key < 1st in this node */
      *no_key_match = -1;
      }  /* end given key < 1st in this node */
    else if( outcome == KEYS_MATCH )  {  /* given key == 1st in this node */
      *slot_selected = -1;
      *selected_slotidx = dtiptr->dtstbl[lowidx];
      }  /* end given key == 1st in this node */
    else {  /* given key > 1st in this node */

      dbip_rc = dTree_key_extract( dtiptr, highidx, this_name, this_name_len,
				     is_root, is_leaf, inorecptr ); /*
			    * note that we can proceed with (some)
			    * confidence since we never search a directory
			    * until after we've verified it's structure
			    */
      if( dbip_rc == FSCK_OK ) { /* got the last key */
	dbip_rc = dTree_key_compare( given_name, given_name_len, this_name,
				     (*this_name_len), &outcome );
	if( outcome == LEFT_KEY_HIGHER ) {  /* given key > last in the node */
	  *slot_selected = -1;
	  *selected_slotidx = dtiptr->dtstbl[highidx];
	  }  /* end given key > last in the node */
	else if( outcome == KEYS_MATCH ) {
	  *slot_selected = -1;
	  *selected_slotidx = dtiptr->dtstbl[highidx];
	  }
	}  /* end got the last key */
      }  /* end else given key > last in this node */
    }  /* end got the 0th key */

     /*
      * Find the first key equal or, if no exact match exists, find the
      * last key lower.
      */
  while( (!(*slot_selected)) && (!(*no_key_match))  &&
	 (dbip_rc == FSCK_OK) ) { /*
			  * haven't chosen one but haven't ruled anything out
			  */
    mididx = ((highidx - lowidx) >> 1) + lowidx;
    dbip_rc = dTree_key_extract( dtiptr, mididx, this_name, this_name_len,
				 is_root, is_leaf, inorecptr ); /*
			    * note that we can proceed with (some)
			    * confidence since we never search a directory
			    * until after we've verified it's structure
			    */
    if( dbip_rc == FSCK_OK ) { /* got the mid key */
      dbip_rc = dTree_key_compare( given_name, given_name_len, this_name,
				   (*this_name_len), &outcome );
      if( dbip_rc == FSCK_OK ) {  /* business as usual */

	if( outcome == KEYS_MATCH ) { /* given name == mid key */
	  *slot_selected = -1;
	  *selected_slotidx = dtiptr->dtstbl[mididx];
	  }  /* end given name == mid key */

	else if( outcome == LEFT_KEY_HIGHER ) {  /* given name > mid key */
	  next_idx = mididx + 1;
	  dbip_rc = dTree_key_extract( dtiptr, next_idx, this_name,
				       this_name_len, is_root, is_leaf,
				       inorecptr ); /*
			    * note that we can proceed with (some)
			    * confidence since we never search a directory
			    * until after we've verified it's structure
			    */
	  if( dbip_rc == FSCK_OK ) {  /* got next key */
	    dbip_rc = dTree_key_compare( given_name, given_name_len,
					 this_name, (*this_name_len), &outcome );
	    if( dbip_rc == FSCK_OK )  {  /* nothing untoward */
	      if( outcome == LEFT_KEY_LOWER ) {  /* the next one is higher */
		*slot_selected = -1;
		*selected_slotidx = dtiptr->dtstbl[mididx];
		}
	      else if( outcome == KEYS_MATCH ) {  /* since we've done the
				* extract and compare might as well see if
				* we lucked into a match
				*/
		*slot_selected = -1;
		*selected_slotidx = dtiptr->dtstbl[next_idx];
		}  /* end since we've done the extract and compare... */
	      else {  /* not on or just before the money */
		lowidx = mididx;   /* this key is higher than the middle */
		}  /* end not on or just before the money */
	      }  /* end nothing untoward */
	    }  /* end got the next key */
	  }  /* end given name > mid key */

	else  {  /* given name < mid key */
	  prev_idx = mididx - 1;
	  dbip_rc = dTree_key_extract( dtiptr, prev_idx, this_name,
				       this_name_len, is_root, is_leaf,
				       inorecptr ); /*
			    * note that we can proceed with (some)
			    * confidence since we never search a directory
			    * until after we've verified it's structure
			    */
	  if( dbip_rc == FSCK_OK ) {  /* got previous key */
	    dbip_rc = dTree_key_compare( given_name, given_name_len,
					 this_name, (*this_name_len), &outcome );
	    if( dbip_rc == FSCK_OK )  {  /* nothing untoward */
	      if( outcome == LEFT_KEY_HIGHER ) {  /* the prev one is lower */
		*slot_selected = -1;
		*selected_slotidx = dtiptr->dtstbl[prev_idx];
		}
	      else if( outcome == KEYS_MATCH ) {  /* since we've done the
				* extract and compare might as well see if
				* we stumbled onto a match
				*/
		*slot_selected = -1;
		*selected_slotidx = dtiptr->dtstbl[prev_idx];
		}  /* end since we've done the extract and compare... */
	      else {  /* not on or just after the money */
		highidx = mididx;   /* this key is lower than the middle */
		}  /* end not on or just after the money */
	      }  /* end nothing untoward */
	    }  /* end got the previous key */
	  }  /* end else given name < mid key */
	}  /* end business as usual */
      }  /* end got the mid key */
    }  /* end while haven't chosen one, haven't seen a match,  but ... */

  return( dbip_rc );
}                              /* end of dTree_binsrch_internal_page ()  */


/*****************************************************************************
 * NAME: dTree_binsrch_leaf
 *
 * FUNCTION: Perform a binary search, on the given dTree leaf node, for the
 *           entry (if any) which contains the given filename.
 *
 * PARAMETERS:
 *      dtiptr            - input - pointer to an fsck record describing the
 *                                  directory tree
 *      given_name        - input - pointer to the name to search for 
 *      given_name_len    - input - number of characters in given_name
 *      case_insensitive  - input - !0 => do a case insensitive search
 *                                   0 => do a case sensitive search
 *      is_root           - input - !0 => specified node is a B+ Tree root
 *                                   0 => specified node is not a B+ Tree root
 *      no_key_match      - input - pointer to a variable in which to return
 *                                  an indication of whether the search has
 *                                  been completed because it has been 
 *                                  determined that no entry in the directory
 *                                  matches given_name
 *                                  !0 if the search should be ended, No 
 *                                     match found 
 *                                   0 if either the search should continue or
 *                                     a match has been found
 *      key_matched       - input - pointer to a variable in which to return
 *                                   !0 if an exact match has been found
 *                                    0 if no match has been found
 *      slot_selected     - input - pointer to a variable in which to return
 *                                  an indication of whether the search has
 *                                  been completed at this level by finding
 *                                  a match or reason to reason to believe 
 *                                  we may still find a match.
 *                                  !0 if a slot has been selected
 *                                   0 if either the search should continue or
 *                                     it has been determined that there is
 *                                     no match in the directory
 *      selected_slotidx  - input - pointer to a variable in which to return
 *                                  the number n such that slot[n] contains
 *                                  (or begins) the key which is a match for
 *                                  given_name
 *      inorecptr         - input - pointer to an fsck inode record describing
 *                                  the inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_binsrch_leaf( fsck_Dtree_info_ptr dtiptr,
			      UniChar            *given_name,
			      reg_idx_t           given_name_len,
			      int8                case_insensitive,
			      int8                is_root,
			      int8               *no_key_match,
			      int8               *key_matched,
			      int8               *slot_selected,
			      int8               *selected_slotidx,
			      fsck_inode_recptr   inorecptr
			    )
{
  retcode_t dbl_rc = FSCK_OK;

  UniChar    *this_name;
  reg_idx_t  *this_name_len;
  int         lowidx, mididx, highidx;
  int         prev_idx, next_idx;
  int8        is_leaf = -1;
  int         outcome;

  this_name = &(key[1][0]);
  this_name_len = &(key_len[1]);

  *no_key_match = 0;
  *slot_selected = 0;
  *selected_slotidx = 0;

  lowidx = 0;
  highidx = dtiptr->last_dtidx;

  dbl_rc = dTree_key_extract( dtiptr, lowidx, this_name, this_name_len,
			       is_root, is_leaf, inorecptr ); /*
			    * note that we can proceed with (some)
			    * confidence since we never search a directory
			    * until after we've verified it's structure
			    */

  if( dbl_rc == FSCK_OK ) { /* got the 0th key */
		/*
		 * If this is a case insensitive search, we need to fold the
		 * extracted key to upper case before doing the comparison.
		 * The given key should already be in all upper case.
		 */
	if( case_insensitive ) {
	    dTree_key_to_upper( this_name,   &(ukey[0][0]), (*this_name_len) );
	    dbl_rc = dTree_key_compare( given_name, given_name_len, 
					    &(ukey[0][0]), 
					    (*this_name_len), &outcome );
	    }
	else {  /* not case insensitive */
	
	    dbl_rc = dTree_key_compare( given_name, given_name_len, 
					   this_name,
					    (*this_name_len), &outcome );
	    }  /* end else not case insensitive */
    if( outcome == LEFT_KEY_LOWER ) {  /* given key < 1st in this node */
      *no_key_match = -1;
      }  /* end given key < 1st in this node */
    else if( outcome == KEYS_MATCH )  {  /* given key == 1st in this node */
      *no_key_match = 0;
      *key_matched = -1;
      *slot_selected = -1;
      *selected_slotidx = dtiptr->dtstbl[lowidx];
      }  /* end given key == 1st in this node */
    else {  /* given key > 1st in this node */
      dbl_rc = dTree_key_extract( dtiptr, highidx, this_name, this_name_len,
				   is_root, is_leaf, inorecptr ); /*
			    * note that we can proceed with (some)
			    * confidence since we never search a directory
			    * until after we've verified it's structure
			    */
      if( dbl_rc == FSCK_OK ) { /* got the last key */
		/*
		 * If this is a case insensitive search, we need to fold the
		 * extracted key to upper case before doing the comparison.
		 * The given key should already be in all upper case.
		 */
	if( case_insensitive ) {
	    dTree_key_to_upper( this_name,   &(ukey[0][0]), (*this_name_len) );
	    dbl_rc = dTree_key_compare( given_name, given_name_len, 
					    &(ukey[0][0]), 
					    (*this_name_len), &outcome );
	    }
	else {  /* not case insensitive */
	
	    dbl_rc = dTree_key_compare( given_name, given_name_len, 
					   this_name,
					    (*this_name_len), &outcome );
	    }  /* end else not case insensitive */
	if( outcome == LEFT_KEY_HIGHER ) {  /* given key > last in the node */
	  *no_key_match = -1;
	  }  /* end given key > last in the node */
	else if( outcome == KEYS_MATCH ) {
	  *no_key_match = 0;
	  *key_matched = -1;
	  *slot_selected = -1;
	  *selected_slotidx = dtiptr->dtstbl[highidx];
	  }
	}  /* end got the last key */
      }  /* end else given key > last in this node */
    }  /* end got the 0th key */

     /*
      * Try to find a name match
      */
  while( (!(*slot_selected)) && (!(*no_key_match)) &&
	 (dbl_rc == FSCK_OK) ) { /*
			  * haven't chosen one, haven't seen a match,
			  * but haven't ruled anything out
			  */
    mididx = ((highidx - lowidx) >> 1) + lowidx;
    dbl_rc = dTree_key_extract( dtiptr, mididx, this_name, this_name_len,
				 is_root, is_leaf, inorecptr ); /*
			    * note that we can proceed with (some)
			    * confidence since we never search a directory
			    * until after we've verified it's structure
			    */
    if( dbl_rc == FSCK_OK ) { /* got the mid key */
		/*
		 * If this is a case insensitive search, we need to fold the
		 * extracted key to upper case before doing the comparison.
		 * The given key should already be in all upper case.
		 */
	if( case_insensitive ) {
	    dTree_key_to_upper( this_name,   &(ukey[0][0]), (*this_name_len) );
	    dbl_rc = dTree_key_compare( given_name, given_name_len, 
					    &(ukey[0][0]), 
					    (*this_name_len), &outcome );
	    }
	else {  /* not case insensitive */
	
	    dbl_rc = dTree_key_compare( given_name, given_name_len, 
					   this_name,
					    (*this_name_len), &outcome );
	    }  /* end else not case insensitive */

      if( dbl_rc == FSCK_OK ) {  /* business as usual */

	if( outcome == KEYS_MATCH ) { /* given name == mid key */
	  *no_key_match = 0;
	  *key_matched = -1;
	  *slot_selected = -1;
	  *selected_slotidx = dtiptr->dtstbl[mididx];
	  }  /* end given name == mid key */

	else if( outcome == LEFT_KEY_HIGHER ) {  /* given name > mid key */
	  next_idx = mididx + 1;
	  dbl_rc = dTree_key_extract( dtiptr, next_idx, this_name,
				      this_name_len, is_root, is_leaf,
				      inorecptr ); /*
			    * note that we can proceed with (some)
			    * confidence since we never search a directory
			    * until after we've verified it's structure
			    */
	  if( dbl_rc == FSCK_OK ) {  /* got next key */
		/*
		 * If this is a case insensitive search, we need to fold the
		 * extracted key to upper case before doing the comparison.
		 * The given key should already be in all upper case.
		 */
	    if( case_insensitive ) {
	      dTree_key_to_upper( this_name,   &(ukey[0][0]), (*this_name_len) );
	      dbl_rc = dTree_key_compare( given_name, given_name_len, 
					      &(ukey[0][0]), 
					      (*this_name_len), &outcome );
	      }
	    else {  /* not case insensitive */
//PS21032004 Begin	
	      dbl_rc = dTree_key_compare( given_name, given_name_len, 
					     this_name,
					      (*this_name_len), &outcome );
	      }  /* end else not case insensitive */
	    if( dbl_rc == FSCK_OK )  {  /* nothing untoward */
	      if( outcome == LEFT_KEY_LOWER ) {  /* the next one is higher */
		*no_key_match = -1;
		}
	      else if( outcome == KEYS_MATCH ) {  /* since we've done the
				* extract and compare might as well see if
				* we lucked into a match
				*/
		*no_key_match = 0;
		*key_matched = -1;
		*slot_selected = -1;
		*selected_slotidx = dtiptr->dtstbl[next_idx];
		}  /* end since we've done the extract and compare... */
	      else {  /* not on or just before the money */
		lowidx = mididx;   /* this key is higher than the middle */
		}  /* end not on or just before the money */
	      }  /* end nothing untoward */
	    }  /* end got the next key */
	  }  /* end given name > mid key */

	else  {  /* given name < mid key */
	  prev_idx = mididx - 1;
	  dbl_rc = dTree_key_extract( dtiptr, prev_idx, this_name,
				      this_name_len, is_root, is_leaf,
				      inorecptr ); /*
			    * note that we can proceed with (some)
			    * confidence since we never search a directory
			    * until after we've verified it's structure
			    */
	  if( dbl_rc == FSCK_OK ) {  /* got previous key */
		/*
		 * If this is a case insensitive search, we need to fold the
		 * extracted key to upper case before doing the comparison.
		 * The given key should already be in all upper case.
		 */
	    if( case_insensitive ) {
	        dTree_key_to_upper( this_name,   &(ukey[0][0]), (*this_name_len) );
	        dbl_rc = dTree_key_compare( given_name, given_name_len, 
					    &(ukey[0][0]), 
					    (*this_name_len), &outcome );
	        }
	    else {  /* not case insensitive */
	
	        dbl_rc = dTree_key_compare( given_name, given_name_len, 
					   this_name,
					    (*this_name_len), &outcome );
	        }  /* end else not case insensitive */
	    if( dbl_rc == FSCK_OK )  {  /* nothing untoward */
	      if( outcome == LEFT_KEY_HIGHER ) {  /* the prev one is lower */
		*no_key_match = -1;
	       }
	      else if( outcome == KEYS_MATCH ) {  /* since we've done the
				* extract and compare might as well see if
				* we stumbled onto a match
				*/
		*no_key_match = 0;
		*key_matched = -1;
		*slot_selected = -1;
		*selected_slotidx = dtiptr->dtstbl[prev_idx];
		}  /* end since we've done the extract and compare... */
	      else {  /* not on or just after the money */
		highidx = mididx;   /* this key is lower than the middle */
		}  /* end not on or just after the money */
	      }  /* end nothing untoward */
	    }  /* end got the previous key */
	  }  /* end else given name < mid key */
	}  /* end business as usual */
      }  /* end got the mid key */
    }  /* end while haven't chosen one, haven't seen a match,  but ... */

  return( dbl_rc );
}                              /* end of dTree_binsrch_leaf ()  */


/*****************************************************************************
 * NAME: dTree_key_compare
 *
 * FUNCTION: Compare the two strings which are given.
 *
 * PARAMETERS:
 *      left_key       - input - pointer to the first in a pair of keys to
 *                               compare
 *      left_key_len   - input - number of UniChars in left_key
 *      right_key      - input - pointer to the second in a pair of keys to
 *                               compare
 *      right_key_len  - input - number of UniChars in right_key
 *      keys_relation  - input - pointer to a variable in which to return
 *                               { LEFT_KEY_LOWER | KEYS_MATCH | LEFT_KEY_HIGHER }
 *
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_key_compare ( UniChar *left_key,
			      uint8    left_key_num_unichars,
			      UniChar *right_key,
			      uint8    right_key_num_unichars,
			      int     *keys_relation
			    )
{
  retcode_t dkc_rc = FSCK_OK;
  int outcome;
  int left_key_len, right_key_len;

  left_key_len = left_key_num_unichars;           //PS21032004
  right_key_len = right_key_num_unichars;         //PS21032004

  if( right_key_len < left_key_len ) { /* right key is shorter */
    outcome = UniStrncmp( (void *) left_key,
		            (void *) right_key,
		            right_key_len );         //PS21032004
    if( outcome < 0 )  {  /* right key is alphabetically greater */
      *keys_relation = LEFT_KEY_LOWER;
      }
    else {
      *keys_relation = LEFT_KEY_HIGHER;
      }
    }  /* end right key is shorter */

  else if( right_key_len > left_key_len ) {  /* right key is longer */

    outcome = UniStrncmp( (void *) left_key,
		            (void *) right_key,
		            left_key_len );             //PS21032004
    if( outcome <= 0 )  {  /* right key is alphabetically greater */
      *keys_relation = LEFT_KEY_LOWER;
      }
    else {
      *keys_relation = LEFT_KEY_HIGHER;
      }
    }  /* end right key is longer */

  else {  /* keys same length */
    outcome = UniStrncmp( (void *) left_key,
		            (void *) right_key,
		            left_key_len );
    if( outcome < 0 )  {  /* right key is alphabetically greater */
      *keys_relation = LEFT_KEY_LOWER;
      }
    else 
      {
      if( outcome > 0 )
        {
        *keys_relation = LEFT_KEY_HIGHER;
        }
      else
        {
        *keys_relation = KEYS_MATCH; //+++ PS
        }
      }
    }  /* end keys same length */
  return( dkc_rc );
}                              /* end of dTree_key_compare ()  */


/*****************************************************************************
 * NAME: dTree_key_compare_leaflvl
 *
 * FUNCTION: Compare the 2 given strings according to the rules for 
 *           sibling entries in a leaf node.
 *
 * PARAMETERS:
 *      left_key       - input - pointer to the first in a pair of keys to
 *                               compare
 *      left_key_len   - input - number of UniChars in left_key
 *      right_key      - input - pointer to the second in a pair of keys to
 *                               compare
 *      right_key_len  - input - number of UniChars in right_key
 *      keys_ok        - input - pointer to a variable in which to return
 *                               !0 if the relation between left_key and
 *                                  right_key, on the dTree leaf level,
 *                                  is valid
 *                                0 if the relation between left_key and
 *                                  right_key, on the dTree leaf level,
 *                                  is not valid
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_key_compare_leaflvl( UniChar *left_key,
				     uint8    left_key_len,
				     UniChar *right_key,
				     uint8    right_key_len,
				     int8    *keys_ok
				    )
{
  retcode_t dkcl_rc = FSCK_OK;
  int outcome;

  *keys_ok = 0;           /* assume incorrect relation between the keys */

  if( sb_ptr->s_flag & JFS_OS2 ) {  /* case is preserved but ignored */
    dkcl_rc = dTree_key_to_upper( left_key, &(ukey[0][0]),
				  (int32) left_key_len );
    if( dkcl_rc == FSCK_OK ) {
      dkcl_rc = dTree_key_to_upper( right_key, &(ukey[1][0]),
				    (int32) right_key_len );
      if( dkcl_rc == FSCK_OK ) {
	dkcl_rc = dTree_key_compare( &(ukey[0][0]), left_key_len,
				     &(ukey[1][0]), right_key_len,
				     &outcome );
	}
      }
    }  /* end case is preserved but ignored */

  else {  /* case sensitive */
    dkcl_rc = dTree_key_compare( left_key, left_key_len,
				 right_key, right_key_len,
				 &outcome );
    }  /* end else case sensitive */

  if( dkcl_rc == FSCK_OK ) {  /* nothing unexpected happened */
//PS21032004    if( outcome == LEFT_KEY_LOWER ) {  /* right key greater */
    if( (outcome == KEYS_MATCH) || (outcome == LEFT_KEY_LOWER ) ) {  /* right key greater */
      *keys_ok = -1;
      }  /* end right key greater */
    }  /* end nothing unexpected happened */

  return( dkcl_rc );
}                              /* end of dTree_key_compare_leaflvl ()  */


/*****************************************************************************
 * NAME: dTree_key_compare_prntchld
 *
 * FUNCTION: Compare the two given strings according to the rules for 
 *           a parent entry key and the first (sorted) entry key in the 
 *           node described by the parent entry.
 *
 * PARAMETERS:
 *      dtiptr          - input - pointer to an fsck record describing the
 *                                directory tree
 *      parent_key      - input - pointer to the key extracted from the
 *                                parent node entry
 *      parent_key_len  - input - number of UniChars in parent_key
 *      child_key       - input - pointer to the key extracted from the first
 *                                (sorted) entry in the child node described
 *                                by the entry from which parent_key was
 *                                taken
 *      child_key_len   - input - number of UniChars in child_key
 *      keys_ok         - input - pointer to a variable in which to return
 *                                !0 if the relation between parent_key and
 *                                   child_key, where child_key is the first
 *                                   key in the child node, is valid
 *                                 0 if the relation between parent_key and
 *                                   child_key, where child_key is the first
 *                                   key in the child node, is not valid
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_key_compare_prntchld( fsck_Dtree_info_ptr  dtiptr,
				      UniChar             *parent_key,
				      uint8                parent_key_len,
				      UniChar             *child_key,
				      uint8                child_key_len,
				      int8                *keys_ok
				     )
{
  retcode_t dnfk_rc = FSCK_OK;
  int outcome;

  *keys_ok = 0;           /* assume incorrect relation between the keys */

  if( dtiptr->leaf_level == dtiptr->this_Qel->node_level )  {  /*
			     * the child is a leaf so its key is mixed case
			     */
    dnfk_rc = dTree_key_to_upper( child_key,
				    &(ukey[0][0]),
				    (int32) child_key_len );
    if( dnfk_rc == FSCK_OK ) {
      dnfk_rc = dTree_key_compare( parent_key, parent_key_len,
		                   &(ukey[0][0]), child_key_len,
				   &outcome );
      }
    }  /* end the child is a leaf so its key is mixed case */

  else {  /* the child is not a leaf */
    dnfk_rc = dTree_key_compare( parent_key, parent_key_len,
				 child_key,  child_key_len,
				 &outcome );
    }  /* end else the child is not a leaf */

  if(    (dnfk_rc == FSCK_OK)  &&
         ((outcome == KEYS_MATCH) || (outcome == LEFT_KEY_LOWER))  
     )  { /* parent is less than or equal to first child */
    *keys_ok = -1;
    }

  return( dnfk_rc );
}                              /* end of dTree_key_compare_prntchld ()  */


/*****************************************************************************
 * NAME: dTree_key_compare_samelvl
 *
 * FUNCTION: Compare the 2 given strings according to the rules for 
 *           sibling entries in an internal node.
 *
 * PARAMETERS:
 *      left_key       - input - pointer to the first in a pair of keys to
 *                               compare
 *      left_key_len   - input - number of UniChars in left_key
 *      right_key      - input - pointer to the second in a pair of keys to
 *                               compare
 *      right_key_len  - input - number of UniChars in right_key
 *      keys_ok        - input - pointer to a variable in which to return
 *                               !0 if the relation between left_key and
 *                                  right_key, on the same dTree level,
 *                                  is valid
 *                                0 if the relation between left_key and
 *                                  right_key, on the same dTree level,
 *                                  is not valid
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_key_compare_samelvl( UniChar *left_key,
				     uint8    left_key_len,
				     UniChar *right_key,
				     uint8    right_key_len,
				     int8    *keys_ok
				    )
{
  retcode_t dkcs_rc = FSCK_OK;
  int outcome;

  *keys_ok = 0;           /* assume incorrect relation between the keys */
  dkcs_rc = dTree_key_compare( left_key, left_key_len,
			       right_key, right_key_len,
			       &outcome );

  if( dkcs_rc == FSCK_OK ) {  /* nothing unexpected happened */

    if( outcome == LEFT_KEY_LOWER ) {  /* right key greater */
      *keys_ok = -1;
      }  /* end right key greater */
    }  /* end nothing unexpected happened */
  return( dkcs_rc );
}                              /* end of dTree_key_compare_samelvl ()  */


/**************************************************************************
 * NAME:  dTree_key_extract
 *
 * FUNCTION: Extract the specified directory entry (either internal or
 *           leaf) key and concatenate it's segments (if more than one).
 *           Assume the directory structure has already been validated.
 *
 * PARAMETERS:
 *      dtiptr       - input - pointer to an fsck record describing the
 *                             directory tree
 *      start_dtidx  - input - index of the entry in the directory node's
 *                             sorted entry index table containing the
 *                             slot number of the (first segment of the)
 *                             key to extract
 *      key_space    - input - pointer to a buffer in which to return
 *                             the directory key (a complete filename if
 *                             the node is a leaf) extracted
 *      key_length   - input - pointer to a variable in which to return
 *                             the length of the directory key being returned
 *                             in *key_space
 *      is_root      - input - !0 => the specified node is a B+ Tree root
 *                              0 => the specified node is not a B+ Tree root
 *      is_leaf      - input - !0 => the specified node is a leaf node
 *                              0 => the specified node is not a leaf node
 *      inorecptr    - input - pointer to an fsck inode record describing
 *                             the inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_key_extract( fsck_Dtree_info_ptr dtiptr,
			     int                 start_dtidx,
			     UniChar            *key_space,
			     reg_idx_t          *key_length,
			     int8                is_root,
			     int8                is_leaf,
			     fsck_inode_recptr   inorecptr
			    )
{
  retcode_t dek_rc = FSCK_OK;
  int this_slotidx, next_slotidx;
  idtentry_t *ientry_ptr;
  ldtentry_t *lentry_ptr;
  UniChar    *name_seg;
  int        seg_length, UniChars_left, seg_max;
  dtslot_t   *contin_entry_ptr;

  this_slotidx = dtiptr->dtstbl[start_dtidx];

  if( is_leaf )  {
    lentry_ptr = (ldtentry_t *) &(dtiptr->slots[this_slotidx]);
    name_seg = &(lentry_ptr->name[0]);
    *key_length = lentry_ptr->namlen;
    next_slotidx = lentry_ptr->next;
    seg_max = DTLHDRDATALEN;
    }
  else {
    ientry_ptr = (idtentry_t *) &(dtiptr->slots[this_slotidx]);
    name_seg = &(ientry_ptr->name[0]);
    *key_length = ientry_ptr->namlen;
    next_slotidx = ientry_ptr->next;
    seg_max = DTIHDRDATALEN;
    }

  if( (*key_length) > JFS_NAME_MAX) {  /* name too long */
    inorecptr->ignore_alloc_blks = 1;
    }  /* end name too long */
  else {
    UniChars_left = *key_length;
    *key_length = 0;
    }

  while( (dek_rc == FSCK_OK) &&
	 (this_slotidx != -1) &&
	 (!inorecptr->ignore_alloc_blks)  ) {

    if( (this_slotidx > dtiptr->max_slotidx) ||
        (this_slotidx < DTENTRYSTART)          ) {  /* idx out of bounds */
      inorecptr->ignore_alloc_blks = 1;
      }  /* end index out of bounds */
    else {  /* else no reason to think there's a problem */
      if( UniChars_left > seg_max )  {  /* this isn't the last */
	seg_length = seg_max;
	UniChars_left = UniChars_left - seg_max;
	}
      else {
	seg_length = UniChars_left;
	UniChars_left = 0;
	}
      memcpy( (void *) &(key_space[*key_length]),
	      (void *) &(name_seg[0]),
	      (seg_length * sizeof(UniChar))
	    );  /* copy this section of the name into the buffer */
      *key_length += seg_length;

      this_slotidx = next_slotidx;
      if( next_slotidx != -1 )  {  /* it's not the end of chain marker */
	contin_entry_ptr =&(dtiptr->slots[this_slotidx]);
	name_seg = &(contin_entry_ptr->name[0]);
	seg_length = contin_entry_ptr->cnt;
	next_slotidx = contin_entry_ptr->next;
	seg_max = DTSLOTDATALEN;
	}  /* end it's not the end of chain marker */
      }  /* end else no reason to think there's a problem */
    }  /* end while */

  return( dek_rc );
}                              /* end of dTree_key_extract ()  */


/*****************************************************************************
 * NAME:  dTree_key_extract_cautiously
 *
 * FUNCTION: Extract the specified directory entry (either internal or
 *           leaf) key and concatenate it's segments (if more than one).
 *           Do not assume the directory structure has been validated.
 *
 * PARAMETERS:
 *      dtiptr       - input - pointer to an fsck record describing the
 *                             directory tree
 *      start_dtidx  - input - index of the entry in the directory node's
 *                             sorted entry index table containing the
 *                             slot number of the (first segment of the)
 *                             key to extract
 *      key_space    - input - pointer to a buffer in which to return
 *                             the directory key (a complete filename if
 *                             the node is a leaf) extracted
 *      key_length   - input - pointer to a variable in which to return
 *                             the length of the directory key being returned
 *                             in *key_space
 *      is_root      - input - !0 => the specified node is a B+ Tree root
 *                              0 => the specified node is not a B+ Tree root
 *      is_leaf      - input - !0 => the specified node is a leaf node
 *                              0 => the specified node is not a leaf node
 *      inorecptr    - input - pointer to an fsck inode record describing
 *                             the inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_key_extract_cautiously( fsck_Dtree_info_ptr dtiptr,
					int                 start_dtidx,
					UniChar            *key_space,
					reg_idx_t          *key_length,
					int8                is_root,
					int8                is_leaf,
					fsck_inode_recptr   inorecptr
				       )
{
  retcode_t deck_rc = FSCK_OK;
  int8 temp_slot_map[DTPAGEMAXSLOT];
  int this_slotidx, next_slotidx, this_charidx;
  idtentry_t *ientry_ptr;
  ldtentry_t *lentry_ptr;
  UniChar    *name_seg;
  int        seg_length, UniChars_left, seg_max;
  dtslot_t   *contin_entry_ptr;

  memset( (void *) &(temp_slot_map[0]), 0, DTPAGEMAXSLOT );

  this_slotidx = dtiptr->dtstbl[start_dtidx];

  if( is_leaf )  {
    lentry_ptr = (ldtentry_t *) &(dtiptr->slots[this_slotidx]);
    name_seg = &(lentry_ptr->name[0]);
    *key_length = lentry_ptr->namlen;
    next_slotidx = lentry_ptr->next;
    seg_max = DTLHDRDATALEN;
    }
  else {
    ientry_ptr = (idtentry_t *) &(dtiptr->slots[this_slotidx]);
    name_seg = &(ientry_ptr->name[0]);
    *key_length = ientry_ptr->namlen;
    next_slotidx = ientry_ptr->next;
    seg_max = DTIHDRDATALEN;
    }

  if( (*key_length) > JFS_NAME_MAX) {  /* name too long */
    inorecptr->ignore_alloc_blks = 1;
    }  /* end name too long */
  else {
    UniChars_left = *key_length;
    *key_length = 0;
    }

  while( (deck_rc == FSCK_OK) &&
	 (this_slotidx != -1) &&
	 (!inorecptr->ignore_alloc_blks)  ) {

    if( (this_slotidx > dtiptr->max_slotidx) || 
        (this_slotidx < DTENTRYSTART)        || /*
			     * index is out of bounds OR
			     */
	(temp_slot_map[this_slotidx] != 0)  /*
			     * index was seen earlier this key chain
			     */
       )  {  /* bad chain */
      inorecptr->ignore_alloc_blks = 1;
      }  /* end bad chain */
    else {  /* else no reason to think there's a problem */
      temp_slot_map[this_slotidx] = -1;   /* mark the slot used */
      if( UniChars_left > seg_max )  {  /* this isn't the last */
	seg_length = seg_max;
	UniChars_left = UniChars_left - seg_max;
	}
      else {
	seg_length = UniChars_left;
	UniChars_left = 0;
	}
      memcpy( (void *) &(key_space[*key_length]),
	      (void *) &(name_seg[0]),
	      (seg_length * sizeof(UniChar))
	    );  /* copy this section of the name into the buffer */
      *key_length += seg_length;

      this_slotidx = next_slotidx;
      if( next_slotidx != -1 )  {  /* it's not the end of chain marker */
	contin_entry_ptr =&(dtiptr->slots[this_slotidx]);
	name_seg = &(contin_entry_ptr->name[0]);
	seg_length = contin_entry_ptr->cnt;
	next_slotidx = contin_entry_ptr->next;
	seg_max = DTSLOTDATALEN;
	}  /* end it's not the end of chain marker */
      }  /* end else no reason to think there's a problem */
    }  /* end while */
	  
	/*
	 * check for a null character embedded in the name
	 */
  this_charidx = 0;
  while( (deck_rc == FSCK_OK) &&
	 (!inorecptr->ignore_alloc_blks) &&
	 (this_charidx < *key_length )         )  {
    if( ((unsigned short *)key_space)[this_charidx] == (unsigned short)NULL )  {
      inorecptr->ignore_alloc_blks = 1;
      }
    else  {
      this_charidx++;
      }
    }

  return( deck_rc );
}                              /* end of dTree_key_extract_cautiously ()  */


/*****************************************************************************
 * NAME:  dTree_key_extract_record
 *
 * FUNCTION: Extract the specified directory entry (either internal or
 *           leaf) key and concatenate it's segments (if more than one).
 *           Directory structure validation is in progress.
 *
 * PARAMETERS:
 *      dtiptr       - input - pointer to an fsck record describing the
 *                             directory tree
 *      start_dtidx  - input - index of the entry in the directory node's
 *                             sorted entry index table containing the
 *                             slot number of the (first segment of the)
 *                             key to extract
 *      key_space    - input - pointer to a buffer in which to return
 *                             the directory key (a complete filename if
 *                             the node is a leaf) extracted
 *      key_length   - input - pointer to a variable in which to return
 *                             the length of the directory key being returned
 *                             in *key_space
 *      is_root      - input - !0 => the specified node is a B+ Tree root
 *                              0 => the specified node is not a B+ Tree root
 *      is_leaf      - input - !0 => the specified node is a leaf node
 *                              0 => the specified node is not a leaf node
 *      inorecptr    - input - pointer to an fsck inode record describing
 *                             the inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_key_extract_record( fsck_Dtree_info_ptr dtiptr,
				    int                 start_dtidx,
				    UniChar            *key_space,
				    reg_idx_t          *key_length,
				    int8                is_root,
				    int8                is_leaf,
				    fsck_inode_recptr   inorecptr
				  )
{
  retcode_t derk_rc = FSCK_OK;
  int this_slotidx, next_slotidx, this_charidx;
  idtentry_t *ientry_ptr;
  ldtentry_t *lentry_ptr;
  UniChar    *name_seg;
  int        seg_length, UniChars_left, seg_max;
  dtslot_t   *contin_entry_ptr;

  this_slotidx = dtiptr->dtstbl[start_dtidx];

  if( is_leaf )  {
    lentry_ptr = (ldtentry_t *) &(dtiptr->slots[this_slotidx]);
    name_seg = &(lentry_ptr->name[0]);
    *key_length = lentry_ptr->namlen;
    next_slotidx = lentry_ptr->next;
    seg_max = DTLHDRDATALEN;
    }
  else {
    ientry_ptr = (idtentry_t *) &(dtiptr->slots[this_slotidx]);
    name_seg = &(ientry_ptr->name[0]);
    *key_length = ientry_ptr->namlen;
    next_slotidx = ientry_ptr->next;
    seg_max = DTIHDRDATALEN;
    }

  if( (*key_length) > JFS_NAME_MAX) {  /* name too long */
    inorecptr->ignore_alloc_blks = 1;
#ifdef _JFS_DEBUG_PS
 printf("dTree_key_extract_record Name is long=%u\n\r",*key_length);
#endif
    }  /* end name too long */
  else {
    UniChars_left = *key_length;
    *key_length = 0;
    }

  while( (derk_rc == FSCK_OK) &&
	 (this_slotidx != -1) &&
	 (!inorecptr->ignore_alloc_blks)  ) {

    if( (this_slotidx > dtiptr->max_slotidx) || 
        (this_slotidx < DTENTRYSTART)        ||  /*
			     * index is out of bounds OR
			     */
	(dtiptr->slot_map[this_slotidx] != 0)  /*
			     * index was seen in a previous key chain
			     */
       )  {  /* bad chain */
      inorecptr->ignore_alloc_blks = 1;
#ifdef _JFS_DEBUG
 printf("dTree_key_extract_record Bad chain idx=%u max=%u DTENTRYSTART=%u\n\r",this_slotidx,dtiptr->max_slotidx,DTENTRYSTART);
#endif
      }  /* end bad chain */
    else {  /* else no reason to think there's a problem */
      dtiptr->slot_map[this_slotidx] = -1;   /* mark the slot used */
      if( UniChars_left > seg_max )  {  /* this isn't the last */
	seg_length = seg_max;
	UniChars_left = UniChars_left - seg_max;
	}
      else {
	seg_length = UniChars_left;
	UniChars_left = 0;
	}
      memcpy( (void *) &(key_space[*key_length]),
	      (void *) &(name_seg[0]),
	      (seg_length * sizeof(UniChar))
	    );  /* copy this section of the name into the buffer */
      *key_length += seg_length;

      this_slotidx = next_slotidx;
      if( next_slotidx != -1 )  {  /* it's not the end of chain marker */
	contin_entry_ptr =&(dtiptr->slots[this_slotidx]);
	name_seg = &(contin_entry_ptr->name[0]);
	seg_length = contin_entry_ptr->cnt;
	next_slotidx = contin_entry_ptr->next;
	seg_max = DTSLOTDATALEN;
	}  /* end it's not the end of chain marker */
      }  /* end else no reason to think there's a problem */
    }  /* end while */
	  
	/*
	 * check for a null character embedded in the name
	 */
  this_charidx = 0;
  while( (derk_rc == FSCK_OK) &&
	 (!inorecptr->ignore_alloc_blks) &&
	 (this_charidx < *key_length )         )  {
    if( ((unsigned short *)key_space)[this_charidx] == (unsigned short)NULL )  {
#ifdef _JFS_DEBUG
 printf("dTree_key_extract_record Pointer is NULL, index=%u\n\r",this_charidx);
#endif
      inorecptr->ignore_alloc_blks = 1;
      }
    else  {
      this_charidx++;
      }
    }

  return( derk_rc );
}                              /* end of dTree_key_extract_record ()  */


/*****************************************************************************
 * NAME: dTree_key_to_upper
 *
 * FUNCTION: Fold the given mixed-case string to upper case.
 *
 * PARAMETERS:
 *      given_name     - input - pointer to the name which is to be folded to
 *                               upper case
 *      name_in_upper  - input - pointer to a buffer in which to return the
 *                               string which results from folding given_name
 *                               to upper case
 *      name_len       - input - the number of UniChars in given_name
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_key_to_upper( UniChar    *given_name,
			      UniChar    *name_in_upper,
			      int32       name_len
			     )
{
  retcode_t dktu_rc = FSCK_OK;
  int charidx;
  for( charidx = 0; (charidx < name_len); charidx++ ) {
    name_in_upper[charidx] = UniToupper(given_name[charidx]);  
    }
  return( dktu_rc );
}                              /* end of dTree_key_to_upper ()  */
	
	
/*****************************************************************************
 * NAME: dTree_node_first_key
 *
 * FUNCTION:  Assists dTree_processing.
 *
 * PARAMETERS:
 *      dtiptr          - input - pointer to an fsck record describing the
 *                                directory tree
 *      inorecptr       - input - pointer to an fsck inode record describing
 *                                the inode
 *      msg_info_ptr    - input - pointer to data needed to issue messages 
 *                                about the inode
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_node_first_key( fsck_Dtree_info_ptr dtiptr,
				fsck_inode_recptr   inorecptr,
				fsck_msg_info_ptr   msg_info_ptr,
				int                 desired_action
			      )
{
  retcode_t dnfk_rc = FSCK_OK;
  int8 keys_ok;
  dnfk_rc = dTree_key_compare_prntchld( dtiptr,
					&(dtiptr->this_Qel->node_key[0]),
					dtiptr->this_Qel->node_key_len,
					&(dtiptr->key[dtiptr->this_key_idx][0]),
					dtiptr->key_len[dtiptr->this_key_idx],
					&keys_ok );
#ifdef _JFS_DEBUG_PS
 printf("dTree key compare %u\n\r",dnfk_rc);
 printf("inorecptr->ignore_alloc_blks %u keys_ok\n\r",inorecptr->ignore_alloc_blks,keys_ok);
#endif
  if( dnfk_rc == FSCK_OK ) {  /* nothing strange happened */
    if( !keys_ok )  {  /* invalid key in first slot */
      inorecptr->ignore_alloc_blks = 1;
      if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported yet */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = msg_info_ptr->msg_inotyp;
        msgprms[1] = message_parm_1;
        msgprmidx[1] = msg_info_ptr->msg_inopfx;
        sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%d", 1 );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        fsck_send_msg( fsck_BADKEYS, 0, 4 );
	}  /* end not reported yet */
      }  /* end invalid key in first slot */
    else {  /* 1st slot may be ok */
      if( dtiptr->last_level == dtiptr->this_Qel->node_level ) {  /*
			      * the node is not 1st in its level
			      */
	if( dtiptr->leaf_level == dtiptr->this_Qel->node_level )  {  /*
			      * it's a leaf
			      */
	  dnfk_rc = dTree_key_compare_leaflvl(
				       &(dtiptr->key[dtiptr->last_key_idx][0]),
				       dtiptr->key_len[dtiptr->last_key_idx],
				       &(dtiptr->key[dtiptr->this_key_idx][0]),
				       dtiptr->key_len[dtiptr->this_key_idx],
				       &keys_ok );
	  }  /* end it's a leaf */
	else {  /* not a leaf */
	  dnfk_rc = dTree_key_compare_samelvl(
				       &(dtiptr->key[dtiptr->last_key_idx][0]),
				       dtiptr->key_len[dtiptr->last_key_idx],
				       &(dtiptr->key[dtiptr->this_key_idx][0]),
				       dtiptr->key_len[dtiptr->this_key_idx],
				       &keys_ok );
	  }  /* end else not a leaf */
#ifdef _JFS_DEBUG_PS
 printf("dTree next key compare %u keys_ok %d\n\r",dnfk_rc,keys_ok);
 printf("inorecptr->ignore_alloc_blks %u\n\r",inorecptr->ignore_alloc_blks);
#endif
	if( !keys_ok )  {  /* keys out of sort order! */
	  inorecptr->ignore_alloc_blks = 1;
	  if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported */
            msgprms[0] = message_parm_0;
            msgprmidx[0] = msg_info_ptr->msg_inotyp;
            msgprms[1] = message_parm_1;
            msgprmidx[1] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
            msgprms[2] = message_parm_2;
            msgprmidx[2] = 0;
            sprintf( message_parm_3, "%d", 2 );
            msgprms[3] = message_parm_3;
            msgprmidx[3] = 0;
            fsck_send_msg( fsck_BADKEYS, 0, 4 );
	    }  /* end not reported */
	  }  /* end keys out of sort order! */
	}  /* end the node is not 1st in its level */
      }  /* end else 1st slot may be ok */
    }  /* end nothing strange happened */
  return( dnfk_rc );
}                              /* end of dTree_node_first_key ()  */
	
	
/*****************************************************************************
 * NAME: dTree_node_first_in_level
 *
 * FUNCTION:  Assists dTree_processing.
 *
 * PARAMETERS:
 *      dtiptr          - input - pointer to an fsck record describing the
 *                                directory tree
 *      inorecptr       - input - pointer to an fsck inode record describing
 *                                the inode
 *      msg_info_ptr    - input - pointer to data needed to issue messages 
 *                                about the inode
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_node_first_in_level( fsck_Dtree_info_ptr dtiptr,
					 fsck_inode_recptr   inorecptr,
					 fsck_msg_info_ptr   msg_info_ptr,
					 int                 desired_action
					)
{
  retcode_t dnfil_rc = FSCK_OK;
	  
  if( dtiptr->dtp_ptr->header.prev != 0 ) {  /* bad back pointer! */
    inorecptr->ignore_alloc_blks = 1;
    if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_inotyp;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = msg_info_ptr->msg_inopfx;
      sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_BADBSBLCHN, 0, 3 );
      }  /* end not reported */
    }  /* end bad back pointer! */
	  
  return( dnfil_rc );
}                           /* end of dTree_node_first_in_level ()  */
		
		
/*****************************************************************************
 * NAME: dTree_node_last_in_level
 *
 * FUNCTION:  Assists dTree_processing.
 *
 * PARAMETERS:
 *      dtiptr          - input - pointer to an fsck record describing the
 *                                directory tree
 *      inorecptr       - input - pointer to an fsck inode record describing
 *                                the inode
 *      msg_info_ptr    - input - pointer to data needed to issue messages 
 *                                about the inode
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_node_last_in_level( fsck_Dtree_info_ptr dtiptr,
					fsck_inode_recptr   inorecptr,
					fsck_msg_info_ptr   msg_info_ptr,
					int                 desired_action
				       )
{
  retcode_t dnlil_rc = FSCK_OK;
	 
  if( dtiptr->dtp_ptr->header.next != 0 ) {  /* bad forward pointer! */
    inorecptr->ignore_alloc_blks = 1;
    if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_inotyp;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = msg_info_ptr->msg_inopfx;
      sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_BADFSBLCHN, 0, 3 );
      }  /* end not reported */
    }  /* end bad forward pointer! */
	
  return( dnlil_rc );
}                              /* end of dTree_node_last_in_level ()  */
	
	
/*****************************************************************************
 * NAME: dTree_node_not_first_in_level
 *
 * FUNCTION:  Assists dTree_processing.
 *
 * PARAMETERS:
 *      dtiptr          - input - pointer to an fsck record describing the
 *                                directory tree
 *      inorecptr       - input - pointer to an fsck inode record describing
 *                                the inode
 *      msg_info_ptr    - input - pointer to data needed to issue messages 
 *                                about the inode
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_node_not_first_in_level( fsck_Dtree_info_ptr dtiptr,
					 fsck_inode_recptr   inorecptr,
					 fsck_msg_info_ptr   msg_info_ptr,
					 int                 desired_action
					)
{
  retcode_t dnnfil_rc = FSCK_OK;
	
  if( dtiptr->dtp_ptr->header.prev != dtiptr->last_node_addr ) { /*
			     * bad back pointer!
			     */
    inorecptr->ignore_alloc_blks = 1;
    if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_inotyp;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = msg_info_ptr->msg_inopfx;
      sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_BADBSBLCHN, 0, 3 );
      }  /* end not reported */
    }  /* end bad back pointer! */
	
  return( dnnfil_rc );
}                           /* end of dTree_node_not_first_in_level ()  */
		
		
/*****************************************************************************
 * NAME: dTree_node_not_last_in_level
 *
 * FUNCTION:  Assists dTree_processing.
 *
 * PARAMETERS:
 *      dtiptr          - input - pointer to an fsck record describing the
 *                                directory tree
 *      inorecptr       - input - pointer to an fsck inode record describing
 *                                the inode
 *      msg_info_ptr    - input - pointer to data needed to issue messages 
 *                                about the inode
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_node_not_last_in_level( fsck_Dtree_info_ptr dtiptr,
					fsck_inode_recptr   inorecptr,
					fsck_msg_info_ptr   msg_info_ptr,
					int                 desired_action
				       )
{
  retcode_t dnnlil_rc = FSCK_OK;
  int8 is_leaf = 0;
  int8 is_root = 0;               /* it can't be the root */
	
  if( dtiptr->dtp_ptr->header.next != dtiptr->next_Qel->node_addr ) { /*
			     * bad forward pointer!
			     */
    inorecptr->ignore_alloc_blks = 1;
    if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported */
      msgprms[0] = message_parm_0;
      msgprmidx[0] = msg_info_ptr->msg_inotyp;
      msgprms[1] = message_parm_1;
      msgprmidx[1] = msg_info_ptr->msg_inopfx;
      sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_BADFSBLCHN, 0, 3 );
      }  /* end not reported */
    }  /* end bad forward pointer! */
  else {  /* forward pointer looks fine */
    if( dtiptr->leaf_level == dtiptr->this_Qel->node_level ) {  /*
			     * it's a leaf
			     */
      is_leaf = -1;
      }  /* end it's a leaf */
    dnnlil_rc = dTree_key_extract_cautiously( dtiptr, dtiptr->last_dtidx,
				  &(dtiptr->key[dtiptr->last_key_idx][0]),
				  &(dtiptr->key_len[dtiptr->last_key_idx]),
				  is_root, is_leaf, inorecptr );  /*
			     * get the last key in the directory.
			     * Since this is the first time we've
			     * extracted it, we need to guard against
			     * a loop even though we aren't recording
			     * it now.
			     */
    if( (dnnlil_rc == FSCK_OK) && (inorecptr->ignore_alloc_blks) ) {  /*
			     * no mishaps but the tree is bad
			     */
      if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported yet */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = msg_info_ptr->msg_inotyp;
        msgprms[1] = message_parm_1;
        msgprmidx[1] = msg_info_ptr->msg_inopfx;
        sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%d", 13 );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        fsck_send_msg( fsck_BADKEYS, 0, 4 );
	}  /* end not reported yet */
      }  /* end no mishaps but the tree is bad */
    }  /* end else forward pointer looks fine */
	
  return( dnnlil_rc );
}                              /* end of dTree_node_not_last_in_level ()  */
		
		
/**************************************************************************
 * NAME:  dTree_node_size_check
 *
 * FUNCTION: Validate the size of the given dTree node.
 *
 * PARAMETERS:
 *      dtiptr          - input - pointer to an fsck record describing the
 *                                directory tree
 *      is_leaf         - input - !0 => the node is a leaf node
 *                                 0 => the node is an internal node
 *      first_in_level  - input - !0 => the node is the leftmost in its level
 *                                 0 => the node is not leftmost in its level
 *      last_in_level   - input - !0 => the node is the rightmost in its level
 *                                 0 => the node is not rightmost in its level
 *      inorecptr       - input - pointer to an fsck inode record describing
 *                                the inode
 *      msg_info_ptr    - input - pointer to data needed to issue messages 
 *                                about the inode
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_node_size_check( fsck_Dtree_info_ptr dtiptr,
				 int8                is_leaf,
				 int8                first_in_level,
				 int8                last_in_level,
				 fsck_inode_recptr   inorecptr,
				 fsck_msg_info_ptr   msg_info_ptr,
				 int                 desired_action
				)
{
  retcode_t dnsc_rc = FSCK_OK;
  reg_idx_t ext_length;
  reg_idx_t acceptable_size;
  uint8 total_slots;
	
  ext_length = dtiptr->this_Qel->node_size * sb_ptr->s_bsize;
  total_slots = dtiptr->dtp_ptr->header.maxslot;
  if( ext_length == BYTESPERPAGE )  {  /* it is exactly 1 page long */
    if( total_slots != (BYTESPERPAGE / DTSLOTSIZE) ) {  /*
			    * but max slots doesn't work out right
			    */
      inorecptr->ignore_alloc_blks = 1;
      if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = msg_info_ptr->msg_inopfx;
        sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        fsck_send_msg( fsck_BADDINONODESIZ, 0, 2 );
	}  /* end not reported */
      }  /* end but max slots doesn't work out right */
    }  /* end it is exactly 1 page long */
  else {  /* it isn't exactly 1 page long */
	    /*
	     * the only valid directory node which can have a length
	     * different from 1 page is the node which is the only
	     * non-root node in the tree
	     */
    if( (!is_leaf) || (!(dtiptr->this_Qel->node_level == 1)) ||
	(!first_in_level) || (!last_in_level)                  ) { /*
			   * this node does not qualify
			   */
      inorecptr->ignore_alloc_blks = 1;
      if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = msg_info_ptr->msg_inopfx;
        sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        fsck_send_msg( fsck_BADDINOODDNODESIZ, 0, 2 );
	}  /* end not reported */
      }  /* end this node does not qualify */
    else {  /* it might be ok */
      acceptable_size = sb_ptr->s_bsize;    /* start at fs blocksize */
      while( (acceptable_size < BYTESPERPAGE) &&
	     (ext_length != acceptable_size) ) {
	acceptable_size = acceptable_size << 1;  /* double it */
	}  /* end while */
      if( ext_length != acceptable_size )  {  /* invalid size */
	inorecptr->ignore_alloc_blks = 1;
	if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = msg_info_ptr->msg_inopfx;
          sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
          msgprms[1] = message_parm_1;
          msgprmidx[1] = 0;
          fsck_send_msg( fsck_BADDINOODDNODESIZ, 0, 2 );
	  }  /* end not reported */
	}  /* end invalid size */
      else {  /* the size is ok */
	if( total_slots != (ext_length / DTSLOTSIZE) ) {  /*
			    * but max slots doesn't work out right
			    */
	  inorecptr->ignore_alloc_blks = 1;
	  if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported */
            msgprms[0] = message_parm_0;
            msgprmidx[0] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            fsck_send_msg( fsck_BADDINONODESIZ, 0, 2 );
	    }  /* end not reported */
	  }  /* end but max slots doesn't work out right */
	}  /* end else the size is ok */
      }  /* end else it might be ok */
    }  /* end else it isn't exactly 1 page long */
	
  return( dnsc_rc );
}                              /* end of dTree_node_size_check ()  */
	
	
/*****************************************************************************
 * NAME: dTree_process_internal_slots
 *
 * FUNCTION: Perform the specified action on the slots in the specified
 *           dTree internal node.
 *
 * PARAMETERS:
 *      dtiptr          - input - pointer to an fsck record describing the
 *                                directory tree
 *      inorecptr       - input - pointer to an fsck inode record describing
 *                                the inode
 *      msg_info_ptr    - input - pointer to data needed to issue messages 
 *                                about the inode
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_process_internal_slots( fsck_Dtree_info_ptr dtiptr,
					fsck_inode_recptr   inorecptr,
					fsck_msg_info_ptr   msg_info_ptr,
					int                 desired_action
				       )
{
  retcode_t dpis_rc = FSCK_OK;
  int8 this_key, last_key;
  int dtidx;
  dtreeQ_elem_ptr new_Qelptr;
  reg_idx_t ext_length, adjusted_length;
  fsblkidx_t ext_addr;
  int8 ext_ok, key_ok;
  int8 first_entry = -1;
  int8 is_EA = 0;
  int8 is_ACL = 0;
  int8 is_leaf = 0;
  int8 is_root = 0;
  idtentry_t *idtptr;
	
  if( dtiptr->this_Qel->node_level == 0 )  {
    is_root = -1;
    }
  this_key = 0;
  last_key = 1;
  key_len[this_key] = 0;
  key_len[last_key] = 0;
  for( dtidx = 0;
       ( (dpis_rc == FSCK_OK) &&
	 (!inorecptr->ignore_alloc_blks) &&
	 (dtidx <= dtiptr->last_dtidx)      );
       dtidx++ ) {
	
    dpis_rc = dTree_key_extract_record( dtiptr, dtidx,
					&(key[this_key][0]),
					&(key_len[this_key]),
					is_root, is_leaf, inorecptr );
    if( dpis_rc == FSCK_OK ) {  /* no mishaps */
      if( inorecptr->ignore_alloc_blks ) {  /* but the tree is bad */
	if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported yet */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = msg_info_ptr->msg_inotyp;
          msgprms[1] = message_parm_1;
          msgprmidx[1] = msg_info_ptr->msg_inopfx;
          sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
          msgprms[2] = message_parm_2;
          msgprmidx[2] = 0;
          sprintf( message_parm_3, "%d", 14 );
          msgprms[3] = message_parm_3;
          msgprmidx[3] = 0;
          fsck_send_msg( fsck_BADKEYS, 0, 4 );
	  }  /* end not reported yet */
	}  /* end but the tree is bad */
      else {  /* got the key value */
	 /*
	  * the key for the first entry is verified elsewhere
	  */
	if( first_entry ) {
	  first_entry = 0;
	  }
	else {  /* it's not the first entry on the page */
	  dpis_rc = dTree_key_compare_samelvl( &(key[last_key][0]),
					       key_len[last_key],
					       &(key[this_key][0]),
					       key_len[this_key],
					       &key_ok );
	  if( dpis_rc == FSCK_OK ) {  /* nothing fatal comparing keys */
	    if( !key_ok ) {  /* but the key is bad */
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
                sprintf( message_parm_3, "%d", 3 );
                msgprms[3] = message_parm_3;
                msgprmidx[3] = 0;
                fsck_send_msg( fsck_BADKEYS, 0, 4 );
		}  /* end not reported yet */
	      }  /* end but the key is bad */
	    }  /* end nothing fatal comparing keys */
	  }  /* end else it's not the first entry on the page */
	}  /* end else got the key value */
      }  /* end no mishaps */
    if( (dpis_rc == FSCK_OK) && (!inorecptr->ignore_alloc_blks)) {  /*
			   * the key is good
			   */
      idtptr = (idtentry_t *) &(dtiptr->slots[dtiptr->dtstbl[dtidx]]);
      ext_addr = addressPXD( &(idtptr->xd) );
      ext_length = lengthPXD( &(idtptr->xd) );
      dpis_rc = process_extent( inorecptr, ext_length, ext_addr, is_EA, 
				is_ACL, msg_info_ptr, &adjusted_length,
				&ext_ok, desired_action );
      if( (dpis_rc == FSCK_OK) &&
	  (ext_ok) ) {  /* extent looks good in general */
	if( (desired_action == FSCK_RECORD)         ||
	    (desired_action == FSCK_RECORD_DUPCHECK)  ) {
	  agg_recptr->blocks_this_fset += adjusted_length;
	  agg_recptr->this_inode.all_blks += adjusted_length;
	  agg_recptr->this_inode.data_size += adjusted_length * sb_ptr->s_bsize;
	  }
	else if( desired_action == FSCK_UNRECORD ) {
	  agg_recptr->blocks_this_fset -= adjusted_length;
	  agg_recptr->this_inode.all_blks -= adjusted_length;
	  }
	dpis_rc = dtreeQ_get_elem( &new_Qelptr );
	if( dpis_rc == FSCK_OK ) {  /* got a Queue element */
	  new_Qelptr->node_pxd = idtptr->xd;
	  new_Qelptr->node_addr = ext_addr;
	  new_Qelptr->node_level = dtiptr->this_Qel->node_level + 1;
	  new_Qelptr->node_size = ext_length;
	  new_Qelptr->node_key_len = key_len[this_key];
	  memcpy( (void *) &(new_Qelptr->node_key[0]),
		  (void *) &(key[this_key][0]),
		  key_len[this_key] * sizeof(UniChar) );
	  dpis_rc = dtreeQ_enqueue( new_Qelptr );
	  if( this_key )  {  /* this_key == 1 */
	    this_key = 0;
	    last_key = 1;
	    }  /* end this key == 1 */
	  else {  /* this_key == 0 */
	    this_key = 1;
	    last_key = 0;
	    }  /* end else this_key == 0 */
	  key_len[this_key] = 0;
	  }  /* end got a Queue element */
	}  /* end extent looks good in general */
      }  /* end the key is good */
    }  /* end for */
	
  return( dpis_rc );
}                              /* end of dTree_process_internal_slots ()  */
		
		
/*****************************************************************************
 * NAME: dTree_process_leaf_slots
 *
 * FUNCTION: Perform the specified action on the slots in the specified
 *           dTree leaf node.
 *
 * PARAMETERS:
 *      dtiptr          - input - pointer to an fsck record describing the
 *                                directory tree
 *      inorecptr       - input - pointer to an fsck inode record describing
 *                                the inode
 *      inonum          - input - the inode number of the directory inode
 *      msg_info_ptr    - input - pointer to data needed to issue messages 
 *                                about the inode
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_process_leaf_slots( fsck_Dtree_info_ptr dtiptr,
				    fsck_inode_recptr   inorecptr,
				    ino_t               inonum,
				    fsck_msg_info_ptr   msg_info_ptr,
				    int                 desired_action
				   )
{
  retcode_t dpls_rc = FSCK_OK;
  int8 this_key, last_key;
  int dtidx;
  int8 key_ok;
  int8 is_leaf = -1;
  int8 is_root;
  int8 first_entry = -1;
  ldtentry_t *ldtptr;
  inoidx_t          child_inoidx;
  fsck_inode_recptr child_inorecptr;
  fsck_inode_extptr this_ext;
  int is_aggregate = 0;
  int alloc_ifnull = -1;
	
  if( dtiptr->this_Qel->node_level == 0 )  {
    is_root = -1;
    }
  this_key = 0;
  last_key = 1;
  key_len[this_key] = 0;
  key_len[last_key] = 0;
  for( dtidx = 0;
       ( (dpls_rc == FSCK_OK) &&
	 (!inorecptr->ignore_alloc_blks) &&
	 (dtidx <= dtiptr->last_dtidx)      );
       dtidx++ ) {
	
    dpls_rc = dTree_key_extract_record( dtiptr, dtidx,
					&(key[this_key][0]),
					&(key_len[this_key]),
					is_root, is_leaf, inorecptr );
    if( dpls_rc == FSCK_OK ) {  /* no mishaps */
      if( inorecptr->ignore_alloc_blks ) {  /* but the tree is bad */
#ifdef _JFS_DEBUG_PS
 printf("dTree_process_leaf_slots dTree key extraxt index=%u\n\r",dtidx);
#endif
	if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported yet */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = msg_info_ptr->msg_inotyp;
          msgprms[1] = message_parm_1;
          msgprmidx[1] = msg_info_ptr->msg_inopfx;
          sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
          msgprms[2] = message_parm_2;
          msgprmidx[2] = 0;
          sprintf( message_parm_3, "%d", 15 );
          msgprms[3] = message_parm_3;
          msgprmidx[3] = 0;
          fsck_send_msg( fsck_BADKEYS, 0, 4 );
	  }  /* end not reported yet */
	}  /* end but the tree is bad */
      else {  /* got the key value */
	 /*
	  * the key for the first entry is verified elsewhere
	  */
	if( first_entry ) {
	  first_entry = 0;
	  }
	else {  /* it's not the first entry on the page */
	  dpls_rc = dTree_key_compare_leaflvl( &(key[last_key][0]),
					       key_len[last_key],
					       &(key[this_key][0]),
					       key_len[this_key],
					       &key_ok );
	  if( dpls_rc == FSCK_OK ) {  /* nothing fatal comparing keys */
	    if( !key_ok ) {  /* but the key is bad */
#ifdef _JFS_DEBUG
 printf("dTree key extraxt index=%u key_OK\n\r",dtidx);
#endif
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
                sprintf( message_parm_3, "%d", 4 );
                msgprms[3] = message_parm_3;
                msgprmidx[3] = 0;
                fsck_send_msg( fsck_BADKEYS, 0, 4 );
		}  /* end not reported yet */
	      }  /* end but the key is bad */
	    }  /* end nothing fatal comparing keys */
	  }  /* end else it's not the first entry on the page */
	}  /* end else got the key value */
      }  /* end no mishaps */
    if( (dpls_rc == FSCK_OK) && (!inorecptr->ignore_alloc_blks)) {  /*
			   * the key is good
			   */
      if( this_key )  {  /* this_key == 1 */
	this_key = 0;
	last_key = 1;
	}  /* end this key == 1 */
      else {  /* this_key == 0 */
	this_key = 1;
	last_key = 0;
	}  /* end else this_key == 0 */
      key_len[this_key] = 0;
	  /*
	   * If the desired action is anything besides RECORD_DUPCHECK
	   * then we're really only interested in things that indicate
	   * a bad B+ tree.  Since bad inode references in the directory
	   * entries don't imply anything about the tree itself, we only
	   * want to check them if we're doing RECORD_DUPCHECK.
	   */
      if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
			   * the inode references haven't been checked yet
			   */
	ldtptr = (ldtentry_t *) &(dtiptr->slots[dtiptr->dtstbl[dtidx]]);
	child_inoidx = (inoidx_t) ldtptr->inumber;
	if( child_inoidx > agg_recptr->fset_inode_count ) {  /*
			   * it can't be right because it's out of range
			   */
	  dpls_rc = get_inode_extension( &this_ext );
	  if( dpls_rc == FSCK_OK )  {  /* got extension */
	    this_ext->ext_type = rmv_badentry_extension;
	    this_ext->inonum = ldtptr->inumber;
	    this_ext->next = inorecptr->ext_rec;
	    inorecptr->ext_rec = this_ext;
	    inorecptr->adj_entries = 1;
	    key[this_key][key_len[last_key]] = '\0'; /* add terminator */
            msgprms[0] = message_parm_0;
            msgprmidx[0] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            if (UniStrFromUcs( uconv_object, message_parm_2, key[last_key],
			       MAXPARMLEN ) == ULS_SUCCESS)
	      msgprms[2] = message_parm_2;
	    else
	      msgprms[2] = "(Conversion Failed)";
            msgprmidx[2] = 0;
            msgprms[3] = " ";
            msgprmidx[3] = 0;
            sprintf( message_parm_4, "%ld", child_inoidx );
            msgprms[4] = message_parm_4;
            msgprmidx[4] = 0;
            fsck_send_msg( fsck_BADINOREF, 0, 5 );
	    }  /* end got the extension */
	  }  /* end it can't be right because it's out of range */
	else if( child_inoidx < FILESET_OBJECT_I ) {  /*
			   * it can't be right because it's in the range
			   * reserved for special metadata and the root dir
			   */
	  dpls_rc = get_inode_extension( &this_ext );
	  if( dpls_rc == FSCK_OK )  {  /* got extension */
	    this_ext->ext_type = rmv_badentry_extension;
	    this_ext->inonum = ldtptr->inumber;
	    this_ext->next = inorecptr->ext_rec;
	    inorecptr->ext_rec = this_ext;
	    inorecptr->adj_entries = 1;
	    key[this_key][key_len[last_key]] = '\0'; /* add terminator */
            msgprms[0] = message_parm_0;
            msgprmidx[0] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            if (UniStrFromUcs( uconv_object, message_parm_2, key[last_key],
			       MAXPARMLEN ) == ULS_SUCCESS)
	      msgprms[2] = message_parm_2;
	    else
	      msgprms[2] = "(Conversion Failed)";
            msgprmidx[2] = 0;
            msgprms[3] = " ";
            msgprmidx[3] = 0;
            sprintf( message_parm_4, "%ld", child_inoidx );
            msgprms[4] = message_parm_4;
            msgprmidx[4] = 0;
            fsck_send_msg( fsck_ILLINOREF, 0, 5 );
	    }  /* end got the extension */
	  }  /* end else it can't be right because it's in the range ... */
	else {  /* it might be ok */
	  dpls_rc = get_inorecptr( is_aggregate, alloc_ifnull,
				   child_inoidx, &child_inorecptr );
	  if( dpls_rc == FSCK_OK ) {  /* got a record for the child */
	    child_inorecptr->link_count += 1;
	    if( child_inorecptr->parent_inonum == 0 )  {  /*
			   * no parent recorded yet
			   */
	      child_inorecptr->parent_inonum = inonum;
	      }  /* end no parent recorded yet */
	    else {  /* this is not the first parent seen */
	      dpls_rc = get_inode_extension( &this_ext );
	      if( dpls_rc == FSCK_OK )  {  /* got extension */
		this_ext->ext_type = parent_extension;
		this_ext->inonum = inonum;
		this_ext->next = child_inorecptr->ext_rec;
		child_inorecptr->ext_rec = this_ext;
		}  /* end got the extension */
	      }  /* end else this is not the first parent seen */
	    }  /* end got a record for the child */
	  }  /* end it might be ok */
	}  /* end the inode references haven't been checked yet */
      }  /* end the key is good */
    }  /* end for */
	
  return( dpls_rc );
}                              /* end of dTree_process_leaf_slots ()  */
		
		
/*****************************************************************************
 * NAME: dTree_processing
 *
 * FUNCTION: Perform the specified action on the dTree rooted in the 
 *           specified inode.
 *
 * PARAMETERS:
 *      inoptr          - input - pointer to the directory inode in an fsck 
 *                                buffer
 *      inoidx          - input - ordinal number of the inode
 *      inorecptr       - input - pointer to an fsck inode record describing
 *                                the inode
 *      msg_info_ptr    - input - pointer to data needed to issue messages 
 *                                about the inode
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_processing ( dinode_t          *inoptr,
			     inoidx_t           inoidx,
			     fsck_inode_recptr  inorecptr,
			     fsck_msg_info_ptr  msg_info_ptr,
			     int                desired_action
			    )
{
 retcode_t dp_rc = FSCK_OK;
 int slotidx;
 int8 old_ignore_alloc_blks;
 int ixpxd_unequal = 0;
 idtentry_t *idtentry_ptr;
 ldtentry_t *ldtentry_ptr;
 fsck_Dtree_info_t   dtinfo;
 fsck_Dtree_info_ptr dtiptr;
 int8 first_in_level = 0;
 int8 last_in_level = 0;
 int8 is_leaf = 0;
 int8 is_root = -1;
 int8 dtstbl_last_slot;
 reg_idx_t nodesize_in_bytes;
 int8 msg_reason = 0;

  dtiptr = &dtinfo;
  dtiptr->this_Qel = NULL;
  dtiptr->next_Qel = NULL;
  dtiptr->last_level = -1;          /* -1 so the root will be recognized
				     * as 1st node in level 0
				     */
  dtiptr->leaf_level = -1;          /* so we won't get a match until the
				     * actual leaf level is found
				     */
  dtiptr->this_key_idx = 0;
  dtiptr->last_key_idx = 1;
  dtiptr->key_len[dtiptr->this_key_idx] = 0;
  dtiptr->key_len[dtiptr->last_key_idx] = 0;
  dtiptr->leaf_seen = 0;
  memset( (void *) &(dtiptr->slot_map[0]), 0, DTPAGEMAXSLOT );  /*
			      * set the flags for all slots in the
			      * current page to 'not used'
			      */
  dtiptr->dtr_ptr = (dtroot_t *) &(inoptr->di_btroot);
  dtiptr->max_slotidx = DTROOTMAXSLOT - 1;
  if( dtiptr->dtr_ptr->header.nextindex > 0 ) {  /*
			     * there is at least 1 entry
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
    dtiptr->key_len[dtiptr->last_key_idx] = 0;
    dtiptr->dtstbl = (int8 *) &(dtiptr->dtr_ptr->header.stbl[0]);
    dtiptr->slots = &(dtiptr->dtr_ptr->slot[0]);
    dtiptr->freelist_first_dtidx = dtiptr->dtr_ptr->header.freelist;
    dtiptr->freelist_count = dtiptr->dtr_ptr->header.freecnt;
    dtiptr->last_dtidx = dtiptr->dtr_ptr->header.nextindex - 1;
        /*
         * Do a sanity check to make sure this looks like a
         * DTree root node
         */
 if( dtiptr->last_dtidx > dtiptr->max_slotidx ) {
#ifdef _JFS_DEBUG_PS
 printf("dTree proc ignore_alloc_blks set dtiptr->last_dtidx %u > %u\n\r",dtiptr->last_dtidx,dtiptr->max_slotidx);
#endif
   inorecptr->ignore_alloc_blks = 1;
   msg_reason = 16;
   }
 else if( dtiptr->freelist_count < 0 ) {
#ifdef _JFS_DEBUG_PS
 printf("dTree proc ignore_alloc_blks set dtiptr->freelist_count %d < 0\n\r",dtiptr->freelist_count);
#endif
   inorecptr->ignore_alloc_blks = 1;
   msg_reason = 17;
   }
 else if( dtiptr->freelist_count > dtiptr->max_slotidx ) {
   inorecptr->ignore_alloc_blks = 1;
   msg_reason = 18;
     }
 else if( dtiptr->freelist_first_dtidx < -1 ) {
   inorecptr->ignore_alloc_blks = 1;
   msg_reason = 19;
   }
 else if( dtiptr->freelist_first_dtidx > dtiptr->max_slotidx ) {
   inorecptr->ignore_alloc_blks = 1;
   msg_reason = 20;
   }
 else if( (dtiptr->last_dtidx == dtiptr->max_slotidx) &&
          (dtiptr->freelist_count != 0)                  ) {
   inorecptr->ignore_alloc_blks = 1;
   msg_reason = 21;
   }
 else if( (dtiptr->last_dtidx == dtiptr->max_slotidx) &&
          (dtiptr->freelist_first_dtidx != -1)            ) {
   inorecptr->ignore_alloc_blks = 1;
   msg_reason = 22;
   }
 else if( (dtiptr->freelist_first_dtidx != -1) &&
          (dtiptr->freelist_count == 0)                  ) {
   inorecptr->ignore_alloc_blks = 1;
   msg_reason = 23;
   }
 else if( (dtiptr->freelist_first_dtidx == -1) &&
          (dtiptr->freelist_count != 0)                  ) {
   inorecptr->ignore_alloc_blks = 1;
   msg_reason = 24;
   }
#ifdef _JFS_DEBUG_PS
 printf("dTree proc ignore_alloc_blks set , msg=%u\n\r",msg_reason);
 printf("inorecptr->ignore_alloc_blks %u\n\r",inorecptr->ignore_alloc_blks);
#endif
 if( inorecptr->ignore_alloc_blks && (msg_reason != 0) ) {
   if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported yet */
     msgprms[0] = message_parm_0;
     msgprmidx[0] = msg_info_ptr->msg_inotyp;
     msgprms[1] = message_parm_1;
     msgprmidx[1] = msg_info_ptr->msg_inopfx;
     sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
     msgprms[2] = message_parm_2;
     msgprmidx[2] = 0;
     sprintf( message_parm_3, "%d", msg_reason );
     msgprms[3] = message_parm_3;
     msgprmidx[3] = 0;
     fsck_send_msg( fsck_BADKEYS, 0, 4 );
     }  /* end not reported yet */
   }
 else {  /* the root looks like it's probably a root */
	/*
	 * mark all slots occupied by the header as being in use
	 */
#ifdef _JFS_DEBUG_PS
 printf("dTree processing root look\n\r");
 printf("inorecptr->ignore_alloc_blks %u\n\r",inorecptr->ignore_alloc_blks);
#endif
    for( slotidx = 0; (slotidx < DTENTRYSTART); slotidx++) {
      dtiptr->slot_map[slotidx] = -1;
      }
	/*
	 * get a queue element and set it up for the root
	 */
    dp_rc = dtreeQ_get_elem( &dtiptr->this_Qel );
#ifdef _JFS_DEBUG_PS
 printf("dTree get elem %u\n\r",dp_rc);
 printf("inorecptr->ignore_alloc_blks %u\n\r",inorecptr->ignore_alloc_blks);
#endif
    if( dp_rc == FSCK_OK )  {  /* got a queue element */
      dtiptr->this_Qel->node_level = 0;
      if( dtiptr->dtr_ptr->header.flag & BT_LEAF ) {  /* root leaf */
	dtiptr->leaf_seen = -1;
	dtiptr->leaf_level = dtiptr->this_Qel->node_level;
	is_leaf = -1;
	}
      else if( !(dtiptr->dtr_ptr->header.flag & BT_INTERNAL) ) {  /*
				* but it's not an internal node either!
				*/
#ifdef _JFS_DEBUG_PS
 printf("DROVA inorecptr->ignore_alloc_blks %u\n\r",inorecptr->ignore_alloc_blks);
#endif
        if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported yet */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = msg_info_ptr->msg_inotyp;
          msgprms[1] = message_parm_1;
          msgprmidx[1] = msg_info_ptr->msg_inopfx;
          sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
          msgprms[2] = message_parm_2;
          msgprmidx[2] = 0;
          sprintf( message_parm_3, "%d", 11 );
          msgprms[3] = message_parm_3;
          msgprmidx[3] = 0;
          fsck_send_msg( fsck_BADKEYS, 0, 4 );
          }  /* end not reported yet */
        }  /* end but it's not an internal node either! */

	/*
	 * get the first key, but don't mark the slot_map
	 */
      if( ! inorecptr->ignore_alloc_blks ) {
        dp_rc = dTree_key_extract_cautiously( dtiptr, 0,
				 &(dtiptr->key[dtiptr->this_key_idx][0]),
				 &(dtiptr->key_len[dtiptr->this_key_idx]),
				 is_root, is_leaf, inorecptr );  /*
			     * get the first key in the directory.
			     * Since this is the first time we've
			     * extracted it, we need to guard against
			     * a loop even though we aren't recording
			     * it now.
			     */
        }
#ifdef _JFS_DEBUG_PS
 printf("dTree key extract %u leaf=%d\n\r",dp_rc,is_leaf);
 printf("inorecptr->ignore_alloc_blks %u\n\r",inorecptr->ignore_alloc_blks);
#endif
      if( dp_rc == FSCK_OK ) {  /* No mishaps */
	if( inorecptr->ignore_alloc_blks ) {  /* but directory is bad */
	  if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported yet */
               msgprms[0] = message_parm_0;
               msgprmidx[0] = msg_info_ptr->msg_inotyp;
               msgprms[1] = message_parm_1;
               msgprmidx[1] = msg_info_ptr->msg_inopfx;
               sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
               msgprms[2] = message_parm_2;
               msgprmidx[2] = 0;
               sprintf( message_parm_3, "%d", 40 );
               msgprms[3] = message_parm_3;
               msgprmidx[3] = 0;
               fsck_send_msg( fsck_BADKEYS, 0, 4 );
	    }  /* end not reported yet */
	  }  /* end but directory is bad */
	else {  /* we got the first key */
	  if( is_leaf )  {
	    dp_rc = dTree_process_leaf_slots( dtiptr,
					      inorecptr, inoptr->di_number,
					      msg_info_ptr, desired_action
					     );
	    }
	  else {
	    dp_rc = dTree_process_internal_slots( dtiptr, inorecptr,
						  msg_info_ptr, desired_action
						 );
	    }
#ifdef _JFS_DEBUG_PS
 printf("dTree slots %u of %d\n\r",dp_rc,is_leaf);
 printf("inorecptr->ignore_alloc_blks %u \n\r",inorecptr->ignore_alloc_blks);
#endif
	  if( (dp_rc == FSCK_OK) &&
	      (!inorecptr->ignore_alloc_blks) ) {  /* node is looking good */
	     dtiptr->key_len[dtiptr->last_key_idx] = 0;
	    dp_rc = dTree_verify_slot_freelist( dtiptr, inorecptr,
						msg_info_ptr, desired_action );
	    }  /* end node is looking good */
	  }  /* end else we got the first key */
	}  /* end no mishaps */
      }  /* end got a queue element */
   }  /* end the root looks like it's probably a root */
	
    while( (dp_rc == FSCK_OK)       &&     /* nothing fatal and */
	   (!inorecptr->ignore_alloc_blks) &&     /* tree looks ok and */
	   (agg_recptr->dtreeQ_back != NULL)   ) {  /* queue not empty */
      dp_rc = dtreeQ_dequeue( &dtiptr->next_Qel );
#ifdef _JFS_DEBUG_PS
 printf("dTree dequeue %u\n\r",dp_rc);
 printf("inorecptr->ignore_alloc_blks %u\n\r",inorecptr->ignore_alloc_blks);
#endif
      if( dp_rc == FSCK_OK )  {  /*
			     * got another element from the queue
			     */
	if( dtiptr->this_Qel->node_level !=
				       dtiptr->next_Qel->node_level )  { /*
			     * it's the last in this level
			     */
	  last_in_level = -1;
	  if( !is_root ) {
	    dp_rc = dTree_node_last_in_level( dtiptr, inorecptr,
						msg_info_ptr,
						desired_action );
	    }
	  }  /* end it's the last in this level */
	else {  /* it's not the last in its level */
	  last_in_level = 0;
	  dp_rc = dTree_node_not_last_in_level( dtiptr, inorecptr,
						msg_info_ptr,
						desired_action );
	  }  /* end else it's not the last in its level */
	}  /* end got another element from the queue */
      if( (dp_rc == FSCK_OK) &&
	  (!inorecptr->ignore_alloc_blks) ) {  /* still in business */
	dtiptr->last_level = dtiptr->this_Qel->node_level;
	dtiptr->last_node_addr = dtiptr->this_Qel->node_addr;
	dp_rc = dtreeQ_rel_elem( dtiptr->this_Qel );
#ifdef _JFS_DEBUG_PS
 printf("dTree rel elem %u\n\r",dp_rc);
 printf("inorecptr->ignore_alloc_blks %u\n\r",inorecptr->ignore_alloc_blks);
#endif
	if( dp_rc != FSCK_OK ) {
	  dtiptr->this_Qel = NULL;  /* don't try again after loop! */
	  }
	else {  /* released the older element */
	  dtiptr->this_Qel = dtiptr->next_Qel; /* promote newer element */
	  dtiptr->next_Qel = NULL;         /* to avoid releasing it twice */
	  is_root = 0;
	  dp_rc = dnode_get( dtiptr->this_Qel->node_addr, BYTESPERPAGE,
			     &dtiptr->dtp_ptr );
	  if( dp_rc != FSCK_OK ) {  /* read failed */
	    dp_rc = FSCK_OK;	/* this isn't a chkdsk failure -- it's a symptom
				 * of a bad dtree
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
	      sprintf( message_parm_3, "%d", 38 );
	      msgprms[3] = message_parm_3;
	      msgprmidx[3] = 0;
	      fsck_send_msg( fsck_BADKEYS, 0, 4 );
	      }  /* end not reported yet */
	    }  /* end read failed */
	  else {  /* got the new node */
	    ixpxd_unequal = memcmp( (void *) &(dtiptr->dtp_ptr->header.self),
                                                     (void *) &(dtiptr->this_Qel->node_pxd),
                                                     sizeof(pxd_t) );       /* returns 0 if equal */
	    if( ixpxd_unequal ) {  /* bad self pxd in header */
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
	      }  /* end bad self pxd in header */
	    else if( dtiptr->dtp_ptr->header.nextindex == 0 )  {  /*
			     * an empty non-root node
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
                     fsck_send_msg( fsck_BADINOMTNODE, 0, 3 );
		}  /* end not reported yet */
	      }  /* end an empty non-root node */
	    else  { /* node is not empty */
	      dtiptr->max_slotidx = dtiptr->dtp_ptr->header.maxslot - 1;
	      if( dtiptr->dtp_ptr->header.flag & BT_LEAF ) {  /* a leaf */
		dtiptr->leaf_seen = -1;
		dtiptr->leaf_level = dtiptr->this_Qel->node_level;
		is_leaf = -1;
		}  /* end a leaf */
	      else if( !(dtiptr->dtp_ptr->header.flag & BT_INTERNAL) ) {  /*
				* but it's not an internal node either!
				*/
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
		    sprintf( message_parm_3, "%d", 41 );
		    msgprms[3] = message_parm_3;
		    msgprmidx[3] = 0;
		    fsck_send_msg( fsck_BADKEYS, 0, 4 );
		    }  /* end not reported yet */
		}  /* end but it's not an internal node either! */
	      }  /* end else node is not empty */
	    }  /* end else got the new node */
	  }  /* end released the older element */
	}  /* end still in business */
#ifdef _JFS_DEBUG_PS
 printf("dTree end business %u\n\r",dp_rc);
 printf("inorecptr->ignore_alloc_blks %u\n\r",inorecptr->ignore_alloc_blks);
#endif
      if( (dp_rc == FSCK_OK) &&
	  (!inorecptr->ignore_alloc_blks) ) {  /* no problems yet */
	memset( (void *) &(dtiptr->slot_map[0]), 0, DTPAGEMAXSLOT );  /*
			      * set the flags for all slots in the
			      * current page to 'not used'
			      */
	dtiptr->dtstbl = (int8 *)
		       &(dtiptr->dtp_ptr->slot[dtiptr->dtp_ptr->header.stblindex]);
	dtiptr->slots = &(dtiptr->dtp_ptr->slot[0]);
	dtiptr->freelist_first_dtidx = dtiptr->dtp_ptr->header.freelist;
	dtiptr->freelist_count = dtiptr->dtp_ptr->header.freecnt;
	dtiptr->last_dtidx = dtiptr->dtp_ptr->header.nextindex - 1;
	    /*
	     * mark all slots occupied by the header as being in use
	     */
	for( slotidx = 0; (slotidx < DTENTRYSTART); slotidx++) {
	  dtiptr->slot_map[slotidx] = -1;
	  }
	    /*
	     * figure out which slots are occupied by the dtstbl and
	     * mark them as being in use
	     */
	nodesize_in_bytes = dtiptr->this_Qel->node_size *
			    sb_ptr->s_bsize;
	if( nodesize_in_bytes > DTHALFPGNODEBYTES ) {
	  dtstbl_last_slot = (DTHALFPGNODETSLOTS << 1) +
			     dtiptr->dtp_ptr->header.stblindex - 1;
	  }
	else if( nodesize_in_bytes == DTHALFPGNODEBYTES ) {
	  dtstbl_last_slot = DTHALFPGNODETSLOTS +
			     dtiptr->dtp_ptr->header.stblindex - 1;
	  }
	else {
	  dtstbl_last_slot = (DTHALFPGNODETSLOTS >> 1) +
			     dtiptr->dtp_ptr->header.stblindex - 1;
	  }
              /*
               * Do a sanity check to make sure this looks like a
               * DTree root node
               */
       if( (dtiptr->dtp_ptr->header.maxslot != DT8THPGNODESLOTS)  &&
           (dtiptr->dtp_ptr->header.maxslot != DTQTRPGNODESLOTS)  &&
           (dtiptr->dtp_ptr->header.maxslot != DTHALFPGNODESLOTS) &&
           (dtiptr->dtp_ptr->header.maxslot != DTFULLPGNODESLOTS)    ) {
         inorecptr->ignore_alloc_blks = 1;
         msg_reason = 25;
         }
       else if( dtiptr->last_dtidx > dtiptr->max_slotidx ) {
         inorecptr->ignore_alloc_blks = 1;
         msg_reason = 26;
         }
       else if( dtiptr->freelist_count < 0) {
         inorecptr->ignore_alloc_blks = 1;
         msg_reason = 27;
         }
       else if( dtiptr->freelist_count > dtiptr->max_slotidx ) {
         inorecptr->ignore_alloc_blks = 1;
         msg_reason = 28;
           }
       else if( dtiptr->freelist_first_dtidx < -1 ) {
         inorecptr->ignore_alloc_blks = 1;
         msg_reason = 29;
         }
       else if( dtiptr->freelist_first_dtidx > dtiptr->max_slotidx ) {
         inorecptr->ignore_alloc_blks = 1;
         msg_reason = 30;
         }
       else if( (dtiptr->last_dtidx == dtiptr->max_slotidx) &&
                (dtiptr->freelist_count != 0)                  ) {
         inorecptr->ignore_alloc_blks = 1;
         msg_reason = 31;
         }
       else if( (dtiptr->last_dtidx == dtiptr->max_slotidx) &&
                (dtiptr->freelist_first_dtidx != -1)            ) {
         inorecptr->ignore_alloc_blks = 1;
         msg_reason = 32;
         }
       else if( (dtiptr->freelist_first_dtidx != -1) &&
                (dtiptr->freelist_count == 0)                  ) {
         inorecptr->ignore_alloc_blks = 1;
         msg_reason = 33;
         }
       else if( (dtiptr->freelist_first_dtidx == -1) &&
                (dtiptr->freelist_count != 0)                  ) {
         inorecptr->ignore_alloc_blks = 1;
         msg_reason = 34;
         }
       else if( dtiptr->dtp_ptr->header.stblindex > dtiptr->max_slotidx ) {
         inorecptr->ignore_alloc_blks = 1;
         msg_reason = 35;
         }
       else if( dtiptr->dtp_ptr->header.stblindex < 1 ) {
         inorecptr->ignore_alloc_blks = 1;
         msg_reason = 36;
         }
#ifdef _JFS_DEBUG_PS
 printf("dTree proc after , msg=%u\n\r",msg_reason);
 printf("inorecptr->ignore_alloc_blks %u\n\r",inorecptr->ignore_alloc_blks);
#endif
       if( inorecptr->ignore_alloc_blks && (msg_reason != 0) ) {
         if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported yet */
           msgprms[0] = message_parm_0;
           msgprmidx[0] = msg_info_ptr->msg_inotyp;
           msgprms[1] = message_parm_1;
           msgprmidx[1] = msg_info_ptr->msg_inopfx;
           sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
           msgprms[2] = message_parm_2;
           msgprmidx[2] = 0;
           sprintf( message_parm_3, "%d", msg_reason );
           msgprms[3] = message_parm_3;
           msgprmidx[3] = 0;
           fsck_send_msg( fsck_BADKEYS, 0, 4 );
           }  /* end not reported yet */
         }
       else {  /* the node looks like it's probably a real node */
	for( slotidx = dtiptr->dtp_ptr->header.stblindex;
	     (slotidx <= dtstbl_last_slot);
	     slotidx++) {
	  dtiptr->slot_map[slotidx] = -1;
	  }
	  /*
	   * get the first key, but don't mark the slot_map
	   */
	dp_rc = dTree_key_extract_cautiously( dtiptr, 0,
				   &(dtiptr->key[dtiptr->this_key_idx][0]),
				   &(dtiptr->key_len[dtiptr->this_key_idx]),
				   is_root, is_leaf, inorecptr );  /*
			     * get the first key in the directory.
			     * Since this is the first time we've
			     * extracted it, we need to guard against
			     * a loop even though we aren't recording
			     * it now.
			     */
	if( dp_rc == FSCK_OK ) {  /* No mishaps */
	  if( inorecptr->ignore_alloc_blks ) {  /* but directory is bad */
	    if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported yet */
                 msgprms[0] = message_parm_0;
                 msgprmidx[0] = msg_info_ptr->msg_inotyp;
                 msgprms[1] = message_parm_1;
                 msgprmidx[1] = msg_info_ptr->msg_inopfx;
                 sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
                 msgprms[2] = message_parm_2;
                 msgprmidx[2] = 0;
                 sprintf( message_parm_3, "%d", 12 );
                 msgprms[3] = message_parm_3;
                 msgprmidx[3] = 0;
                 fsck_send_msg( fsck_BADKEYS, 0, 4 );
	      }  /* end not reported yet */
	    }  /* end but directory is bad */
	  else {  /* we got the first key */
	    dp_rc = dTree_node_first_key( dtiptr, inorecptr,
					msg_info_ptr, desired_action );
	    }  /* end else we got the first key */
	  }  /* end no mishaps */
	}  /* end no problems yet */
       }  /* end else the node looks like it's probably a real node */
	
      if( (dp_rc == FSCK_OK) &&
	  (!inorecptr->ignore_alloc_blks) ) {  /*
			     * tree still interesting
			     */
	if( dtiptr->dtp_ptr->header.flag & BT_LEAF ) {
	  is_leaf = -1;
	  }
	else if( !(dtiptr->dtp_ptr->header.flag & BT_INTERNAL) ) {  /*
				* but it's not an internal node either!
				*/
	  inorecptr->ignore_alloc_blks=1;                   //+++PS
	  if( desired_action == FSCK_RECORD_DUPCHECK ) {  /* not reported yet */
	    msgprms[0] = message_parm_0;
	    msgprmidx[0] = msg_info_ptr->msg_inotyp;
	    msgprms[1] = message_parm_1;
	    msgprmidx[1] = msg_info_ptr->msg_inopfx;
	    sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
	    msgprms[2] = message_parm_2;
	    msgprmidx[2] = 0;
	    sprintf( message_parm_3, "%d", 42 );
	    msgprms[3] = message_parm_3;
	    msgprmidx[3] = 0;
	    fsck_send_msg( fsck_BADKEYS, 0, 4 );
	    }  /* end not reported yet */
	  }  /* end but it's not an internal node either! */
	else {
	  is_leaf = 0;
	  }
	if( !inorecptr->ignore_alloc_blks ) {  /* tree still looks ok */
	  if( is_leaf )  {
	    dp_rc = dTree_process_leaf_slots( dtiptr, inorecptr,
					    inoptr->di_number,
					    msg_info_ptr, desired_action );
	    }  /* end a leaf node */
	  else {  /* an internal node */
	    if( dtiptr->last_level != dtiptr->this_Qel->node_level )  {  /*
				   * this node is the first in a new level
				   * of internal nodes.
				   */
	      agg_recptr->this_inode.data_size = 0;  /* This is the size of
				   * the leaf nodes, so we only want the
				   * total described by the LAST level of
				   * internal nodes.
				   */
	      }  /* This is the size of the leaf nodes, so we only want ... */
	    dp_rc = dTree_process_internal_slots( dtiptr, inorecptr,
						msg_info_ptr,
						desired_action );
	    }  /* end else an internal node */
	  }  /* end tree still ok */ 
	if( (dp_rc == FSCK_OK) &&
	    (!inorecptr->ignore_alloc_blks) ) {  /* tree still looks ok */
	  if( dtiptr->last_level != dtiptr->this_Qel->node_level )  {  /*
			     * this is a new level
			     */
	    first_in_level = -1;
	    dtiptr->key_len[dtiptr->last_key_idx] = 0;
	    if( !is_root ) {
	      dp_rc = dTree_node_first_in_level(dtiptr, inorecptr,
						msg_info_ptr,
						desired_action );
	      }
	    }  /* end this is a new level */
	  else {  /* not 1st in level */
	    first_in_level = 0;
	    dp_rc = dTree_node_not_first_in_level( dtiptr, inorecptr,
						   msg_info_ptr,
						   desired_action );
	    }  /* end else not 1st in level */
	  }  /* end tree still looks ok */
	if( (dp_rc == FSCK_OK)      &&
	    (!inorecptr->ignore_alloc_blks)   )  {  /*
			     * still on track
			     */
	  dp_rc = dTree_node_size_check( dtiptr, is_leaf,
					 first_in_level, last_in_level,
					 inorecptr, msg_info_ptr,
					 desired_action );
	  }  /* end still on track */
	if( (dp_rc == FSCK_OK) &&
	    (!inorecptr->ignore_alloc_blks) ) {  /* node is looking good */
	  dp_rc = dTree_verify_slot_freelist( dtiptr, inorecptr,
					      msg_info_ptr, desired_action );
	  }  /* end node is looking good */
	}  /* end tree still interesting */
      if( (dp_rc == FSCK_OK)       &&     /* nothing fatal and */
	(!inorecptr->ignore_alloc_blks) &&     /* tree looks ok and */
	(agg_recptr->dtreeQ_back == NULL)   ) {  /* queue is now empty */

	last_in_level = -1;
	if( !is_root ) {
	  dp_rc = dTree_node_last_in_level( dtiptr, inorecptr,
						msg_info_ptr,
						desired_action );
	  }
	}  /* end nothing fatal and tree looks ok and queue is now empty */
      }  /* end while nothing fatal and tree looks ok and ... */
	/*
	 * there's at least 1 more Q element to release for this node, and
	 * if the tree is bad there may still be some on the queue as well.
	 *
	 * (If there's a processing error all the dynamic storage is going
	 * to be released so there's no point in preparing these elements
	 * for reuse.)
	 */
    if( dp_rc == FSCK_OK)  {
      if( dtiptr->this_Qel != NULL )  {
	dp_rc = dtreeQ_rel_elem( dtiptr->this_Qel );
	}
      }
    if( dp_rc == FSCK_OK)  {
      if( dtiptr->next_Qel != NULL )  {
	dp_rc = dtreeQ_rel_elem( dtiptr->next_Qel );
	}
      }
    agg_recptr->dtreeQ_back = NULL;
    while( (dp_rc == FSCK_OK) &&
	   (agg_recptr->dtreeQ_front != NULL) ) {
      dtiptr->this_Qel = agg_recptr->dtreeQ_front;
      agg_recptr->dtreeQ_front = dtiptr->this_Qel->next;
      dp_rc = dtreeQ_rel_elem( dtiptr->this_Qel );
      }  /* end while */
    if( dp_rc == FSCK_OK )  {  /* not planning to quit */
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
	  dp_rc = FSCK_INTERNAL_ERROR_10;
	  }  /* end the flag is set but the bit didn't get turned ... */
	else if( !old_ignore_alloc_blks &&
		inorecptr->ignore_alloc_blks )  {  /*
			    * the flag is off but the bit got turned on.
			    * This means that the first time we went
			    * through this tree it looked ok but this
			    * time we decided that it is corrupt.
			    */
	  dp_rc = FSCK_INTERNAL_ERROR_11;
	  }  /* end the flag is off but the bit got turned on ... */
	}  /* end we altered the corrupt tree bit */
      }  /* not planning to quit */
    }  /* end there is at least 1 entry */
		
 return( dp_rc );
}                              /* end of dTree_processing ()  */
	
	
/*****************************************************************************
 * NAME: dTree_search
 *
 * FUNCTION: Search the dTree rooted in the specified inode for the given
 *           filename.
 *
 * PARAMETERS:
 *      dir_inoptr               - input - pointer to the directory inode in 
 *                                         an fsck buffer
 *      given_key                - input - pointer to the filename to find, in
 *                                         mixed case
 *      given_key_length         - input - length of the string in given_key
 *      given_key_folded         - input - pointer to the given_key in all
 *                                         upper case
 *      given_key_folded_length  - input - length of the string in 
 *                                         given_key_folded
 *      addr_slot_ptr            - input - pointer to a variable in which to 
 *                                         return the address of the slot
 *                                         which contains (or begins) the 
 *                                         the key which is a match for 
 *                                         given_key
 *      match_found              - input - pointer to a variable in which to 
 *                                         return !0 if an entry with given_key
 *                                                   or given_key_folded was
 *                                                   found in the given 
 *                                                   directory inode
 *                                                 0 if no entry with given_key
 *                                                   or given_key_folded was
 *                                                   found in the given 
 *                                                   directory inode
 *      inorecptr                - input - pointer to an fsck inode record 
 *                                         describing the inode
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_search( dinode_t         *dir_inoptr,
			UniChar          *given_key,
			reg_idx_t         given_key_length,
			UniChar          *given_key_folded,
			reg_idx_t         given_key_folded_length,
			dtslot_t        **addr_slot_ptr,
			int8             *match_found,
			fsck_inode_recptr inorecptr
		       )
{
  retcode_t ds_rc = FSCK_OK;
  int8   slot_selected;
  int8   selected_slotidx;
  int8   is_root = -1;
  int8   leaf_found = 0;
  int8   not_there = 0;
  int8   key_matched = 0;
  int8   case_insensitive = 0;
  idtentry_t *intentry;
  ldtentry_t *lfentry;
  fsblkidx_t node_addr;
  reg_idx_t  node_length;
  fsck_Dtree_info_t   dtinfo;
  fsck_Dtree_info_ptr dtiptr;
		
  dtiptr = &dtinfo;
	
  if( sb_ptr->s_flag & JFS_OS2 ) {  /*
		      * This aggregate is enabled for case insensitive
		      * directory searching.  Therefore the keys in the
		      * internal nodes have been folded to upper case
		      */
    case_insensitive = -1;
    }
	
  dtiptr->dtr_ptr = (dtroot_t *) &(dir_inoptr->di_btroot);
  dtiptr->dtstbl = (int8 *) &(dtiptr->dtr_ptr->header.stbl[0]);
  dtiptr->slots = (dtslot_t *) &(dtiptr->dtr_ptr->slot[0]);
	
  if( dtiptr->dtr_ptr->header.nextindex > 0 ) {  /*
				* the directory isn't empty
				*/
    dtiptr->last_dtidx = dtiptr->dtr_ptr->header.nextindex - 1;
    dtiptr->max_slotidx = DTROOTMAXSLOT - 1;
    if( dtiptr->dtr_ptr->header.flag & BT_LEAF ) {  /* it's a root leaf */
      leaf_found = -1;
      }
	
    while( (!leaf_found) && (!not_there) && (ds_rc == FSCK_OK) ) {
	/*
	 * notice that internal directory pages are always searched
	 * case insensitive
	 */
      ds_rc = dTree_binsrch_internal_page( dtiptr, given_key_folded,
					     given_key_folded_length,
					     case_insensitive, is_root,
					     &not_there, &slot_selected,
					     &selected_slotidx, inorecptr );
      if( (slot_selected) && (ds_rc == FSCK_OK) ) { /* still in business */
        intentry = (idtentry_t *) &(dtiptr->slots[selected_slotidx]);
        node_addr = addressPXD( &(intentry->xd) );
        node_length = lengthPXD( &(intentry->xd) );
        ds_rc = dnode_get( node_addr, BYTESPERPAGE, &(dtiptr->dtp_ptr) );
        if( ds_rc == FSCK_OK ) {  /* got the node */
	is_root = 0;
	dtiptr->dtstbl = (int8 *)
		  &(dtiptr->dtp_ptr->slot[dtiptr->dtp_ptr->header.stblindex]);
	dtiptr->slots = (dtslot_t *) &(dtiptr->dtp_ptr->slot[0]);
	dtiptr->last_dtidx = dtiptr->dtp_ptr->header.nextindex - 1;
	dtiptr->max_slotidx = (node_length * sb_ptr->s_bsize) / DTSLOTSIZE;
	if( dtiptr->dtp_ptr->header.flag & BT_LEAF ) {
	  leaf_found = -1;
	  }
	}  /* end got the node */
        }  /* end still in business */
      }  /* end while !leaf_found and !not_there */
	
    if( (!not_there) && (ds_rc == FSCK_OK) ) {  /* might be in there */
      if( case_insensitive ) { /* use the folded version of the name */
        ds_rc = dTree_binsrch_leaf( dtiptr, given_key_folded,
				   given_key_folded_length,
				   case_insensitive, is_root,
				   &not_there, &key_matched, &slot_selected,
				   &selected_slotidx, inorecptr );
        }  /* end use the folded version of the name */
      else { /* use the name in lower/mixed case */
        ds_rc = dTree_binsrch_leaf( dtiptr, given_key, given_key_length,
  				  case_insensitive, is_root,
				  &not_there, &key_matched, &slot_selected,
				  &selected_slotidx, inorecptr );
        }  /* end else use the name in lower/mixed case */
      }  /* end might be in there */
    }  /* end directory isn't empty */
	
  if( (ds_rc == FSCK_OK) && (key_matched) ) {  /* found it! */
    *addr_slot_ptr = &(dtiptr->slots[selected_slotidx]);
    *match_found = -1;
    }  /* end found it!  */
  else {  /* no luck */
    *addr_slot_ptr = NULL;
    *match_found = 0;
    }  /* end else no luck */
	
  return( ds_rc );
}                              /* end of dTree_search ()  */
		
		
/*****************************************************************************
 * NAME: dTree_verify_slot_freelist
 *
 * FUNCTION: Verify the structure and contents of the slot freelist in the
 *           specified directory node.
 *
 * PARAMETERS:
 *      dtiptr          - input - pointer to an fsck record describing the
 *                                directory tree
 *      inorecptr       - input - pointer to an fsck inode record describing
 *                                the inode
 *      msg_info_ptr    - input - pointer to data needed to issue messages 
 *                                about the inode
 *      desired_action  - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                  FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dTree_verify_slot_freelist( fsck_Dtree_info_ptr dtiptr,
				      fsck_inode_recptr   inorecptr,
				      fsck_msg_info_ptr   msg_info_ptr,
				      int                 desired_action
				     )
{
  retcode_t dvsf_rc = FSCK_OK;
  uint8 slotidx;
  dtslot_t *slot_ptr;
  int8 freelist_entry;
  int8 freelist_size = 0;
  int8 is_leaf = 0;
	
  if( dtiptr->this_Qel->node_level == dtiptr->leaf_level )  {
    is_leaf = -1;
    }
  if( dtiptr->freelist_first_dtidx == -1 ) {  /* the list is empty */
    if( dtiptr->freelist_count > 0 ) {  /* but the counter is nonzero */
      inorecptr->ignore_alloc_blks = 1;
      if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
			     * not reported yet
			     */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = msg_info_ptr->msg_inopfx;
        sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        fsck_send_msg( fsck_BADDINOFREELIST1, 0, 2 );
	}  /* end not reported yet */
      }  /* end but the counter is nonzero */
    }  /* end the list is empty */
  else {  /* the list is not empty */
    freelist_entry = dtiptr->freelist_first_dtidx;
    while( (freelist_entry != -1) &&
	   (dvsf_rc == FSCK_OK) &&
	   (!inorecptr->ignore_alloc_blks) &&
	   (freelist_size <= dtiptr->freelist_count) ) {
      if( dtiptr->slot_map[freelist_entry] ) {  /* already marked! */
	inorecptr->ignore_alloc_blks = 1;
	if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
			     * not reported yet
			     */
          msgprms[0] = message_parm_0;
          msgprmidx[0] = msg_info_ptr->msg_inopfx;
          sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
          msgprms[1] = message_parm_1;
          msgprmidx[1] = 0;
          fsck_send_msg( fsck_BADDINOFREELIST4, 0, 2 );
	  }  /* end not reported yet */
	}  /* end already marked!  */
      else {  /* not claimed yet */
	dtiptr->slot_map[freelist_entry] = -1;   /* mark this one */
	freelist_size++;                         /* count this one */
	slot_ptr = (dtslot_t *) &(dtiptr->slots[freelist_entry]);
	freelist_entry = slot_ptr->next;
	}  /* end else not claimed yet */
      }  /* end while */
    }  /* end else the list is not empty */
  if( !inorecptr->ignore_alloc_blks )  {  /* nothing wrong yet */
    if( freelist_size != dtiptr->freelist_count ) {  /* size is wrong */
      inorecptr->ignore_alloc_blks = 1;
      if( desired_action == FSCK_RECORD_DUPCHECK ) {  /*
			     * not reported yet
			     */
        msgprms[0] = message_parm_0;
        msgprmidx[0] = msg_info_ptr->msg_inopfx;
        sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        fsck_send_msg( fsck_BADDINOFREELIST2, 0, 2 );
	}  /* end not reported yet */
      }  /* end size is wrong */
    }  /* end nothing wrong yet */
  if( !inorecptr->ignore_alloc_blks )  {  /* still looks good */
    for( slotidx = 0;
	 ((slotidx <= dtiptr->max_slotidx) && (!inorecptr->ignore_alloc_blks));
	 slotidx++ ) {
      if( dtiptr->slot_map[slotidx] == 0 ) {  /*
			     * a slot which is not in a key chain and is
			     * also not on the free list
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
          fsck_send_msg( fsck_BADDINOFREELIST3, 0, 2 );
	  }  /* end not reported yet */
	}  /* end a slot which is not in a key chain and is also ... */
      }  /* end for */
    }  /* end still looks good */
		
  return( dvsf_rc );
}                              /* end of dTree_verify_slot_freelist ()  */
		
		
/*****************************************************************************
 * NAME: find_first_dir_leaf
 *
 * FUNCTION: Locate the leftmost leaf node in the dTree rooted in the 
 *           given inode.
 *
 * PARAMETERS:
 *      inoptr           - input - pointer to the directory inode in an 
 *                                 fsck buffer 
 *      addr_leaf_ptr    - input - pointer to a variable in which to return
 *                                 the address, in an fsck buffer, of the
 *                                 inode's left-most leaf.
 *      leaf_agg_offset  - input - pointer to a variable in which to return
 *                                 the offset, in the aggregate, of the
 *                                 leftmost leaf in the B+ Tree rooted in
 *                                 the directory inode.  (This has a 
 *                                 special meaning if the directory root is
 *                                 not an internal node. See the code below.)
 *      is_inline        - input - pointer to a variable in which to return
 *                                 !0 if the directory data is within the
 *                                    inode
 *                                  0 if the directory data is not within
 *                                    the inode 
 *      is_rootleaf      - input - pointer to a variable in which to return
 *                                 !0 if the B+ Tree rooted in the directory
 *                                    inode has a root leaf
 *                                  0 if the B+ Tree rooted in the directory
 *                                      inode does not have a root leaf
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t find_first_dir_leaf( dinode_t    *inoptr,
			       dtpage_t   **addr_leaf_ptr,
			       fsblkidx_t  *leaf_agg_offset,
			       int8        *is_inline,
			       int8        *is_rootleaf
			      )
{
  retcode_t ffdl_rc = FSCK_OK;
  dtpage_t   *dtpg_ptr;
  dtroot_t   *dtroot_ptr;
  idtentry_t *idtentry_ptr;
  fsblkidx_t  first_child_addr;
  reg_idx_t   first_child_length;
  int8       *dtstbl;
	
  *is_rootleaf = 0;                /* assume inode has no data */
  *is_inline = 0;                  /* assume inode has no data */
  *addr_leaf_ptr = NULL;           /* assume inode has no data */
  *leaf_agg_offset = 0;            /* assume inode has no data */
  dtroot_ptr = (dtroot_t *) &(inoptr->di_btroot);
  if( (dtroot_ptr->header.flag & BT_LEAF) &&
      (dtroot_ptr->header.flag & DXD_INDEX) )  {  /* it's a root-leaf */
    *is_rootleaf = -1;
    *leaf_agg_offset = addressPXD(&(inoptr->di_ixpxd));
    }  /* end it's a root-leaf */
  else {  /* it's a tree */
    idtentry_ptr = (idtentry_t *)
			&( dtroot_ptr->slot[
					    dtroot_ptr->header.stbl[0]
					    ] ); /*
			    * the slot number of the entry describing
			    * the first child is in the 0th entry of
			    * the header.stbl array.
			    */
    first_child_addr = addressPXD( &(idtentry_ptr->xd) );
    first_child_length = lengthPXD( &(idtentry_ptr->xd) );
		
    ffdl_rc = dnode_get( first_child_addr, BYTESPERPAGE, &dtpg_ptr );
		
    while( (ffdl_rc == FSCK_OK) &&
	   (*leaf_agg_offset == 0)        ) {
      if( dtpg_ptr->header.flag & BT_LEAF )  {  /* found it!  */
	*addr_leaf_ptr = dtpg_ptr;
	*leaf_agg_offset = first_child_addr;
	}  /* end found it!  */
      else {  /* keep moving down the tree */
	dtstbl = (int8 *) &(dtpg_ptr->slot[ dtpg_ptr->header.stblindex ]);
	idtentry_ptr = (idtentry_t *)
			&( dtpg_ptr->slot[ dtstbl[0] ] ); /*
			    * the address of the idtentry describing
			    * the first child
			    */
	first_child_addr = addressPXD( &(idtentry_ptr->xd) );
	first_child_length = lengthPXD( &(idtentry_ptr->xd) );
	ffdl_rc = dnode_get( first_child_addr, BYTESPERPAGE,
			     &dtpg_ptr );
	}  /* end else keep moving down the tree */
      }  /* end while */
    }  /* end it's a tree */
	
  return( ffdl_rc );
}                              /* end of find_first_dir_leaf ()  */
		
		
/*****************************************************************************
 * NAME: init_dir_tree
 *
 * FUNCTION: Initialize the dTree rooted at the given address.
 *
 * PARAMETERS:
 *      btroot_ptr  - input - pointer to the root of the B+ Tree rooted in
 *                            the directory inode
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
void init_dir_tree( dtroot_t *btroot_ptr )
{
  int slot_idx;
		
  btroot_ptr->header.flag = (BT_ROOT | BT_LEAF | DXD_INDEX);
  btroot_ptr->header.nextindex = 0;
  btroot_ptr->header.freecnt = DTROOTMAXSLOT - 1;
  btroot_ptr->header.freelist = 1;
  for( slot_idx = 1; (slot_idx < DTROOTMAXSLOT); slot_idx++ ) {
    btroot_ptr->slot[slot_idx].next = slot_idx + 1;
    }
  btroot_ptr->slot[DTROOTMAXSLOT - 1].next = -1;
		
  return;
}                              /* end of init_dir_tree ()  */
	
	
/*****************************************************************************
 * NAME: process_valid_dir_data
 *
 * FUNCTION: Perform the specified action on the nodes of the specified dTree. 
 *           Assume that the dTree structure has already been validated.
 *
 * PARAMETERS:
 *      inoptr          - input - pointer to the directory inode in an fsck 
 *                                buffer
 *      inoidx          - input - ordinal number of the inode 
 *      inorecptr       - input - pointer to an fsck inode record describing
 *                                the inode
 *      msg_info_ptr    - input - pointer to data needed to issue messages 
 *                                about the inode
 *      desired_action  - input - { FSCK_RECORD | FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t process_valid_dir_data  ( dinode_t          *inoptr,
				    inoidx_t           inoidx,
				    fsck_inode_recptr  inorecptr,
				    fsck_msg_info_ptr  msg_info_ptr,
				    int                desired_action
				   )
{
  retcode_t pvdd_rc = FSCK_OK;
  dtroot_t *dtroot_ptr;
  dtpage_t *dtpage_ptr;
  int8       *dtstbl;
  dtslot_t   *slots;
  int         last_dtidx;
  idtentry_t *idtentry_ptr;       /* internal node entry */
  fsblkidx_t node_addr_fsblks;
  reg_idx_t node_length_fsblks;
  fsblkidx_t first_child_addr;
  reg_idx_t first_child_length;
  int8 all_done = 0;
		
  dtroot_ptr = (dtroot_t *) &(inoptr->di_btroot);
if( dtroot_ptr->header.flag != 0 ) {  /* there is data for this inode */
  if( dtroot_ptr->header.flag & DXD_INDEX )  {  /* it's a tree */
    if( !(dtroot_ptr->header.flag & BT_LEAF) ) {  /* not a root-leaf */
      dtstbl = (int8 *) &(dtroot_ptr->header.stbl[0]);
      slots = &(dtroot_ptr->slot[0]);
      last_dtidx = dtroot_ptr->header.nextindex - 1;
      idtentry_ptr = (idtentry_t *) &( slots[ dtstbl[0] ] ); /*
			       * slot describing the first child
			       */
      first_child_addr = addressPXD( &(idtentry_ptr->xd) );
      first_child_length = lengthPXD( &(idtentry_ptr->xd) );
      pvdd_rc = process_valid_dir_node( dtstbl, slots, last_dtidx,
					inorecptr, msg_info_ptr,
					desired_action );
      while( (pvdd_rc == FSCK_OK) &&
	     (!all_done) )  {  /* while not done */
	  /*
	   * We have the address of the first node in the new level.
	   * Get the first node in the new level.
	   */
	pvdd_rc = dnode_get( first_child_addr, BYTESPERPAGE,
			     &dtpage_ptr );
	if( pvdd_rc == FSCK_OK ) {  /* got the first child */
	  if( dtpage_ptr->header.flag & BT_LEAF )  {  /* we're done */
	    all_done = -1;
	    }  /* end we're done */
	  else {  /* not down to the leaf level yet */
		/*
		 * Set up to process this level.
		 */
	    dtstbl = (int8 *)
		      &(dtpage_ptr->slot[ dtpage_ptr->header.stblindex ]);
	    slots = &(dtpage_ptr->slot[0]);
	    last_dtidx = dtpage_ptr->header.nextindex - 1;
		/*
		 * save the address of this node's first child
		 */
	    idtentry_ptr = (idtentry_t *) &( slots[ dtstbl[0] ] ); /*
			       * slot describing the first child
			       */
	    first_child_addr = addressPXD( &(idtentry_ptr->xd) );
	    first_child_length = lengthPXD( &(idtentry_ptr->xd) );
		 /*
		  * process everything described by this node
		  */
	    pvdd_rc = process_valid_dir_node( dtstbl, slots, last_dtidx,
					      	inorecptr, msg_info_ptr,
						desired_action );
		 /*
		  * process everything described by this node's
		  * siblings and cousins (if any)
		  */
	    while( (pvdd_rc == FSCK_OK) &&
		   (dtpage_ptr->header.next != ((int64)0)) )  {  /*
			       * more to process on this level
			       */
	      node_addr_fsblks = dtpage_ptr->header.next;
	      node_length_fsblks = sizeof(dtpage_t);
		    /*
		     * Get the next node in this level.
		     */
	      pvdd_rc = dnode_get( node_addr_fsblks, BYTESPERPAGE,
				   &dtpage_ptr );
	      if( pvdd_rc == FSCK_OK ) {  /* got the node */
		      /*
		       * Set up to process this node.
		       */
		dtstbl = (int8 *)
			&(dtpage_ptr->slot[ dtpage_ptr->header.stblindex ]);
		slots = &(dtpage_ptr->slot[0]);
		last_dtidx = dtpage_ptr->header.nextindex - 1;
		pvdd_rc = process_valid_dir_node( dtstbl, slots,
						  last_dtidx, inorecptr,
						  msg_info_ptr, desired_action );
		}  /* end got the node */
	      }  /* end while more to process on this level */
	    }  /* end not down to the leaf level yet */
	  }  /* end got the first child */
	}  /* end while not done */
      }  /* end not a root-leaf */
    }  /* end it's a Tree */
  }  /* end there is data for this inode */
		
 return( pvdd_rc );
}                              /* end of process_valid_dir_data ()  */
			
		
/*****************************************************************************
 * NAME: process_valid_dir_node
 *
 * FUNCTION: Perform the specified action on the specified dTree node.  
 *           Assume that the node structure has already been validated.
 *
 * PARAMETERS:
 *      dtstbl          - input - pointer to the sorted entry index table
 *                                in the directory node
 *      slots           - input - pointer to slot[0] in the directory node
 *      last_dtidx      - input - last valid entry in the directory node's
 *                                sorted entry index table
 *      inorecptr       - input - pointer to an fsck inode record describing
 *                                the inode
 *      msg_info_ptr    - input - pointer to data needed to issue messages 
 *                                about the inode
 *      desired_action  - input - { FSCK_RECORD | FSCK_UNRECORD | FSCK_QUERY } 
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t process_valid_dir_node ( int8              *dtstbl,
				   dtslot_t          *slots,
				   int                last_dtidx,
				   fsck_inode_recptr  inorecptr,
				   fsck_msg_info_ptr  msg_info_ptr,
				   int                desired_action
				  )
{
  retcode_t pvdn_rc = FSCK_OK;
  int         dtidx;
  idtentry_t *idtentry_ptr;       /* internal node entry */
  fsblkidx_t child_addr;
  reg_idx_t  child_length;
  int8 is_EA = 0;
  int8 is_ACL = 0;
  int8 extent_is_valid;
  reg_idx_t block_count;
		
  for( dtidx = 0;
       ( (pvdn_rc == FSCK_OK) && (dtidx <= last_dtidx) );
       dtidx++ ) {  /* for each entry used in the dtstbl */
    idtentry_ptr = (idtentry_t *) &( slots[ dtstbl[dtidx] ] ); /*
			     * slot describing the first child
			     */
    child_addr = addressPXD( &(idtentry_ptr->xd) );
    child_length = lengthPXD( &(idtentry_ptr->xd) );
    pvdn_rc = process_extent( inorecptr, child_length, child_addr,
			      is_EA, is_ACL, msg_info_ptr, &block_count,
			      &extent_is_valid, desired_action );
    agg_recptr->blocks_this_fset -= block_count;
    agg_recptr->this_inode.all_blks -= block_count;
    agg_recptr->this_inode.data_blks -= block_count;
    if( (desired_action == FSCK_RECORD)         ||
	(desired_action == FSCK_RECORD_DUPCHECK)  ) {
      agg_recptr->blocks_this_fset += block_count;
      agg_recptr->this_inode.all_blks += block_count;
      agg_recptr->this_inode.data_blks += block_count;
      }
    else if( desired_action == FSCK_UNRECORD ) {
      agg_recptr->blocks_this_fset -= block_count;
      agg_recptr->this_inode.all_blks -= block_count;
      agg_recptr->this_inode.data_blks -= block_count;
      }
    }  /* end for each entry used in the dtstbl */
		
 return( pvdn_rc );
}                              /* end of process_valid_dir_node ()  */
		
		
/*****************************************************************************
 * NAME: reconnect_fs_inodes
 *
 * FUNCTION: Add a directory entry to /lost+found/ for each unconnected 
 *           inode in the list at agg_recptr->inode_reconn_extens
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t reconnect_fs_inodes ( )
{
  retcode_t rfsi_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  char inopfx = ' ';             /* fileset inodes */
  uint dir_inodes_reconnected = 0;
  uint dir_inodes_not_reconnected = 0;
  uint inodes_reconnected = 0;
  uint inodes_not_reconnected = 0;
  char inoname[13];                         /* @D2 */
  char *ino_name;
  fsck_inode_extptr this_ext;
  fsck_inode_recptr inorecptr;
  int aggregate_inode = 0;
  int alloc_ifnull = 0;
  int is_aggregate = 0;                  /* aggregate has no dirs       */
  int which_it = FILESYSTEM_I;           /* only 1 fileset in release 1 */
  dinode_t *lsfn_inoptr, *root_inoptr;                                 /* @F1 */
  int64 lsfn_dasd_used, root_dasd_used;                             /* @F1 */
	
  ino_name = &(inoname[0]);
  this_ext = agg_recptr->inode_reconn_extens;
  intermed_rc = inode_get( is_aggregate, which_it, agg_recptr->lsfn_inonum,
			   &lsfn_inoptr );
  if( intermed_rc != FSCK_OK ) {  /* this is unexpected but not fatal to
				   * this run or to the filesystem
				   */
    agg_recptr->lsfn_ok = 0;
    }  /* end this is unexpected but not fatal to this run or to the ... */
  else {  /* prime the lost+found DASDused adjustment talley               @F1
             *
             * We may have just created \lost+found, or it may have existed 
             * already.  Reconnecting inodes may cause blocks to be allocated
             * for the dtree rooted in \lost+found.  If we subtract the number of
             * blocks allocated to \lost+found before we start reconnecting, and 
             * then add the the number of blocks allocated to \lost+found after
             * we're done reconnecting, the adjustment will include only those
             * blocks allocated to \lost+found during this reconnection session.
             */ 
    agg_recptr->lsfn_dasdused_adjustment = (-1) * lsfn_inoptr->di_nblocks; /* @F1 */
    }                                                                                       /* @F1 */
  
  while( (rfsi_rc == FSCK_OK) &&
	 (this_ext != NULL)             ) {
    rfsi_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
			     this_ext->inonum, &inorecptr );
    if( (rfsi_rc == FSCK_OK) && (inorecptr == NULL) ) {
      rfsi_rc = FSCK_INTERNAL_ERROR_47;
      }
    else if( rfsi_rc == FSCK_OK ) {
      if( agg_recptr->lsfn_ok ) {  /* /lost+found/ resolved ok */
	   /*
	    * compute a name for the directory entry
	    */
	if( inorecptr->inode_type == directory_inode ) {
	  sprintf( ino_name, "%s%07u%s", MsgText[fsck_dirpfx],
		   this_ext->inonum, MsgText[fsck_dotext]);
	  }
	else {
	  sprintf( ino_name, "%s%07u%s", MsgText[fsck_inopfx],
		   this_ext->inonum, MsgText[fsck_dotext]);
	  }
	(void) UniStrToUcs( uconv_object, Uni_Name, ino_name, JFS_NAME_MAX );
	intermed_rc = direntry_add( lsfn_inoptr, this_ext->inonum, Uni_Name );
	if( intermed_rc != FSCK_OK ) {  /* couldn't do it */
	  if( intermed_rc < 0 ) {  /* something fatal */
	    rfsi_rc = intermed_rc;
	    }
	  else {  /* not fatal */
	    agg_recptr->lsfn_ok = 0;
	       /*
		* don't move this_ext to next extension record.  execute
		* the loop for this one again so that it will be
		* handled as one that can't be reconnected.
		*/
	    }  /* end else not fatal */
	  }  /* end couldn't do it */
	else {  /* entry added */
	  msgprms[0] = &(inopfx);
	  msgprmidx[0] = 0;
	  sprintf( message_parm_1, "%ld", this_ext->inonum );
	  msgprms[1] = message_parm_1;
	  msgprmidx[1] = 0;
            agg_recptr->lsfn_dasdused_adjustment += inorecptr->cumm_blocks; /* @F1 */
	  if( inorecptr->inode_type == directory_inode ) {
	    lsfn_inoptr->di_nlink += 1;
	    dir_inodes_reconnected += 1;
	    fsck_send_msg( fsck_INOINLSFND, 0, 2 );
	    }  /* end a directory inode */
	  else {  /* not a directory inode */
	    inodes_reconnected += 1;
	    fsck_send_msg( fsck_INOINLSFNF, 0, 2 );
	    }  /* end else not a directory inode */
	  this_ext = this_ext->next;
	  }  /* end else entry added */
	}  /* end /lost+found/ resolved ok */
      else {  /* /lost+found/ is not ok */
	msgprms[0] = &(inopfx);
	msgprmidx[0] = 0;
	sprintf( message_parm_1, "%ld", this_ext->inonum );
	msgprms[1] = message_parm_1;
	msgprmidx[1] = 0;
	if( inorecptr->inode_type == directory_inode ) {
	  dir_inodes_not_reconnected += 1;
	  fsck_send_msg( fsck_MNCNTRCNCTINOD, 0, 2 );
	  }  /* end a directory inode */
	else {  /* not a directory inode */
	  inodes_not_reconnected += 1;
	  fsck_send_msg( fsck_MNCNTRCNCTINOF, 0, 2 );
	  }  /* end else not a directory inode */
	this_ext = this_ext->next;
	}  /* end else /lost+found/ is not ok */
      }
    }  /* end while */
	
  if( (rfsi_rc == FSCK_OK) && 
      ( (dir_inodes_reconnected != 0) || (inodes_reconnected != 0) ) ) { /* @F1 */
    agg_recptr->lsfn_dasdused_adjustment += lsfn_inoptr->di_nblocks; /* @F1 */
    lsfn_dasd_used = DASDUSED( &(lsfn_inoptr->di_DASD) ) +
                            agg_recptr->lsfn_dasdused_adjustment;               /* @F1 */
    setDASDUSED(&(lsfn_inoptr->di_DASD), lsfn_dasd_used);                /* @F1 */
    rfsi_rc = inode_put( lsfn_inoptr );

    if( rfsi_rc == FSCK_OK ) {                                                           /* @F1 */
      rfsi_rc = inode_get( is_aggregate, which_it, ROOT_I, &root_inoptr ); /* @F1 */
      if( rfsi_rc == FSCK_OK ) {                                                         /* @F1 */
        root_dasd_used = DASDUSED( &(root_inoptr->di_DASD) ) +
                                agg_recptr->lsfn_dasdused_adjustment;           /* @F1 */
        setDASDUSED(&(root_inoptr->di_DASD), root_dasd_used);           /* @F1 */
        rfsi_rc = inode_put( root_inoptr );                                            /* @F1 */
        }                                                                                         /* @F1 */
      }                                                                                           /* @F1 */
    }
   /*
    * put out some summary messages in case we're not processing verbose
    */
  if( rfsi_rc >= FSCK_OK ) {  /* not fleeing something horrendous */
    sprintf( message_parm_0, "%ld", dir_inodes_not_reconnected );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    if( dir_inodes_not_reconnected > 0 )  {
      if( dir_inodes_not_reconnected == 1 )  {
	fsck_send_msg( fsck_MNCNTRCNCTINOSD, 0, 1 );
	}
      else {
	fsck_send_msg( fsck_MNCNTRCNCTINOSDS, 0, 1 );
	}
      }
    if( dir_inodes_reconnected > 0 ) {
      sprintf( message_parm_0, "%ld", dir_inodes_reconnected );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      if( dir_inodes_reconnected == 1 )  {
          fsck_send_msg( fsck_INOSINLSFND, 0, 1 );
	}
      else {
          fsck_send_msg( fsck_INOSINLSFNDS, 0, 1 );
	}
      }
    if( inodes_not_reconnected > 0 ) {
      sprintf( message_parm_0, "%ld", inodes_not_reconnected );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      if( inodes_not_reconnected == 1 )  {
	fsck_send_msg( fsck_MNCNTRCNCTINOSF, 0, 1 );
	}
      else {
	fsck_send_msg( fsck_MNCNTRCNCTINOSFS, 0, 1 );
	}
      }
    if( inodes_reconnected > 0 ) {
      sprintf( message_parm_0, "%ld", inodes_reconnected );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      if( inodes_reconnected == 1 )  {
	fsck_send_msg( fsck_INOSINLSFNF, 0, 1 );
	}
      else {
	fsck_send_msg( fsck_INOSINLSFNFS, 0, 1 );
	}
      }
    }  /* end not fleeing something horrendous */
		
  return( rfsi_rc );
}                              /* end of reconnect_fs_inodes ()  */
