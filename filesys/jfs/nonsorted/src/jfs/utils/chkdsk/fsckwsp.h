/* $Id: fsckwsp.h,v 1.2 2004/03/21 02:43:24 pasha Exp $ */

/* static char *SCCSID = "@(#)1.20.1.2  12/2/99 11:13:56 src/jfs/utils/chkdsk/fsckwsp.h, jfschk, w45.fs32, fixbld";*/
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
 *   MODULE_NAME:		fsckwsp.h
 *
 *   COMPONENT_NAME: 	jfschk
 *
 *
 */
#ifndef H_FSCKWSP
#define	H_FSCKWSP

#include "fscklog.h"


/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * fsck uses the structures and types in this file to define its workspace.
 *
 *++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 */

/*----------------------------------------------------------------------------
 * type dtree_Q_elem describes one node which is to be visited in the
 *                  traversal of a directory B+ Tree
 *
 */
typedef struct dtreeQelem {
   struct dtreeQelem *next;
   struct dtreeQelem *prev;
   fsblkidx_t node_addr;
   pxd_t       node_pxd;
   uint8      node_key_len;
   UniChar    node_key[JFS_NAME_MAX];
   int8       node_level;
   reg_idx_t  node_size;
   } dtreeQelem_t;

typedef struct dtreeQelem *dtreeQ_elem_ptr;

#define dtreeQ_elem_length sizeof(struct dtreeQelem)

/*----------------------------------------------------------------------------
 * type tree_Q_elem describes one node which is to be visited in the
 *                  traversal of a non-directory B+ Tree
 *
 */
typedef struct treeQelem {
   struct treeQelem *next;
   struct treeQelem *prev;
   fsblkidx_t node_addr;
   fsblkidx_t node_first_offset;
   pxd_t       node_pxd;
   int8   node_level;
   int8   last_ext_uneven;
   } treeQelem_t;

typedef struct treeQelem *treeQ_elem_ptr;

#define treeQ_elem_length sizeof(struct treeQelem)

/*----------------------------------------------------------------------------
 * type dupall_blkrec describes one aggregate block which has been found to
 *                    have more than 1 inode claiming to own it.
 *
 */
typedef struct dupall_blkrec {
   fsblkidx_t            blk_number;
   uint8                 first_ref_resolved;
   uint8                 avail_byte;
   uint16                owner_count;
   struct dupall_blkrec *next;
   } dupall_blkrec_t;

typedef struct dupall_blkrec *dupall_blkrec_ptr;
typedef struct dupall_blkrec *dupall_blkrec_tkn;

#define dupall_blkrec_length sizeof(struct dupall_blkrec)


/*----------------------------------------------------------------------------
 *
 * Each (4096 byte) page in the block map describes 32768
 * blocks in the aggregate.
 */

/*
 * the fsck Workspace Block Map control page
 *
 */

typedef struct fsck_blk_map_hdr {
	/*
	 * the 1st 1024 bytes are the clrbblks to chkdsk
	 * communication area 
	 */
   struct fsckcbbl_record  cbblrec; 	/* 128 */
   char		avail_0[896]; 		/*  896 */
				/* 1024 byte boundary */
	/*
	 * the next 2048 bytes are the chkdsk block map
	 * header area
	 */
   struct {
     char        eyecatcher[8];               /*    8 */
     int32      last_entry_pos;                /*    4 */
     int32      next_entry_pos;               /*    4 */
     char       start_time[20];                /*   20 */                // @D2
     char       end_time[20];                  /*   20 */                // @D2
     int32      return_code;                   /*    4 */
     char      *super_buff_addr;             /*    4 -- for the dump reader */
     char      *agg_record_addr;            /*    4 -- for the dump reader */
     char      *bmap_record_addr;          /*    4 -- for the dump reader */  
     char       avail_1[8];                     /*   8 */               // @D2
     agg_byteidx_t fscklog_agg_offset;  /*    8 */
     int8       fscklog_full;                     /*    1 */
     int8       fscklog_buf_allocated;      /*    1 */
     int8       fscklog_buf_alloc_err;       /*    1 */
     char       avail_2[1];                      /*    1 */
     int32      num_logwrite_errors;        /*    4 */
     char       avail_3[32];                     /*   32 */

     fscklog_error_t logerr[125];            /* 1920 = 120 * 16 */
     } hdr;                                           /* subtotal, 2048 bytes */
	/*
	 * the last 1024 bytes are available 
	 */
   char		avail_5[1024]; 		/*  1024 */
   } fsck_blk_map_hdr_t;                      /*   total: 4096 bytes */

#define fbmh_eyecatcher_string "wspblkmp"

