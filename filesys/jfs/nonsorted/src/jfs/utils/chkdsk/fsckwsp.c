/* $Id: fsckwsp.c,v 1.3 2004/03/21 02:43:26 pasha Exp $ */

static char *SCCSID = "@(#)1.23  6/28/99 13:50:12 src/jfs/utils/chkdsk/fsckwsp.c, jfschk, w45.fs32, fixbld";
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
 *   MODULE_NAME:		fsckwsp.c
 *
 *   COMPONENT_NAME: 	jfschk
 *
 *
 *   FUNCTIONS:
 *              alloc_vlarge_buffer
 *              blkall_decrement_owners
 *              blkall_increment_owners
 *              blkall_ref_check
 *              dire_buffer_alloc
 *              dire_buffer_release
 *              directory_buffers_alloc
 *              directory_buffers_release
 *              dtreeQ_dequeue
 *              dtreeQ_enqueue
 *              dtreeQ_get_elem
 *              dtreeQ_rel_elem
 *              establish_agg_workspace
 *              establish_ea_iobuf
 *              establish_fs_workspace
 *              establish_io_buffers
 *              establish_wsp_block_map_ctl
 *              extent_record
 *              extent_unrecord
 *              fsck_alloc_fsblks
 *              fsck_dealloc_fsblks
 *              fscklog_end
 *              fscklog_init
 *              fscklog_start
 *              get_inode_extension
 *              get_inorecptr
 *              get_inorecptr_first
 *              get_inorecptr_next
 *              init_agg_record
 *              process_extent
 *              release_inode_extension
 *              release_logredo_allocs
 *              temp_inode_buf_alloc
 *              temp_inode_buf_release
 *              temp_node_buf_alloc
 *              treeQ_dequeue
 *              treeQ_enqueue
 *              treeQ_get_elem
 *              treeQ_rel_elem
 *              treeStack_pop
 *              treeStack_push
 *              workspace_release
 *
 *              alloc_wrksp
 *              alloc_wsp_extent
 *              dupall_extract_blkrec
 *              dupall_find_blkrec
 *              dupall_get_blkrec
 *              dupall_insert_blkrec
 *              establish_wsp_block_map
 *              extent_1stref_chk
 *              extent_record_dupchk
 *              fsblk_count_avail
 *              fsblk_next_avail
 *              inorec_agg_search
 *              inorec_agg_search_insert
 *              inorec_fs_search
 *              inorec_fs_search_insert
 *              locate_inode
 *              treeStack_get_elem
 *              treeStack_rel_elem
 *
 */

/*
 * defines and includes common among the xfsck modules
 */
#include "xfsckint.h"
#include "debug.h"


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
  * fsck block map info structure and pointer
  *
  *      defined in xchkdsk.c
  */
extern struct fsck_bmap_record *bmap_recptr;

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

extern DATETIME fsck_DateTime;

extern char *MsgText[];

extern char *Vol_Label;

extern UniChar uni_LSFN_NAME[];
extern UniChar uni_lsfn_name[];

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

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * Unicode path strings information.
  *
  *     values are assigned when the fsck aggregate record is initialized.
  *     accessed via addresses in the fack aggregate record.
  */
extern UniChar uni_LSFN_NAME[11];
extern UniChar uni_lsfn_name[11];

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 *
 * The following are internal to this file
 *
 */

retcode_t alloc_wsp_extent ( reg_idx_t, int );

                                                                         /* 2 lines deleted    @F1 */

retcode_t dupall_extract_blkrec ( dupall_blkrec_ptr, dupall_blkrec_tkn );

retcode_t dupall_find_blkrec ( fsblkidx_t, dupall_blkrec_ptr *,
                               dupall_blkrec_tkn * );

retcode_t dupall_get_blkrec ( dupall_blkrec_ptr * );

retcode_t dupall_insert_blkrec ( fsblkidx_t );

retcode_t establish_wsp_block_map ( void );

retcode_t extent_1stref_chk ( fsblkidx_t, fsblkidx_t, int8, int8,
                              fsck_msg_info_ptr, fsck_inode_recptr );

retcode_t extent_record_dupchk ( fsblkidx_t, fsblkidx_t, int8, int8, int8,
                                 fsck_msg_info_ptr, fsck_inode_recptr );

retcode_t fsblk_count_avail( fsck_bitmap_ptr, int32*, int32*, int32, int32 * );

retcode_t fsblk_next_avail( fsck_bitmap_ptr,  int32, int32, int32 *,
                            int32 *, int * );

retcode_t inorec_agg_search( ino_t, fsck_inode_recptr * );           /* @F1 */

retcode_t inorec_agg_search_insert( ino_t, fsck_inode_recptr * );  /* @F1 */

retcode_t inorec_fs_search( ino_t, fsck_inode_recptr * );              /* @F1 */

retcode_t inorec_fs_search_insert( ino_t, fsck_inode_recptr * );     /* @F1 */

void locate_inode( ino_t, int32 *, int32 *, int32 * );                       /* @F1 */

retcode_t treeStack_get_elem ( treeStack_ptr * );

retcode_t treeStack_rel_elem ( treeStack_ptr );


/*
 * The following are used for reporting storage related errors
 */
extern int wsp_dynstg_action;
extern int wsp_dynstg_object;


/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */


/****************************************************************************
 * NAME: alloc_vlarge_buffer
 *
 * FUNCTION: Allocate the very large multi-use buffer
 *
 *
 * PARAMETERS:	none
 *
 * NOTES:	This must be called before logredo since the purpose
 *		is to ensure a buffer which has been obtained via
 *		malloc(), whether we're called from the command line
 *		or during autocheck.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t alloc_vlarge_buffer( )
{
  retcode_t avb_rc = FSCK_OK;

  agg_recptr->vlarge_buf_ptr = (char *) malloc(VLARGE_BUFSIZE);
  agg_recptr->vlarge_buf_length = VLARGE_BUFSIZE;
  agg_recptr->vlarge_current_use = NOT_CURRENTLY_USED;

 return( avb_rc );
}                                    /* end alloc_vlarge_buffer()    */


/****************************************************************************
 * NAME: alloc_wrksp
 *
 * FUNCTION:  Allocates and initializes (to guarantee the storage is backed)
 *            dynamic storage for the caller.
 *
 * PARAMETERS:
 *      length         - input - the number of bytes of storage which are needed
 *      dynstg_object  - input - a constant (see xfsck.h) identifying the purpose
 *                               for which the storage is needed (Used in error 
 *                               message if the request cannot be satisfied.
 *      addr_wrksp_ptr - input - the address of a variable in which this routine
 *                               will return the address of the dynamic storage
 *                               allocated for the caller
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t alloc_wrksp ( reg_idx_t   length,
                        int         dynstg_object,
                        int         for_logredo,
                        void      **addr_wrksp_ptr )
{
  retcode_t awsp_rc = FSCK_OK;
  char *wsp_ptr = NULL;
  wsp_ext_rec_ptr this_fer;
  reg_idx_t bytes_avail;
  reg_idx_t min_length;

  *addr_wrksp_ptr = NULL;      /* initialize return value */
  min_length = ((length + 3) / 4) * 4;  /* round up to an 4 byte boundary */

  while ( (wsp_ptr == NULL) && (awsp_rc == FSCK_OK) ) {
    this_fer = agg_recptr->wsp_extent_list;

    while( (this_fer != NULL)  &&
           (wsp_ptr == NULL)   &&
           (awsp_rc == FSCK_OK)  ) {
      if( (for_logredo) && !(this_fer->for_logredo) ) {	/*
				* requestor is logredo and
				* fer describes an allocation not for logredo 
				*/
        this_fer = this_fer->next;
        }  /* end requestor is logredo and ... */
      else {  /* this fer describes an eligible allocation */
        bytes_avail = this_fer->extent_length - this_fer->last_byte_used;
        if( bytes_avail >= min_length ) {  /* there's enough here */
          wsp_ptr = this_fer->extent_addr + this_fer->last_byte_used + 1;
          this_fer->last_byte_used += min_length;
          }  /* end there's enough here */
        else {  /* try the next fer */
          this_fer = this_fer->next;
          }  /* end else try the next fer */
        }  /* end else this fer describes an eligible allocation */
      }  /* end while */

    if( (awsp_rc == FSCK_OK) && (wsp_ptr == NULL) ) {  /*
                            * nothing fatal but we didn't find the
                            * storage yet
                            */
      awsp_rc = alloc_wsp_extent( min_length, for_logredo );  /* 
                            * will allocate some number of memory segments
                            *  and put the fer describing it on the beginning
                            *  of the list.
                            */
      }  /* end nothing fatal but we didn't find the storage yet */
    }  /* end while */

  if( awsp_rc == FSCK_OK ) {  /* we allocated virtual storage */
        /*
         * now initialize the storage 
         */
    memset( (void *) wsp_ptr, 0, length );

    *addr_wrksp_ptr = (void *) wsp_ptr;  /* set the return value */
    }  /* end we allocated virtual storage */

 return( awsp_rc );
}                                      /* end alloc_wrksp    */


/****************************************************************************
 * NAME: alloc_wsp_extent
 *
 * FUNCTION: Extend the workspace
 *
 *           Since OS/2 always allocates a new memory segment when
 *           it encounters a malloc(), for optimum use of the
 *           storage we'll always allocate a whole segment.  (Then
 *           the alloc_wrksp routine portions it out as needed.)
 *
 * PARAMETERS:
 *      minimum_length - input - minimum number of bytes of contiguous storage
 *                               needed
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t alloc_wsp_extent ( reg_idx_t 	minimum_length,
                             int		for_logredo          )
{
  retcode_t awe_rc = FSCK_OK;
  wsp_ext_rec_ptr new_fer;
  int32 extent_length = MEMSEGSIZE;
  char *extent_addr = NULL;
  int8  from_high_memory = 0;

     /*
      * the user has specified the minimum s/he needs.  We must allocate
      * at least 16 more than that because we're going to use 16 bytes
      * at the beginning to keep track of it.
      */
  while( extent_length < (minimum_length + 16) ) {
    extent_length += MEMSEGSIZE;
    }

  wsp_dynstg_object = dynstg_iobufs;
  wsp_dynstg_action = dynstg_allocation;

  extent_addr = (char *) malloc(extent_length);

  if( extent_addr == NULL ) {  /* the malloc failed */
    if( agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK] ) {  /* 
                        * we're in autocheck mode 
                        */
      if( !agg_recptr->high_mem_allocated ) {  /* 
                        * haven't allocated the high memory yet
                        */
        fsck_send_msg( fsck_ALLOCHIGHMEM, 0, 0 );
   
        from_high_memory = -1;
        alloc_high_mem( &extent_addr, &extent_length );

        sprintf( message_parm_0, "0x0%x", extent_addr );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "(d) %d", extent_length );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        fsck_send_msg( fsck_ALLOCHIGHMEMRSLT, 0, 2 );
#ifdef _JFS_DEBUG
printf("allocated from high memory:  (d)%d bytes at 0x0%lx\n", extent_length, extent_addr);
#endif
        }  /* end haven't allocated the high memory yet */
      }  /* end we're in autocheck mode */
    }  /* end the malloc failed */

  if( extent_addr == NULL ) { /* allocation failure */
    awe_rc = FSCK_FAILED_DYNSTG_EXHAUST4;
    if( ! for_logredo ) {
      sprintf( message_parm_0, "%d", wsp_dynstg_action );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", dynstg_wspext );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      fsck_send_msg( fsck_EXHDYNSTG, 0, 2 );
      }  /* end not for logredo */
    }  /* end allocation failure */

  else {  /* got the dynamic storage  */

        /*
         * use the first 16 bytes of it to keep track of it
         */
    new_fer = (wsp_ext_rec_ptr) extent_addr;
    new_fer->extent_length = extent_length;
    new_fer->for_logredo = for_logredo;
    new_fer->from_high_memory = from_high_memory;
    new_fer->extent_addr = extent_addr;
    new_fer->last_byte_used = sizeof(fsck_extent_record) - 1;

    new_fer->next = agg_recptr->wsp_extent_list;
    agg_recptr->wsp_extent_list = new_fer;
    }  /* end else got the dynamic storage */

 return( awe_rc );
}                                    /* end alloc_wsp_extent    */


/*                                                                  250 lines deleted      @F1 */ 


/****************************************************************************
 * NAME: blkall_decrement_owners
 *
 * FUNCTION: Adjust the fsck workspace to show one less owner for the 
 *           indicated block.
 *
 * PARAMETERS:
 *      blk_num  - input - ordinal number of the filesystem block whose owner
 *                         count is to be adjusted.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t blkall_decrement_owners ( fsblkidx_t blk_num )
{
 retcode_t ddo_rc = FSCK_OK;
 dupall_blkrec_ptr this_blkrec;
 dupall_blkrec_tkn this_blktkn;
 mappgidx_t page_num;
 reg_idx_t word_offset;
 fsck_dword_bitmask_t bit_mask;
 blk_pageptr this_page;
 fsck_bitmap_ptr this_word;

 /*
  * if the given block number is a multiply-allocated block,
  * decrement its count of inode owners.
  */
 ddo_rc = dupall_find_blkrec( blk_num, &this_blkrec, &this_blktkn );
 if( this_blkrec != NULL ) {  /* block is multiply-allocated */
   this_blkrec->owner_count--;

   if( this_blkrec->owner_count == 1 ) {  /* now only singly-allocated */
     if( !(this_blkrec->first_ref_resolved) ) {
       agg_recptr->unresolved_1stref_count--;
       }
     agg_recptr->dup_block_count--;
     ddo_rc = dupall_extract_blkrec( this_blkrec, this_blktkn ); /* take
                         * the block off the list of multiply-allocated
                         * blocks
                         */
     }  /* end now only singly-allocated */
   }  /* end block is multiply-allocated */

 else {  /* the block is not multiply-allocated */

   ddo_rc = blkmap_find_bit( blk_num, &page_num, &word_offset, &bit_mask);
   ddo_rc = blkmap_get_page( page_num, &this_page );
   if( ddo_rc == FSCK_OK ) { /* got the page */
     this_word = (fsck_bitmap_ptr) ((uint64) this_page + word_offset);
     (*this_word) &= ~bit_mask;  /*
                         * mark it not allocated at all
                         */
     ddo_rc = blkmap_put_page( page_num );  /* write it to workspace */
     }  /* end got the page */
   }  /* end the block is not multiply-allocated */

 return( ddo_rc );
}                                         /* end blkall_decrement_owners()  */


/****************************************************************************
 * NAME: blkall_increment_owners
 *
 * FUNCTION: Adjust the fsck workspace to show one more owner for the 
 *           indicated block.
 *
 * PARAMETERS:
 *      blk_num  - input - ordinal number of the filesystem block whose owner
 *                         count is to be adjusted.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t blkall_increment_owners ( fsblkidx_t blk_num )
{
 retcode_t dio_rc = FSCK_OK;
 dupall_blkrec_ptr this_blkrec;
 dupall_blkrec_tkn this_blktkn;
 mappgidx_t page_num;
 reg_idx_t word_offset;
 fsck_dword_bitmask_t bit_mask;
 blk_pageptr this_page;
 fsck_bitmap_ptr this_word;
 int is_a_dup = 0;

 dio_rc = blkmap_find_bit( blk_num, &page_num, &word_offset, &bit_mask );
 dio_rc = blkmap_get_page( page_num, &this_page );

 if( dio_rc == FSCK_OK ) { /* got the page */
   this_word = (fsck_bitmap_ptr) ((uint64) this_page + word_offset);

   if( ((*this_word) & bit_mask) != bit_mask ) { /*
                                 * not allocated yet
                                 */
     (*this_word) |= bit_mask; /* mark it allocated */

     dio_rc = blkmap_put_page( page_num );  /* write it to workspace */
     if( dio_rc != FSCK_OK )  {
       dio_rc = FSCK_FAILED_WRITE_FBLKMP;
       }  /* end failure here is fatal but return code is in the ... */
     }  /* end not allocated yet */

   else {  /* already allocated */
     is_a_dup = 1;
     dio_rc = dupall_find_blkrec( blk_num, &this_blkrec, &this_blktkn );

     if( this_blkrec != NULL ) {  /* block is already multiply-allocated */
       this_blkrec->owner_count++;  /* increment owners */
       }  /* end block is already multiply-allocated */

     else {  /* else this is the 2nd owner */
       dio_rc = dupall_insert_blkrec( blk_num );
       agg_recptr->dup_block_count++;
       agg_recptr->unresolved_1stref_count++;
       }  /* end else this is the 2nd owner */
     }  /* end else already allocated */
   }  /* end got the page */

 if( dio_rc == FSCK_OK ) {
   dio_rc = is_a_dup;
   }

 return( dio_rc );
}                                  /* end blkall_increment_owners()  */


/****************************************************************************
 * NAME: blkall_ref_check
 *
 * FUNCTION: Determine whether the given block is multiply-allocated and, if
 *           so, whether the first reference to it is unresolved.  (In this
 *           case, the current reference must be the first reference.)
 *
 * PARAMETERS:
 *      blk_num       - input - ordinal number of the filesystem block to be
 *                              checked
 *      ref_resolved  - input - pointer to a variable in which the results of the
 *                              query are returned.  If the current reference is
 *                              the first reference to a multiply-allocated block,
 *                              -1 is returned.  Otherwise, 0 is returned.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t blkall_ref_check ( fsblkidx_t  blk_num,
                             int        *ref_resolved )
{
 retcode_t brc_rc = FSCK_OK;
 dupall_blkrec_ptr this_blkrec;
 dupall_blkrec_tkn this_blktkn;

 /*
  * if the given block number is a multiply-allocated block with
  * an unresolved first reference, this must be that first reference
  */
 brc_rc = dupall_find_blkrec( blk_num, &this_blkrec, &this_blktkn );

 if( (brc_rc == FSCK_OK) && (this_blkrec != NULL) ) {  /*
                      * block is multiply-allocated
                      */
   if( this_blkrec->first_ref_resolved == 0 ) { /*
                      * haven't seen the first ref yet
                      */
     this_blkrec->first_ref_resolved = 1;
     agg_recptr->unresolved_1stref_count--;
     *ref_resolved = -1;
     }
   else {  /* we have already resolved the first ref for this block */
     *ref_resolved = 0;
     }  /* end else we have already resolved the first ref ... */
   }
 else {  /* block is not multiply-allocated */
   *ref_resolved = 0;
   }  /* end else block is not multiply-allocated */

 return( brc_rc );
}                                                /* end blkall_ref_check ()  */


