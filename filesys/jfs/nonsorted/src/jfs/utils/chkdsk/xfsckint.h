/* $Id: xfsckint.h,v 1.2 2004/03/21 02:43:26 pasha Exp $ */

/* static char *SCCSID = "@(#)1.22  6/28/99 13:50:29 src/jfs/utils/chkdsk/xfsckint.h, jfschk, w45.fs32, fixbld";*/
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
 *   MODULE_NAME:		xfsckint.h
 *
 *      prototypes for functions NOT INTENDED FOR CONSUMPTION
 *                               OUTSIDE THE FSCK UTILITY
 *
 *   COMPONENT_NAME: 	jfschk
 *
 */

#ifndef H_XFSCKINT
#define H_XFSCKINT


#ifndef _JFS_FSCKDIRE
#include "xfsck.h"
#else
typedef int    retcode_t;
#endif

#include "xchkdsk.h"
#include "fsckmsgc.h"
#include "fsckpfs.h"
#include "fsckwsp.h"



/*

  ---------------------- functions defined in fsckbmap ---------------------

*/

retcode_t rebuild_blkall_map ( void );

retcode_t verify_blkall_map ( void );


/*

  ---------------------- functions defined in fsckcbbl ---------------------

*/

retcode_t ClrBdBlkLst_processing ( void );

retcode_t check_BdBlkLst_FillLevel ( void );


/*

  ---------------------- functions defined in fsckconn ---------------------

*/

int32 adjust_parents ( fsck_inode_recptr, inoidx_t );

int32 check_connectedness ( void );

int32 check_dir_integrity ( void );

int32 check_link_counts ( void );



/*

  ---------------------- functions defined in fsckdire ---------------------

*/

int32 fsck_dtDelete( dinode_t *, component_t *, ino_t * );

int32 fsck_dtInsert( dinode_t *, component_t *, ino_t * );



/*

  ---------------------- functions defined in fsckdtre ---------------------

*/

retcode_t direntry_add ( dinode_t *, ino_t, UniChar * );

retcode_t direntry_get_inonum ( ino_t,  int,  UniChar *,  int, 
                                UniChar *, ino_t *); 

retcode_t direntry_get_objnam ( ino_t, ino_t, int *, UniChar *); 

retcode_t direntry_remove ( dinode_t *,  ino_t ); 

retcode_t dTree_processing ( dinode_t *, inoidx_t,fsck_inode_recptr,
                             fsck_msg_info_ptr, int ); 

retcode_t dTree_search( dinode_t *, UniChar *,  reg_idx_t, UniChar *,  
                        reg_idx_t,   dtslot_t **,  int8 *,   fsck_inode_recptr );

retcode_t find_first_dir_leaf ( dinode_t *,  dtpage_t **, fsblkidx_t *, 
                                int8 *, int8 * ); 

void init_dir_tree( dtroot_t * );  

retcode_t process_valid_dir_data ( dinode_t *,inoidx_t, fsck_inode_recptr, 
				fsck_msg_info_ptr, int ); 

retcode_t reconnect_fs_inodes( void );


/*

  ---------------------- functions defined in fsckimap ---------------------

*/

retcode_t AIS_redundancy_check( void );

retcode_t AIS_replication( void );

retcode_t rebuild_agg_iamap ( void );

retcode_t rebuild_fs_iamaps ( void );

retcode_t record_dupchk_inode_extents ( void );

retcode_t verify_agg_iamap ( void );

retcode_t verify_fs_iamaps ( void );


/*

  ---------------------- functions defined in fsckino ---------------------

*/

#define inode_type_recognized(iptr)\
              ( ISDIR(iptr->di_mode) || \
                ISREG(iptr->di_mode) || \
                ISLNK(iptr->di_mode)       )

retcode_t backout_ACL ( dinode_t  *, fsck_inode_recptr ); 

retcode_t backout_EA ( dinode_t  *, fsck_inode_recptr ); 
                          
retcode_t calculate_dasd_used ( void );  

retcode_t clear_ACL_field ( fsck_inode_recptr, dinode_t * );  
                          
retcode_t clear_EA_field ( fsck_inode_recptr, dinode_t * );  
                          
retcode_t display_path( inoidx_t, int, ino_t, char  *, 
                        fsck_inode_recptr ); 
                          
retcode_t display_paths ( inoidx_t, fsck_inode_recptr, fsck_msg_info_ptr );

retcode_t first_ref_check_inode ( dinode_t *, inoidx_t, fsck_inode_recptr,    
                                  fsck_msg_info_ptr );

retcode_t get_path ( inoidx_t, ino_t,  char **, fsck_inode_recptr ); 

retcode_t in_inode_data_check( fsck_inode_recptr, fsck_msg_info_ptr );