/*
 * a page in the fsck Workspace Block Map
 */
typedef struct fsck_blk_map_page {
   fsck_dword_bitmap_t fsck_blkmap_words[1024];  /* 4096 bytes */
   } fsck_blk_map_page_t;

typedef struct fsck_blk_map_hdr       *blk_cntrlptr;
typedef struct fsck_blk_map_page      *blk_pageptr;


/*----------------------------------------------------------------
 * structure blkmap_wspace_t describes the portion of the aggregate
 *                           record allocated for use when verifying
 *                           the aggregate block map.
 *
 */
typedef struct blkmap_wspace {
     uint32     dmap_map[LPERDMAP];        /* bit map of blk allocations */
                                    
     int8       dmap_wsp_tree[TREESIZE - LPERDMAP];
     int8       dmap_wsp_leafs[LPERDMAP]; 
     int8       L0_wsp_tree[CTLTREESIZE - LPERCTL];
     int8       L0_wsp_leafs[LPERCTL];     
     int8       L1_wsp_tree[CTLTREESIZE - LPERCTL];
     int8       L1_wsp_leafs[LPERCTL];     
     int8       L2_wsp_tree[CTLTREESIZE - LPERCTL];
     int8       L2_wsp_leafs[LPERCTL];    
     fsblkidx_t AG_free[MAXAG];
     }  blkmap_wspace_t;


/*----------------------------------------------------------------
 * structure fsck_iag_record describes one Inode Allocation
 *                           Group belonging to an inode table in
 *                           the aggregate
 *
 */
struct fsck_iag_record {
   reg_idx_t AG_num;
   reg_idx_t backed_inodes;
   reg_idx_t unused_backed_inodes;
   reg_idx_t unbacked_extents;
   };

typedef struct fsck_iag_record *fsck_iag_recptr;

#define iag_record_length sizeof(struct fsck_iag_record)

/*----------------------------------------------------------------------------
 * type fsck_iag_tbl is an array of fsck_iag_records.
 *
 *
 * There is one entry in this table for each iag allocated for the
 * owner (either the aggregate or the fileset).
 *
 * The number of elements in an array of this type is not known until
 * run-time.  Therefore, the storage will be obtained dynamically and
 * then referenced via a variable of this type.
 */
typedef struct fsck_iag_record fsck_iag_tbl;

/*----------------------------------------------------------------
 * structure fsck_ag_record describes one Allocation Group
 *                           as it relates to an inode table in the
 *                           aggregate
 *
 */
struct fsck_ag_record {
   reg_idx_t backed_inodes;
   reg_idx_t unused_backed_inodes;
   iagno_t   frext_list_first;
   iagno_t   frext_list_last;
   int32     frext_list_len;
   iagno_t   frino_list_first;
   iagno_t   frino_list_last;
   int32     frino_list_len;
   int8      frext_list_bad;
   int8      frino_list_bad;
   char      unused[14];
   };

typedef struct fsck_ag_record *fsck_ag_recptr;

#define ag_record_length sizeof(struct fsck_ag_record)

/*----------------------------------------------------------------------------
 * type fsck_ag_tbl is an array of fsck_ag_records.
 *
 *
 * There is one entry in this table for each ag in the aggregate
 *
 * The number of elements in an array of this type is not known until
 * run-time.  Therefore, the storage will be obtained dynamically and
 * then referenced via a variable of this type.
 */
typedef struct fsck_ag_record fsck_ag_tbl;

/*----------------------------------------------------------------
 * structure fsck_iam_record describes one Inode Allocation Map
 *                           in the aggregate
 *
 */
struct fsck_iam_record {
   agg_byteidx_t first_leaf_offset; /* offset of first leaf for imap inode */
   int8          imap_is_rootleaf;
   int8          friag_list_bad;
   char          unused[2];
   int32         bkd_inodes;        /* count of backed inodes    */
   int32         unused_bkd_inodes; /* count of available backed inodes */
   int32         num_iags;          /* number of IAGs in the inode tbl */
   iagno_t       friag_list_last;
   int32         friag_list_len;
   fsck_iag_tbl *iag_tbl;     /* ptr to info by iag              */
   fsck_ag_tbl  *ag_tbl;      /* ptr to info by ag               */
   };

typedef struct fsck_iam_record fsck_iamrec_t;
typedef struct fsck_iam_record *fsck_iam_recptr;

#define iam_record_length sizeof(struct fsck_iam_record)


/*----------------------------------------------------------------
 * structure fsck_inode_record describes one inode belonging to
 *                             the aggregate or to a fileset
 *                             in the aggregate
 *
 */

    /*
     * the possible values for inode_type
     *   (since inode_type is 4 bits wide, valid range is 0-15)
     */