/****************************************************************************
 * NAME: dire_buffer_alloc
 *
 * FUNCTION:  Allocate an I/O buffer for use during directory entry insertion
 *            and removal processing.
 *
 * PARAMETERS:
 *      addr_dnode_ptr  - input - pointer to a variable in which to return 
 *                                the address of the allocated buffer (or
 *                                NULL if no buffer could be allocated)
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dire_buffer_alloc ( dtpage_t **addr_dnode_ptr )
{
  retcode_t rba_rc = FSCK_OK;
  reg_idx_t bufrec_length, bytes_available;
  recon_bufrec_ptr bufrec_ptr;

  if( agg_recptr->recon_buf_stack != NULL ) {  /* stack not empty */
    bufrec_ptr = agg_recptr->recon_buf_stack;
    agg_recptr->recon_buf_stack = bufrec_ptr->stack_next;

    bufrec_ptr->stack_next = NULL;
    bufrec_ptr->dnode_blkoff = 0;
    bufrec_ptr->dnode_byteoff = 0;
    *addr_dnode_ptr = &(bufrec_ptr->dnode_buf);
    }  /* end stack not empty */

  else {  /* the stack is empty */

    bufrec_length = sizeof( struct recon_buf_record );
    bytes_available = agg_recptr->recon_buf_extent->extent_length -
                      agg_recptr->recon_buf_extent->last_byte_used;

    if( bytes_available < bufrec_length ) {  /* we've used up a whole
                                * extent of dynamic storage -- something
                                * strange is going on
                                */
      *addr_dnode_ptr = NULL;
      rba_rc = FSCK_INSUFDSTG4RECON;
      }  /* end we've used up a whole extent of dynamic storage... */

    else {  /* there is enough dynamic storage for another one */

      bufrec_ptr = (recon_bufrec_ptr)
                         (((int32)agg_recptr->recon_buf_extent->extent_addr) +
                         agg_recptr->recon_buf_extent->last_byte_used + 1);
      agg_recptr->recon_buf_extent->last_byte_used += bufrec_length;

         /*
          * now initialize the record 
          */
      wsp_dynstg_object = dynstg_recondnodebuf;
      wsp_dynstg_action = dynstg_initialization;
      memset( (void *) bufrec_ptr, 0, bufrec_length );
      *addr_dnode_ptr = &(bufrec_ptr->dnode_buf);
      }  /* end else there is enough dynamic storage for another one */
    }  /* end else the stack is empty */

  return( rba_rc );
}                                /* end dire_buffer_alloc */


/****************************************************************************
 * NAME: dire_buffer_release
 *
 * FUNCTION:  Deallocate (make available for reuse) an I/O buffer for used
 *            during directory entry insertion and removal processing.
 *
 * PARAMETERS:
 *      dnode_ptr  - input - the address of the buffer to release
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dire_buffer_release ( dtpage_t *dnode_ptr )
{
  retcode_t rbr_rc = FSCK_OK;
  recon_bufrec_ptr bufrec_ptr;

  bufrec_ptr = (recon_bufrec_ptr) dnode_ptr;
  bufrec_ptr->stack_next = agg_recptr->recon_buf_stack;
  agg_recptr->recon_buf_stack = bufrec_ptr;

  return( rbr_rc );
}                                /* end dire_buffer_release */


/****************************************************************************
 * NAME: directory_buffers_alloc
 *
 * FUNCTION:  Allocate storage for use as I/O buffers while inserting and
 *            removing directory entries during file system repair processing.
 * FUNCTION: 	Make use of the VeryLarge Multi-Use Buffer for 
 *		I/O buffers while inserting and removing directory entries
 *		during file system repair processing.
 *
 * PARAMETERS:  none
 *
 * NOTES:  	The directory buffers are the only use of the VeryLarge Buffer
 *		during Phase 6 processing.
 *
 * PARAMETERS:  none
 *
 * NOTES:  The address of the storage allocated for this purpose is stored 
 *         in the aggregate record, field: recon_buf_extent
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t directory_buffers_alloc( )
{
  retcode_t dba_rc = FSCK_OK;

  agg_recptr->vlarge_current_use = USED_FOR_DIRPAGE_BUFS;
  agg_recptr->recon_buf_extent = 
                                              (wsp_ext_rec_ptr) agg_recptr->vlarge_buf_ptr;

  agg_recptr->recon_buf_extent->next = NULL;
  agg_recptr->recon_buf_extent->extent_length = 
                                              agg_recptr->vlarge_buf_length;
  agg_recptr->recon_buf_extent->extent_addr =
                                              (char *) agg_recptr->recon_buf_extent;
  agg_recptr->recon_buf_extent->last_byte_used =
                                              sizeof(fsck_extent_record) - 1;

 return( dba_rc );
}                                /* end directory_buffers_alloc */


/****************************************************************************
 * NAME: directory_buffers_release
 *
 * FUNCTION:  Free storage which was allocated for use as I/O buffers while 
 *            inserting and removing directory entries during file system 
 *            repair processing.
 *
 * PARAMETERS:  none
 *
 * NOTES:  The address of the storage allocated for this purpose is stored 
 *         in the aggregate record, field: recon_buf_extent
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t directory_buffers_release ( )
{
  retcode_t dbr_rc = FSCK_OK;

  if( agg_recptr->recon_buf_extent != NULL ) {  /* something is allocated */

    agg_recptr->recon_buf_extent = NULL;
    agg_recptr->vlarge_current_use = NOT_CURRENTLY_USED;
    }  /* end something is allocated */

 return( dbr_rc );
}                                /* end directory_buffers_release */


/****************************************************************************
 * NAME: dtreeQ_dequeue
 *
 * FUNCTION:  If the directory tree queue is not empty, remove the front
 *            element and return a pointer to it.  Otherwise, return NULL.
 *
 * PARAMETERS:
 *      dtreeQ_elptr - input - pointer to a variable in which the address of
 *                             the front queue element should be returned
 *
 * NOTES:  The directory tree queue is described in the aggregate record,
 *         fields: dtreeQ_front, dtreeQ_back
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dtreeQ_dequeue ( dtreeQ_elem_ptr *dtreeQ_elptr )
{
 retcode_t dQd_rc = FSCK_OK;

 *dtreeQ_elptr = agg_recptr->dtreeQ_front;

 if( agg_recptr->dtreeQ_back == agg_recptr->dtreeQ_front )  {  /* empty */
   agg_recptr->dtreeQ_back = agg_recptr->dtreeQ_front = NULL;
   }  /* end empty */
 else {  /* not empty */
   agg_recptr->dtreeQ_front = agg_recptr->dtreeQ_front->next;
   agg_recptr->dtreeQ_front->prev = NULL;
   }  /* end else not empty */

 return( dQd_rc );
}                                     /* end dtreeQ_dequeue      */


/****************************************************************************
 * NAME: dtreeQ_enqueue
 *
 * FUNCTION:  Adds the given element to the back of the directory tree queue.
 *
 * PARAMETERS:
 *      dtreeQ_elptr - input - address of the element to add to the queue.
 *
 * NOTES:  The directory tree queue is described in the aggregate record,
 *         fields: dtreeQ_front, dtreeQ_back
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dtreeQ_enqueue ( dtreeQ_elem_ptr dtreeQ_elptr )
{
 retcode_t dQe_rc = FSCK_OK;

 if( agg_recptr->dtreeQ_back == NULL )  {  /* empty queue */
   agg_recptr->dtreeQ_back = agg_recptr->dtreeQ_front = dtreeQ_elptr;
   dtreeQ_elptr->prev = dtreeQ_elptr->next = NULL;
   }  /* end empty queue */
 else {  /* queue not empty */
   dtreeQ_elptr->next = NULL;
   dtreeQ_elptr->prev = agg_recptr->dtreeQ_back;
   agg_recptr->dtreeQ_back->next = dtreeQ_elptr;
   agg_recptr->dtreeQ_back = dtreeQ_elptr;
   }  /* end else queue not empty */

 return( dQe_rc );
}                                        /* end dtreeQ_enqueue  */


/****************************************************************************
 * NAME: dtreeQ_get_elem
 *
 * FUNCTION: Allocates workspace storage for an fsck directory tree queue element
 *
 * PARAMETERS:
 *      addr_dtreeQ_ptr  - input - pointer to a variable in which the address
 *                                 of the new element should be returned.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dtreeQ_get_elem ( dtreeQ_elem_ptr *addr_dtreeQ_ptr )
{
 retcode_t dge_rc = FSCK_OK;
  int I_am_logredo = 0;

 if( agg_recptr->free_dtreeQ != NULL ) { /* free list isn't empty */

   *addr_dtreeQ_ptr = agg_recptr->free_dtreeQ;
   agg_recptr->free_dtreeQ = agg_recptr->free_dtreeQ->next;
   memset( (void *) (*addr_dtreeQ_ptr), 0, dtreeQ_elem_length );
   }  /* end freelist isn't empty */

 else {  /* else the free list is empty */

   dge_rc = alloc_wrksp( dtreeQ_elem_length, dynstg_dtreeQ_elem,
                                  I_am_logredo, (void**) addr_dtreeQ_ptr );
   }  /* end else the free list is empty */

 return( dge_rc );
}                                        /* end dtreeQ_get_elem     */


/****************************************************************************
 * NAME: dtreeQ_rel_elem
 *
 * FUNCTION: Makes an fsck directory tree queue element available for reuse
 *
 * PARAMETERS:
 *      dtreeQ_elptr  - input - the address of the element to release
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dtreeQ_rel_elem ( dtreeQ_elem_ptr dtreeQ_elptr )
{
 retcode_t dQre_rc = FSCK_OK;

 dtreeQ_elptr->next = agg_recptr->free_dtreeQ;
 agg_recptr->free_dtreeQ = dtreeQ_elptr;

 return( dQre_rc );
}                                               /* end dtreeQ_rel_elem     */


/****************************************************************************
 * NAME: dupall_extract_blkrec
 *
 * FUNCTION: Remove the given (previously found) record from the list of 
 *           duplicate allocation block records.
 *
 * PARAMETERS:
 *      block_recptr  - input - the address of the record to remove
 *      block_rectkn  - input - the token which was returned with the address
 *                              from the find operation.
 *
 * NOTES:  The duplicate allocation list is described in the aggregate record,
 *         field: dup_alloc_lst
 *
 *         The token actually contains the address of the list record which
 *         points to the record to remove (or NULL if the record is at the
 *         beginning of the list).  This is needed to avoid searching the
 *         list again since the list is only singly-linked.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dupall_extract_blkrec ( dupall_blkrec_ptr block_recptr,
                                  dupall_blkrec_tkn block_rectkn
                                )
{
 retcode_t deb_rc = FSCK_OK;

 /*
  * remove it from the list of multiply-allocated block
  */

 if( block_rectkn == NULL )  { /* first in list */
   agg_recptr->dup_alloc_lst = block_recptr->next;
   }  /* end first in list */

 else {  /* not at front of list */
   block_rectkn->next = block_recptr->next;
  }  /* end else not at front of list */

 /*
  * release it for reuse
  */
 block_recptr->next = agg_recptr->free_dupall_blkrec;
 agg_recptr->free_dupall_blkrec = block_recptr;

 return( deb_rc );
}                                         /* end dupall_extract_blkrec()  */


/****************************************************************************
 * NAME: dupall_find_blkrec
 *
 * FUNCTION:  Search for a record with the given block number in the duplicate
 *            allocation list.
 *
 * PARAMETERS:
 *      block_number  - input - ordinal number of the filesystem block to match
 *      block_recptr  - input - pointer to a variable in which to return the
 *                              address of the record found.  If no record is
 *                              found, NULL is returned.
 *      block_rectkn  - input - pointer to a variable in which to return a token
 *                              if a record is found.  If no record is found, 
 *                              NULL is returned.
 *
 * NOTES:  The duplicate allocation list is described in the aggregate record,
 *         field: dup_alloc_lst
 *
 *         The token actually contains the address of the list record which
 *         points to the record found (or NULL if the record is at the
 *         beginning of the list).  This is a required input to certain other
 *         routines which operate on the duplicate allocation list.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dupall_find_blkrec ( fsblkidx_t         block_number,
                               dupall_blkrec_ptr *block_recptr,
                               dupall_blkrec_tkn *block_rectkn
                             )
{
 retcode_t dfb_rc = FSCK_OK;
 dupall_blkrec_ptr prev_blkrec, this_blkrec;
 int dfb_done = 0;

 prev_blkrec = NULL;
 this_blkrec = agg_recptr->dup_alloc_lst;

 if( this_blkrec == NULL ) {  /* empty list */
   dfb_done = 1;
   }  /* empty list */

 else {  /* list of multiply-allocated blocks is not empty */

   while( !dfb_done ) {

     if( this_blkrec->blk_number == block_number ) {  /* found match */
       dfb_done = 1;
       }  /* end found match */
     else if( this_blkrec->blk_number > block_number ) {  /*
                            * won't be a match
                            */
       dfb_done = 1;
       prev_blkrec = NULL;
       this_blkrec = NULL;
       }  /* end else won't be a match */
     else if( this_blkrec->next == NULL ) {  /* end of list */
       dfb_done = 1;
       prev_blkrec = NULL;
       this_blkrec = NULL;
       }  /* end else end of list */
     else {  /* try the next one */
       prev_blkrec = this_blkrec;
       this_blkrec = this_blkrec->next;
       }  /* end else try the next one */
     }  /* end while */
   }  /* end else list of multiply-allocated blocks is not empty */

 *block_recptr = this_blkrec;
 *block_rectkn = prev_blkrec;

 return( dfb_rc );
}                                             /* end dupall_find_blkrec()  */


/****************************************************************************
 * NAME: dupall_get_blkrec
 *
 * FUNCTION: Allocates workspace storage for a duplicate allocation
 *           block record.
 *
 * PARAMETERS:
 *      addr_blkrec_ptr  - input - pointer to a variable in which the address
 *                                 of the new record will be returned.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dupall_get_blkrec ( dupall_blkrec_ptr *addr_blkrec_ptr )
{
 retcode_t dgb_rc = FSCK_OK;
  int I_am_logredo = 0;

 if( agg_recptr->free_dupall_blkrec != NULL ) { /* free list isn't empty */

   *addr_blkrec_ptr = agg_recptr->free_dupall_blkrec;
   agg_recptr->free_dupall_blkrec = agg_recptr->free_dupall_blkrec->next;
   }  /* end freelist isn't empty */

 else {  /* else the free list is empty */

   dgb_rc = alloc_wrksp( dupall_blkrec_length, dynstg_dupall_blkrec,
                                   I_am_logredo, (void **) addr_blkrec_ptr );
   }  /* end else the free list is empty */

 return( dgb_rc );
}                                              /* end dupall_get_blkrec     */


/*****************************************************************************
 * NAME: dupall_insert_blkrec
 *
 * FUNCTION: Allocate a duplicate allocation record for the given block and 
 *           insert it into the sorted, singly-linked list of duplicate 
 *           allocation records.
 *
 * PARAMETERS:
 *      block_num  - input - the block number for which the record is to be 
 *                           allocated.
 *
 * NOTES:  The duplicate allocation list is described in the aggregate record,
 *         field: dup_alloc_lst
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dupall_insert_blkrec ( fsblkidx_t block_num )
{
 retcode_t dib_rc = FSCK_OK;
 dupall_blkrec_ptr new_blkrec, prev_blkrec, this_blkrec;
 int dib_done = 0;

 dib_rc = dupall_get_blkrec( &new_blkrec );

 if( dib_rc == FSCK_OK ) {  /* got a block record */

   new_blkrec->blk_number = block_num;
   new_blkrec->owner_count = 2;

   if( agg_recptr->dup_alloc_lst == NULL ) { /* list now empty */
     new_blkrec->next = NULL;
     agg_recptr->dup_alloc_lst = new_blkrec;
     }  /* end list now empty */

   else {  /* list not empty */

     if( agg_recptr->dup_alloc_lst->blk_number > block_num ) {  /*
                              * goes at front
                              */
       new_blkrec->next = agg_recptr->dup_alloc_lst;
       agg_recptr->dup_alloc_lst = new_blkrec;
       }  /* end goes at front */

     else { /* doesn't go at the front */

       prev_blkrec = agg_recptr->dup_alloc_lst;
       this_blkrec = agg_recptr->dup_alloc_lst->next;
       while( !dib_done ) {
         if( this_blkrec == NULL ) {  /* goes at the end */
           new_blkrec->next = NULL;
           prev_blkrec->next = new_blkrec;
           dib_done = 1;
           }  /* end goes at the end */
         else if( this_blkrec->blk_number > block_num ) { /*
                             * goes in front of this one
                             */
           new_blkrec->next = this_blkrec;
           prev_blkrec->next = new_blkrec;
           dib_done = 1;
           }  /* end goes in front of this one */
         else {  /* try the next one */
           prev_blkrec = this_blkrec;
           this_blkrec = this_blkrec->next;
           }  /* end try the next one */
         }  /* end while */
       }  /* end else doesn't go at the front */
     }  /* end list not empty */
   }  /* end got a block record */

 return( dib_rc );
}                                            /* end dupall_insert_blkrec()  */