retcode_t inode_is_in_use ( dinode_t *, ino_t ); 

retcode_t parent_count( fsck_inode_recptr );

retcode_t record_valid_inode ( dinode_t *, inoidx_t, fsck_inode_recptr, 
                               fsck_msg_info_ptr );

retcode_t release_inode ( inoidx_t, fsck_inode_recptr, dinode_t * );

retcode_t unrecord_valid_inode ( dinode_t *, inoidx_t, fsck_inode_recptr, 
                                 fsck_msg_info_ptr );

retcode_t validate_ACL ( dinode_t  *, inoidx_t, fsck_inode_recptr,    
                        fsck_msg_info_ptr );
                          
retcode_t validate_dasd_used ( void );  
                       
retcode_t validate_data ( dinode_t  *, inoidx_t, fsck_inode_recptr, 
                          fsck_msg_info_ptr );

retcode_t validate_dir_data ( dinode_t  *, inoidx_t, fsck_inode_recptr,    
                              fsck_msg_info_ptr );

retcode_t validate_EA ( dinode_t  *, inoidx_t, fsck_inode_recptr,    
                        fsck_msg_info_ptr );
                       
retcode_t validate_record_fileset_inode ( ino_t, inoidx_t, dinode_t *,
                                          fsck_msg_info_ptr );

/*

  ---------------------- functions defined in fsckmeta ---------------------

*/

retcode_t agg_clean_or_dirty ( void );

retcode_t fatal_dup_check ( void );

retcode_t first_ref_check_agg_metadata ( void );

retcode_t first_ref_check_fixed_metadata ( void );

retcode_t first_ref_check_fs_metadata ( void );

retcode_t first_ref_check_inode_extents ( void );

retcode_t record_fixed_metadata (void );

retcode_t replicate_superblock ( void );

retcode_t validate_fs_metadata ( void );

retcode_t validate_repair_superblock ( void );

retcode_t validate_select_agg_inode_table ( void );


/*

  ---------------------- functions defined in fsckmsgs ---------------------

*/

void fsck_send_msg( int, int, int );


/*

  ---------------------- functions defined in fsckpfs ---------------------

*/

retcode_t ait_node_get ( fsblkidx_t, xtpage_t  * );

retcode_t ait_node_put ( fsblkidx_t, xtpage_t  * );

retcode_t ait_special_read_ext1 ( int );  

void alloc_high_mem( char **, int32 * );

retcode_t blkmap_find_bit( fsblkidx_t, mappgidx_t *, reg_idx_t *, fsck_bitmask_ptr ); 

retcode_t blkmap_flush( void );

retcode_t blkmap_get_ctl_page ( blk_cntrlptr );

retcode_t blkmap_get_page( mappgidx_t, blk_pageptr *); 

retcode_t blkmap_put_ctl_page ( blk_cntrlptr );

retcode_t blkmap_put_page( mappgidx_t ); 

retcode_t blktbl_ctl_page_put ( dbmap_t * ); 

retcode_t blktbl_dmap_get( fsblkidx_t, dmap_t ** );

retcode_t blktbl_dmap_put( dmap_t * );

retcode_t blktbl_dmaps_flush( void );

retcode_t blktbl_Ln_page_get( int8, fsblkidx_t, dmapctl_t ** );

retcode_t blktbl_Ln_page_put( dmapctl_t * );

retcode_t blktbl_Ln_pages_flush( void );

retcode_t close_volume( void );

retcode_t default_volume( void );

retcode_t dnode_get ( fsblkidx_t, reg_idx_t, dtpage_t ** );  

retcode_t ea_get ( fsblkidx_t, reg_idx_t, char *, reg_idx_t *, 
                   reg_idx_t *, agg_byteidx_t * );

retcode_t  fscklog_put_buffer ( void );

retcode_t iag_get ( int, int, int, iagno_t,  iag_t ** ); 

retcode_t iag_get_first ( int, int,  int,  iag_t ** );   

retcode_t iag_get_next ( iag_t ** );

retcode_t iag_put ( iag_t * );  

retcode_t iags_flush ( void );

retcode_t inodes_flush ( void );

retcode_t inode_get ( int, int, inoidx_t, dinode_t ** );  

retcode_t inode_get_first_fs ( int, inoidx_t *,dinode_t ** ); 

retcode_t inode_get_next ( inoidx_t *, dinode_t ** ); 

retcode_t inode_put ( dinode_t * );   

retcode_t inotbl_get_ctl_page ( int, dinomap_t ** ); 

retcode_t inotbl_put_ctl_page ( int, dinomap_t * ); 

retcode_t mapctl_get ( fsblkidx_t, void  ** );      

retcode_t mapctl_put ( void * );  