#define unused_inode        0
#define file_inode          1
#define strm_descr_inode    2
#define stream_inode        3
#define directory_inode     4
#define symlink_inode       5
#define metadata_inode     14
#define unrecognized_inode 15


#define add_direntry_extension 'A'
#define rmv_direntry_extension 'R'
#define rmv_badentry_extension 'B'
#define parent_extension 'P'

struct fsck_inode_ext_record {
   char       ext_type;            /* 1  : extens type (see constants above) */
   unsigned   ino_type:4;        /*  .5: see constants above  
                                          *      describes the inonum inode.
                                          *      This is only interesting when
                                          *      type is rmv_direntry_extension.
                                          */
   unsigned   unused_byte:20;      /* 2.5: unused      */
   struct fsck_inode_ext_record  *next;  /*
                                          * 4  : addr next extension record
                                          */
   ino_t      inonum;               /* 4  : inode number */
   };

typedef struct fsck_inode_ext_record *fsck_inode_extptr;

      /* 
      * the inode record data
      */
struct fsck_inode_record {
   ino_t                     inonum;        /* 4.0 : key         */
                                                                      /* 6 lines deleted         @F1 */
                                /* 3 : 24 flag bits */
   unsigned in_use:1;
   unsigned selected_to_rls:1;
   unsigned crrct_link_count:1;
   unsigned crrct_prnt_inonum:1;

   unsigned adj_entries:1;
   unsigned cant_chkea:1;
   unsigned clr_ea_fld:1;
   unsigned clr_acl_fld:1;

   unsigned ignore_ea_blks:1;
   unsigned ignore_acl_blks:1;
   unsigned ignore_alloc_blks:1;
   unsigned inline_data_err:1;

   unsigned reconnect:1;
   unsigned unxpctd_prnts:1;
   unsigned badblk_inode:1;
   unsigned involved_in_dups:1;

   unsigned crrct_cumm_blks:1;
   unsigned avail_flagbit_1:1;                    /* @F1 */
   unsigned inlineea_on:1;
   unsigned inlineea_off:1;

   unsigned avail_flagbits:4;
                                /*        end of flag bits           */
   int8 inode_type;       /* 1 : see constants above          */

   int32    link_count;    /* 4 : Stored link count is added to this.
                                 *    Decremented for each observed link.
                                 */
   ino_t    parent_inonum;    /* 
                                 * 4 : If this is a directory inode and
                                 * the unxpctd_prnts bit is set, then
                                 * this field contains the inode number
                                 * stored in the parent field on disk.
                                 * Otherwise, this is the inode number
                                 * of the first observed parent.
                                */
   int64    cumm_blocks;  /*   @F1
			 * 8 : number of blocks allocated to the 
			 * inode and, if this is a directory, to each
			 * inode described in an entry
			 */
   int32    dtree_level;	/*     @F1
			 * 4 : distance, in the fileset directory tree,
			 * from the root (inode 2)
			 */
   fsck_inode_extptr ext_rec;   /* 4.0: extension record address         */
    };


typedef struct fsck_inode_record *fsck_inode_recptr;

#define inode_record_length sizeof(struct fsck_inode_record)

#define inode_is_metadata( X ) \
             ( (int8) ((X)->inode_type) == (int8) metadata_inode )


                                                                      /* 9 lines deleted         @F1 */


/*----------------------------------------------------------------------------
 * type inode_tbl_t is an array of pointers to inode records
 *
 * There is one of these tables for each allocated inode extent which
 * has at least one inode in use.
 *
 */
struct inode_tbl_t {
    char               eyecatcher[8];   /* 8.0                                   @F1 */
    fsck_inode_recptr   inorectbl[32];  /* 128                               @F1 */      
   };                                                                                  /* @F1 */

typedef struct inode_tbl_t *ino_tblptr;                                   /* @F1 */

#define inode_tbl_length sizeof( struct inode_tbl_t )               /* @F1 */

/*----------------------------------------------------------------------------
 * type inode_ext_tbl_t is an array of pointers to inode tables
 *
 * There is one of these tables for each allocated IAG which
 * has at least one allocated extent with an inode in use.
 *
 */
struct inode_ext_tbl_t {
    char               eyecatcher[8];  /* 8.0                                     @F1 */
    ino_tblptr        inotbl[128];       /* 512                                    @F1 */
   };                                                                                   /* @F1 */

typedef struct inode_ext_tbl_t *inoext_tblptr;                          /* @F1 */

#define inode_ext_tbl_length sizeof( struct inode_ext_tbl_t )      /* @F1 */