/****************************************************************************
 * NAME: establish_agg_workspace
 *
 * FUNCTION: Obtain storage for and initialize the fsck aggregate workspace.
 *
 * PARAMETERS:  none
 *
 * NOTES: The various parts of the workspace are described in the aggregate
 *        record.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t establish_agg_workspace ( )
{
 retcode_t eaw_rc = FSCK_OK;
 reg_idx_t mapsize_bytes;
  int I_am_logredo = 0;

  /*
   * establish the fsck workspace block map
   */
 eaw_rc = establish_wsp_block_map( );

 if( eaw_rc == FSCK_OK ) {  /* block map has been established */

   agg_recptr->agg_imap.num_iags = 1;
   agg_recptr->agg_imap.bkd_inodes = INOSPEREXT;
   agg_recptr->agg_imap.unused_bkd_inodes = INOSPEREXT - 4;
   agg_recptr->inode_count = INOSPEREXT;  /* in release 1 there is always
                                * exactly one extent of inodes allocated
                                * for the aggregate
                                */
      /*
       * now establish the fsck aggregate imap workspace
       */
   mapsize_bytes = agg_recptr->agg_imap.num_iags *
                   sizeof(struct fsck_iag_record);
   eaw_rc = alloc_wrksp( mapsize_bytes, dynstg_agg_iagtbl,
                                   I_am_logredo, 
                                  (void **) &(agg_recptr->agg_imap.iag_tbl) );
   if( eaw_rc == FSCK_OK ) { /* AIM workspace established              @F1 */
          /*
           * now establish the fsck aggregate inode table workspace
           * 
           * (since there is always exactly one inode extent, we don't
           * bother with an IAG table of pointers to extent address tables
           * or with an extent address table of pointers to inode record
           * address tables.)
           */
     eaw_rc = alloc_wrksp( inode_tbl_length, dynstg_ait_inotbl,
                                   I_am_logredo, 
                                  (void **) &(agg_recptr->AIT_ext0_tbl) );   /* @F1 */
     memcpy( (void *) &(agg_recptr->AIT_ext0_tbl->eyecatcher),  
                   (void *) "InodeTbl", 8 );                                          /* @F1 */
     }  /* end AIM workspace established */                                    /* @F1 */
   }  /* end block map has been established */

 return( eaw_rc );
}                                       /* end establish_agg_workspace      */


/****************************************************************************
 * NAME: establish_ea_iobuf
 *
 * FUNCTION: 	Make use of the VeryLarge Multi-Use Buffer
 *		for reading and validating EA data.
 *
 * PARAMETERS:  none
 *
 * NOTES:  	The ea I/O buffer is the only user of the VeryLarge Buffer
 *		during Phase 1 processing.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t establish_ea_iobuf( )
{
  retcode_t eei_rc = FSCK_OK;

  agg_recptr->vlarge_current_use = USED_FOR_EA_BUF;
  agg_recptr->ea_buf_ptr = agg_recptr->vlarge_buf_ptr;

  wsp_dynstg_action = dynstg_initialization;
  memset( (void *) (agg_recptr->ea_buf_ptr), '\0', EA_IO_BUFSIZE );

  agg_recptr->ea_buf_length = agg_recptr->vlarge_buf_length;
  agg_recptr->ea_buf_data_len = 0;
  agg_recptr->ea_agg_offset = 0;

  return( eei_rc );
}                                             /* end establish_ea_iobuf    */


/****************************************************************************
 * NAME: establish_fs_workspace
 *
 * FUNCTION: Obtain storage for and initialize the fsck file sets workspace.
 *
 * PARAMETERS:  none
 *
 * NOTES: The various parts of the workspace are described in the aggregate
 *        record.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t establish_fs_workspace ( )
{
 retcode_t efsw_rc = FSCK_OK;
 reg_idx_t mapsize_bytes;
 reg_idx_t buffer_size;
 int aggregate_inode, which_ait;
 inoidx_t inoidx;
 dinode_t *inoptr;
 int I_am_logredo = 0;
 IAG_tblptr IAGtbl;                                                /* @F1 */
 inoext_tblptr  inoexttbl;                                       /* @F1 */
 ino_tblptr inotbl;                                                 /* @F1 */

      /*
       * allocate a buffer in which path names can be constructed
       */
 buffer_size = (JFS_PATH_MAX + 2) * sizeof( char );
 efsw_rc = alloc_wrksp( buffer_size, dynstg_fsit_map,
                                  I_am_logredo,
                                  (void **) &(agg_recptr->path_buffer) );

 if( efsw_rc == FSCK_OK ) {  /* got it */
   agg_recptr->path_buffer_length = buffer_size;

 /*
  * Figure out how many IAGs have been allocated for the fileset.
  * (Note that in release 1 there is always exactly 1 fileset in the
  * aggregate)
  *
  * At this point the aggregate inode describing the fileset has been
  * validated.  The data described by that inode is 1 page of control
  * information plus some number of IAGs.  di_size is the number of
  * bytes allocated for that data.
  */
   if( agg_recptr->primary_ait_4part2 ) {
     which_ait = fsck_primary;
     efsw_rc = ait_special_read_ext1( fsck_primary );
     if( efsw_rc != FSCK_OK ) {  /* read failed */ 
       report_readait_error( efsw_rc, 
                             FSCK_FAILED_CANTREADAITEXTC, 
                             fsck_primary );
       efsw_rc = FSCK_FAILED_CANTREADAITEXTC;
       }  /* end read failed */
     }
   else {
     which_ait = fsck_secondary;
     efsw_rc = ait_special_read_ext1( fsck_secondary );
     if( efsw_rc != FSCK_OK ) {  /* read failed */
       report_readait_error( efsw_rc, 
                             FSCK_FAILED_CANTREADAITEXTD, 
                             fsck_secondary );
       efsw_rc = FSCK_FAILED_CANTREADAITEXTD;
       }  /* end read failed */
     }
   }  /* end got it */

 if( efsw_rc == FSCK_OK ) { /* got the first AIT extent */
   aggregate_inode = -1;
   inoidx = FILESYSTEM_I;
   efsw_rc = inode_get( aggregate_inode, which_ait, inoidx, &inoptr );

   if( efsw_rc == FSCK_OK ) {  /* got the fileset IT inode */
     agg_recptr->fset_imap.num_iags = (inoptr->di_size/SIZE_OF_MAP_PAGE) - 1;

     agg_recptr->fset_inode_count = agg_recptr->fset_imap.num_iags * INOSPERIAG; /*
                                * a high estimate of the inodes
                                * allocated for the fileset
                                */

       /*
         * now establish the fsck fileset imap workspace
         */
     if(efsw_rc == FSCK_OK ) { /* inode map established */
       mapsize_bytes = agg_recptr->fset_imap.num_iags *
                                    sizeof(struct fsck_iag_record);
       efsw_rc = alloc_wrksp( mapsize_bytes, dynstg_agg_iagtbl,
                                        I_am_logredo,
                                       (void **) &(agg_recptr->fset_imap.iag_tbl) );
       if( efsw_rc == FSCK_OK ) {  /* inode map workspace allocated   @F1 */

           /*
            * now establish the fsck fileset imap workspace
            *
            * We start out knowing that IAG 0, extent 0 is allocated and
            * has an inode in use.  We'll allocate enough to cover that.
            */
         mapsize_bytes = 8 + 
                                agg_recptr->fset_imap.num_iags * sizeof(inoext_tblptr); /*
                                                                                                    @F1 */
         efsw_rc = alloc_wrksp( mapsize_bytes, dynstg_fsit_iagtbl,
                                   I_am_logredo, (void **) &IAGtbl );               /* @F1 */
         if( efsw_rc == FSCK_OK ) {  /* we got the IAG table                    @F1 */            
           memcpy( (void *) &(IAGtbl->eyecatcher), (void *) "FSAITIAG", 8 );  /*
                                                                                                    @F1 */
           agg_recptr->FSIT_IAG_tbl = IAGtbl;                                      /* @F1 */

           efsw_rc = alloc_wrksp( inode_ext_tbl_length, dynstg_fsit_inoexttbl, 
                                            I_am_logredo, (void **) &inoexttbl );  /* @F1 */
           if( efsw_rc == FSCK_OK ) {  /* we got the inode extent table     @F1 */ 
             memcpy( (void *) &(inoexttbl->eyecatcher), (void *) "FSAITEXT", 8 ); /*
                                                                                                    @F1 */
             IAGtbl->inoext_tbl[0] = inoexttbl;                                      /* @F1 */

             efsw_rc = alloc_wrksp( inode_tbl_length, dynstg_fsit_inotbl,
                                              I_am_logredo, (void **) &inotbl );    /* @F1 */
             if( efsw_rc == FSCK_OK ) {  /* we got the inode table             @F1 */
               memcpy( (void *) &(inotbl->eyecatcher), (void *) "FSAITINO", 8 ); /*
                                                                                                    @F1 */
               inoexttbl->inotbl[0] = inotbl;                                          /* @F1 */
               }  /* end we got the inode table                                        @F1 */
             }  /* end we got the inode extent table                                @F1 */
           }  /* end we got the IAG table                                               @F1 */
         }  /* end inode map workspace allocated                                  @F1 */
       }  /* end inode map established */
     }  /* end got the fileset IT inode */
   }  /* end got the first AIT extent */

 return( efsw_rc );
}                                        /* end establish_fs_workspace      */


/****************************************************************************
 * NAME: establish_io_buffers
 *
 * FUNCTION:  Allocate storage for dedicated I/O buffers.
 *
 * PARAMETERS:  none
 *
 * NOTES:  The I/O buffers are described in the aggregate record.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t establish_io_buffers ( )
{
  retcode_t eiob_rc = FSCK_OK;
  int I_am_logredo = 0;

  eiob_rc = alloc_wrksp( IAG_IO_BUFSIZE, dynstg_iobufs,
                                  I_am_logredo,
                                 (void **) &(agg_recptr->iag_buf_ptr) );
  if( eiob_rc == FSCK_OK ) {  /* successful IAG allocation */

    agg_recptr->iag_buf_length = sizeof(iag_t);
    agg_recptr->iag_buf_data_len = 0;
    agg_recptr->iag_agg_offset = 0;
    agg_recptr->iag_buf_write = 0;

    agg_recptr->bmapdm_buf_ptr = agg_recptr->iag_buf_ptr;
    agg_recptr->bmapdm_buf_length = IAG_IO_BUFSIZE;
    agg_recptr->bmapdm_buf_data_len = 0;
    agg_recptr->bmapdm_agg_offset = 0;
    agg_recptr->bmapdm_buf_write = 0;
    }  /* end successful IAG allocation */

  if( eiob_rc == FSCK_OK ) {  /* successful IAG allocation */

    eiob_rc = alloc_wrksp( INODE_IO_BUFSIZE, dynstg_iobufs,
                                    I_am_logredo,
                                   (void **) &(agg_recptr->ino_buf_ptr) );
    }  /* end successful ea buffer allocation */

  if( eiob_rc == FSCK_OK ) {  /* successful inode allocation */
    agg_recptr->ino_buf_length = INODE_IO_BUFSIZE;
    agg_recptr->ino_buf_data_len = 0;
    agg_recptr->ino_buf_agg_offset = 0;
    agg_recptr->ino_buf_write = 0;

    eiob_rc = alloc_wrksp( NODE_IO_BUFSIZE, dynstg_iobufs,
                                    I_am_logredo,
                                   (void **) &(agg_recptr->node_buf_ptr) );
    }  /* end successful inode allocation */

  if( eiob_rc == FSCK_OK ) {  /* successful node allocation */
    agg_recptr->node_buf_length = NODE_IO_BUFSIZE;
    agg_recptr->node_buf_data_len = 0;
    agg_recptr->node_agg_offset = 0;
    agg_recptr->node_buf_write = 0;

    eiob_rc = alloc_wrksp( MAPLEAF_IO_BUFSIZE, dynstg_iobufs,
                                    I_am_logredo,
                                    (void **) &(agg_recptr->mapleaf_buf_ptr) );
    }  /* end successful node allocation */

  if( eiob_rc == FSCK_OK ) {  /* successful mapleaf allocation */
    agg_recptr->mapleaf_buf_length = MAPLEAF_IO_BUFSIZE;
    agg_recptr->mapleaf_buf_data_len = 0;
    agg_recptr->mapleaf_agg_offset = 0;
    agg_recptr->mapleaf_buf_write = 0;

    eiob_rc = alloc_wrksp( MAPCTL_IO_BUFSIZE, dynstg_iobufs,
                                    I_am_logredo,
                                   (void **) &(agg_recptr->mapctl_buf_ptr) );
    }  /* end successful mapleaf allocation */

  if( eiob_rc == FSCK_OK ) {  /* successful map control allocation */
    agg_recptr->mapctl_buf_length = MAPCTL_IO_BUFSIZE;
    agg_recptr->mapctl_buf_data_len = 0;
    agg_recptr->mapctl_agg_offset = 0;
    agg_recptr->mapctl_buf_write = 0;
    eiob_rc = alloc_wrksp( BMAPLV_IO_BUFSIZE, dynstg_iobufs,
                                    I_am_logredo,
                                    (void **) &(agg_recptr->bmaplv_buf_ptr) );
    }  /* end successful map control allocation */
  if( eiob_rc == FSCK_OK ) {  /* successful map level allocation */
    agg_recptr->bmaplv_buf_length = BMAPLV_IO_BUFSIZE;
    agg_recptr->bmaplv_buf_data_len = 0;
    agg_recptr->bmaplv_agg_offset = 0;
    agg_recptr->bmaplv_buf_write = 0;
    }  /* successful map level allocation */

 return ( eiob_rc );
}                                /* end establish_io_buffers ( ) */