retcode_t mapctl_flush ( void );

retcode_t node_get ( fsblkidx_t, xtpage_t ** );   

retcode_t open_volume ( char * ); 

retcode_t readwrite_device( agg_byteidx_t, reg_idx_t, reg_idx_t *, 
                            void *, int);       

retcode_t recon_dnode_assign ( fsblkidx_t, dtpage_t ** );

retcode_t recon_dnode_get( fsblkidx_t, dtpage_t ** );

retcode_t recon_dnode_put( dtpage_t * );

retcode_t recon_dnode_release( dtpage_t * );

retcode_t refresh_bootsec ( void );


/*

  ---------------------- functions defined in fsckruns ---------------------

*/

void fsck_hbeat_start ( void );

void fsck_hbeat_stop ( void );


/*

  ---------------------- functions defined in fsckwsp ---------------------

*/

int alloc_vlarge_buffer( void ); 

int alloc_wrksp( uint32, int, int, void ** );	/* called from both chkdsk modules
						 * and from logredo modules
						 */

/* 3 lines deleted    @F1 */

retcode_t blkall_decrement_owners ( fsblkidx_t );

retcode_t blkall_increment_owners ( fsblkidx_t );

retcode_t blkall_ref_check ( fsblkidx_t, int * );

retcode_t dire_buffer_alloc( dtpage_t ** );

retcode_t dire_buffer_release( dtpage_t * );

retcode_t directory_buffers_alloc( void );

retcode_t directory_buffers_release( void );

retcode_t dtreeQ_dequeue( dtreeQ_elem_ptr * );

retcode_t dtreeQ_enqueue( dtreeQ_elem_ptr );

retcode_t dtreeQ_get_elem ( dtreeQ_elem_ptr * );

retcode_t dtreeQ_rel_elem ( dtreeQ_elem_ptr );

retcode_t establish_agg_workspace ( void );

retcode_t establish_ea_iobuf ( void );

retcode_t establish_fs_workspace ( void );

retcode_t establish_io_buffers ( void );

retcode_t establish_wsp_block_map_ctl ( void );

retcode_t extent_record ( fsblkidx_t, fsblkidx_t );

retcode_t extent_unrecord ( fsblkidx_t, fsblkidx_t );

retcode_t fsck_alloc_fsblks ( int32, fsblkidx_t * );

retcode_t fsck_dealloc_fsblks ( int32, fsblkidx_t );

retcode_t fscklog_end ( void );

retcode_t fscklog_init ( void );

retcode_t fscklog_start ( void );

retcode_t get_inode_extension ( fsck_inode_extptr * );

retcode_t get_inorecptr ( int, int, inoidx_t, fsck_inode_recptr * );

retcode_t get_inorecptr_first ( int, inoidx_t *, fsck_inode_recptr * );

retcode_t get_inorecptr_next ( int, inoidx_t *, fsck_inode_recptr * );

retcode_t init_agg_record ( void );

retcode_t process_extent( fsck_inode_recptr, uint32, fsblkidx_t, int8, 
                          int8, fsck_msg_info_ptr, uint32 *, int8 *, int ); 

retcode_t release_inode_extension ( fsck_inode_extptr );

retcode_t release_logredo_allocs( void );

retcode_t temp_inode_buf_alloc( char ** );

retcode_t temp_inode_buf_release( char * );

retcode_t temp_node_buf_alloc( char ** );

retcode_t temp_node_buf_release( char * );

retcode_t treeQ_dequeue( treeQ_elem_ptr * );

retcode_t treeQ_enqueue( treeQ_elem_ptr );

retcode_t treeQ_get_elem ( treeQ_elem_ptr * );

retcode_t treeQ_rel_elem ( treeQ_elem_ptr );

retcode_t treeStack_pop ( fsck_inode_recptr * );

retcode_t treeStack_push ( fsck_inode_recptr );

retcode_t workspace_release ( void );



/*

  ---------------------- functions defined in fsckxtre ---------------------

*/

retcode_t find_first_leaf ( dinode_t *, xtpage_t **,  fsblkidx_t *, 
                            int8 *, int8 * ); 

retcode_t init_xtree_root( dinode_t * );

retcode_t process_valid_data ( dinode_t *, inoidx_t, fsck_inode_recptr,
                               fsck_msg_info_ptr, int ); 

retcode_t xTree_processing( dinode_t *, inoidx_t, fsck_inode_recptr,
                            fsck_msg_info_ptr, int ); 

retcode_t xTree_search( dinode_t *, fsblkidx_t, xad_t **, int8 * ); 


/*

  ---------------------- functions defined in xchkdsk.c ---------------------

*/

void report_readait_error( retcode_t, retcode_t, int8 );

#endif