/*----------------------------------------------------------------------------
 * type IAG_tbl_t is an array of pointers to inode extent tables
 *
 * There is one of these tables for each fileset in the aggregate,
 * and is allocated dynamically when the number of IAGs in the
 * fileset Inode Map is known.
 *
 */
struct IAG_tbl_t {
    char               eyecatcher[8];  /* 8.0                         @F1 */
    inoext_tblptr    inoext_tbl[];       /*  4.0                     @F1 */
   };                                                                      /* @F1 */

typedef struct IAG_tbl_t  *IAG_tblptr;                        /* @F1 */


/*----------------------------------------------------------------------------
 * type fsck_extent_record describes one extent of storage which fsck
 *                         has allocated for its workspace.
 *
 */
typedef struct wsp_ext_rec {
   struct wsp_ext_rec *next;
   int8            from_high_memory;
   int8            for_logredo;
   char           avail[2];
   reg_idx_t    extent_length;
   char         *extent_addr;
   reg_idx_t    last_byte_used;
   } fsck_extent_record;

typedef struct wsp_ext_rec *wsp_ext_rec_ptr;


/*----------------------------------------------------------------------------
 * type recon_buf_record   describes one dnode I/O buffer used during 
 *                         inode reconnect processing 
 *
 */
typedef struct recon_buf_record {

   dtpage_t    dnode_buf;

   struct recon_buf_record *stack_next;
   fsblkidx_t               dnode_blkoff;
   agg_byteidx_t            dnode_byteoff;
   char                     reserved[12];
   };

typedef struct recon_buf_record *recon_bufrec_ptr;


/*----------------------------------------------------------------------------
 * type treeStack_record describes one stack element used during DASD 
 *                         limits processing 
 *
 */
typedef struct treeStack_record {
   fsck_inode_recptr	inorec;
   struct treeStack_record *next;
   };

typedef struct treeStack_record *treeStack_ptr;

#define treeStack_elem_length sizeof(struct treeStack_record)


/*----------------------------------------------------------------------------
 * structure fsck_agg_record describes the aggregate
 */