/****************************************************************************
 * NAME: establish_wsp_block_map
 *
 * FUNCTION: If the in-aggregate fsck workspace is available for use,
 *           initialize it as the fsck workspace block map.
 *
 *           Otherwise, obtain and initialize dynamic storage for the fsck
 *           workspace block map.
 *
 * PARAMETERS:  none
 *
 * NOTES:  The fsck workspace block map is described in the aggregate record.
 *
 *         If the in-aggregate fsck workspace is used, the aggregate record
 *         fields describe the dedicated I/O buffer used for the fsck workspace
 *         block map.  Otherwise, they describe the entire workspace block
 *         map in dynamic storage.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t establish_wsp_block_map ( )
{
 retcode_t ewbm_rc = FSCK_OK;
 int32 blkmap_size_bytes;
 int32 blkmap_size_in_pages;
 int32 idx;
 fsblkidx_t this_device_offset;
  int I_am_logredo = 0;
	
 ewbm_rc = establish_wsp_block_map_ctl();
	 
 if( ewbm_rc == FSCK_OK ) {  /* allocated and initialized blk map ctl page */
   blkmap_size_bytes = agg_recptr->ondev_wsp_byte_length;
   agg_recptr->blkmp_pagecount = blkmap_size_bytes / BYTESPERPAGE;
   agg_recptr->blkmp_agg_offset = agg_recptr->ondev_wsp_byte_offset +
                                  BYTESPERPAGE; /*
                                    * whether or not we actually write
                                    * to the on-disk fsck workspace,
                                    * this buffer represents it logically.
                                    */
   agg_recptr->blkmp_blkmp_offset = 0;
   agg_recptr->blkmp_buf_data_len = 0;
   agg_recptr->blkmp_buf_write = 0;
   agg_recptr->blkmp_blkmp_offset = 0;
   agg_recptr->blkmp_buf_data_len = 0;
   agg_recptr->blkmp_buf_write = 0;

   if( agg_recptr->processing_readonly ) {  /* can't touch the aggregate */
	 
     ewbm_rc = alloc_wrksp( blkmap_size_bytes, dynstg_blkmap,
                                        I_am_logredo,
                                       (void **) &(agg_recptr->blkmp_buf_ptr) );
     if( ewbm_rc == FSCK_OK ) {  /* allocated and initialized block map */
       wsp_dynstg_object = 0;
       wsp_dynstg_action = 0;
       agg_recptr->blkmp_buf_length = blkmap_size_bytes;
       agg_recptr->blkmp_buf_data_len = agg_recptr->blkmp_buf_length;
       }  /* end allocated and initialized block map */
     }  /* end can't touch the aggregate */
   else  {  /* use storage reserved for fsck in the aggregate */
     ewbm_rc = alloc_wrksp( BLKMP_IO_BUFSIZE, dynstg_blkmap_buf,
                                        I_am_logredo,
                                        (void **) &(agg_recptr->blkmp_buf_ptr) );
     if( ewbm_rc == FSCK_OK ) {  /* allocated and initialized block map */
       agg_recptr->blkmp_buf_length = BLKMP_IO_BUFSIZE;
       agg_recptr->blkmp_buf_data_len = agg_recptr->blkmp_buf_length;
       ewbm_rc = blkmap_put_ctl_page( agg_recptr->blkmp_ctlptr );
       if( ewbm_rc == FSCK_OK ) {  /* successful write to Block Map Control Page */
         blkmap_size_in_pages = blkmap_size_bytes / BYTESPERPAGE;  /* this
                                    * is guaranteed (by mkfs) to be an even
                                    * number
                                    */
         for ( idx=1;
             ((idx < blkmap_size_in_pages) && (ewbm_rc == FSCK_OK)  );
             idx++ ) {  /* for each map page (after the control page) */
	
           this_device_offset = agg_recptr->ondev_wsp_byte_offset +
                                (idx*BYTESPERPAGE);
           ewbm_rc = ujfs_rw_diskblocks( Dev_IOPort,
                                         this_device_offset,
                                         BYTESPERPAGE,
                                         (void *) agg_recptr->blkmp_buf_ptr,
                                         PUT ); /*
                                    * write the initialized buffer page to
                                    * the map page on disk
                                    */
           if( ewbm_rc != FSCK_OK ) {  /* I/O failure */
                /*
                 * message to user 
                 */
  DBG_ERROR(("CHK:establish_wsp_block_map \n"));
             msgprms[0] = message_parm_0;
             msgprmidx[0] = fsck_metadata;
             msgprms[1] = Vol_Label;
             msgprmidx[1] = 0;
             sprintf( message_parm_2, "%d", 1 );
             msgprms[2] = message_parm_2;
             msgprmidx[2] = 0;
             fsck_send_msg( fsck_URCVWRT, 0, 3 );
                /*
                 * message to debugger 
                 */
             sprintf( message_parm_0, "%d", ewbm_rc );
             msgprms[0] = message_parm_0;
             msgprmidx[0] = 0;
             msgprms[1] = msgprms[0]; 
             msgprmidx[1] = msgprmidx[0];
             sprintf( message_parm_2, "%d", fsck_WRITE );
             msgprms[2] = message_parm_2;
             msgprmidx[2] = 0;
             sprintf( message_parm_3, "%lld", this_device_offset );
             msgprms[3] = message_parm_3;
             msgprmidx[3] = 0;
             sprintf( message_parm_4, "%ld", BYTESPERPAGE );
             msgprms[4] = message_parm_4;
             msgprmidx[4] = 0;
             sprintf( message_parm_5, "%ld", -1 );
             msgprms[5] = message_parm_5;
             msgprmidx[5] = 0;
             fsck_send_msg( fsck_ERRONWSP, 0, 6 );
             }  /* end I/O failure */
           }  /* end for each map page */
         }  /* end successful write to Block Map Control Page */
       }  /* end allocated and initialized block map */
     }  /* end else use storage reserved for fsck in the aggregate */
   }  /* end allocated and initialized blk map ctl page */

 return ( ewbm_rc );
}                                 /* end establish_wsp_block_map ( )      */
	 
	 
/****************************************************************************
 * NAME: establish_wsp_block_map_ctl
 *
 * FUNCTION: If the in-aggregate fsck workspace is available for use,
 *           initialize the first page as the fsck workspace block map control
 *           page.
 *
 *           Otherwise, obtain and initialize dynamic storage for the fsck
 *           workspace block map control page.
 *
 * PARAMETERS:  none
 *
 * NOTES:  The fsck workspace block map is described in the aggregate record.
 *
 *         If the in-aggregate fsck workspace is used, the aggregate record
 *         fields describe the dedicated I/O buffer used for the fsck workspace
 *         block map.  Otherwise, they describe the entire workspace block
 *         map in dynamic storage.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t establish_wsp_block_map_ctl ( )
{
 retcode_t ewbmc_rc = FSCK_OK;
 int32 blkmap_size_bytes;
 int32 blkmap_size_in_pages;
 int32 idx;
 fsblkidx_t this_device_offset;
  int I_am_logredo = 0;
	
 ewbmc_rc = alloc_wrksp( sizeof(fsck_blk_map_hdr_t), dynstg_blkmap_hdr,
                                      I_am_logredo,
                                     (void **) &(agg_recptr->blkmp_ctlptr) );
	 
 if( ewbmc_rc == FSCK_OK ) {  /* allocated and initialized blk map ctl page */
   strncpy( agg_recptr->blkmp_ctlptr->hdr.eyecatcher, fbmh_eyecatcher_string,
            strlen(fbmh_eyecatcher_string));  /* fill eyecatcher */
   agg_recptr->blkmp_ctlptr->hdr.super_buff_addr = (char *) sb_ptr;       
   agg_recptr->blkmp_ctlptr->hdr.agg_record_addr = (char *) agg_recptr;     
   agg_recptr->blkmp_ctlptr->hdr.bmap_record_addr = (char *) bmap_recptr;   
   agg_recptr->blkmp_ctlptr->hdr.fscklog_full = agg_recptr->fscklog_full;
   agg_recptr->blkmp_ctlptr->hdr.fscklog_buf_allocated = 
                                           agg_recptr->fscklog_buf_allocated; 
   agg_recptr->blkmp_ctlptr->hdr.fscklog_buf_alloc_err = 
                                           agg_recptr->fscklog_buf_alloc_err; 
   agg_recptr->blkmp_ctlptr->hdr.fscklog_agg_offset = 
                                           agg_recptr->ondev_fscklog_byte_offset;
   DosGetDateTime( &(fsck_DateTime) );
   if( fsck_DateTime.year > 2000 ) {
     sprintf( &(agg_recptr->blkmp_ctlptr->hdr.start_time[0]), "%d/%d/%d%d.%d.%d", 
              fsck_DateTime.month, fsck_DateTime.day, (fsck_DateTime.year%2000),
              fsck_DateTime.hours, fsck_DateTime.minutes, fsck_DateTime.seconds );
     }
   else {
     sprintf( &(agg_recptr->blkmp_ctlptr->hdr.start_time[0]), "%d/%d/%d%d.%d.%d", 
              fsck_DateTime.month, fsck_DateTime.day, (fsck_DateTime.year%1900),
              fsck_DateTime.hours, fsck_DateTime.minutes, fsck_DateTime.seconds );
     }
	 
   if( !(agg_recptr->processing_readonly) ) {  /* 
   				* use storage reserved for fsck in the 
				* aggregate 
				*/
     ewbmc_rc = blkmap_put_ctl_page( agg_recptr->blkmp_ctlptr );
     if( ewbmc_rc != FSCK_OK ) {  /* I/O failure */
                /*
                 * message to user 
                 */
  DBG_ERROR(("CHK:establish_wsp_block_map_ctl\n"));
       msgprms[0] = message_parm_0;
       msgprmidx[0] = fsck_metadata;
       msgprms[1] = Vol_Label;
       msgprmidx[1] = 0;
       sprintf( message_parm_2, "%d", 1 );
       msgprms[2] = message_parm_2;
       msgprmidx[2] = 0;
       fsck_send_msg( fsck_URCVWRT, 0, 3 );
                /*
                 * message to debugger 
                 */
       sprintf( message_parm_0, "%d", ewbmc_rc );
       msgprms[0] = message_parm_0;
       msgprmidx[0] = 0;
       msgprms[1] = msgprms[0]; 
       msgprmidx[1] = msgprmidx[0];
       sprintf( message_parm_2, "%d", fsck_WRITE );
       msgprms[2] = message_parm_2;
       msgprmidx[2] = 0;
       sprintf( message_parm_3, "%lld", this_device_offset );
       msgprms[3] = message_parm_3;
       msgprmidx[3] = 0;
       sprintf( message_parm_4, "%ld", BYTESPERPAGE );
       msgprms[4] = message_parm_4;
       msgprmidx[4] = 0;
       sprintf( message_parm_5, "%ld", -1 );
       msgprms[5] = message_parm_5;
       msgprmidx[5] = 0;
       fsck_send_msg( fsck_ERRONWSP, 0, 6 );
       }  /* end I/O failure */
     }  /* end else use storage reserved for fsck in the aggregate */
   }  /* end allocated and initialized blk map ctl page */
	 
 return ( ewbmc_rc );
}                                 /* end establish_wsp_block_map_ctl ( )      */
	 
	 
/*****************************************************************************
 * NAME: extent_1stref_chk
 *
 * FUNCTION:  Determine whether the given extent contains the first reference
 *            to a multiply-allocated block.  If it does, perform duplicate
 *            allocation processing on the owning inode.
 *
 * PARAMETERS:
 *      first_block   - input - ordinal number of the first block in the extent
 *                              to check
 *      last_block    - input - ordinal number of the last block in the extent
 *                              to check
 *      is_EA         - input - !0 => the extent contains an inode's EA
 *                               0 => the extent contains something else
 *      msg_info_ptr  - input - pointer to a data area containing information
 *                              needed to issue messages for this extent
 *      ino_recptr    - input - pointer to the fsck inode record describing the
 *                              inode to which this extent is allocated
 *
 * NOTES:  As fsck scans the inodes sequentially, recording the blocks allocated,
 *         it doesn't know a particular block is multiply-allocated until the
 *         second reference is detected.  At that time the first reference to the
 *         block is unresolved since no list of owners is built (only a count of 
 *         owners, in which a 1 in the bit map represents a count of 1).
 *
 *         After all inodes have been scanned and their block allocations recorded, 
 *         if any multiply-allocated blocks have been detected, the inodes are
 *         scanned sequentially again until all first references to 
 *         multiply-allocated blocks are resolved.  This routine is invoked during
 *         that rescan.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t extent_1stref_chk ( fsblkidx_t        first_block,
                                          fsblkidx_t        last_block,
                                          int8                 is_EA,
                                          int8                 is_ACL,
                                          fsck_msg_info_ptr msg_info_ptr,
                                          fsck_inode_recptr ino_recptr
                                          )
{
  retcode_t eq_rc = FSCK_OK;
  fsblkidx_t  blkidx;
  fsblkidx_t first_in_dup_range;
  reg_idx_t size_of_dup_range = 0;
  dupall_blkrec_ptr this_blkptr;
  int is_dup = 0;
  int dups_detected = 0;

  for( blkidx = first_block;
       ((blkidx <= last_block) &&
        (eq_rc == FSCK_OK) &&
        (agg_recptr->unresolved_1stref_count > 0));
       blkidx++ ) {
    eq_rc = blkall_ref_check( blkidx, &is_dup );  /* see if this contains the first
                                      * reference to some multiply-allocated
                                      * block
                                      */
   if( eq_rc == FSCK_OK ) {
     if( is_dup ) {  /* a block that's already allocated */
       if( size_of_dup_range == 0 ) {  /* this is a new range */
         dups_detected = 1;
         first_in_dup_range = blkidx;
         size_of_dup_range = 1;
         }  /* end this is a new range */
       else {  /* else not a new range */
         size_of_dup_range++;
         }  /* end else not a new range */
       }  /* end it's a block that's already allocated */
     else {  /* it's not already allocated */
       if( size_of_dup_range > 0 )  {  /* just finished a range */
         sprintf( message_parm_0, "%ld", size_of_dup_range );
         msgprms[0] = message_parm_0;
         msgprmidx[0] = 0;
         sprintf( message_parm_1, "%lld", first_in_dup_range );
         msgprms[1] = message_parm_1;
         msgprmidx[1] = 0;
         msgprms[2] = message_parm_2;
         msgprmidx[2] = msg_info_ptr->msg_inotyp;
         msgprms[3] = message_parm_3;
         msgprmidx[3] = msg_info_ptr->msg_inopfx;
         sprintf( message_parm_4, "%ld", msg_info_ptr->msg_inonum );
         msgprms[4] = message_parm_4;
         msgprmidx[4] = 0;
         fsck_send_msg( fsck_DUPBLKREF, 0, 5 );
         size_of_dup_range = 0;
         }  /* end just finished a range */
       }  /* end else it's not already allocated */
     }
   }  /* end for */
 if( (eq_rc == FSCK_OK) && (size_of_dup_range > 0) )  {  /*
                            * last block(s) finished a range
                            */
   sprintf( message_parm_0, "%ld", size_of_dup_range );
   msgprms[0] = message_parm_0;
   msgprmidx[0] = 0;
   sprintf( message_parm_1, "%lld", first_in_dup_range );
   msgprms[1] = message_parm_1;
   msgprmidx[1] = 0;
   msgprms[2] = message_parm_2;
   msgprmidx[2] = msg_info_ptr->msg_inotyp;
   msgprms[3] = message_parm_3;
   msgprmidx[3] = msg_info_ptr->msg_inopfx;
   sprintf( message_parm_4, "%ld", msg_info_ptr->msg_inonum );
   msgprms[4] = message_parm_4;
   msgprmidx[4] = 0;
   fsck_send_msg( fsck_DUPBLKREF, 0, 5 );
   size_of_dup_range = 0;
   }  /* end last block(s) finished a range */
 if( eq_rc == FSCK_OK ) {  /* nothing fatal yet */
   if( dups_detected ) {  /* claims at least 1 multiply allocated block */
     ino_recptr->involved_in_dups = 1;
     msgprms[0] = message_parm_0;
     msgprmidx[0] = msg_info_ptr->msg_inopfx;
     sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
     msgprms[1] = message_parm_1;
     msgprmidx[1] = 0;
     fsck_send_msg( fsck_DUPBLKREFS, 0, 2 );
     if( !(inode_is_metadata(ino_recptr)) ) {  /* not a metadata inode */
       if( is_EA ) {  /* an extended attributes extent */
         ino_recptr->clr_ea_fld = 1;
         agg_recptr->corrections_needed = 1;
         }
       else if( is_ACL ) {  /* an Access Control List */
         ino_recptr->clr_acl_fld = 1;
         agg_recptr->corrections_needed = 1;
         }
       else {  /* internal node or data */
         ino_recptr->selected_to_rls = 1;
         agg_recptr->corrections_needed = 1;
         }  /* end not an extended attributes extent */
       }  /* end else this is not a metadata inode */
     }  /* end claims at least 1 multiply allocated block */
   }  /* end nothing fatal yet */

  return( eq_rc );
}                              /* end of extent_1stref_chk ()  */


/*****************************************************************************
 * NAME: extent_record
 *
 * FUNCTION:  Record that each of the blocks in the given extent is allocated
 *            to some inode.
 *
 * PARAMETERS:
 *      first_block   - input - ordinal number of the first block in the extent
 *                              to check
 *      last_block    - input - ordinal number of the last block in the extent
 *                              to check
 *
 * NOTES:  Under certain special circumstances, it is necessary to record that
 *         each block in some extent are allocated, and IT IS ALREADY KNOWN
 *         that the extent is valid and that the blocks are not multiply
 *         allocated.  (Rather, that no duplicate references to the blocks
 *         have yet been detected and the block map currently indicates them
 *         to be unallocated.)
 *
 *         This function could be accomplished using routine extent_record_dupchk
 *         but, for performance reasons, this streamlined routine exists to 
 *         minimize processing time.
 *
 *         To be precise, before this routine is invoked for an extent, 
 *               o routine extent_record_dupchk was invoked for the extent, then 
 *                 extent_unrecord was invoked for the extent.  
 *          
 *            OR o after all block allocations were recorded in the fsck workspace
 *                 block map, and then the block map was adjusted as needed for
 *                 inode repairs, the block map was scanned to find an extent
 *                 of available blocks.  This routine is then called to record
 *                 the allocation of those blocks.  
 *
 *            OR o possibly some other path which accomplishes the same effect.
 *                 (I.e., this may not be an exhaustive list)
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t extent_record ( fsblkidx_t first_block,
                          fsblkidx_t last_block
                        )
{
  retcode_t er_rc = FSCK_OK;
  fsblkidx_t blkidx;

  for( blkidx = first_block;
       (blkidx <= last_block) && (er_rc >= FSCK_OK);
       blkidx++ ) {
    er_rc = blkall_increment_owners( blkidx );
    }  /* end for */

  return( er_rc );
}                              /* end of extent_record ()  */


/*****************************************************************************
 * NAME: extent_record_dupchk
 *
 * FUNCTION: Validate that the block number in the given extent are valid
 *           within the range valid for the aggregate (after the reserved
 *           space and fixed metadata and before the fsck workspace),
 *           record that the blocks are allocated, determine whether any
 *           prior allocations of the blocks have been recorded and, if
 *           so, perform duplicate allocation processing on the owning inode.
 *
 * PARAMETERS:
 *      first_block   - input - ordinal number of the first block in the extent
 *                              to check
 *      last_block    - input - ordinal number of the last block in the extent
 *                              to check
 *      is_EA         - input - !0 => the extent contains an inode's EA
 *                               0 => the extent contains something else
 *      msg_info_ptr  - input - pointer to a data area containing information
 *                              needed to issue messages for this extent
 *      ino_recptr    - input - pointer to the fsck inode record describing the
 *                              inode to which this extent is allocated
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t extent_record_dupchk ( fsblkidx_t        first_block,
                                                fsblkidx_t        last_block,
                                                int8              range_adj,
                                                int8              is_EA,
                                                int8              is_ACL,
                                               fsck_msg_info_ptr msg_info_ptr,
                                               fsck_inode_recptr ino_recptr
                                               )
{
 retcode_t erd_rc = FSCK_OK;
 fsblkidx_t block_idx;
 int is_a_dup = 0;
 int dups_detected = 0;
 fsblkidx_t first_in_dup_range;
 reg_idx_t size_of_dup_range = 0;
 dupall_blkrec_t this_blkrec;

 if( range_adj ) { /* the xad described an invalid range */
   msgprms[0] = message_parm_0;
   msgprmidx[0] = msg_info_ptr->msg_dxdtyp;
   msgprms[1] = message_parm_1;
   msgprmidx[1] = msg_info_ptr->msg_inotyp;
   msgprms[2] = message_parm_2;
   msgprmidx[2] = msg_info_ptr->msg_inopfx;
   sprintf( message_parm_3, "%ld", msg_info_ptr->msg_inonum );
   msgprms[3] = message_parm_3;
   msgprmidx[3] = 0;
   fsck_send_msg( fsck_BADBLKNO, 0, 4 );
   }  /* end the xad described an invalid range */
 for( block_idx = first_block;
      ((block_idx <= last_block) && (erd_rc >= FSCK_OK));
      block_idx++ ) { /* for each block */
   erd_rc = blkall_increment_owners( block_idx );
   if( erd_rc >= 0 ) {
     is_a_dup = erd_rc;
     erd_rc = FSCK_OK;
     }
   if( is_a_dup ) {  /* it's a block that's already allocated */
     if( size_of_dup_range == 0 ) {  /* this is a new range */
       dups_detected = 1;
       first_in_dup_range = block_idx;
       size_of_dup_range = 1;
       }  /* end this is a new range */
     else {  /* else not a new range */
       size_of_dup_range++;
       }  /* end else not a new range */
     }  /* end it's a block that's already allocated */
   else {  /* it's not already allocated */
     if( size_of_dup_range > 0 )  {  /* just finished a range */
       sprintf( message_parm_0, "%ld", size_of_dup_range );
       msgprms[0] = message_parm_0;
       msgprmidx[0] = 0;
       sprintf( message_parm_1, "%lld", first_in_dup_range );
       msgprms[1] = message_parm_1;
       msgprmidx[1] = 0;
       msgprms[2] = message_parm_2;
       msgprmidx[2] = msg_info_ptr->msg_inotyp;
       msgprms[3] = message_parm_3;
       msgprmidx[3] = msg_info_ptr->msg_inopfx;
       sprintf( message_parm_4, "%ld", msg_info_ptr->msg_inonum );
       msgprms[4] = message_parm_4;
       msgprmidx[4] = 0;
       fsck_send_msg( fsck_DUPBLKREF, 0, 5 );
       size_of_dup_range = 0;
       }  /* end just finished a range */
     }  /* end else it's not already allocated */
   }  /* end for block_idx */
 if( (erd_rc == FSCK_OK) && (size_of_dup_range > 0) )  {  /*
                            * last block(s) finished a range
                            */
   sprintf( message_parm_0, "%ld", size_of_dup_range );
   msgprms[0] = message_parm_0;
   msgprmidx[0] = 0;
   sprintf( message_parm_1, "%lld", first_in_dup_range );
   msgprms[1] = message_parm_1;
   msgprmidx[1] = 0;
   msgprms[2] = message_parm_2;
   msgprmidx[2] = msg_info_ptr->msg_inotyp;
   msgprms[3] = message_parm_3;
   msgprmidx[3] = msg_info_ptr->msg_inopfx;
   sprintf( message_parm_4, "%ld", msg_info_ptr->msg_inonum );
   msgprms[4] = message_parm_4;
   msgprmidx[4] = 0;
   fsck_send_msg( fsck_DUPBLKREF, 0, 5 );
   size_of_dup_range = 0;
   }  /* end last block(s) finished a range */
 if( erd_rc == FSCK_OK ) {  /* nothing fatal yet */
   if( dups_detected ) {  /* claims at least 1 multiply allocated block */
     ino_recptr->involved_in_dups = 1;
     msgprms[0] = message_parm_0;
     msgprmidx[0] = msg_info_ptr->msg_inopfx;
     sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
     msgprms[1] = message_parm_1;
     msgprmidx[1] = 0;
     fsck_send_msg( fsck_DUPBLKREFS, 0, 2 );
     if( !(inode_is_metadata(ino_recptr)) ) {  /* not a metadata inode */
       if( is_EA ) {  /* an extended attributes extent */
         ino_recptr->clr_ea_fld = 1;
         agg_recptr->corrections_needed = 1;
         }
       else if( is_ACL ) {  /* an extended attributes extent */
         ino_recptr->clr_acl_fld = 1;
         agg_recptr->corrections_needed = 1;
         }
       else {  /* internal node or data */
         ino_recptr->selected_to_rls = 1;
         agg_recptr->corrections_needed = 1;
         }  /* end not an extended attributes extent */
       }  /* end else this is not a metadata inode */
     }  /* end claims at least 1 multiply allocated block */
   }  /* end nothing fatal yet */

 return( erd_rc );
}                              /* end of extent_record_dupchk ()  */


/*****************************************************************************
 * NAME: extent_unrecord
 *
 * FUNCTION:  Decrement, in the fsck workspace block record, the owner count 
 *            for each block in the given extent.
 *
 * PARAMETERS:
 *      first_block   - input - ordinal number of the first block in the extent
 *                              to check
 *      last_block    - input - ordinal number of the last block in the extent
 *                              to check
 *
 * NOTES:  Under certain circumstances, it is necessary to back out the record
 *         of an inode's ownership the recording of some extent already verified
 *         valid.
 *
 *         This function could be accomplished using other routines which 
 *         include extent validation code, but, for performance reasons, 
 *         this streamlined routine exists to minimize processing time.
 *
 *         Examples of these circumstances include:
 *            o Storage allocated for an inode's EA is valid, but the B+Tree
 *              rooted in the inode is structurally corrupt.  Then the portions 
 *              of the tree which were recorded before the corruption was detected
 *              were backed out using routines which include validation code, and
 *              finally this routine is called to 'unrecord' the storage allocated
 *              for the EA.
 *
 *            o The B+Tree rooted in an inode was verified structurally correct,
 *              but the di_nblocks in the inode was found to be inconsistent with
 *              the tree.  In this case we assume the tree to be corrupt and 
 *              back it out of the fsck workspace block map immediately.  This
 *              routine would be used for that purpose since the tree has already
 *              been verified structurally correct.
 *
 *            o An inode has been found to be valid, but claims ownership of at 
 *              least one block claimed by another inode. At least one of the 
 *              inodes is actually damaged.  The user has given permission to 
 *              delete this inode, and so we need to decrement the number of
 *              owners for each block allocated to this inode.  This routine 
 *              would be used for that purpose since the tree has already been
 *              verified structurally correct.
 *  
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t extent_unrecord ( fsblkidx_t  first_block,
                            fsblkidx_t  last_block
                          )
{
  retcode_t eu_rc = FSCK_OK;
  fsblkidx_t  blkidx;

  for( blkidx = first_block;
       (blkidx <= last_block) && (eu_rc >= FSCK_OK);
       blkidx++ ) {
    eu_rc = blkall_decrement_owners( blkidx );
    }  /* end for */

  return( eu_rc );
}                              /* end of extent_unrecord ()  */


/****************************************************************************
 * NAME: fsblk_count_avail
 *
 * FUNCTION:  Count the number of contiguous aggregate blocks which are 
 *            available, according to the fsck workspace block map, starting 
 *            with the given (available) aggregate block.
 *
 * PARAMETERS:       
 *      wspbits     - input - pointer to a page in the fsck workspace block map
 *      wordidx     - input - the ordinal number, in the page pointed to by 
 *                            wspbits, of the word containing the bit representing
 *                            some particular aggregate block.
 *                            at routine entry: the block represented is the available
 *                                              block with which counting should start
 *                            at routine exit: the block represented is the 1st block
 *                                             AFTER the last block counted
 *      bitidx      - input - the ordinal number, in the word identified by wordidx,
 *                            of the bit representing some particular aggregate block.
 *                            at routine entry: the block represented is the available
 *                                              block with which counting should start
 *                            at routine exit: the block represented is the 1st block
 *                                             AFTER the last block counted
 *      num_wanted  - input - number of blocks wanted.  (i.e., when to stop counting
 *                            even if the run of contiguous, available blocks has
 *                            not ended.
 *      num_avail   - input - number of contiguous, available blocks counted starting
 *                            with the block described by wspbits, wordidx, and 
 *                            bitidx when the routine was entered.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t fsblk_count_avail ( fsck_bitmap_ptr wspbits,
                              int32          *wordidx,
                              int32          *bitidx,
                              int32           num_wanted,
                              int32          *num_avail
                             )
{
  retcode_t fbca_rc = FSCK_OK;
  int done_counting = 0;
  fsck_dword_bitmask_t bitmask;

  *num_avail = 0;
  while( ((*wordidx) < LPERDMAP) && (!done_counting) ) {
    bitmask = 0x80000000u;
    bitmask = bitmask >> (*bitidx);
    while( ((*bitidx) < DBWORD) && (!done_counting) ) {
      if( wspbits[*wordidx] & bitmask ) {  /* this one's not available */
        done_counting = -1;
        }
      else {  /* this one's available */
        (*num_avail)++;
        if( (*num_avail) == num_wanted ) {
          done_counting = -1;
          }
        else {
          bitmask = bitmask >> 1;
          }
        (*bitidx)++;
        }  /* end this one's available */
      }  /* end while bitidx */
    if( !done_counting ) {
      *bitidx = 0;
      *wordidx += 1;
      }
    }  /* end while wordidx */

  return( fbca_rc );
}                                      /* end fsblk_count_avail    */


/****************************************************************************
 * NAME: fsblk_next_avail
 *
 * FUNCTION:  Find the next available aggregate block, according to the 
 *            fsck workspace block map, starting with the given block.
 *
 * PARAMETERS:
 *      wspbits     - input - pointer to a page in the fsck workspace block map
 *      startword   - input - the ordinal number, in the page pointed to by 
 *                            wspbits, of the word containing the bit representing
 *                            the aggregate block at which to start searching
 *      startbit    - input - the ordinal number, in the word identified by wordidx,
 *                            of the bit representing the aggregate block at which 
 *                            to start searching
 *      foundword   - input - the ordinal number, in the page pointed to by 
 *                            wspbits, of the word containing the bit representing
 *                            the available aggregate block found, if any
 *      foundbit    - input - the ordinal number, in the word identified by wordidx,
 *                            of the bit representing the available aggregate block,
 *                            if any
 *      block_found - input - pointer to a variable in which the search results are
 *                            returned.  !0 => an available block was found in the
 *                                             given page at/after the specified start
 *                                        0 => no available block was found in the
 *                                             given page at/after the specified start
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t fsblk_next_avail ( fsck_bitmap_ptr wspbits,
                             int32           startword,
                             int32           startbit,
                             int32          *foundword,
                             int32          *foundbit,
                             int            *block_found
                            )
{
  retcode_t fbna_rc = FSCK_OK;
  int32 wordidx, bitidx, firstbit;
  fsck_dword_bitmask_t bitmask;
  fsck_dword_bitmask_t mask_all_on = 0xFFFFFFFFu;

  *block_found = 0;
  firstbit = startbit;
  for( wordidx = startword;
       ( (wordidx < LPERDMAP) && (!(*block_found)) );
       wordidx++ ) {
    if( wspbits[wordidx] != mask_all_on ) {  /* a zero in this map word */
      bitmask = 0x80000000u;
      bitmask = bitmask >> firstbit;
      for( bitidx = firstbit;
           ( (bitidx < DBWORD) && (!(*block_found)) );
           bitidx++ ) {
        if( !(wspbits[wordidx] & bitmask) ) {  /* it's available */
          *foundword = wordidx;
          *foundbit = bitidx;
          *block_found = -1;
          }  /* end it's available */
        else {  /* it's in use */
          bitmask = bitmask >> 1;
          }  /* end else it's in use */
        }  /* end for bitidx */
      }  /* end a zero in this map word */
    firstbit = 0;
    }  /* end for wordidx */

  return( fbna_rc );
}                                      /* end fsblk_next_avail  */


/****************************************************************************
 * NAME: fsck_alloc_fsblks
 *
 * FUNCTION: Allocate storage in the aggregate.
 *
 * PARAMETERS:
 *      blocks_wanted  - input - the number of contiguous blocks of storage
 *                               wanted
 *      blocks         - input - pointer to a variable in which the ordinal 
 *                               number of the first block allocated will be
 *                               returned  (or 0 if the storage cannot be 
 *                               allocated)
 *
 * NOTES: o This routine is only called when fsck has write access to the 
 *          aggregate.
 *  
 *        o This routine can not be called before the end of Phase 1 (that is,
 *          not before all block allocations existing in the aggregate have
 *          been recorded in the fsck workspace block map).
 *  
 *        o The optimum time to call this routine is after all inode repairs 
 *          have been performed (a step performed in Phase 6) since aggregates
 *          blocks may be made available by releasing inodes and/or clearing
 *          extents allocated for EAs.
 *
 *        o This routine can not be called after the beginning of Phase 8 (that
 *          is, not after fsck begins to rebuild the aggregate block map from
 *          the information in the fsck workspace block map).
 *
 *        o Currently, this routine is only called 
 *           - during inode reconnect processing (the last step in Phase 6) to 
 *              create new internal nodes for the directory to which the inode(s) 
 *              is(are) reconnected.
 *           - during replication of the Aggregate Inode Map (Phase 7) when 
 *              building the tree for the fileset AIM inode.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t fsck_alloc_fsblks ( int32        blocks_wanted,
                              fsblkidx_t  *blocks
                             )
{
  retcode_t fafsb_rc = FSCK_OK;
  mappgidx_t wsp_pagenum = 0;
  blk_pageptr wsp_page = NULL;
  int32 this_word = 0, this_bit = 0;
  int32 found_word = -1, found_bit = -1;
  int found_a_block;
  int32 blocks_found = 0;
  fsck_bitmap_ptr wsp_bits;
  *blocks = 0;
	
  while( (wsp_pagenum < agg_recptr->blkmp_pagecount) &&
         (blocks_found != blocks_wanted)              &&
         (fafsb_rc == FSCK_OK)                             ) {
    fafsb_rc = blkmap_get_page( wsp_pagenum, &wsp_page );
    if( fafsb_rc == FSCK_OK ) {  /* got a page */
      wsp_bits = (fsck_bitmap_ptr) wsp_page;
      this_word = 0;
      this_bit = 0;
      found_a_block = 0;
      blocks_found = 0;
      fafsb_rc = fsblk_next_avail( wsp_bits, this_word, this_bit,
                                 &found_word, &found_bit, &found_a_block );
      while( (found_a_block) && 
                (fafsb_rc == FSCK_OK) &&
                (blocks_found != blocks_wanted) ) {
        this_word = found_word;
        this_bit = found_bit;
        blocks_found = 0;
        fafsb_rc == fsblk_count_avail( wsp_bits, &found_word, &found_bit,
                                                   blocks_wanted, &blocks_found );
        if( fafsb_rc == FSCK_OK ) {  /* nothing bizarre happened */
          if( blocks_found == blocks_wanted ) {  /* success! */
            *blocks = (wsp_pagenum << log2BITSPERPAGE) +
                          (this_word << log2BITSPERDWORD) +
                          this_bit;
              }  /* end success! */
          else {  /* nothing useful yet */
            this_word = found_word;  /* word containing 1st 1 after the zeroes */
            this_bit = found_bit;    /* bit postion in that word */
            found_a_block = 0;
            fafsb_rc = fsblk_next_avail( wsp_bits, this_word, this_bit,
                                                    &found_word, &found_bit, 
                                                    &found_a_block );
            }  /* end else nothing useful yet */
          }  /* end nothing bizarre happened */
        }  /* end while */
      if( (fafsb_rc == FSCK_OK) && (!found_a_block) ) {  /* no avail block found */
          wsp_pagenum++;               /* maybe in the next page */
          }  /* end no avail block found */
      }  /* end got a page */
    }  /* end while */
	 
  if( fafsb_rc == FSCK_OK ) {  /* nothing fatal along the way */
    if( (*blocks) == 0 ) {  /* didn't find the blocks */
      fafsb_rc = FSCK_BLKSNOTAVAILABLE;
      sprintf( message_parm_0, "%d", blocks_wanted );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      fsck_send_msg( fsck_EXHFILSYSSTG, 0, 1 );
      }  /* end didn't find the blocks */
    else if( (*blocks) > agg_recptr->highest_valid_fset_datablk ) {  /*
                                   * the first available blocks were in the work area
                                   */
      *blocks = 0;
      fafsb_rc = FSCK_BLKSNOTAVAILABLE;
      sprintf( message_parm_0, "%d", blocks_wanted );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      fsck_send_msg( fsck_EXHFILSYSSTG, 0, 1 );
      }  /* end the first available blocks were in the work area */
    else {  /* we found the requested blocks */
      fafsb_rc = extent_record( *blocks, (*blocks + blocks_wanted - 1) ); /*
                                 * allocate these blocks for the caller
                                 * by marking them 'in use' in the fsck
                                 * workspace block map
                                 */
      }  /* end else we found the requested blocks */
    }  /* end nothing fatal along the way */

  return( fafsb_rc );
}                                      /* end fsck_alloc_fsblks    */


/****************************************************************************
 * NAME: fsck_dealloc_fsblks
 *
 * FUNCTION: Deallocate storage in the aggregate.
 *
 * PARAMETERS:
 *      blk_length  - input - the number of contiguous aggregate blocks 
 *                            to release 
 *      first_blk   - input - the ordinal number of the first allocated
 *                            aggregate block to release
 *
 * NOTES: o This routine is only called when fsck has write access to the 
 *          aggregate.
 *  
 *        o This routine can only be called to release blocks whose allocation
 *          has already been recorded in the fsck workspace block map.
 *
 *        o This routine can not be called after the beginning of Phase 8 (that
 *          is, not after fsck begins to rebuild the aggregate block map from
 *          the information in the fsck workspace block map).
 *
 *        o Currently, this routine is only called during Phase 6, during inode 
 *          repair processing to release internal nodes from a directory while
 *          removing a directory entry and during reconnect processing to back
 *          out partial processing for directory entry add which cannot be 
 *          completed because of a storage allocation failure.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t fsck_dealloc_fsblks ( int32      blk_length,
                                fsblkidx_t first_blk
                               )
{
  retcode_t fdfsb_rc = FSCK_OK;

  fdfsb_rc = extent_unrecord( first_blk, (first_blk + blk_length - 1) ); /*
                                 * deallocate these blocks by marking
                                 * them 'available' in the fsck workspace
                                 * block map
                                 */

  return( fdfsb_rc );
}                                      /* end fsck_dealloc_fsblks    */


/****************************************************************************
 * NAME: fscklog_end
 *
 * FUNCTION:  Put the last buffer in the chkdsk service log.  Cleanup the 
 *            first record in any remaining pages in this service log
 *            on the device so the log reader will know when to stop.
 *
 * PARAMETERS:  none
 *
 * NOTES:     If we don't have write access then there's nothing to do.
 *
 * RETURNS:
 *      success: FSCK_OK
 */
retcode_t fscklog_end ( )
{
  retcode_t fle_rc = FSCK_OK;
  int32          buffer_bytes_left;
  char          *remaining_buffer_chars;
     /*
      * If there's a partial buffer, write it to the device
      */
  if( agg_recptr->fscklog_buf_data_len > 0 ) {  /*
                         * there's something in there to be written
                         */
    buffer_bytes_left = agg_recptr->fscklog_buf_length - 
                        agg_recptr->fscklog_buf_data_len;
    agg_recptr->fscklog_last_msghdr->entry_length += buffer_bytes_left;
    fscklog_put_buffer();
    }  /* end there's something in there to be written */

  agg_recptr->fscklog_buf_allocated = 0; /*
                         * disable logging for the duration
                         */

  return( fle_rc );
}                                      /* end fscklog_end     */


/****************************************************************************
 * NAME: fscklog_init
 *
 * FUNCTION:  Initialize the current chkdsk service log
 *
 * PARAMETERS:  none
 *
 * NOTES:	- If we have write access to the aggregate, write
 *		   nulls over all pages in the log so that garbage
 *		   from a prior run does not end up appended to the
 *		   current service log.
 *
 *		- This MUST be called before logredo since logredo
 *		   will write messages to the chkdsk service log.
 *
 * RETURNS:
 *      success: FSCK_OK
 */