typedef struct   fsck_agg_record {
   char          eyecatcher[8];
   reg_idx_t     ondev_jlog_fsblk_length;
   fsblkidx_t    ondev_jlog_fsblk_offset;
   agg_byteidx_t ondev_wsp_byte_length;
   agg_byteidx_t ondev_wsp_byte_offset;
   reg_idx_t     ondev_wsp_fsblk_length;
   fsblkidx_t    ondev_wsp_fsblk_offset;
   reg_idx_t     ondev_fscklog_fsblk_length;
   fsblkidx_t    ondev_fscklog_fsblk_offset;
   agg_byteidx_t ondev_fscklog_byte_length;
   agg_byteidx_t ondev_fscklog_byte_offset;
   reg_idx_t     log2_blksize;
   reg_idx_t     blksperpg;
   reg_idx_t     log2_blksperpg;
   reg_idx_t     log2_blksperag;
   fsblkidx_t    sb_agg_fsblk_length;
   uint32        inode_stamp;       /* taken from the agg self inode, every
                                     * in-use, valid inode must have a 
                                     * matching di_inostamp.
                                     */
   int           inode_count;       /* num inodes owned by the aggregate */
   int           inodes_inuse;      /* num aggregate nodes now in use    */
   int           fset_inodes_inuse; /* num fileset inodes in use    */
   int           fset_inodes_seen;  /* num fileset inodes scanned.   */
   int           fset_inode_count;  /* num inodes in the filesets    */
   fsblkidx_t    dup_block_count;   /* num multiply-allocated
                                     * blocks seen
                                     */
   fsblkidx_t    unresolved_1stref_count; /* num unresolved first
                                  * references to multiply allocated
                                  * blocks
                                  */
   char         aggrec_rsvd0[4];
   fsblkidx_t   blocks_for_inodes;        /* by fsck's count             */
   fsblkidx_t   blocks_for_files;         /* by fsck's count             */
   fsblkidx_t   blocks_for_dirs;          /* by fsck's count             */
   fsblkidx_t   blocks_for_eas;           /* by fsck's count             */
   fsblkidx_t   blocks_for_acls;          /* by fsck's count             */
   long_idx_t   inodes_in_aggregate;      /* by fsck's count             */
   long_idx_t   files_in_aggregate;       /* by fsck's count             */
   long_idx_t   dirs_in_aggregate;        /* by fsck's count             */
   fsblkidx_t   free_blocks_in_aggregate; /* by fsck's count             */
   fsblkidx_t   blocks_used_in_aggregate; /* by fsck's count             */
   long_idx_t   blocks_this_fset;         /* by fsck's count             */

   int32        logredo_rc;         /* logredo return code             */
   int32        logformat_rc;       /* logformat return code           */
   char         aggrec_rsvd2[8];

   struct {           /* data for the current inode          */
           char       eyecatcher[8];
           fsblkidx_t all_blks;         /* all blocks allocated              */
           fsblkidx_t data_blks;        /* blocks allocated to data          */
           long_idx_t data_size;        /* byte offset of last data if file  */
           fsblkidx_t ea_blks;          /* blocks allocated to EAs           */
           fsblkidx_t acl_blks;          /* blocks allocated to ACLs           */
           uint16     ea_inline;
           uint16     inline_ea_offset;
           uint16     inline_ea_length; 
           uint16     acl_inline;
           uint16     inline_acl_offset;
           uint16     inline_acl_length; 
           uint16     in_inode_data_length; /* length of inline data or of
                                       * xad list in the inode
                                       */
           char       aggrec_rsvd4[2];
           } this_inode;

   wsp_ext_rec_ptr  recon_buf_extent;
   recon_bufrec_ptr recon_buf_stack;

   char           agg_imap_eyecatcher[8];
   fsck_iamrec_t  agg_imap;     /* describes the aggregate imap    */

   char           fset_imap_eyecatcher[8];
   fsck_iamrec_t  fset_imap;    /* describes the fileset imap      */

   reg_idx_t    num_ag;               /* number of AGs in the aggregate  */
   int          ag_blk_size;          /* aggregate block size            */

   short int    aggrec_rsvd6[1];
   char         delim_char;           /* path delimiter char             */
                          /* unicharacter equivalents                    */
   UniChar     *UniChar_lsfn_name;    /* equiv to lost+found             */
   UniChar     *UniChar_LSFN_NAME;    /* equiv to LOST+FOUND             */
                          /* end of unicharacter equivalents             */
   int64        lsfn_dasdused_adjustment;
   ino_t        lsfn_inonum;        /* inode number of /lost+found/      */
   ino_t        avail_inonum;       /* inode number of an inode observed
                                     * to be allocated and not in use (if
                                     * any)
                                     */
   short int    parm_options[16];   /* parms specified                 */
   char         aggrec_rsvd7[2];    /* pad to quadword boundary    */
   char         effective_msg_level;
   char         aggrec_rsvd8[7];    /* pad to quadword boundary    */
   char         flags_eyecatcher[8];
                          /* 4 bytes of flags   */
   unsigned     parm_options_nologredo:1;  /* first byte, first nibble */
   unsigned     processing_readonly:1;
   unsigned     processing_readwrite:1;
   unsigned     messaging_verbose:1;
   unsigned     superblk_ok:1;             /* first byte, second nibble */
   unsigned     aggregate_is_mounted:1;
   unsigned     ag_modified:1;
   unsigned     ag_dirty:1;
   unsigned     lsfn_ok:1;                 /* second byte, first nibble */
   unsigned     cant_write_primary_sb:1;
   unsigned     cant_write_secondary_sb:1;
   unsigned     cant_write_primary_ait:1;
   unsigned     nonmeta_write_error_msg:1; /* second byte, second nibble */
   unsigned     fsck_is_done:1;
   unsigned     device_is_open:1;
   unsigned     device_is_locked:1;
   unsigned     primary_ait_4part1:1;      /* third byte, first nibble */
   unsigned     primary_ait_4part2:1;      
   unsigned     ait_aim_update_failed:1;
   unsigned     corrections_needed:1;
   unsigned     corrections_approved:1;    /* third byte, second nibble */
   unsigned     avail_inode_found:1;   
   unsigned     initializing_fscklog:1;
   unsigned     warning_pending:1;         
   unsigned     high_mem_allocated:1;      /* fourth byte, first nibble */
   unsigned     rootdir_rebuilt:1;
   unsigned     active_dasd_limits:1;
   unsigned     stdout_redirected:1;            
   unsigned     prime_dasd_limits:1;        /* fourth byte, second nibble    @F1 */
   unsigned     unused_4_2:3;                /*                                          @F1 */         
                          /* end of flag bytes  */
   int          path_buffer_length; /* length of path_buffer          */
   char        *path_buffer;  /* storage allocated (if any) for
                               * assembling the string containing
                               * an inode's path for a message
                               */
   xtpage_t    *prim_nodeptr;  /* storage allocated (if any) for 
                               * an xtpage of an inode in the Primary Agg 
                               * Inode table
                               */
   xtpage_t    *second_nodeptr;  /* storage allocated (if any) for 
                               * an xtpage of an inode in the Secondary 
                               * Agg Inode table
                               */
   agg_byteidx_t lowest_valid_fset_datablk; /* the lowest (aggregate
                               * blocksized) block in the aggregate
                               * AFTER the blocks used for initial
                               * metadata
                               */
   agg_byteidx_t highest_valid_fset_datablk; /* the highest (aggregate
                               * blocksized) block in the aggregate before
                               * the beginning of the fsck workspace
                               */
   fsck_inode_extptr free_inode_extens; /* available workspace inode record
                               * extensions.
                               */
   fsck_inode_extptr inode_reconn_extens;  /* list of inode record
                               * extensions describing the directory
                               * entries to be added when reconnecting
                               * inodes
                               */
   treeStack_ptr       treeStack; /* head of the stack used during tree
                               * traversal
                               */
   treeStack_ptr       free_treeStack; /* available tree stack elements */
   int32	                  tree_height;
   dtreeQ_elem_ptr    dtreeQ_front;
   dtreeQ_elem_ptr    dtreeQ_back;
   dtreeQ_elem_ptr    free_dtreeQ; /* available dtree stack elements     */
   treeQ_elem_ptr     treeQ_front;
   treeQ_elem_ptr     treeQ_back;
   treeQ_elem_ptr     free_treeQ; /* available tree stack elements     */
   dupall_blkrec_t  *dup_alloc_lst; /* list of multiply allocated blocks */
   dupall_blkrec_t  *free_dupall_blkrec; /* free dupall_blkrec records   */
   wsp_ext_rec_ptr   wsp_extent_list; /* linked list of records describing
                               * the workspace extents.
                               */
   char         aggrec_rsvda[4];          /* pad to quadword boundary    */

   char             AIT_eyecatcher[8];   /*                                                @F1 */
   ino_tblptr      AIT_ext0_tbl;         /* ptr to table for Agg Inode Extent 0 @F1 */
   int32             agg_last_inoidx;     /* used for find first, find next           @F1 */

   char             FSIT_eyecatcher[8];  /*                                                 @F1 */
   IAG_tblptr      FSIT_IAG_tbl;        /* ptr to table for FSet IAGs                @F1 */
   int32             fs_last_iagidx;        /* used for find first, find next            @F1 */
   int32             fs_last_extidx;        /* used for find first, find next            @F1 */
   int32             fs_last_inoidx;        /* used for find first, find next             @F1 */

   struct {           /* data for accessing all allocated inodes in an
                       * inode table sequentially
                       */
           char          eyecatcher[8];
           iagno_t       this_iagnum;    /* ordinal of the current iag */
           inoidx_t      this_inoidx;    /* ordinal of the current inode */
           reg_idx_t     rootleaf_imap;  /* 0 => not a rootleaf imap */
           xtpage_t     *this_mapleaf;   /* current mapleaf in buffer */
           iagno_t       iagidx_now;     /* index into current mapleaf */
           iagno_t       iagidx_max;
           iag_t        *iagptr;         /* current iag in buffer */
           reg_idx_t     extidx_now;     /* index into current iag */
           reg_idx_t     extidx_max;
           dinode_t     *extptr;         /* current inode extent in buffer */
           inoidx_t      inoidx_now;     /* index into current inode extent */
           inoidx_t      inoidx_max;
           dinode_t     *inoptr;         /* current inode in buffer */
           } fais;    /* (For Allocated Inodes Sequentially */

   char         aggrec_rsvde[4];          /* pad to quadword boundary    */

       /*
        * The fsck I/O buffer information is below.  Certain information
        * is kept for each of the buffers:
        *
        *    %_buf_ptr      = buffer address
        *    %_buf_length   = buffer length
        *    %_buf_data_len = length of data read into buffer at last read
        *    %_agg_offset   = byte offset in aggregate of buffer contents
        *    %_buf_write    = flag: !0 => buffer contents modified since
        *                                 last write to device
        *
        * where  % = {blkmp, iag, mapleaf, ino, node, wsp}
        *
        *
        * N.B. To ensure proper boundary alignment, each IO buffersize
        *      must be an even number of pages (i.e. 4096 byte pages).
        *
        */

           /* ****** very large, multi purpose BUFFER ***** */

#define VLARGE_BUFSIZE (2 * MEMSEGSIZE)

#define NOT_CURRENTLY_USED  0
#define USED_FOR_EA_BUF  1			/* phase 1 late			*/
#define USED_FOR_DIRPAGE_BUFS  2		/* phase 6			*/
#define USED_FOR_INOEXT_BUF  3		/* phase 1 early, phase 7 	*/
#define USED_FOR_SUPER_VALIDATION 4	/* initial processing		*/

   char          vlarge_info_eyecatcher[8];
   char         *vlarge_buf_ptr;
   reg_idx_t    vlarge_buf_length;
   reg_idx_t    vlarge_current_use;
   char           aggrec_rsvdf[4];          /* pad to quadword boundary    */

           /* ****** fsck log BUFFER ***** */

#define FSCKLOG_BUFSIZE (2 * BYTESPERPAGE)

   char          fscklog_info_eyecatcher[8];
   blk_pageptr   fscklog_buf_ptr;
   reg_idx_t     fscklog_buf_length;
   reg_idx_t     fscklog_buf_data_len;
   agg_byteidx_t fscklog_agg_offset;
   agg_byteidx_t fscklog_log_offset;
   logent_hdptr  fscklog_last_msghdr;
   int8          fscklog_full;
   int8          fscklog_buf_allocated; 
   int8          fscklog_buf_alloc_err; 
   char          aggrec_rsvdg[5];          /* pad to quadword boundary    */


           /* ****** fsck block map BUFFER *****
            *
            * N.B. If the fsck workspace block map is instantiated in
            *      dynamic storage, then the following describe the entire
            *      block map.  However, if the fsck workspace block map
            *      is instantiated within the aggregate, then the following
            *      describe the I/O buffer associated with it.
            */

#define BLKMP_IO_BUFSIZE 4 * BYTESPERPAGE

   char          blkmp_info_eyecatcher[8];
   blk_pageptr   blkmp_buf_ptr;
   reg_idx_t     blkmp_buf_length;
   reg_idx_t     blkmp_buf_data_len;
   agg_byteidx_t blkmp_agg_offset;
   agg_byteidx_t blkmp_blkmp_offset;
   int8          blkmp_buf_write;
   blk_cntrlptr  blkmp_ctlptr;
   int32         blkmp_pagecount;


           /* ****** BLOCK MAP DMAP PAGE I/O BUFFER ****** */
           /*
            * note: The EA buffer is treated differently from the
            *       other I/O buffers allocated for fsck.  
            *       Specifically,
            *
            *            - it is larger than any other fsck I/O buffer 
            *              (sized to accomodate the largest legal ea)
            *            - since it is exactly 1 memory segment, it is
            *              not allocated as a normal wsp extent (described
            *              by an fer on the wsp_extent_list) but is 
            *              allocated as a special case.  
            *            - this buffer is released as soon as phase 1 is
            *              completed (does not persist until fsck has
            *              finished processing)
            *            - BTW the EA data is read by fsck, but never 
            *              written.
            */

#define EA_IO_BUFSIZE   16*BYTESPERPAGE

   char           ea_info_eyecatcher[8];
   char          *ea_buf_ptr;
   reg_idx_t      ea_buf_length;
   reg_idx_t      ea_buf_data_len;
   agg_byteidx_t  ea_agg_offset;

   char          aggrec_rsvdh[4];          /* pad to quadword boundary    */


           /* ****** IAG I/O BUFFER ****** */
           /*
            * note: This is not actually a unique buffer.  Since
            *       the Inode Allocation Maps verification is
            *       completed before the Block Allocation Map
            *       verification, buffer space is allocated and
            *       used first for Inode Alloc Map verification
            *       processing and then for Block Allocation Map
            *       verification.
            */

#define IAG_IO_BUFSIZE 4 * BYTESPERPAGE

   char           iag_info_eyecatcher[8];
   char          *iag_buf_ptr;
   reg_idx_t      iag_buf_length;
   reg_idx_t      iag_buf_data_len;
   agg_byteidx_t  iag_agg_offset;
   ino_t          iag_buf_1st_inode;  /* inode number of 1st in buffer   */
   inoidx_t       iag_fsnum;
   int8           iag_for_aggregate;
   int8           iag_which_it;
   int8           iag_buf_write;
   char         aggrec_rsvdi[9];          /* pad to quadword boundary    */


           /* ****** MAP CONTROL PAGE I/O BUFFER ****** */

#define MAPCTL_IO_BUFSIZE BYTESPERPAGE

   char           mapctl_info_eyecatcher[8];
   char          *mapctl_buf_ptr;
   reg_idx_t      mapctl_buf_length;
   reg_idx_t      mapctl_buf_data_len;
   agg_byteidx_t  mapctl_agg_offset;
   int8           mapctl_buf_write;
   char         aggrec_rsvdk[3];          /* pad to quadword boundary    */


           /* ****** MAP LEAF I/O BUFFER ****** */

#define MAPLEAF_IO_BUFSIZE   \
              ((((XTPAGE_SIZE*4)+BYTESPERPAGE-1)/BYTESPERPAGE)*BYTESPERPAGE)

   char           maplf_info_eyecatcher[8];
   char          *mapleaf_buf_ptr;
   reg_idx_t      mapleaf_buf_length;
   reg_idx_t      mapleaf_buf_data_len;
   agg_byteidx_t  mapleaf_agg_offset;
   int8           mapleaf_for_aggregate;
   int8           mapleaf_which_it;
   int8           mapleaf_buf_write;
   char         aggrec_rsvdm[1];          /* pad to quadword boundary    */


           /* ****** BLOCK MAP LEVEL PAGE I/O BUFFER ****** */

#define BMAPLV_IO_BUFSIZE BYTESPERPAGE

   char           bmplv_info_eyecatcher[8];
   char          *bmaplv_buf_ptr;
   reg_idx_t      bmaplv_buf_length;
   reg_idx_t      bmaplv_buf_data_len;
   agg_byteidx_t  bmaplv_agg_offset;
   agg_byteidx_t  bmaplv_logical_offset;
   int8           bmaplv_current_level;
   int8           bmaplv_buf_write;
   char         aggrec_rsvdo[10];          /* pad to quadword boundary    */


           /* ****** BLOCK MAP DMAP PAGE I/O BUFFER ****** */
           /*
            * note: This is not actually a unique buffer.  Since
            *       the Inode Allocation Maps verification is
            *       completed before the Block Allocation Map
            *       verification, buffer space is allocated and
            *       used first for Inode Alloc Map verification
            *       processing and then for Block Allocation Map
            *       verification.
            */

#define BMAPDMP_IO_BUFSIZE IAG_IO_BUFSIZE

   char           bmpdm_info_eyecatcher[8];
   char          *bmapdm_buf_ptr;
   reg_idx_t      bmapdm_buf_length;
   reg_idx_t      bmapdm_buf_data_len;
   agg_byteidx_t  bmapdm_agg_offset;
   agg_byteidx_t  bmapdm_logical_offset;
   int8           bmapdm_buf_write;
   char         aggrec_rsvdq[11];          /* pad to quadword boundary    */

           /* ****** INODE I/O BUFFER ****** */

#define INODE_IO_BUFSIZE  \
            (((INODE_EXTENT_SIZE+BYTESPERPAGE-1)/BYTESPERPAGE)*BYTESPERPAGE)

   char           inobuf_info_eyecatcher[8];
   char          *ino_buf_ptr;
   reg_idx_t      ino_buf_length;
   reg_idx_t      ino_buf_data_len;
   agg_byteidx_t  ino_buf_agg_offset; /* agg byte offset of buf contents */
   inoidx_t       ino_buf_1st_ino;    /* 1st inode number in buffer      */
   inoidx_t       ino_fsnum;
   pxd_t          ino_ixpxd;
   int8           ino_for_aggregate;
   int8           ino_which_it;
   int8           ino_buf_write;
   char         aggrec_rsvds[1];          /* pad to quadword boundary    */

           /* ****** INTERNAL/LEAF NODE I/O BUFFER ****** */

#define NODE_IO_BUFSIZE  \
            ((((XTPAGE_SIZE*4)+BYTESPERPAGE-1)/BYTESPERPAGE)*BYTESPERPAGE)

   char           nodbuf_info_eyecatcher[8];
   char          *node_buf_ptr;
   reg_idx_t      node_buf_length;
   reg_idx_t      node_buf_data_len;
   agg_byteidx_t  node_agg_offset;
   int8           node_buf_write;
   char         aggrec_rsvdu[3];          /* pad to quadword boundary    */

           /* **************************** instantiation of imap AG tables */

   char   agg_AGTbl_eyecatcher[8];
   char         aggrec_rsvdw[8];          /* pad to quadword boundary    */
   struct fsck_ag_record agg_AGTbl[MAXAG];

   char   fset_AGTbl_eyecatcher[8];
   char         aggrec_rsvdy[8];          /* pad to quadword boundary    */
   struct fsck_ag_record fset_AGTbl[MAXAG];

           /* ********************** instantiation of imap IAG workspace */
   char   amap_eyecatcher[8];
   uint32 amap[EXTSPERIAG];        /* 512 : inode allocation map  */
   char         aggrec_rsvd00[8];          /* pad to quadword boundary    */
   char   fextsumm_eyecatcher[8];
   int32 fextsumm[SMAPSZ];         /*  16 : free extent summary map */
   char         aggrec_rsvd02[8];          /* pad to quadword boundary    */
   char   finosumm_eyecatcher[8];
   int32 finosumm[SMAPSZ];         /*  16 : free inode summary map */

           /* ********************** instantiation of blockmap workspace */

   blkmap_wspace_t    blkmap_wsp;
   } fsck_agg_record;


#endif