retcode_t fscklog_init ( )
{
  retcode_t fli_rc = FSCK_OK;
  agg_byteidx_t  log_bytes_left;
  struct fsck_blk_map_page *tmpbuf_addr = NULL;
  struct fsck_blk_map_page *tmp_buf_ptr;
  reg_idx_t                 tmp_buf_data_len;
  agg_byteidx_t             tmp_agg_offset;
  agg_byteidx_t             tmp_log_offset;

  if( agg_recptr->processing_readwrite )  {  /* have write access */
	/*
	 * this is safe because we do it before calling logredo
	 */
    tmpbuf_addr = (struct fsck_blk_map_page *) malloc(FSCKLOG_BUFSIZE);

    if( tmpbuf_addr == NULL ) {  /* didn't get the space */
     fsck_send_msg( fsck_CANTINITSVCLOG, 0, 0 );  /* log this fact so
                      * that any residual messages will be ignored
                      */
      }  /* end didn't get the space */
    else {  /* temp buffer allocated */

      agg_recptr->initializing_fscklog = 1;
      memset( (void *) tmpbuf_addr, 0, FSCKLOG_BUFSIZE );

          /* 
           * save the current fscklog values 
           */
      tmp_buf_ptr = agg_recptr->fscklog_buf_ptr;
      tmp_buf_data_len = agg_recptr->fscklog_buf_data_len;
      tmp_agg_offset = agg_recptr->fscklog_agg_offset;
      tmp_log_offset = agg_recptr->fscklog_log_offset;
          /* 
           * store values to describe the temp buffer 
           */
      agg_recptr->fscklog_buf_ptr = tmpbuf_addr;
      agg_recptr->fscklog_buf_data_len = FSCKLOG_BUFSIZE;


      log_bytes_left = (agg_recptr->ondev_fscklog_byte_length / 2) -
                       agg_recptr->fscklog_log_offset;
      while( log_bytes_left >= agg_recptr->fscklog_buf_length ) {
        fscklog_put_buffer();
        log_bytes_left = (agg_recptr->ondev_fscklog_byte_length / 2) -
                         agg_recptr->fscklog_log_offset;
        }  /* end while */

      free( (void *) tmpbuf_addr );

          /* 
           * restore the actual fscklog values 
           */
      agg_recptr->fscklog_buf_ptr = tmp_buf_ptr;
      agg_recptr->fscklog_buf_data_len = tmp_buf_data_len;
      agg_recptr->fscklog_agg_offset = tmp_agg_offset;
      agg_recptr->fscklog_log_offset = tmp_log_offset;

      agg_recptr->initializing_fscklog = 0;
      }  /* end else temp buffer allocated */
    }  /* end have write access */

 return( fli_rc );
}                                      /* end fscklog_init     */


/****************************************************************************
 * NAME: fscklog_start
 *
 * FUNCTION:  Allocate an I/O buffer and log the chkdsk start.
 *
 * PARAMETERS:  none
 *
 * NOTES:     Even if we don't have write access to the 
 *            aggregate we will do all parts of logging except
 *            actually writing to the disk.  This is to 
 *            provide diagnostic information if a dump is 
 *            taken during chkdsk execution.
 *
 * RETURNS:
 *      success: FSCK_OK
 */
retcode_t fscklog_start ( )
{
  retcode_t fls_rc = FSCK_OK;
  retcode_t iml_rc = FSCK_OK;
  int I_am_logredo = 0;

  iml_rc = alloc_wrksp( FSCKLOG_BUFSIZE, dynstg_iobufs,
                                 I_am_logredo,
                                (void **) &(agg_recptr->fscklog_buf_ptr) );

  if( iml_rc == FSCK_OK ) {  /* successful fsck service log buffer alloc */
    agg_recptr->fscklog_buf_length = FSCKLOG_BUFSIZE;
    agg_recptr->fscklog_buf_data_len = 0;
    agg_recptr->fscklog_log_offset = 0;
    agg_recptr->fscklog_full = 0;
    agg_recptr->fscklog_buf_allocated = -1;
    agg_recptr->fscklog_buf_alloc_err = 0;
    }  /* end successful fsck service log buffer alloc */
  else {
    agg_recptr->fscklog_buf_allocated = 0;
    agg_recptr->fscklog_buf_alloc_err = -1;
    }

 return( fls_rc );
}                                      /* end fscklog_start     */


/****************************************************************************
 * NAME: get_inode_extension
 *
 * FUNCTION: Allocates workspace storage for an fsck inode extension record
 *
 * PARAMETERS:
 *      inoext_ptr  - input - pointer to a variable in which the address of the  
 *                            new inode extension will be returned.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t get_inode_extension ( fsck_inode_extptr *inoext_ptr )
{
 retcode_t gir_rc = FSCK_OK;
  int I_am_logredo = 0;

 if( agg_recptr->free_inode_extens != NULL )  {
    *inoext_ptr = agg_recptr->free_inode_extens;
    agg_recptr->free_inode_extens = agg_recptr->free_inode_extens->next;
    memset ( *inoext_ptr, 0, sizeof(struct fsck_inode_ext_record) );
    }
 else {
   gir_rc = alloc_wrksp( sizeof(struct fsck_inode_ext_record),
                                 dynstg_inoextrec, I_am_logredo,
                                 (void **) inoext_ptr );
   }

 return( gir_rc );
}                                      /* end get_inode_extension     */


/****************************************************************************
 * NAME: get_inorecptr
 *
 * FUNCTION: Return a pointer to the fsck inode record describing the 
 *           specified inode.  
 *
 *           If no such record exists, then if allocation is specified, 
 *           allocate one, insert it into the fsck workspace inode record
 *           structures, and return the address of the new record.  Otherwise,
 *           (no such record exists but allocation was not specified) return
 *           NULL.
 *
 * PARAMETERS:   
 *      is_aggregate    - input - !0 => the requested inode is owned by the aggregate
 *                                 0 => the requested inode is owned by the fileset
 *      alloc           - input - !0 => do allocate a record if none has yet been
 *                                      allocated for the inode
 *                                 0 => do not allocate a record if none has been
 *                                      allocated for the inode
 *      inonum          - input - ordinal number of the inode whose fsck inode 
 *                                record is wanted
 *      addr_inorecptr  - input - pointer to a variable in which the address of 
 *                                of the found (or newly allocated) record will
 *                                be returned
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t get_inorecptr ( int                is_aggregate,
                          int                alloc,
                          inoidx_t           inonum,
                          fsck_inode_recptr *addr_inorecptr
                         )
{
 retcode_t gir_rc = FSCK_OK;

 if( is_aggregate ) {  /* for an aggregate inode */
   if( alloc ) {  /* request is to allocate if not found */
    gir_rc = inorec_agg_search_insert( inonum, addr_inorecptr );          /* @F1 */
    }  /* request is to allocate if not found */
   else {  /* search only */
     gir_rc = inorec_agg_search( inonum, addr_inorecptr );                  /* @F1 */
     }  /* end else search only */
   }  /* end for an aggregate inode */
 else {  /* for a fileset inode */
   if( alloc ) {  /* request is to allocate if not found */
     gir_rc = inorec_fs_search_insert( inonum, addr_inorecptr );           /* @F1 */
     }  /* request is to allocate if not found */
   else {  /* search only */
     gir_rc = inorec_fs_search( inonum, addr_inorecptr );                    /* @F1 */
     }  /* end else search only */
   }  /* end else for a fileset inode */

 return( gir_rc );
}                                          /* end get_inorecptr     */


/****************************************************************************
 * NAME: get_inorecptr_first
 *
 * FUNCTION: Return a pointer to the fsck inode record describing the inode
 *           which has the lowest ordinal number of all allocated inodes in
 *           the specified group (i.e., either of all allocated aggregate 
 *           inodes or of all allocated fileset inodes).
 *
 *           Initialize the balanced binary sort tree header record for a
 *           sequential traversal of the nodes in the tree.
 *
 * PARAMETERS:
 *      is_aggregate    - input - !0 => the requested inode is owned by the aggregate
 *                                 0 => the requested inode is owned by the fileset
 *      inonum          - input - pointer to a variable in which to return the ordinal
 *                                number of the inode whose fsck inode record address
 *                                is being returned in addr_inorecptr
 *      addr_inorecptr  - input - pointer to a variable in which the address of 
 *                                of the fsck inode record will be returned
 *
 * NOTES: o This routine should not be called before the end of phase 1 during
 *          which an fsck inode record is allocated to describe each inode in 
 *          the aggregate (both those owned by the aggregate and those owned by 
 *          the fileset in the aggregate).
 *
 *         o At entry to this routine, if the nodes in the specified balanced binary
 *           sort tree have not yet been linked into a sorted list, this list is
 *           created for the specified balanced binary sort tree.
 *
 *         o The fsck balanced binary sort tree header record contains the
 *           fields describing the sorted list of nodes in the tree, which
 *           are used to initialize a traversal and to remember the current
 *           list position of the traversal.
 *
 *         o Currently, this routine is only called for the balanced binary sort tree
 *           containing fsck inode records describing fileset owned inodes.  
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t get_inorecptr_first ( int                is_aggregate,
                                inoidx_t          *inonum,
                                fsck_inode_recptr *addr_inorecptr
                               )
{
    retcode_t girf_rc = FSCK_OK;
    int32 iagidx, extidx, inoidx;                                                       /* @F1 */
    inoext_tblptr inoexttbl;                                                            /* @F1 */
    ino_tblptr inotbl;                                                                     /* @F1 */
	
	/*
	 * find first active aggregate inode record 
	 */
    if( is_aggregate ) {  /* for an aggregate inode */
        if( agg_recptr->AIT_ext0_tbl == NULL ) {                                 /* @F1 */
            girf_rc = FSCK_INTERNAL_ERROR_68;                                   /* @F1 */

            sprintf( message_parm_0, "%ld", girf_rc );                            /* @F1 */
            msgprms[0] = message_parm_0;                                          /* @F1 */
            msgprmidx[0] = 0;                                                             /* @F1 */
            sprintf( message_parm_1, "%ld", 0 );                                    /* @F1 */
            msgprms[1] = message_parm_1;                                          /* @F1 */
            msgprmidx[1] = 0;                                                             /* @F1 */
            sprintf( message_parm_2, "%ld", 0 );                                   /* @F1 */
            msgprms[2] = message_parm_2;                                          /* @F1 */
            msgprmidx[2] = 0;                                                             /* @F1 */
            sprintf( message_parm_3, "%ld", 0 );                                    /* @F1 */
            msgprms[3] = message_parm_3;                                          /* @F1 */
            msgprmidx[3] = 0;                                                             /* @F1 */
            fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                       /* @F1 */
            }                                                                                   /* @F1 */
        else {                                                                                 /* @F1 */
		/*
		 * the first allocated aggregate inode is inode 1,
		 * by definition.
		 */
            inoidx = 1;                                                                      /* @F1 */
            *addr_inorecptr = 
                           agg_recptr->AIT_ext0_tbl->inorectbl[inoidx];       /* @F1 */
            agg_recptr->agg_last_inoidx = inoidx;                                /* @F1 */
            }                                                                                   /* @F1 */
        }  /* end for an aggregate inode                                               @F1 */

	/*
	 * find first active fileset inode record 
	 */
    else {  /* for a fileset inode */                                                   /* @F1 */
        if( agg_recptr->FSIT_IAG_tbl == NULL ) {                                 /* @F1 */
            girf_rc = FSCK_INTERNAL_ERROR_69;                                   /* @F1 */

            sprintf( message_parm_0, "%ld", girf_rc );                            /* @F1 */
            msgprms[0] = message_parm_0;                                          /* @F1 */
            msgprmidx[0] = 0;                                                             /* @F1 */
            sprintf( message_parm_1, "%ld", 0 );                                   /* @F1 */
            msgprms[1] = message_parm_1;                                          /* @F1 */
            msgprmidx[1] = 0;                                                             /* @F1 */
            sprintf( message_parm_2, "%ld", 0 );                                    /* @F1 */
            msgprms[2] = message_parm_2;                                          /* @F1 */
            msgprmidx[2] = 0;                                                             /* @F1 */
            sprintf( message_parm_3, "%ld", 0 );                                    /* @F1 */
            msgprms[3] = message_parm_3;                                          /* @F1 */
            msgprmidx[3] = 0;                                                             /* @F1 */
            fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                       /* @F1 */
            }                                                                                    /* @F1 */
        else {  /* the table is allocated                                                  @F1 */
		/* 
		 * IAG 0 is always the first active IAG in the
		 * fileset and extent 0 is always the first active
		 * extent since inode 2 (the root dir) is always
		 * the first active inode.
		 */
            iagidx = 0;                                                                       /* @F1 */
            extidx = 0;                                                                       /* @F1 */
            inoidx = 2;                                                                       /* @F1 */

            inoexttbl = agg_recptr->FSIT_IAG_tbl->inoext_tbl[iagidx];      /* @F1 */
            inotbl = inoexttbl->inotbl[extidx];                                       /* @F1 */
            *addr_inorecptr = inotbl->inorectbl[inoidx];                          /* @F1 */
		/*
		 * set things up for find next
		 */
            agg_recptr->fs_last_iagidx = iagidx;                                    /* @F1 */
            agg_recptr->fs_last_extidx = extidx;                                   /* @F1 */
            agg_recptr->fs_last_inoidx = inoidx;                                    /* @F1 */  
            }  /* end else the table is allocated                                        @F1 */
        }  /* end else for a fileset inode                                                 @F1 */                 
	 
    if( girf_rc == FSCK_OK ) {  /* got one */
        *inonum = (*addr_inorecptr)->inonum;
        }

    return( girf_rc );
}                                          /* end get_inorecptr_first     */


/****************************************************************************
 * NAME: get_inorecptr_next
 *
 * FUNCTION: Return a pointer to the fsck inode record describing the inode
 *           which has the ordinal number greater than the inode record most 
 *           recently returned in the current sequential traversal, and which
 *           describes the lowest ordinal inode number of all records not yet
 *           returned in the current traversal.
 *
 *           That is, return a pointer to the fsck inode record describing 
 *           the 'next' inode in the group (i.e., either next among the 
 *           aggregate-owned inodes or next among the fileset-owned inodes).
 *
 * PARAMETERS:
 *      is_aggregate    - input - !0 => the requested inode is owned by the aggregate
 *                                 0 => the requested inode is owned by the fileset
 *      inonum          - input - pointer to a variable in which to return the ordinal
 *                                number of the inode whose fsck inode record address
 *                                is being returned in addr_inorecptr
 *      addr_inorecptr  - input - pointer to a variable in which the address of 
 *                                of the fsck inode record will be returned.  If 
 *                                the most recently returned (in the current
 *                                traversal) record described the last inode in
 *                                the group, NULL is returned.
 *
 * NOTES:  o This routine is called iteratively in order to traverse the 
 *           inodes in the specified group in ascending key order efficiently.
 *
 *         o EACH sequential traversal of the inodes in a group (ie inodes
 *           owned by the aggregate or inodes owned by the fileset) MUST be
 *           initialized by an invocation of routine get_inorecptr_first.
 *
 *         o The fsck balanced binary sort tree header record contains the
 *           fields describing the sorted list of nodes in the tree, which
 *           are used to initialize a traversal and to remember the current
 *           list position of the traversal.
 *
 *         o fsck inode records can be accessed randomly (via routine 
 *           get_inorecptr which accepts inode number as key) independently.  
 *           That is, invocations of get_inorecptr may be intermingled with
 *           invocations of this routine (get_inorecptr_next) with no impact
 *           on the sequential traversal in progress.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t get_inorecptr_next ( int                is_aggregate,
                               inoidx_t          *inonum,
                               fsck_inode_recptr *addr_inorecptr
                              )
{
    retcode_t girn_rc = FSCK_OK;
    int32 iagidx, extidx, inoidx;                                                       /* @F1 */
    int32 extidx_init, inoidx_init;                                                      /* @F1 */
    inoext_tblptr inoexttbl;                                                            /* @F1 */
    ino_tblptr inotbl;                                                                     /* @F1 */
	
	/*
	 * find next active aggregate inode record 
	 */
 if( is_aggregate ) {  /* for an aggregate inode                                   @F1 */
   if( agg_recptr->AIT_ext0_tbl == NULL ) {                                     /* @F1 */
     girn_rc = FSCK_INTERNAL_ERROR_54;                                        /* @F1 */
     sprintf( message_parm_0, "%ld", girn_rc );                                 /* @F1 */
     msgprms[0] = message_parm_0;                                                /* @F1 */
     msgprmidx[0] = 0;                                                                   /* @F1 */
     sprintf( message_parm_1, "%ld", 0 );                                         /* @F1 */
     msgprms[1] = message_parm_1;                                                /* @F1 */
     msgprmidx[1] = 0;                                                                   /* @F1 */
     sprintf( message_parm_2, "%ld", 0 );                                         /* @F1 */
     msgprms[2] = message_parm_2;                                                /* @F1 */
     msgprmidx[2] = 0;                                                                   /* @F1 */
     sprintf( message_parm_3, "%ld", 0 );                                          /* @F1 */
     msgprms[3] = message_parm_3;                                                /* @F1 */
     msgprmidx[3] = 0;                                                                   /* @F1 */
     fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                             /* @F1 */
     }                                                                                          /* @F1 */
   else {                                                                                      /* @F1 */
        *addr_inorecptr = NULL;                                                        /* @F1 */
        for( inoidx = agg_recptr->agg_last_inoidx + 1;    
              ( (inoidx < INOSPEREXT) && (*addr_inorecptr == NULL) ); 
              inoidx++ ) {                                                                  /* @F1 */
            agg_recptr->agg_last_inoidx = inoidx;                                 /* @F1 */
            if( agg_recptr->AIT_ext0_tbl->inorectbl[inoidx] != NULL ) {    /* @F1 */
                *addr_inorecptr = agg_recptr->AIT_ext0_tbl->inorectbl[inoidx]; /* @F1 */
                }  /* end if                                                                    @F1 */
            }  /* end for                                                                      @F1*/
        }  /* end else                                                                        @F1 */
   }  /* end for an aggregate inode                                                   @F1 */

	/*
	 * find next active fileset inode record 
	 */
 else {  /* for a fileset inode                                                             @F1 */
        if( agg_recptr->FSIT_IAG_tbl == NULL ) {                                /* @F1 */
            girn_rc = FSCK_INTERNAL_ERROR_55;                                 /* @F1 */

            sprintf( message_parm_0, "%ld", girn_rc );                          /* @F1 */
            msgprms[0] = message_parm_0;                                        /* @F1 */
            msgprmidx[0] = 0;                                                           /* @F1 */
            sprintf( message_parm_1, "%ld", 0 );                                 /* @F1 */
            msgprms[1] = message_parm_1;                                        /* @F1 */
            msgprmidx[1] = 0;                                                           /* @F1 */
            sprintf( message_parm_2, "%ld", 0 );                                  /* @F1 */
            msgprms[2] = message_parm_2;                                         /* @F1 */
            msgprmidx[2] = 0;                                                            /* @F1 */
            sprintf( message_parm_3, "%ld", 0 );                                  /* @F1 */
            msgprms[3] = message_parm_3;                                         /* @F1 */
            msgprmidx[3] = 0;                                                            /* @F1 */
            fsck_send_msg( fsck_INTERNALERROR, 0, 4 );                      /* @F1 */
            }                                                                                   /* @F1 */
        else {  /* the table is allocated                                                  @F1 */
            extidx_init = agg_recptr->fs_last_extidx;                             /* @F1 */
            inoidx_init = agg_recptr->fs_last_inoidx + 1;                         /* @F1 */
            *addr_inorecptr = NULL;                                                    /* @F1 */
            for( iagidx = agg_recptr->fs_last_iagidx;                     
                  ( (iagidx < agg_recptr->fset_imap.num_iags) &&  
                     (*addr_inorecptr == NULL) );                                  
                  iagidx++) {                                                               /* @F1 */
                agg_recptr->fs_last_iagidx = iagidx;                               /* @F1 */
                if( agg_recptr->FSIT_IAG_tbl->inoext_tbl[iagidx] != NULL ) { /* @F1 */
                    inoexttbl = agg_recptr->FSIT_IAG_tbl->inoext_tbl[iagidx]; /* @F1 */
                    for( extidx = extidx_init;                              
                           ( (extidx < EXTSPERIAG) && 
                              (*addr_inorecptr == NULL) );
                           extidx++)  {                                                    /* @F1 */
                        agg_recptr->fs_last_extidx = extidx;                      /* @F1 */
                        if( inoexttbl->inotbl[extidx] != NULL ) {                  /* @F1 */
                            inotbl = inoexttbl->inotbl[extidx];                      /* @F1 */
                            for( inoidx = inoidx_init;                                
                                  ( (inoidx < INOSPEREXT) && 
                                     (*addr_inorecptr == NULL) );
                                   inoidx++)  {                                             /* @F1 */
                                agg_recptr->fs_last_inoidx = inoidx;               /* @F1 */
                                if( inotbl->inorectbl[inoidx] != NULL ) {           /* @F1 */
                                    *addr_inorecptr = inotbl->inorectbl[inoidx]; /* @F1 */
                                    }  /* end if                                                 @F1 */
                                }  /* end for inoidx                                         @F1 */
                            }  /* end if the inode table is allocated                  @F1 */
                        inoidx_init = 0;                                                    /* @F1 */
                        }  /* end for extidx                                                 @F1 */
                    }  /* end if the ext table is allocated                             @F1 */
                extidx_init = 0;                                                            /* @F1 */
                }  /* end for iagidx                                                          @F1 */
            }  /* end else the table is allocated                                       @F1 */
   }  /* end else for a fileset inode                                                     @F1 */

 if( ((*addr_inorecptr) != NULL) && (girn_rc == FSCK_OK) ) {  /* got one */
   *inonum = (*addr_inorecptr)->inonum;
   }
	
 return( girn_rc );
}                                          /* end get_inorecptr_next     */


/****************************************************************************
 * NAME: init_agg_record
 *
 * FUNCTION: initializes the global record, fsck_aggregate
 *
 * PARAMETERS:  none
 *
 * NOTES:  The fsck aggregate record is pointed to by the global variable 
 *         agg_recptr.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t init_agg_record( )
{
  retcode_t iar_rc = FSCK_OK;
  UniChar *uni_result_ptr = NULL;

  memset ( agg_recptr, 0, sizeof(fsck_agg_record) );
  memcpy( (void *) &(agg_recptr->eyecatcher), (void *) "fsckagrc", 8 );
  memcpy( (void *) &(agg_recptr->this_inode.eyecatcher),
          (void *) "thisinod", 8 );
  memcpy( (void *) &(agg_recptr->agg_imap_eyecatcher),
          (void *) "agg imap", 8 );
  memcpy( (void *) &(agg_recptr->fset_imap_eyecatcher),
          (void *) "fsetimap", 8 );
  memcpy( (void *) &(agg_recptr->AIT_eyecatcher),
          (void *) "agg ITbl", 8 );                                                      /* @F1 */
  memcpy( (void *) &(agg_recptr->FSIT_eyecatcher),  
          (void *) "fsetITbl", 8 );                                                      /* @F1 */
  memcpy( (void *) &(agg_recptr->flags_eyecatcher),
          (void *) "aggflags", 8 );
  memcpy( (void *) &(agg_recptr->fais.eyecatcher),
          (void *) "faisinfo", 8 );
  memcpy( (void *) &(agg_recptr->vlarge_info_eyecatcher),
          (void *) "vlargebf", 8 );
  memcpy( (void *) &(agg_recptr->fscklog_info_eyecatcher),
          (void *) "fscklog ", 8 );
  memcpy( (void *) &(agg_recptr->blkmp_info_eyecatcher),
          (void *) "blkmpbuf", 8 );
  memcpy( (void *) &(agg_recptr->ea_info_eyecatcher),
          (void *) "eabuffer", 8 );
  memcpy( (void *) &(agg_recptr->iag_info_eyecatcher),
          (void *) "iag buf ", 8 );
  memcpy( (void *) &(agg_recptr->mapctl_info_eyecatcher),
          (void *) "mapctbuf", 8 );
  memcpy( (void *) &(agg_recptr->maplf_info_eyecatcher),
          (void *) "maplfbuf", 8 );
  memcpy( (void *) &(agg_recptr->bmplv_info_eyecatcher),
          (void *) "bmplvbuf", 8 );
  memcpy( (void *) &(agg_recptr->bmpdm_info_eyecatcher),
          (void *) "bmpdmbuf", 8 );
  memcpy( (void *) &(agg_recptr->inobuf_info_eyecatcher),
          (void *) "inodebuf", 8 );
  memcpy( (void *) &(agg_recptr->nodbuf_info_eyecatcher),
          (void *) "node buf", 8 );
  memcpy( (void *) &(agg_recptr->agg_AGTbl_eyecatcher),
          (void *) "aggAGTbl", 8 );
  memcpy( (void *) &(agg_recptr->fset_AGTbl_eyecatcher),
          (void *) "fs AGTbl", 8 );
  memcpy( (void *) &(agg_recptr->amap_eyecatcher),
          (void *) "iagiamap", 8 );
  memcpy( (void *) &(agg_recptr->fextsumm_eyecatcher),
          (void *) "fextsumm", 8 );
  memcpy( (void *) &(agg_recptr->finosumm_eyecatcher),
          (void *) "finosumm", 8 );
                                                           /* 2 lines deleted                  @F1 */

            /* do the conversions from character to UniCharacter */

  agg_recptr->tree_height = 0;
  agg_recptr->delim_char = '\\';                  /* single char    */
  agg_recptr->UniChar_LSFN_NAME = uni_LSFN_NAME;  /* store the address  */
  agg_recptr->UniChar_lsfn_name = uni_lsfn_name;  /* store the address  */
  agg_recptr->agg_imap.ag_tbl = &(agg_recptr->agg_AGTbl[0]);
  agg_recptr->fset_imap.ag_tbl = &(agg_recptr->fset_AGTbl[0]);
   /*
    * start the messaging level out as 'show everything'
    * It may be reset lower when the parms have been parsed.
    */
  agg_recptr->effective_msg_level = fsck_verbose;
   /*
    * check to see whether standard out has been redirected, and
    * set the flag accordingly.
    */
  if( (ujfs_stdout_redirected()) != 0 ) {
    agg_recptr->stdout_redirected = 1;
    }
  else {
    agg_recptr->stdout_redirected = 0;
    }
	
  return( iar_rc );
}                                 /* end init_agg_record       */


/*                                                                                    start @F1 */


/****************************************************************************
 * NAME: inorec_agg_search
 *
 * FUNCTION: Search in the aggregate inode record structures for a record 
 *                describing the requested inode.  
 *
 *                If found, return the address of the record.  
 *
 *                If not found, return a null record address.
 *
 * PARAMETERS:	none
 *
 * NOTES:	
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t inorec_agg_search( ino_t                    inonum,
                                          fsck_inode_recptr *addr_inorecptr )
{
    retcode_t ias_rc = FSCK_OK;

    *addr_inorecptr = NULL;
    if( agg_recptr->AIT_ext0_tbl == NULL ) {
        ias_rc = FSCK_INTERNAL_ERROR_17;

        sprintf( message_parm_0, "%ld", ias_rc );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%ld", inonum );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%ld", 0 );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%ld", 0 );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        fsck_send_msg( fsck_INTERNALERROR, 0, 4 );
        }
    else {
        *addr_inorecptr = agg_recptr->AIT_ext0_tbl->inorectbl[inonum];
        }

    return( ias_rc );
}                                    /* end inorec_agg_search()    */


/****************************************************************************
 * NAME: inorec_agg_search_insert
 *
 * FUNCTION: Search in the aggregate inode record structures for a record 
 *                describing the requested inode.  
 *
 *                If found, return the address of the record.  
 *
 *                If not found, create a record to represent the inode,
 *                insert it into the structure, and return its address.
 *
 * PARAMETERS:	none
 *
 * NOTES:	
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t inorec_agg_search_insert( ino_t                    inonum,
                                                   fsck_inode_recptr *addr_inorecptr )
{
    retcode_t iasi_rc = FSCK_OK;
    fsck_inode_recptr new_inorecptr;
    int I_am_logredo = 0;

    *addr_inorecptr = NULL;
    if( agg_recptr->AIT_ext0_tbl == NULL ) {
        iasi_rc = FSCK_INTERNAL_ERROR_48;

        sprintf( message_parm_0, "%ld", iasi_rc );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%ld", inonum );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%ld", 0 );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%ld", 0 );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        fsck_send_msg( fsck_INTERNALERROR, 0, 4 );
        }
    else {  /* the table is initialized */
        if( agg_recptr->AIT_ext0_tbl->inorectbl[inonum] == NULL ) {  /* not allocated*/
            iasi_rc = alloc_wrksp( inode_record_length, dynstg_inorec,
                                           I_am_logredo, (void **) &new_inorecptr );
            if( iasi_rc == FSCK_OK ) {
                new_inorecptr->inonum = inonum;
                agg_recptr->AIT_ext0_tbl->inorectbl[inonum] = new_inorecptr;
                }
            }
        if( iasi_rc == FSCK_OK ) {
            *addr_inorecptr = agg_recptr->AIT_ext0_tbl->inorectbl[inonum];
            }
        }

    return( iasi_rc );
}                                    /* end inorec_agg_search_insert()    */


/****************************************************************************
 * NAME: inorec_fs_search
 *
 * FUNCTION: Search in the fileset inode record structures for a record 
 *                describing the requested inode.  
 *
 *                If found, return the address of the record.  
 *
 *                If not found, return a null record address.
 *
 * PARAMETERS:	none
 *
 * NOTES:	
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t inorec_fs_search( ino_t                    inonum,
                                        fsck_inode_recptr *addr_inorecptr )
{
    retcode_t ifs_rc = FSCK_OK;
    int32  iag_in_agg, ext_in_iag, ino_in_ext;
    inoext_tblptr  inoexttbl;
    ino_tblptr       inotbl;

    *addr_inorecptr = NULL;
    if( agg_recptr->FSIT_IAG_tbl == NULL ) {
        ifs_rc = FSCK_INTERNAL_ERROR_49;

        sprintf( message_parm_0, "%ld", ifs_rc );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%ld", inonum );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%ld", 0 );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%ld", 0 );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        fsck_send_msg( fsck_INTERNALERROR, 0, 4 );
        }
    else {  /* the IAG table is initialized */

        locate_inode( inonum, &iag_in_agg, &ext_in_iag, &ino_in_ext );

        if( iag_in_agg < agg_recptr->fset_imap.num_iags ) {  /* IAG num in range */
            if( agg_recptr->FSIT_IAG_tbl->inoext_tbl[iag_in_agg] != NULL )  {  /* ext table alloc */
                inoexttbl = agg_recptr->FSIT_IAG_tbl->inoext_tbl[iag_in_agg];
                if( inoexttbl->inotbl[ext_in_iag] != NULL )  {  /* inode table allocated */
                    inotbl = inoexttbl->inotbl[ext_in_iag];
                    *addr_inorecptr = inotbl->inorectbl[ino_in_ext];
                    }  /* end extent's inode table is allocated */
                }  /* end IAG's extent table is allocated */
            }  /* end IAG number in range */
        }  /* end else IAG table is initialized */

    return( ifs_rc );
}                                    /* end inorec_fs_search()    */


/****************************************************************************
 * NAME: inorec_fs_search_insert
 *
 * FUNCTION: Search in the fileset inode record structures for a record 
 *                describing the requested inode.  
 *
 *                If found, return the address of the record.  
 *
 *                If not found, create a record to represent the inode,
 *                insert it into the structure, and return its address.
 *
 * PARAMETERS:	none
 *
 * NOTES:	
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t inorec_fs_search_insert( ino_t                    inonum,
                                                 fsck_inode_recptr *addr_inorecptr )
{
    retcode_t ifsi_rc = FSCK_OK;
    int32  iag_in_agg, ext_in_iag, ino_in_ext;
    inoext_tblptr  inoexttbl, new_inoexttbl;
    ino_tblptr       inotbl, new_inotbl;
    fsck_inode_recptr new_inorecptr;
    int I_am_logredo = 0;

    *addr_inorecptr = NULL;
    if( agg_recptr->FSIT_IAG_tbl == NULL ) {
        ifsi_rc = FSCK_INTERNAL_ERROR_67;

        sprintf( message_parm_0, "%ld", ifsi_rc );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%ld", inonum );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%ld", 0 );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%ld", 0 );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        fsck_send_msg( fsck_INTERNALERROR, 0, 4 );
        }

    else {  /* the IAG table is initialized */

        locate_inode( inonum, &iag_in_agg, &ext_in_iag, &ino_in_ext );

        if( iag_in_agg < agg_recptr->fset_imap.num_iags ) {  /* 
				* the IAG number is in range 
				*/
            if( agg_recptr->FSIT_IAG_tbl->inoext_tbl[iag_in_agg] == NULL )  {  /* 
				* extent table not allocated 
				*/
                    ifsi_rc = alloc_wrksp( inode_ext_tbl_length, dynstg_inoexttbl,
                                                   I_am_logredo, (void **) &new_inoexttbl );
                    if( ifsi_rc == FSCK_OK ) {
                        memcpy( (void *) &(new_inoexttbl->eyecatcher), 
                                      (void *) "InoExTbl", 8 );
                        agg_recptr->FSIT_IAG_tbl->inoext_tbl[iag_in_agg] = 
                                                                                        new_inoexttbl;
                        }
                }  /* end IAG's extent table is not allocated */

            if( ifsi_rc == FSCK_OK ) {
                inoexttbl = agg_recptr->FSIT_IAG_tbl->inoext_tbl[iag_in_agg];

                if( inoexttbl->inotbl[ext_in_iag] == NULL )  {  /* 
				* the inode table is not allocated 
				*/
                   ifsi_rc = alloc_wrksp( inode_tbl_length, dynstg_inotbl,
                                                   I_am_logredo, (void **) &new_inotbl );
                    if( ifsi_rc == FSCK_OK ) {
                        memcpy( (void *) &(new_inotbl->eyecatcher), 
                                      (void *) "InodeTbl", 8 );
                        inoexttbl->inotbl[ext_in_iag] = new_inotbl;
                        }
                    }  /* end extent's inode table is not allocated */
                }

            if( ifsi_rc == FSCK_OK )  {
                inotbl = inoexttbl->inotbl[ext_in_iag];

                if( inotbl->inorectbl[ino_in_ext] == NULL )  {  /* 
				* the inode record is not allocated 
				*/
                    ifsi_rc = alloc_wrksp( inode_record_length, dynstg_inorec,
                                                   I_am_logredo, (void **) &new_inorecptr );
                    if( ifsi_rc == FSCK_OK ) {
                        new_inorecptr->inonum = inonum;
                        inotbl->inorectbl[ino_in_ext] = new_inorecptr;
                        }
                    }  /* end inode record not allocated */
                }

            if( ifsi_rc == FSCK_OK ) {
                *addr_inorecptr = inotbl->inorectbl[ino_in_ext];
                }
            }  /* end IAG number in range */
        }  /* end else IAG table is initialized */

    return( ifsi_rc );
}                                    /* end inorec_fs_search_insert()    */


/****************************************************************************
 * NAME: locate_inode
 *
 * FUNCTION: Given an inode number, calculate the corresponding IAG #,
 *                extent number within the IAG, and inode number within the
 *                extent.
 *
 * PARAMETERS:	none
 *
 * NOTES:	
 *
 * RETURNS:		none
 */
void locate_inode( ino_t     inonum,
                           int32   *iag_in_agg,
                           int32   *ext_in_iag,
                           int32   *ino_in_ext )
{
    int32 extinagg;

    *iag_in_agg = inonum >> L2INOSPERIAG;
    extinagg = inonum >> L2INOSPEREXT;
    *ext_in_iag = extinagg - ((*iag_in_agg) * EXTSPERIAG);
    *ino_in_ext = inonum - (extinagg << L2INOSPEREXT);

}                                    /* end locate_inode()    */


/*                                                                                      end @F1 */


/*****************************************************************************
 * NAME: process_extent
 *
 * FUNCTION:  Verify the given extent, if found to be invalid, record 
 *            information in the fsck inode record describing the owning
 *            inode for corrective measures.  Perform the requested action
 *            on the (adjusted if the bounds were invalid) range of aggregate
 *            blocks in the extent.
 *
 * PARAMETERS:
 *      inorecptr        - input - pointer to the fsck inode record describing
 *                                 the inode which owns the extent
 *      extent_length    - input - the number of blocks in the extent, as recorded
 *                                 in the inode structures
 *      extent_addr      - input - the ordinal number of the first block in the
 *                                 extent, as recorded in the inode structures
 *      is_EA            - input -  0 => the extent describes the inode's EA
 *                                 !0 => the extent describes something else for
 *                                       the inode
 *      msg_info_ptr     - input - pointer to a data area containing information
 *                                 needed to issue messages for this extent
 *      adjusted_length  - input - pointer to a variable in which to return the 
 *                                 number of blocks on which the action was actually
 *                                 performed (if the range was invalid, its bounds
 *                                 are adjusted as needed to make it valid)
 *      extent_is_valid  - input - pointer to a variable in which the results of
 *                                 validating the extent are returned.
 *                                 !0 => the extent bounds, as passed in, are 
 *                                       reasonable for the aggregate
 *                                  0 => the extent bounds, as passed in, are 
 *                                       obviously incorrect 
 *      desired_action   - input - { FSCK_RECORD | FSCK_RECORD_DUPCHECK |
 *                                   FSCK_UNRECORD | FSCK_QUERY }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t process_extent ( fsck_inode_recptr  inorecptr,
                           reg_idx_t          extent_length,
                           fsblkidx_t         extent_addr,
                           int8               is_EA,
                           int8               is_ACL,
                           fsck_msg_info_ptr  msg_info_ptr,
                           reg_idx_t         *adjusted_length,
                           int8              *extent_is_valid,
                           int                desired_action
                          )
{
 retcode_t ve_rc = FSCK_OK;
 fsblkidx_t first_valid;
 fsblkidx_t last_valid;
 int8 range_adjusted = 0;

  if( inorecptr->inode_type == metadata_inode ) {
   first_valid = extent_addr;
   }
  else {  
   first_valid = (extent_addr < agg_recptr->lowest_valid_fset_datablk)
                                    ? agg_recptr->lowest_valid_fset_datablk
                                    : extent_addr;
   }
 last_valid =
        ((extent_addr + extent_length) >
                              agg_recptr->highest_valid_fset_datablk)
                                    ? agg_recptr->highest_valid_fset_datablk
                                    : extent_addr + extent_length -1;
	
 if( ( (first_valid > agg_recptr->highest_valid_fset_datablk) &&
        (inorecptr->inode_type != metadata_inode)                   ) ||  /*
			* starts after end of valid fset area AND  
			* isn't a meta data inode OR
			*/
     ( (last_valid < agg_recptr->lowest_valid_fset_datablk)  &&
        (inorecptr->inode_type != metadata_inode)                   ) ||  /*
			* ends before the beginning of valid fset area AND
			* isn't a meta data inode OR
			*/
     (last_valid < first_valid) ) {  /* ends before it starts */
   *adjusted_length = 0;
   *extent_is_valid = 0;
   if( is_EA ) {      /* this is an extended attributes extent */
     inorecptr->clr_ea_fld = 1;
     inorecptr->ignore_ea_blks = 1;
     agg_recptr->corrections_needed = 1;
     }  /* end this is an extended attributes extent */
   else if( is_ACL ) {      /* this is an access control list extent */
     inorecptr->clr_acl_fld = 1;
     inorecptr->ignore_acl_blks = 1;
     agg_recptr->corrections_needed = 1;
     }  /* end this is an access control list extent */
   else {  /* either a node (internal or leaf) or data */
     inorecptr->selected_to_rls = 1;
     inorecptr->ignore_alloc_blks = 1;
     agg_recptr->corrections_needed = 1;
     }  /* end either a node (internal or leaf) or data */
   }
 else {  /* not out of the question */
   *adjusted_length = last_valid - first_valid + 1;
   if( (first_valid != extent_addr)  ||
       (last_valid != (extent_addr + extent_length - 1)) ) { /* at least some
                                * blocks are not valid for the fileset
                                */
     range_adjusted = -1;
     *extent_is_valid = 0;
     if( is_EA ) {      /* this is an extended attributes extent */
       inorecptr->clr_ea_fld = 1;
       agg_recptr->corrections_needed = 1;
       }  /* end this is an extended attributes extent */
     else if( is_ACL ) {      /* this is an access control list extent */
       inorecptr->clr_acl_fld = 1;
       agg_recptr->corrections_needed = 1;
       }  /* end this is an access control list extent */
     else {  /* either a node (internal or leaf) or data */
       inorecptr->selected_to_rls = 1;
       agg_recptr->corrections_needed = 1;
       }  /* end either a node (internal or leaf) or data */
     }  /* end at least some blocks in the extent are out of the ...  */
   else { /* else the extent is ok */
     *extent_is_valid = -1;
     }  /* end else the extent is ok */
     /*
      * Finally, depending on the parm passed by the caller,
      *
      *   either: record the ownership of the blocks which are within
      *           range and keep a count of multiply allocated blocks.
      *
      *   or:     reverse notations made in the workspace for the ownership
      *           of blocks which are within range and decrement the count
      *           of multiply allocated blocks.
      *
      *   or:     check the extent to see if it contains the first reference
      *           to any multiply allocated block for which the first
      *           reference is still unresolved.
      */
   switch( desired_action ) {
     case FSCK_RECORD:
       ve_rc = extent_record( first_valid, last_valid );
       break;
     case FSCK_RECORD_DUPCHECK:
       ve_rc = extent_record_dupchk( first_valid, last_valid,
                                   range_adjusted, is_EA, is_ACL,
                                   msg_info_ptr, inorecptr );
       break;
     case FSCK_UNRECORD:
       ve_rc = extent_unrecord( first_valid, last_valid );
       break;
     case FSCK_QUERY:
       ve_rc = extent_1stref_chk( first_valid, last_valid,
                                is_EA, is_ACL, msg_info_ptr, inorecptr );
       break;
     default:
       ve_rc = FSCK_INTERNAL_ERROR_7;    /* shouldn't ever get here */
     }  /* end switch */
   }  /* end else not out of the question */
	
 return( ve_rc );
}                              /* end of process_extent ()  */


/****************************************************************************
 * NAME: release_inode_extension
 *
 * FUNCTION: Make an fsck inode extension record available for reuse.
 *
 * PARAMETERS:
 *      inoext_ptr  - input - address of the extension record to release
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t release_inode_extension ( fsck_inode_extptr inoext_ptr )
{
 retcode_t rir_rc = FSCK_OK;

 inoext_ptr->next = agg_recptr->free_inode_extens;
 agg_recptr->free_inode_extens = inoext_ptr;

 return( rir_rc );
}                                       /* end release_inode_extension     */


/****************************************************************************
 * NAME: release_logredo_allocs
 *
 * FUNCTION:  Goes through all storage allocation records and, for each
 *                 record describing an allocation for logredo, marks all storage
 *                 available.
 *
 * PARAMETERS:	none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t release_logredo_allocs( )
{
    retcode_t rla_rc = FSCK_OK;
    char *wsp_ptr = NULL;
    wsp_ext_rec_ptr this_fer;

    this_fer = agg_recptr->wsp_extent_list;

    while( (this_fer != NULL)  &&  (rla_rc == FSCK_OK)  ) {
  
        if( this_fer->for_logredo ) {	
           this_fer->last_byte_used = sizeof(fsck_extent_record) - 1;
           }  
        this_fer = this_fer->next;
        }  /* end while */

 return( rla_rc );
}                                      /* end release_logredo_allocs()	*/


/****************************************************************************
 * NAME: temp_inode_buf_alloc
 *
 * FUNCTION:  Allocate an I/O buffer for use during metadata replication
 *            processing
 *
 * PARAMETERS:
 *      addr_buf_ptr  - input - pointer to a variable in which to return the
 *                              address of the buffer allocated.
 *
 * NOTES: o This very large buffer is only needed for a short time.  It is
 *          used to hold both the first extent of the primary AIT and the
 *          first extent of the secondary AIT.  
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t temp_inode_buf_alloc ( char **addr_buf_ptr )
{
  retcode_t tiba_rc = FSCK_OK;

  agg_recptr->vlarge_current_use = USED_FOR_INOEXT_BUF;
  *addr_buf_ptr = (char *) agg_recptr->vlarge_buf_ptr;

  memset( (void *) (*addr_buf_ptr), '\0', VLARGE_BUFSIZE );

  return ( tiba_rc );
}                                /* end temp_inode_buf_alloc ( ) */


/****************************************************************************
 * NAME: temp_inode_buf_release
 *
 * FUNCTION:  Free storage which was allocated for an I/O buffer for use 
 *            during metadata replication processing
 *
 * PARAMETERS:
 *      buf_ptr  - input - address of the buffer to release
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t temp_inode_buf_release ( char *buf_ptr )
{
  retcode_t tibr_rc = FSCK_OK;

  agg_recptr->vlarge_current_use = NOT_CURRENTLY_USED;

  return ( tibr_rc );
}                                       /* end temp_inode_buf_release()    */


/****************************************************************************
 * NAME: temp_node_buf_alloc
 *
 * FUNCTION:  Allocate an I/O buffer for use during metadata replication
 *            processing
 *
 * PARAMETERS:
 *      addr_buf_ptr  - input - pointer to a variable in which to return the
 *                              address of the buffer allocated.
 *
 * NOTES: o This very large buffer is only needed for a short time.  It is
 *          used to hold both the first extent of the primary AIT and the
 *          first extent of the secondary AIT.  
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t temp_node_buf_alloc ( char **addr_buf_ptr )
{
  retcode_t tnba_rc = FSCK_OK;
  int  I_am_logredo = 0;	 

  tnba_rc = alloc_wrksp( XTPAGE_SIZE, dynstg_tmpinoiobuf, 
                                    I_am_logredo, (void **) addr_buf_ptr );

  if( (*addr_buf_ptr) == NULL ) { /* allocation failure */
    tnba_rc = FSCK_FAILED_DYNSTG_EXHAUST7;
    sprintf( message_parm_0, "%d", wsp_dynstg_action );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    sprintf( message_parm_1, "%d", dynstg_wspext );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    fsck_send_msg( fsck_EXHDYNSTG, 0, 2 );
    }  /* end allocation failure */
	 
 return ( tnba_rc );
}                                /* end temp_node_buf_alloc ( ) */


/****************************************************************************
 * NAME: treeQ_dequeue
 *
 * FUNCTION: Removes an element from the front of the fsck tree-queue
 *           and returns a pointer to it.
 *           If the queue is empty, NULL is returned.
 *
 * PARAMETERS:
 *      treeQ_elptr  - input - pointer to a variable in which to return 
 *                             the address of the element from the front of 
 *                             the queue will be returned (or NULL if the 
 *                             queue is empty)
 *
 * NOTES: The fsck xTree queue is described in the aggregate record, 
 *        fields: treeQ_front, treeQ_back
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t treeQ_dequeue ( treeQ_elem_ptr *treeQ_elptr )
{
 retcode_t tQd_rc = FSCK_OK;

 *treeQ_elptr = agg_recptr->treeQ_front;
 if( agg_recptr->treeQ_back == agg_recptr->treeQ_front )  {  /* empty */
   agg_recptr->treeQ_back = agg_recptr->treeQ_front = NULL;
   }  /* end empty */
 else {  /* not empty */
   agg_recptr->treeQ_front = agg_recptr->treeQ_front->next;
   agg_recptr->treeQ_front->prev = NULL;
   }  /* end else not empty */

 return( tQd_rc );
}                                                /* end treeQ_dequeue      */


/****************************************************************************
 * NAME: treeQ_enqueue
 *
 * FUNCTION: Adds the given element to the back of the fsck tree-queue
 *           stack.
 *
 * PARAMETERS:
 *      treeQ_elptr  - input - the address of the element to add to the queue 
 *
 * NOTES: The fsck xTree queue is described in the aggregate record, 
 *        fields: treeQ_front, treeQ_back
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t treeQ_enqueue ( treeQ_elem_ptr treeQ_elptr )
{
 retcode_t tep_rc = FSCK_OK;

 if( agg_recptr->treeQ_back == NULL )  {  /* empty queue */
   agg_recptr->treeQ_back = agg_recptr->treeQ_front = treeQ_elptr;
   treeQ_elptr->prev = treeQ_elptr->next = NULL;
   }  /* end empty queue */
 else {  /* queue not empty */
   treeQ_elptr->next = NULL;
   treeQ_elptr->prev = agg_recptr->treeQ_back;
   agg_recptr->treeQ_back->next = treeQ_elptr;
   agg_recptr->treeQ_back = treeQ_elptr;
   }  /* end else queue not empty */

 return( tep_rc );
}                                                    /* end treeQ_enqueue  */


/****************************************************************************
 * NAME: treeQ_get_elem
 *
 * FUNCTION: Allocates workspace storage for an fsck tree-queue element
 *
 * PARAMETERS:
 *      addr_treeQ_ptr  - input - pointer to a variable in which to return 
 *                                the address of the element allocated.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t treeQ_get_elem ( treeQ_elem_ptr *addr_treeQ_ptr )
{
 retcode_t gte_rc = FSCK_OK;
 int I_am_logredo = 0;

 if( agg_recptr->free_treeQ != NULL ) { /* free list isn't empty */
   *addr_treeQ_ptr = agg_recptr->free_treeQ;
   agg_recptr->free_treeQ = agg_recptr->free_treeQ->next;
   memset( (void *) (*addr_treeQ_ptr), 0, treeQ_elem_length );
   }  /* end freelist isn't empty */
 else {  /* else the free list is empty */
   gte_rc = alloc_wrksp( treeQ_elem_length, dynstg_treeQ_elem,
                                  I_am_logredo,
                                  (void **) addr_treeQ_ptr );
   }  /* end else the free list is empty */

 return( gte_rc );
}  /* end                                               treeQ_get_elem     */


/****************************************************************************
 *
 * NAME: treeQ_rel_elem
 *
 * FUNCTION: Makes an fsck tree-queue element available for reuse
 *
 * PARAMETERS:
 *      treeQ_elptr  - input - the address of the element to release
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t treeQ_rel_elem ( treeQ_elem_ptr treeQ_elptr )
{
 retcode_t tre_rc = FSCK_OK;
 treeQ_elptr->next = agg_recptr->free_treeQ;
 agg_recptr->free_treeQ = treeQ_elptr;

 return( tre_rc );
}                                                 /* end treeQ_rel_elem     */


/****************************************************************************
 * NAME: treeStack_get_elem
 *
 * FUNCTION: Allocates workspace storage for an fsck tree stack element
 *
 * PARAMETERS:
 *      addr_treeStack_ptr  - input - pointer to a variable in which the address
 *                                 of the new element should be returned.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t treeStack_get_elem ( treeStack_ptr *addr_treeStack_ptr )
{
 retcode_t tSge_rc = FSCK_OK;
 int I_am_logredo = 0;

 if( agg_recptr->free_treeStack != NULL ) { /* free list isn't empty */

   *addr_treeStack_ptr = agg_recptr->free_treeStack;
   agg_recptr->free_treeStack = agg_recptr->free_treeStack->next;
   memset( (void *) (*addr_treeStack_ptr), 0, treeStack_elem_length );
   }  /* end freelist isn't empty */

 else {  /* else the free list is empty */

   tSge_rc = alloc_wrksp( treeStack_elem_length, 
                                    dynstg_treeStack_elem, I_am_logredo,
                                    (void**) addr_treeStack_ptr );
   }  /* end else the free list is empty */

 return( tSge_rc );
}                                        /* end treeStack_get_elem     */


/****************************************************************************
 * NAME: treeStack_rel_elem
 *
 * FUNCTION: Makes an fsck tree stack element available for reuse
 *
 * PARAMETERS:
 *      treeStack_elptr  - input - the address of the element to release
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t treeStack_rel_elem ( treeStack_ptr treeStack_elptr )
{
 retcode_t tSre_rc = FSCK_OK;

 treeStack_elptr->next = agg_recptr->free_treeStack;
 agg_recptr->free_treeStack = treeStack_elptr;

 return( tSre_rc );
}                                               /* end treeStack_rel_elem     */


/****************************************************************************
 * NAME: treeStack_pop
 *
 * FUNCTION:  If the tree stack is not empty, remove the top element,
 *            return the address it contains, and release it.
 *            Otherwise return NULL.
 *
 * PARAMETERS:
 *      addr_inorecptr    - input - the address of a variable in which to
 *                                  return the address in the top element
 *
 * NOTES:  The tree stack is in the aggregate record, field treeStack
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t treeStack_pop ( fsck_inode_recptr *addr_inorecptr )
{
  retcode_t tSp_rc = FSCK_OK;
  treeStack_ptr top_elem;

  if( agg_recptr->treeStack != NULL ) {
    top_elem = agg_recptr->treeStack;
    agg_recptr->treeStack = top_elem->next;
    *addr_inorecptr = top_elem->inorec;
    tSp_rc = treeStack_rel_elem( top_elem );
    }
  else {
    *addr_inorecptr = NULL;
    }    

  return( tSp_rc );
}                                      /* end treeStack_pop     */


/****************************************************************************
 * NAME: treeStack_push
 *
 * FUNCTION:  Get a stack element, store the given address in it, and add
 *                 it to the top of the tree stack.
 *
 * PARAMETERS:
 *      inorecptr  - input - the address to store in a stack element
 *
 * NOTES:  The tree stack is in the aggregate record, field treeStack
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t treeStack_push ( fsck_inode_recptr inorecptr )
{
  retcode_t tSp_rc = FSCK_OK;
  treeStack_ptr  new_elem;

  tSp_rc = treeStack_get_elem( &new_elem );
  if( tSp_rc == FSCK_OK ) {
    new_elem->inorec = inorecptr;
    new_elem->next = agg_recptr->treeStack;
    agg_recptr->treeStack = new_elem;
    }

  return( tSp_rc );
}                                      /* end treeStack_push    */


/*****************************************************************************
 * NAME: workspace_release
 *
 * FUNCTION:  Release the pool of storage allocated for fsck workspace
 *            storage.
 *
 * PARAMETERS: none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t workspace_release ( )
{
  retcode_t wr_rc = FSCK_OK;
  wsp_ext_rec_ptr this_fer, next_fer;
  char *extent_addr;
  /*
   * If the very large buffer is (still) allocated, release it.
   */
  if( agg_recptr->vlarge_buf_ptr != NULL ) {
    free( (void *) agg_recptr->vlarge_buf_ptr );
    agg_recptr->vlarge_buf_ptr = NULL;
    agg_recptr->ea_buf_ptr = NULL;
    agg_recptr->recon_buf_extent = NULL;
    }
  /*
   * release the allocated extents
   */
  this_fer = agg_recptr->wsp_extent_list;
  while( this_fer != NULL ) {  /* for each extent record */
    next_fer = this_fer->next; /* the one after this one (if any) */
    if( ! this_fer->from_high_memory ) {
        free( (void *) this_fer->extent_addr );  /* free the extent this
                              * fer describes (and occupies)
                              */
        }
    this_fer = next_fer;     /* go on to the next one in the list */
    }  /* end for each extent record */

  return( wr_rc );
}                              /* end of workspace_release ()  */

