/* $Id: xchkdsk.c,v 1.3 2004/03/21 02:43:25 pasha Exp $ */

static char *SCCSID = "@(#)1.41.1.3  12/2/99 11:13:59 src/jfs/utils/chkdsk/xchkdsk.c, jfschk, w45.fs32, fixbld";
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
 *   MODULE_NAME:		xchkdsk.c
 *
 *   COMPONENT_NAME: 	jfschk
 *
 *   FUNCTIONS:
 *		main:	chkdsk		16-bit entry point which calls
 *                          	chkdsk32	32-bit entry point
 *
 *              report_dynstg_error
 *              report_readait_error
 *
 *              check_parents_and_first_refs
 *              create_lost_and_found
 *              final_processing
 *              initial_processing
 *              parse_parms
 *              phase0_processing
 *              phase1_processing
 *              phase2_processing
 *              phase3_processing
 *              phase4_processing
 *              phase5_processing
 *              phase6_processing
 *              phase7_processing
 *              phase8_processing
 *              phase9_processing
 *              repair_fs_inodes
 *              report_problems_setup_repairs
 *              resolve_lost_and_found
 *              validate_fs_inodes
 *              verify_parms
 *
 */

/*
 * defines and includes common among the xfsck modules
 */
#include "xfsckint.h"

#include <time.h>
#include "jfs_chkdsk.h"

/*
 * local includes
 */
#include "fsckmsgp.h"         /* message protocol array */
#include "fsckmsge.h"         /* message text, all messages, in english */

/*
 * for inline unicode functions
 */
#define _ULS_UNIDEFK
#include <uni_inln.h>


agg_byteidx_t ondev_jlog_byte_offset;


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * The following is part of the PMCHKDSK support
  *
  */
extern int pm_front_end; 


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * The following indicates the current phase.
  * Constants for the possible values are defined in xfsck.h
  *
  */
int current_fsck_phase = fsck_entry;


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * superblock buffer and pointer
  *
  *    values are assigned by the xchkdsk routine
  */
struct superblock  aggr_superblock;
struct superblock *sb_ptr;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * fsck aggregate info structure and pointer
  *
  *    values are assigned by the xchkdsk routine
  */
struct fsck_agg_record  agg_record;
struct fsck_agg_record *agg_recptr;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * fsck block map info structure and pointer
  *
  *    values are assigned by the xchkdsk routine
  */
struct fsck_bmap_record  bmap_record;
struct fsck_bmap_record *bmap_recptr;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * Unicode Conversion Object used in UniStrFromUCS & UniStrToUCS
  *
  *    value is assigned by the xchkdsk routine
  */

UconvObject uconv_object;

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * For message processing
  *
  *    values are assigned by the xchkdsk routine
  */

char *Vol_Label;
char default_vol[8];

UniChar uni_message_parm[MAXPARMLEN];
UniChar *uni_msgprm = uni_message_parm;

char message_parm_0[MAXPARMLEN];
char message_parm_1[MAXPARMLEN];
char message_parm_2[MAXPARMLEN];
char message_parm_3[MAXPARMLEN];
char message_parm_4[MAXPARMLEN];
char message_parm_5[MAXPARMLEN];
char message_parm_6[MAXPARMLEN];
char message_parm_7[MAXPARMLEN];
char message_parm_8[MAXPARMLEN];
char message_parm_9[MAXPARMLEN];

char *msgprms[10] = { message_parm_0, message_parm_1, message_parm_2,
                      message_parm_3, message_parm_4, message_parm_5,
                      message_parm_6, message_parm_7, message_parm_8,
                      message_parm_9 };

int16 msgprmidx[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

char message_buffer_0[256];
char message_buffer_1[1024];
char message_buffer_2[1024];
char *terse_msg_ptr =   &message_buffer_0[0];
char *verbose_msg_ptr = &message_buffer_1[0];
char *msgs_txt_ptr =    &message_buffer_2[0];
ULONG msgs_txt_maxlen = 1024;

extern char *MsgText[];

DATETIME fsck_DateTime = {0};
char time_stamp[20];                                           // @D2 

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * For directory entry processing
  *
  */
int32   key_len[2];
UniChar key[2][JFS_NAME_MAX];
UniChar ukey[2][JFS_NAME_MAX];

int32   Uni_Name_len;
UniChar Uni_Name[JFS_NAME_MAX];
int32   Str_Name_len;
char    Str_Name[JFS_NAME_MAX];

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * Device information.
  *
  *     values are assigned when (if) the device is opened.
  */
HFILE   Dev_IOPort;
uint32  Dev_blksize;
int32   Dev_SectorSize;


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * Unicode path strings information.
  *
  *     values are assigned when the fsck aggregate record is initialized.
  *     accessed via addresses in the fack aggregate record.
  */
UniChar uni_LSFN_NAME[11] = L"LOST+FOUND";
UniChar uni_lsfn_name[11] = L"lost+found";


 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * fsckwsp error handling fields
  *
  *     values are assigned when the fsck workspace storage is
  *     allocated.
  */
int wsp_dynstg_action;
int wsp_dynstg_object;


/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
 *
 * The following are internal to this file
 *
 */
retcode_t  chkdsk32( int32, char ** );

retcode_t check_parents_and_first_refs ( void );

retcode_t create_lost_and_found ( void );

retcode_t final_processing ( void ) ;

retcode_t initial_processing ( int32, char ** );

retcode_t parse_parms ( int32, char ** );

retcode_t phase0_processing ( void );

retcode_t phase1_processing ( void );

retcode_t phase2_processing ( void );

retcode_t phase3_processing ( void );

retcode_t phase4_processing ( void );

retcode_t phase5_processing ( void );

retcode_t phase6_processing ( void );

retcode_t phase7_processing ( void );

retcode_t phase8_processing ( void );

retcode_t phase9_processing ( void );

retcode_t repair_fs_inodes ( void );
 
retcode_t report_problems_setup_repairs ( void );

retcode_t resolve_lost_and_found ( void );

retcode_t validate_fs_inodes ( void );

retcode_t verify_parms ( void );

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */


 /*
  * module return code
  */
retcode_t rc = FSCK_OK;


/*****************************************************************************
 * NAME: chkdsk
 *       chkdsk32
 *
 * FUNCTION: Entry point for jfs check/repair of aggregate
 *
 * INTERFACE:  
 *            int32 chkdsk <volume specifier>
 *                         [ { -B | -b | /B | /b } ]
 *                                                     Perform LVM Bad Block List utility
 *                                                     processing only
 *                         [ { -F | -f | /F | /f } [ { :0 | :1 | :2 | :3 } ] ]
 *                                                     not specified => /f:0
 *                                                     w/o :n => :2
 *                                                     :0 => read only
 *                                                     :1 => prompt for permission
 *                                                           for each repair
 *                                                     :2 => preen
 *                                                     :3 => assume permission
 *                                                           given for all
 *                                                           repairs.
 *                                                           If exiting with clean file system, invoke
 *                                                           LVM Bad Block List utility processing
 *                                                           on the way out.
 *                                                      *** note *** note *** note ***
 *                                                      :1 and :2 are converted to :3
 *
 *                         [ { -C | -c | /C | /c } ]    process ifdirty after
 *                                                      logredo 
 *
 *                         [ { -V | -v | /V | /v } ]    verbose messaging
 *                                                      *** note *** note *** note ***
 *                                                      this parm is ignored
 *
 *                         [ { -A | -a | /A | /a } ]    *** UNDOCUMENTED PARM ***
 *                                                      autocheck mode
 *
 *                         [ { -P | -p | /P | /p } ]    *** UNDOCUMENTED PARM ***
 *                                                      pmchkdsk mode
 *
 *                         [ { -D | -d | /D | /d } ]    *** UNDOCUMENTED PARM ***
 *                                                      debug mode
 *                                                      (all log messages issued
 *                                                      to stdout, IN ENGLISH)
 *
 *                         [ { -O | -o | /O | /o } ]    *** UNDOCUMENTED PARM ***
 *                                                      omit logredo()
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
/* ----------------------------------------------------------------------
 * 
 * This 16-bit entry point is called by OS/2
 *
 * This routine converts the arguments to 32-bit and calls the
 * 32-bit entry point.
 *
 */
USHORT _Far16 _Pascal _loadds chkdsk(USHORT argc,
                                     UCHAR * _Seg16 * _Seg16 argv,
                                     UCHAR * _Seg16 * _Seg16 envp)
{
  char ** my_argv;
  int32 my_argc;
  int i;
  int rc = 0;
  my_argc = argc;
  my_argv = malloc (argc * sizeof (char*));
  for (i=0; i<argc; i++)
  {
    my_argv[i] = argv[i];
  }
  rc = chkdsk32(my_argc, my_argv);
  free(my_argv);
  return(rc);
}
/* ----------------------------------------------------------------------
 * 
 * This is the 32-bit entry point 
 *
 */
retcode_t  chkdsk32( int32 argc, char **argv )
{
  retcode_t       rc = FSCK_OK;
  int		agg_is_clean = 0;

  /*
   * some basic initializations
   */
  sb_ptr = &aggr_superblock;
  agg_recptr = &agg_record;
  bmap_recptr = &bmap_record;
  pm_front_end = 0;
#ifdef MYJFS       
#include "..\..\ident.h"
DosPutMessage(1,50,IdentF);
    IdentF[0]=0xA;
    IdentF[1]=0xD;
    IdentF[2]=0;
DosPutMessage(1,2,IdentF);
#endif

#ifdef _JFS_DEBUG
printf( "sb_ptr = %p   agg_recptr = %p   bmap_recptr = %p\n", 
sb_ptr, agg_recptr, bmap_recptr );
#endif

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("*************************** entering chkdsk\n\r");
}
#endif


  wsp_dynstg_action = dynstg_unknown;
  wsp_dynstg_object = dynstg_unknown;

  rc = UniCreateUconvObject(L"", &uconv_object);
  if (rc != ULS_SUCCESS)
    (void) UniCreateUconvObject(L"ASCII-7", &uconv_object);

  rc = init_agg_record( );	/* init workspace aggregate record
				 * (the parms will be recorded in it)
				 */
  if( rc == FSCK_OK ) {  /* workspace aggregate record initialized */

    /*
     * now jump right in
     */
	/*
	 * Allocate the multi-purpose buffer now so that it can be
	 * used during superblock verification.
	 *
	 * This must be done at least before calling logredo to ensure
	 * that the malloc() will succeed.  
	 * (In autocheck mode, logredo is likely to eat up all the
	 * low memory.  We don't want to use the alloc_wrksp routine
	 * because we want a page boundary without having to burn
	 * 4096 extra bytes.
	 */
    rc = alloc_vlarge_buffer(); 

    current_fsck_phase = fsck_phase_initial;
    rc = initial_processing( argc, argv );
    if( ! agg_recptr->stdout_redirected ) {
      fsck_hbeat_start();		/* begin the "running" indicator */
      }

	/*
	 * If they specified Clear Bad Blocks List only (aka /B),
	 * release everything that's allocated, close everything 
	 * that's open, and then initiate the requested processing.
	 */
    if( (agg_recptr->parm_options[UFS_CHKDSK_CLRBDBLKLST]) &&
        (!agg_recptr->fsck_is_done) )  {  /* bad block list processing only */
	/*
	 * this path is taken only when /f not specified, so chkdsk
	 * processing is readonly, but the clrbblks processing requires
	 * chkdsk to do some things it only permits when processing
	 * readwrite.  So we reset the switches temporarily and take
	 * care what routines we call.
	 */
      agg_recptr->processing_readwrite = 1;
      agg_recptr->processing_readonly = 0;
	/*
	 * JFS Clear Bad Blocks List processing
	 *
	 * If things go well, this will issue messages and 
	 * write to the service log.
	 */
      rc = establish_wsp_block_map_ctl (  );	/*
				* a portion of the 1st page in the workspace
				* is used for communication between chkdsk
				* and clrbblks.
				*/
      if( rc == FSCK_OK ) {
        rc = ClrBdBlkLst_processing();
        }
	/*
	 * terminate fsck service logging 
	 */
      fscklog_end();
	/*
	 * restore the original values.
	 */
      agg_recptr->processing_readwrite = 0;
      agg_recptr->processing_readonly = 1;
	/*
	 * release any workspace that has been allocated
	 */
      workspace_release(); 
	/*
	 * Close (Unlock) the device
	 */
      if( agg_recptr->device_is_open ) {
        close_volume(); 
        }
	/*
	 * Then exit
	 */ 
      if( ! agg_recptr->stdout_redirected ) {
        fsck_hbeat_stop();		/* end the "running" indicator */
        }
	
      return( rc );
      }  /* end bad block list processing only */

    if( !agg_recptr->fsck_is_done )
      {
      current_fsck_phase = fsck_phase0;
      rc = phase0_processing( );
      if( !agg_recptr->fsck_is_done ) {
        current_fsck_phase = fsck_phase1;
        rc = phase1_processing( );

        if( !agg_recptr->fsck_is_done ) {

          current_fsck_phase = fsck_phase2;
          rc = phase2_processing( );

          if( !agg_recptr->fsck_is_done ) {
            current_fsck_phase = fsck_phase3;
            rc = phase3_processing( );

            if( !agg_recptr->fsck_is_done ) {
              current_fsck_phase = fsck_phase4;
              rc = phase4_processing( );

              if( !agg_recptr->fsck_is_done ) {
                current_fsck_phase = fsck_phase5;
                rc = phase5_processing( );

                if( !agg_recptr->fsck_is_done ) {
                  current_fsck_phase = fsck_phase6;
                  rc = phase6_processing( );

                  if( !agg_recptr->fsck_is_done ) {
                    current_fsck_phase = fsck_phase7;
                    rc = phase7_processing( );

                    if( !agg_recptr->fsck_is_done ) {
                      current_fsck_phase = fsck_phase8;
                      rc = phase8_processing( );

                      if( !agg_recptr->fsck_is_done ) {
                        current_fsck_phase = fsck_phase9;
                        rc = phase9_processing( );
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }

    current_fsck_phase = fsck_phase_final;
    if( agg_recptr->superblk_ok ) {  /* we at least have a superblock */

      if( (rc == FSCK_OK) &&
          (!(agg_recptr->fsck_is_done)) ) {  /* not fleeing an error and
                                              * not making a speedy exit
                                              */

          /*
           * finish up and display some information
           */
        rc = final_processing();

          /*
           * flush the I/O buffers to complete any pending writes
           */
        if( rc == FSCK_OK ) { rc = blkmap_flush(); }
        else { blkmap_flush(); }

        if( rc == FSCK_OK ) { rc = blktbl_dmaps_flush(); }
        else { blktbl_dmaps_flush(); }

        if( rc == FSCK_OK ) { rc = blktbl_Ln_pages_flush(); }
        else { blktbl_Ln_pages_flush(); }

        if( rc == FSCK_OK ) { rc = iags_flush(); }
        else { iags_flush(); }

        if( rc == FSCK_OK ) { rc = inodes_flush(); }
        else { inodes_flush(); }

        if( rc == FSCK_OK ) { rc = mapctl_flush(); }
        else { mapctl_flush(); }

        }  /* end not fleeing an error and not making a speedy exit */


      /*
       * last chance to write to the wsp block map control page...
       */
   DosGetDateTime( &(fsck_DateTime) );
   if( fsck_DateTime.year > 2000 ) {
     sprintf( time_stamp, "%d/%d/%d%d.%d.%d", 
              fsck_DateTime.month, fsck_DateTime.day, 
              (fsck_DateTime.year%2000), fsck_DateTime.hours, 
              fsck_DateTime.minutes, fsck_DateTime.seconds );
     }
   else {
     sprintf( time_stamp, "%d/%d/%d%d.%d.%d", 
              fsck_DateTime.month, fsck_DateTime.day, 
              (fsck_DateTime.year%1900), fsck_DateTime.hours, 
              fsck_DateTime.minutes, fsck_DateTime.seconds );
     }

   if( agg_recptr->processing_readwrite ) {  /* on-device fsck workspace block map */
     if( agg_recptr->blkmp_ctlptr != NULL ) {
       memcpy( &(agg_recptr->blkmp_ctlptr->hdr.end_time[0]),
               &(time_stamp[0]), 16 );
       agg_recptr->blkmp_ctlptr->hdr.return_code = rc;
       blkmap_put_ctl_page( agg_recptr->blkmp_ctlptr );
       }
     }  /* end on-device fsck workspace block map */

    if( rc == FSCK_OK )  {  /* either all ok or nothing fatal */
      if( agg_recptr->processing_readonly ) {
        fsck_send_msg( fsck_MNTFSYS2, 0, 0 );  /* remind the caller not
                        * to take any messages issued too seriously
                        */
        if( agg_recptr->corrections_needed || agg_recptr->corrections_approved ) {
          fsck_send_msg( fsck_ERRORSDETECTED, 0, 0 );
          }
        }
      }  /* end either all ok or nothing fatal */

    if( rc == FSCK_OK ) {	/* no chkdsk failure detected */
      rc = agg_clean_or_dirty(); /* may write to superblocks again */
      if( (sb_ptr->s_state & FM_DIRTY) != FM_DIRTY ) {
        agg_is_clean = -1;
        }
      }

    if( agg_recptr->ag_modified )  {  /* wrote to it at least once */
      fsck_send_msg( fsck_MODIFIED, 0, 0 );
      }  /* end wrote to it at least once */
  
    if( (rc == FSCK_OK) &&
        (agg_recptr->processing_readwrite) &&
        ( agg_is_clean ) ) {   /* doing /f on a file system that's now clean */
	/*
	 * if we're in autocheck mode, just check the dipstick and then
	 * issue a message if the LVM's bad block list is getting full.
	 */
      if( agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK] ) {
        rc = check_BdBlkLst_FillLevel();
        }
      else {
	/*
	 * not in autocheck mode, do JFS Clear Bad Blocks List processing
	 *
	 * If things go well, this will issue messages and
	 * write to the service log.
	 */
        rc = ClrBdBlkLst_processing();
        }
      }  /* end doing /f on a file system that's now clean */

	
    /*
     * Log fsck exit
     */
    msgprms[0] = time_stamp; 
    msgprmidx[0] = 0;
    sprintf( message_parm_1, "%ld", rc );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    fsck_send_msg( fsck_SESSEND, 0, 2 ); 

    /*
     * terminate fsck service logging 
     */
    fscklog_end();

      /*
       * release all workspace that has been allocated
       */
    if( rc == FSCK_OK ) { rc = workspace_release(); }
    else { workspace_release(); }
    }  /* end we at least have a superblock */

    /*
     * Close (Unlock) the device
     */
    if( agg_recptr->device_is_open ) {
      if( rc == FSCK_OK ) 
        { rc = close_volume(); }
      else
        { close_volume(); }
      }
	
    current_fsck_phase = fsck_exit;
	
    }  /* end workspace aggregate record initialized */

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("leaving chkdsk *************************** \n\r");
}
#endif

    if( ! agg_recptr->stdout_redirected ) {
      fsck_hbeat_stop();		/* end the "running" indicator */
      }

    return( rc );
}                                /* end of main()   */

/* ======================================================================*/


/*****************************************************************************
 * NAME: check_parents_and_first_refs
 *
 * FUNCTION:  If any aggregate blocks are multiply allocated, find the 
 *            first reference for each.  Verify that the parent inode
 *            number stored in each directory inode matches the parent
 *            inode observed by fsck for that inode.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t check_parents_and_first_refs ( )
{
  retcode_t cpafr_rc = FSCK_OK;
  inoidx_t ino_idx;
  int aggregate_inode = 0;      /* going for fileset inodes only */
  int alloc_ifnull = 0;
  int which_it = 0;             /* in release 1 there is only fileset 0 */
  int inode_already_read, done;
  int dir_w_hlinks_seen = 0;
  int dir_w_incrrct_prnt_seen = 0;
  int unalloc_ino_w_prnts_seen = 0;
  int unconnected_inode_seen = 0;
  dinode_t *inoptr;
  fsck_inode_extptr this_ext;
  fsck_inode_recptr this_inorec, parent_inorec;

  struct fsck_ino_msg_info  ino_msg_info;
  fsck_msg_info_ptr msg_info_ptr;

  msg_info_ptr = &ino_msg_info;
  msg_info_ptr->msg_inopfx = fsck_fset_inode;  /* all fileset owned */

  cpafr_rc = get_inorecptr_first( aggregate_inode, &ino_idx, &this_inorec );

  while( (cpafr_rc == FSCK_OK) && (this_inorec != NULL) ) {

    msg_info_ptr->msg_inonum = ino_idx;
    if( this_inorec->inode_type == directory_inode ) {
      msg_info_ptr->msg_inotyp = fsck_directory;
      }
    else if( this_inorec->inode_type == symlink_inode ) {
      msg_info_ptr->msg_inotyp = fsck_symbolic_link;
      }
    else {  /* a regular file */
      msg_info_ptr->msg_inotyp = fsck_file;
      }

    if( !this_inorec->in_use ) { /* not in use.  A record allocated means some 
                                    * directory thinks this inode is its parent
                                    */
      done = 0;
      if( (this_inorec->parent_inonum != ROOT_I) || 
          (!agg_recptr->rootdir_rebuilt) ) {  /*
				* either this parent isn't the root or else
				* the root dir has not been rebuilt
				*/
        cpafr_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                                            this_inorec->parent_inonum, &parent_inorec );
        if( (parent_inorec->in_use) &&
            (!parent_inorec->ignore_alloc_blks) &&
            (!parent_inorec->selected_to_rls) )  {  /*
                               * parent inode in use and not selected to release
                               */
          this_inorec->unxpctd_prnts = 1;  /* flag it. */
          agg_recptr->corrections_needed = 1;
          unalloc_ino_w_prnts_seen = 1;
          done = -1;
          if( agg_recptr->processing_readonly ) {  /* won't be able to fix this */
            agg_recptr->ag_dirty = 1;
            msgprms[0] = message_parm_0;
            msgprmidx[0] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_1, "%ld", ino_idx );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            fsck_send_msg( fsck_ROUALINOREF, 0, 2 );
            }  /* end won't be able to fix this */
          }  /* end parent inode in use and not selected to release */
        }  /* end either this parent isn't the root or else the root dir has not ... */
	 
      this_ext = this_inorec->ext_rec;
      while( (this_ext != NULL) && (!done) ) {
         if( this_ext->ext_type == parent_extension ) {  /* it's a parent */
           if( (this_inorec->parent_inonum != ROOT_I) || 
               (!agg_recptr->rootdir_rebuilt) ) {  /*
				* either this parent isn't the root or else
				* the root dir has not been rebuilt
				*/
             cpafr_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                                                 this_ext->inonum, &parent_inorec );
             if( (parent_inorec->in_use) &&
                 (!parent_inorec->ignore_alloc_blks) &&
                 (!parent_inorec->selected_to_rls) )  {  /*
                               * parent inode in use and not selected to release
                               */
               this_inorec->unxpctd_prnts = 1;  /* flag it. */
               agg_recptr->corrections_needed = 1;
               unalloc_ino_w_prnts_seen = 1;
               done = -1;
               if( agg_recptr->processing_readonly ) {  /* won't be able to fix this */
                 agg_recptr->ag_dirty = 1;
                 msgprms[0] = message_parm_0;
                 msgprmidx[0] = msg_info_ptr->msg_inopfx;
                 sprintf( message_parm_1, "%ld", ino_idx );
                 msgprms[1] = message_parm_1;
                 msgprmidx[1] = 0;
                 fsck_send_msg( fsck_ROUALINOREF, 0, 2 );
                 }  /* end won't be able to fix this */
               }  /* end parent inode in use and not selected to release */
             }  /* end either this parent isn't the root or else the root dir has not ... */
           }  /* end it's a parent */
         this_ext = this_ext->next;
         }  /* end while */
      }  /* end not in use.  A record allocated means some directory... */
    else {  /* inode is in use */
      inode_already_read = 0;    /* initialize for inode */
      if( agg_recptr->unresolved_1stref_count != 0 )  {  /*
                                    * there are unresolved 1st references
                                    * to multiply allocated blocks
                                    */
        if( !this_inorec->ignore_alloc_blks ) {  /* blocks for this
                                    * inode are reflected in the current
                                    * workspace block map
                                    */
          cpafr_rc = inode_get( aggregate_inode, which_it, ino_idx, &inoptr );
          if( cpafr_rc == FSCK_OK ) {  /* got the inode */
            inode_already_read = 1;
            cpafr_rc = first_ref_check_inode(inoptr,ino_idx,this_inorec,msg_info_ptr);
            }  /* end got the inode */
          }  /* end blocks for this inode are reflected in the current workspace ...*/
        }  /* end there are unresolved 1st references to multiply allocated blocks */

      if( cpafr_rc == FSCK_OK ) {  /* nothing fatal */

        if( (this_inorec->parent_inonum == 0) &&
            (!this_inorec->unxpctd_prnts)     &&
            (ino_idx >= FILESET_OBJECT_I)        )  {  /*
                                    * no parent recorded and not a dir with unexpected
                                    * parents and not a metadata inode
                                    */
          if( agg_recptr->processing_readonly )  {  /*
                                    * won't be reconnecting this
                                    */
            unconnected_inode_seen = 1;
            msgprms[0] = message_parm_0;
            msgprmidx[0] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_1, "%ld", ino_idx );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            fsck_send_msg( fsck_ROUNCONNIO, 0, 2 );
            }  /* end won't be reconnecting this */
          }  /* end no parents observed */

        else if( this_inorec->inode_type == directory_inode ) {  /*
                                      * a directory
                                      */
          if( !inode_already_read )  {  /* need to read the inode */

            cpafr_rc = inode_get( aggregate_inode, which_it,
                                  ino_idx, &inoptr );
            }  /* end need to read the inode */
                  /*
                   * if this is a directory with 'unexpected parents'
                   * (aka illegal hard links) then the inode number
                   * which was stored in parent_inonum has already
                   * been stored in an extension record.  Save the
                   * parent inode number stored in the on-disk inode
                   * for use in messages.
                   */
          if( cpafr_rc == FSCK_OK ) {  /* nothing fatal */
                                                                        /* 4 lines deleted     @F1 */
            if( this_inorec->unxpctd_prnts ) {  /*
                                    * unexpected parents seen
                                    */
              this_inorec->parent_inonum = inoptr->di_parent;
              dir_w_hlinks_seen = 1;
              if( agg_recptr->processing_readonly ) {  /* won't be able
                                    * to fix this
                                    */
                agg_recptr->ag_dirty = 1;
                msgprms[0] = message_parm_0;
                msgprmidx[0] = msg_info_ptr->msg_inopfx;
                sprintf( message_parm_1, "%ld", ino_idx );
                msgprms[1] = message_parm_1;
                msgprmidx[1] = 0;
                fsck_send_msg( fsck_RODIRWHLKS, 0, 2 );
                }  /* end won't be able to fix this */
              }  /* end unexpected parents seen */
                    /*
                     * Otherwise, make sure a parent was seen and it's
                     * the one whose inode number is stored in the
                     * on-disk inode.
                     */
            else if( (this_inorec->parent_inonum != (ino_t) 0) &&
                     (this_inorec->parent_inonum != inoptr->di_parent) ) {  /*
                                    * the stored parent number is wrong
                                    */
              this_inorec->crrct_prnt_inonum = 1;
              dir_w_incrrct_prnt_seen = 1;
              agg_recptr->corrections_needed = 1;
              agg_recptr->corrections_approved = 1;
              if( agg_recptr->processing_readonly ) {  /* won't be able
                                    * to fix this
                                    */
                agg_recptr->ag_dirty = 1;
                msgprms[0] = message_parm_0;
                msgprmidx[0] = msg_info_ptr->msg_inopfx;
                sprintf( message_parm_1, "%ld", ino_idx );
                msgprms[1] = message_parm_1;
                msgprmidx[1] = 0;
                msgprms[2] = msgprms[0];
                msgprmidx[2] = msgprmidx[0];
                sprintf( message_parm_3, "%ld", this_inorec->parent_inonum );
                msgprms[3] = message_parm_3;
                msgprmidx[3] = 0;
                fsck_send_msg( fsck_ROINCINOREF, 0, 4 );
                }  /* end won't be able to fix this */
              }  /* end the stored parent number is wrong */
            }  /* end nothing fatal */
          }  /* end a directory */
        }  /* end nothing fatal */
      }  /* end else inode is in use */

    if( cpafr_rc == FSCK_OK ) {
      cpafr_rc = get_inorecptr_next( aggregate_inode, &ino_idx, &this_inorec );
      }
    }  /* end while */

  if( agg_recptr->processing_readwrite ) {  /* we can fix these */
    if( unalloc_ino_w_prnts_seen ) {
      fsck_send_msg( fsck_WILLFIXROUALINOREFS, 0, 0 );
      }
    if( unconnected_inode_seen )  {
      fsck_send_msg( fsck_WILLFIXROUNCONNIOS, 0, 0 );
      }
    if( dir_w_hlinks_seen ) {
      fsck_send_msg( fsck_WILLFIXRODIRSWHLKS, 0, 0 );
      }
    if( dir_w_incrrct_prnt_seen ) {
      fsck_send_msg( fsck_WILLFIXROINCINOREFS, 0, 0 );
      }
    }  /* end we can fix these */
  else {  /* don't have write access */
    if( unalloc_ino_w_prnts_seen ) {
      fsck_send_msg( fsck_ROUALINOREFS, 0, 0 );
      }
    if( unconnected_inode_seen )  {
      fsck_send_msg( fsck_ROUNCONNIOS, 0, 0 );
      }
    if( dir_w_hlinks_seen ) {
      fsck_send_msg( fsck_RODIRSWHLKS, 0, 0 );
      }
    if( dir_w_incrrct_prnt_seen ) {
      fsck_send_msg( fsck_ROINCINOREFS, 0, 0 );
      }
    }  /* end else don't have write access */

  return( cpafr_rc );
}                              /* end of check_parents_and_first_refs ()  */


/*****************************************************************************
 * NAME: create_lost_and_found
 *
 * FUNCTION:  During previous processing, fsck observed at least one inode
 *            to be available, and saved the ordinal number of an available
 *            inode in the fsck aggregate record.  Initialize that inode
 *            (and the fsck inode record describing it) for use as 
 *            /lost+found/
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t create_lost_and_found ( )
{
  retcode_t claf_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  inoidx_t new_inoidx;
  fsck_inode_recptr new_inorecptr;

  int aggregate_inode = 0;      /* going for fileset inodes only */
  int alloc_ifnull = -1;
  int which_it = FILESYSTEM_I;  /* in release 1 there is only fileset 0 */
  dinode_t *inoptr;
  dtroot_t *btroot_ptr;
  int ixpxd_unequal = 0;
  DATETIME  now_today;
  int is_aggregate = 0;                  /* aggregate has no dirs       */

      /*
       * find/allocate the fsck workspace inode record
       * for this inode.
       */
  claf_rc = get_inorecptr( aggregate_inode, alloc_ifnull,
                           agg_recptr->avail_inonum, &new_inorecptr );

  if( claf_rc == FSCK_OK )  {  /* still on track */

        /*
         * initialize the workspace inode record for
         * the new lost+found/
         */
    new_inorecptr->in_use = 1;
    new_inorecptr->inode_type = directory_inode;
    new_inorecptr->link_count = 0;
    new_inorecptr->parent_inonum = (ino_t) 0;
    new_inorecptr->cumm_blocks = 0;                                              /* @F1 */
    new_inorecptr->dtree_level = -1;                                               /* @F1 */

    new_inorecptr->selected_to_rls = 0;
    new_inorecptr->crrct_link_count = 0;
    new_inorecptr->crrct_prnt_inonum = 0;
    new_inorecptr->adj_entries = 0;
    new_inorecptr->clr_ea_fld = 0;
    new_inorecptr->clr_acl_fld = 0;
    new_inorecptr->crrct_cumm_blks = 0;
                                                                           /* 1 line deleted  @F1 */
    new_inorecptr->inlineea_on = 0;
    new_inorecptr->inlineea_off = 0;
    new_inorecptr->inline_data_err = 0;
    new_inorecptr->ignore_alloc_blks = 0;
    new_inorecptr->reconnect = 0;
    new_inorecptr->unxpctd_prnts = 0;
    new_inorecptr->involved_in_dups = 0;

      /*
       * get the inode to be used for lost+found
       */
    claf_rc = inode_get( aggregate_inode, which_it,
                         agg_recptr->avail_inonum, &inoptr );


    if( claf_rc == FSCK_OK ) {  /* the new lost+found inode is in the buffer */

      ixpxd_unequal = memcmp( (void *) &(inoptr->di_ixpxd),
                              (void *) &(agg_recptr->ino_ixpxd),
                             sizeof(pxd_t) );       /* returns 0 if equal */

      if( (inoptr->di_inostamp == agg_recptr->inode_stamp) &&
          (!ixpxd_unequal) &&
          (inoptr->di_number == ((ino_t) agg_recptr->avail_inonum)) &&
          (inoptr->di_fileset == agg_recptr->ino_fsnum) ) {  /*
                                 * inode has been used before
                                 */
        inoptr->di_gen++;
        }  /* end inode has been used before */

      else { /* this inode hasn't been used before */

        memset( inoptr, 0, sizeof(dinode_t) );  /* clear it */

              /*
               * initialize the inode
               */
        inoptr->di_inostamp = agg_recptr->inode_stamp;
        inoptr->di_fileset = agg_recptr->ino_fsnum;
        inoptr->di_number = (ino_t) agg_recptr->avail_inonum;
        inoptr->di_gen = 1;

        memcpy( (void *) &(inoptr->di_ixpxd),
                (void *) &(agg_recptr->ino_ixpxd),
                sizeof(pxd_t) );
        }  /* end this inode hasn't been used before */

      inoptr->di_mode = (IDIRECTORY|IFJOURNAL|IFDIR|IREAD|IWRITE|IEXEC);
      inoptr->di_parent = ROOT_I;
      inoptr->di_nlink = 2;         /* one from root and one from self */
      inoptr->di_nblocks = 0;
      inoptr->di_size = IDATASIZE;  
	
      DXDlength(&(inoptr->di_acl), (int32) 0);
      DXDaddress(&(inoptr->di_acl), (int64) 0);
      inoptr->di_acl.flag = 0;
      inoptr->di_acl.size = 0;
      DXDlength(&(inoptr->di_ea), (int32) 0);
      DXDaddress(&(inoptr->di_ea), (int64) 0);
      inoptr->di_ea.flag = 0;
      inoptr->di_ea.size = 0;
	
      inoptr->di_compress = 0;
      inoptr->di_acltype = 0;
      inoptr->di_atime.tv_sec = (uint32) time( NULL );
      inoptr->di_ctime.tv_sec = inoptr->di_atime.tv_sec;
      inoptr->di_mtime.tv_sec = inoptr->di_atime.tv_sec;
      inoptr->di_otime.tv_sec = inoptr->di_atime.tv_sec;

            /*
             * initialize the d-tree
             */
      init_dir_tree( (dtroot_t *) &(inoptr->di_btroot) );

            /*
             * write the inode 
             */
      claf_rc = inode_put( inoptr );
      if( claf_rc == FSCK_OK )  {  /* success */
        new_inorecptr->parent_inonum = ROOT_I;
        new_inorecptr->link_count = 0;  /* The inode is correct.  After
                               * this we'll start accumulating adjustments
                               */

            /*
             * add an entry for it to the root directory
             */
        intermed_rc = inode_get( is_aggregate, which_it, ROOT_I, &inoptr );

        if( intermed_rc == FSCK_OK ) {
          intermed_rc = direntry_add( inoptr, 
                                      (ino_t) agg_recptr->avail_inonum,
                                      agg_recptr->UniChar_lsfn_name );
          }

        if( intermed_rc == FSCK_OK ) {  /* added the entry */
          inoptr->di_nlink += 1;  /* increment the link count in the root
                          * inode because we just added a subdirectory.
                          * (Subdirectories point back to parent.)
                          */
          }  /* end added the entry */
        else if( intermed_rc < 0 )  {  /* it was fatal */
          claf_rc = intermed_rc;
          }  /* end it was fatal */
        else {  /* not successful, but not fatal */
          claf_rc = FSCK_CANT_EXTEND_ROOTDIR;
          new_inorecptr->in_use = 0;
          }  /* end not successful, but not fatal */
        }  /* end success */
      }  /* end the new lost+found inode is in the buffer */
    }  /* end still on track */


  if( claf_rc != FSCK_OK ) {  /* failed to create */
    agg_recptr->lsfn_ok = 0;
    new_inorecptr->in_use = 0;
    fsck_send_msg( fsck_LSFNCNTCRE, 0, 0 );
    }  /* end failed to create */

  return( claf_rc );
}                              /* end of create_lost_and_found ()  */


/***************************************************************************
 * NAME: final_processing
 *
 * FUNCTION:  If processing read/write, replicate the superblock and the 
 *            aggregate inode structures (i.e., the Aggregate Inode Map
 *            and the Aggregate Inode Table).
 *
 *            Notify the user about various things.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t  final_processing ( )
{
  retcode_t pf_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  fsblkidx_t observed_total_blocks, recorded_total_blocks;
  fsblkidx_t reportable_total_blocks;
  fsblkidx_t blocks_reserved;
  fsblkidx_t kbytes_for_dirs, kbytes_for_files;
  fsblkidx_t kbytes_for_eas, kbytes_for_acls;
  fsblkidx_t kbytes_total, kbytes_free, kbytes_reserved;
  fsblkidx_t block_surprise;

    /*
     * report extra or missing aggregate blocks
     *
     * Note that since the agg_record is instantiated in the
     * module (and not a malloc'ed space) it is still available
     * after the workspace has been released.
     *
     */
  reportable_total_blocks = agg_recptr->blocks_used_in_aggregate +
                            agg_recptr->free_blocks_in_aggregate;
  observed_total_blocks = reportable_total_blocks -
                          agg_recptr->ondev_jlog_fsblk_length -
                          agg_recptr->ondev_fscklog_fsblk_length -
                          agg_recptr->ondev_wsp_fsblk_length;
  recorded_total_blocks = sb_ptr->s_size * Dev_blksize /
                          sb_ptr->s_bsize;  /* size in aggregate blocks */

  if( observed_total_blocks > recorded_total_blocks ) {
    block_surprise = observed_total_blocks - recorded_total_blocks;
    sprintf( message_parm_0, "%lld", block_surprise );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    fsck_send_msg( fsck_XTRABLKS, 0, 1 );
    }
  else if( recorded_total_blocks > observed_total_blocks ) {
    block_surprise = recorded_total_blocks - observed_total_blocks;
    sprintf( message_parm_0, "%lld", block_surprise );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    fsck_send_msg( fsck_MSSNGBLKS, 0, 1 );
    }

   if( agg_recptr->processing_readwrite )  {  /* have write access */
		/*
		 * if we have primed the dasd used fields, 
		 * turn off the flag so mount won't do it too.
		 *
		 * Also turn on the dasd enabled flag because if
		 * this there has been no mount since enablement,
		 * it isn't on yet.
		 */
      if( agg_recptr->prime_dasd_limits ) {                                   /* @F1 */
          sb_ptr->s_flag &= (~JFS_DASD_PRIME);                            /* @F1 */
          sb_ptr->s_flag |= JFS_DASD_ENABLED;                              /* @F1 */
          fsck_send_msg( fsck_DASDUSEDPRIMED, 0, 0 );                   /* @F1 */
          if( ! agg_recptr->active_dasd_limits ) {                             /* @F1 */
              fsck_send_msg( fsck_DASDLIMITSPRIMED, 0, 0 );             /* @F1 */
              }
          }                                                                                  /* @F1 */
      pf_rc = replicate_superblock(); /* refresh the redundancy of the
                               * aggregate superblock (and verify
                               * successful write to the one we
                               * haven't been using)
                               */
    }  /* end have write access */

  /*
   * finish up processing
   */
  fsck_send_msg( fsck_FSSMMRY3, 0, 0 );


  msgprmidx[0] = 0;
  msgprmidx[1] = 0;
  msgprms[0] = message_parm_0;
  msgprms[1] = message_parm_1;
     /*
      * log the summary messages originally defined
      */
  sprintf( message_parm_0, "%lld", agg_recptr->blocks_for_inodes );
  fsck_send_msg( fsck_FSSMMRY4, 0, 1 );

  sprintf( message_parm_0, "%lld", agg_recptr->inodes_in_aggregate );
  msgprms[0] = message_parm_0;
  fsck_send_msg( fsck_FSSMMRY5, 0, 1 );

  sprintf( message_parm_0, "%lld", agg_recptr->files_in_aggregate );
  msgprms[0] = message_parm_0;
  fsck_send_msg( fsck_FSSMMRY6, 0, 1 );

  sprintf( message_parm_0, "%lld", agg_recptr->dirs_in_aggregate );
  msgprms[0] = message_parm_0;
  fsck_send_msg( fsck_FSSMMRY9, 0, 1 );

  sprintf( message_parm_0, "%lld", reportable_total_blocks );
  msgprms[0] = message_parm_0;
  fsck_send_msg( fsck_FSSMMRY7, 0, 1 );

  sprintf( message_parm_0, "%lld", agg_recptr->free_blocks_in_aggregate );
  msgprms[0] = message_parm_0;
  fsck_send_msg( fsck_FSSMMRY8, 0, 1 );

     /*
      * issue (and log) the standard OS/2 messages
      */
  kbytes_total = reportable_total_blocks       /* number of blocks */
                 << agg_recptr->log2_blksize   /* times bytes per block */
                 >> log2BYTESPERKBYTE;         /* divided by bytes per kilobyte */
  kbytes_for_dirs = (
                  (agg_recptr->blocks_for_dirs << agg_recptr->log2_blksize)   /* 
                                  * blocks for file inodes times bytes per block  
                                  */
                  + (agg_recptr->dirs_in_aggregate << log2INODESIZE) /*
                                  * plus bytes for the file inodes themselves 
                                  */
                     ) >> log2BYTESPERKBYTE;     /* divided by bytes per kilobyte */
  kbytes_for_files = (
                  (agg_recptr->blocks_for_files << agg_recptr->log2_blksize)   /* 
                                  * blocks for file inodes times bytes per block  
                                  */
                  + (agg_recptr->files_in_aggregate << log2INODESIZE) /*
                                  * plus bytes for the file inodes themselves 
                                  */
                     ) >> log2BYTESPERKBYTE;     /* divided by bytes per kilobyte */
  kbytes_for_eas = agg_recptr->blocks_for_eas    /* number of blocks */
                   << agg_recptr->log2_blksize   /* times bytes per block */
                   >> log2BYTESPERKBYTE;         /* divided by bytes per kilobyte */
  kbytes_for_acls = agg_recptr->blocks_for_acls    /* number of blocks */
                   << agg_recptr->log2_blksize   /* times bytes per block */
                   >> log2BYTESPERKBYTE;         /* divided by bytes per kilobyte */
  kbytes_free = agg_recptr->free_blocks_in_aggregate /* number of blocks */
                << agg_recptr->log2_blksize   /* times bytes per block */
                >> log2BYTESPERKBYTE;         /* divided by bytes per kilobyte */
  kbytes_reserved = kbytes_total -
                    - kbytes_for_dirs 
                    - kbytes_for_files
                    - kbytes_for_eas
                    - kbytes_for_acls
                    - kbytes_free;            /* everything else is reserved */

  sprintf( message_parm_0, "%9lld", kbytes_total );
  msgprms[0] = message_parm_0;
  fsck_send_msg( fsck_STDSUMMARY1, 0, 1 );

  sprintf( message_parm_0, "%9lld", kbytes_for_dirs );
  sprintf( message_parm_1, "%lld", agg_recptr->dirs_in_aggregate );
  msgprms[0] = message_parm_0;
  msgprms[1] = message_parm_1;
  fsck_send_msg( fsck_STDSUMMARY2, 0, 2 );

  sprintf( message_parm_0, "%9lld", kbytes_for_files );
  sprintf( message_parm_1, "%lld", agg_recptr->files_in_aggregate );
  msgprms[0] = message_parm_0;
  msgprms[1] = message_parm_1;
  fsck_send_msg( fsck_STDSUMMARY3, 0, 2 );

  sprintf( message_parm_0, "%9lld", kbytes_for_eas );
  msgprms[0] = message_parm_0;
  fsck_send_msg( fsck_STDSUMMARY4, 0, 1 );

  sprintf( message_parm_0, "%9lld", kbytes_for_acls );
  msgprms[0] = message_parm_0;
  fsck_send_msg( fsck_STDSUMMARY4A, 0, 1 );

  sprintf( message_parm_0, "%9lld", kbytes_reserved );
  msgprms[0] = message_parm_0;
  fsck_send_msg( fsck_STDSUMMARY5, 0, 1 );

  sprintf( message_parm_0, "%9lld", kbytes_free );
  msgprms[0] = message_parm_0;
  fsck_send_msg( fsck_STDSUMMARY6, 0, 1 );

  if( pf_rc != FSCK_OK ) {
    agg_recptr->fsck_is_done = 1;
    }

  return( pf_rc );
}                              /* end of final_processing ()  */
	
	
/*****************************************************************************
 * NAME: report_problems_setup_repairs
 *
 * FUNCTION:  For each inode in the fileset, if fsck has determined that
 *            any repairs are needed, get/verify permission to perform
 *            the repair and, if permission has been given, adjust the 
 *            other inodes for implied repairs as appropriate.  (E.g.,
 *            is inode a is released, then each directory inode parent
 *            of a needs to have the entry for a removed.)
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t report_problems_setup_repairs ( )
{
  retcode_t gpsr_rc = FSCK_OK;
  inoidx_t ino_idx;
  int aggregate_inode = 0;      /* going for fileset inodes only */
  int alloc_ifnull = 0;
  int which_it = 0;             /* in release 1 there is only fileset 0 */
  int inode_already_read;
  int dir_w_hlinks_seen = 0;
  int dir_w_incrrct_prnt_seen = 0;
  int unalloc_ino_w_prnts_seen = 0;
  dinode_t *inoptr;
  fsck_inode_extptr this_ext;
  fsck_inode_extptr ext_list;
  fsck_inode_recptr this_inorec;
  int8 other_adjustments;
  struct fsck_ino_msg_info  ino_msg_info;
  fsck_msg_info_ptr msg_info_ptr;
  char user_reply;
  msg_info_ptr = &ino_msg_info;
  msg_info_ptr->msg_inopfx = fsck_fset_inode;  /* all fileset owned */
  gpsr_rc = get_inorecptr_first( aggregate_inode, &ino_idx, &this_inorec );
  while( (gpsr_rc == FSCK_OK) && (this_inorec != NULL) ) {
    if( (this_inorec->selected_to_rls)        ||
        (this_inorec->clr_ea_fld)               ||
        (this_inorec->clr_acl_fld)              ||
        (this_inorec->inlineea_on)              ||
        (this_inorec->inlineea_off)              ||
        (this_inorec->inline_data_err)        ||
        (this_inorec->crrct_cumm_blks) ||
        (this_inorec->cant_chkea)             ||
        (this_inorec->adj_entries)              ||
        ( (this_inorec->unxpctd_prnts) &&
          (!this_inorec->in_use)         )    ) { /*
                                    * a record is allocated and flagged
                                    * for some repair (other than directory
                                    * with illegal hard links) or warning
                                    */
      msg_info_ptr->msg_inonum = ino_idx;
      if( this_inorec->inode_type == directory_inode ) {
	msg_info_ptr->msg_inotyp = fsck_directory;
	}
      else if( this_inorec->inode_type == symlink_inode ) {
	msg_info_ptr->msg_inotyp = fsck_symbolic_link;
	}
      else {  /* a regular file */
	msg_info_ptr->msg_inotyp = fsck_file;
	}
      gpsr_rc = display_paths( ino_idx, this_inorec, msg_info_ptr );
      if( gpsr_rc == FSCK_OK )  {  /* nothing fatal */
        if( !this_inorec->in_use ) { /* not in use.  */
          if( this_inorec->unxpctd_prnts ) {  /* but with parents */
            if( agg_recptr->processing_readwrite ) {  /* we can fix this */
              gpsr_rc = adjust_parents( this_inorec, ino_idx );
              agg_recptr->corrections_approved = 1;
              fsck_send_msg( fsck_WILLRMVBADREF, 0, 0 );
              }  /* end we can fix this */
            else {       /* we don't have write access */
              fsck_send_msg( fsck_INOBADREF, 0, 0 );
              agg_recptr->ag_dirty = 1;
              }  /* end we don't have write access */
            }  /* end but with parents */
          else {  /* shouldn't have created a record for it! */
            gpsr_rc = FSCK_INTERNAL_ERROR_2;
            }  /* end else shouldn't have created a record for it! */
          }  /* end not in use. */
        else {  /* inode is in use */
          if( this_inorec->selected_to_rls )  {  /* selected to release */
              /*
               * explain the problem(s)
               */
            if( this_inorec->inode_type == unrecognized_inode ) {
              fsck_send_msg( fsck_BADINOFORMATO, 0, 0 );
              }

            msgprmidx[0] = msg_info_ptr->msg_inotyp;
            if( this_inorec->ignore_alloc_blks ) {  /* corrupt tree */
              if( this_inorec->inode_type == file_inode ) {
                fsck_send_msg( fsck_BADINODATAFORMAT, 0, 0 );
                }
              else if( this_inorec->inode_type == directory_inode ) {
                fsck_send_msg( fsck_BADINODATAFORMATD, 0, 0 );
                }
              else  {
                fsck_send_msg( fsck_BADINODATAFORMATO, 0, 0 );
                }
              }  /* corrupt tree */

            if( this_inorec->inline_data_err ) {  /* invalid inline data spec */
              if( this_inorec->inode_type == file_inode ) {
                fsck_send_msg( fsck_BADINODATAFORMAT, 0, 0 );
                }
              else if( this_inorec->inode_type == directory_inode ) {
                fsck_send_msg( fsck_BADINODATAFORMATD, 0, 0 );
                }
              else  {
                fsck_send_msg( fsck_BADINODATAFORMATO, 0, 0 );
                }
               }  /* invalid inline data spec */

            if( this_inorec->involved_in_dups ) {  
              if( this_inorec->inode_type == file_inode ) {
                fsck_send_msg( fsck_BADINOCLAIMSDUPSF, 0, 0 );
                }
              else if( this_inorec->inode_type == directory_inode ) {
                fsck_send_msg( fsck_BADINOCLAIMSDUPSD, 0, 0 );
                }
              else  {
                fsck_send_msg( fsck_BADINOCLAIMSDUPSO, 0, 0 );
                }
              }
              /*
               * notify of intentions (if any)
               */
            msgprms[0] = message_parm_0;
            msgprms[1] = message_parm_1;
            msgprmidx[1] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_2, "%ld", msg_info_ptr->msg_inonum );
            msgprms[2] = message_parm_2;
            msgprmidx[2] = 0;
            if( agg_recptr->processing_readwrite ) {  /* we can fix this */
              gpsr_rc = adjust_parents( this_inorec, ino_idx );
              agg_recptr->corrections_approved = 1;
              fsck_send_msg( fsck_WILLRELEASEINO, 0, 3 );
              }  /* end we can fix this */
            else {       /* we don't have write access */
              this_inorec->selected_to_rls = 0;
                /*
                 * If the tree is corrupt, or the type is unrecognized,
                 * keeping it makes the filesystem dirty.
                 * Otherwise it's just the alleged owner of some
                 * multiply allocated block(s).  In the latter case, the
                 * aggregate isn't necessarily dirty just because we don't
                 * release this particular inode.  If all other alleged
                 * owners are released then this one becomes OK.  After
                 * all releases are done we'll check to see if any block(s)
                 * are still multiply allocated, and if so we'll mark the
                 * aggregate dirty.
                 */
              if( (this_inorec->inode_type == unrecognized_inode) ||
                  (this_inorec->ignore_alloc_blks)                   )  {
                agg_recptr->ag_dirty = 1;
                }
              msgprms[0] = message_parm_0;
              msgprms[1] = message_parm_1;
              msgprms[2] = message_parm_2;
              fsck_send_msg( fsck_CANTREPAIRINO, 0, 3 );
              }  /* end we don't have write access */
            }  /* end selected to release */
          if( (gpsr_rc == FSCK_OK ) &&
              (!this_inorec->ignore_alloc_blks) && /* not corrupt and */
              (!this_inorec->selected_to_rls) )  { /* either never selected
                                 * to release or selected and release
                                 * declined
                                 */
            if( (gpsr_rc == FSCK_OK) &&
                (this_inorec->clr_ea_fld) )  {  /* clear EA */
              if( agg_recptr->processing_readwrite ) {  /* we can fix this */
                agg_recptr->corrections_approved = 1;
                fsck_send_msg( fsck_WILLCLEAREA, 0, 0 );
                }  /* end we can fix this */
              else {       /* we don't have write access */
                this_inorec->clr_ea_fld = 0;
                agg_recptr->ag_dirty = 1;
                fsck_send_msg( fsck_INOEA, 0, 0 );
                }  /* end we don't have write access */
              }  /* end clear EA */
            if( (gpsr_rc == FSCK_OK) &&
                (this_inorec->clr_acl_fld) )  {  /* clear ACL */
              if( agg_recptr->processing_readwrite ) {  /* we can fix this */
                agg_recptr->corrections_approved = 1;
                fsck_send_msg( fsck_WILLCLEARACL, 0, 0 );
                }  /* end we can fix this */
              else {       /* we don't have write access */
                this_inorec->clr_acl_fld = 0;
                agg_recptr->ag_dirty = 1;
                fsck_send_msg( fsck_INOACL, 0, 0 );
                }  /* end we don't have write access */
              }  /* end clear ACL */
            if( (gpsr_rc == FSCK_OK) &&
                (this_inorec->crrct_cumm_blks) )  {  /* fix DASDused */
              if( agg_recptr->processing_readwrite ) {  /* we can fix this */
                agg_recptr->corrections_approved = 1;
                fsck_send_msg( fsck_WILLFIXINOMINOR, 0, 0 );
                }  /* end we can fix this */
              else {       /* we don't have write access */
                this_inorec->inlineea_off = 0;
                agg_recptr->ag_dirty = 1;
                fsck_send_msg( fsck_INOMINOR, 0, 0 );
                }  /* end we don't have write access */
              }  /* end fix DASDused */
            if( (gpsr_rc == FSCK_OK) &&
                (this_inorec->inlineea_off) )  {  /* turn off sect 4 avail flag */
              if( agg_recptr->processing_readwrite ) {  /* we can fix this */
                agg_recptr->corrections_approved = 1;
                fsck_send_msg( fsck_WILLFIXINOMINOR, 0, 0 );
                }  /* end we can fix this */
              else {       /* we don't have write access */
                this_inorec->inlineea_off = 0;
                agg_recptr->ag_dirty = 1;
                fsck_send_msg( fsck_INOMINOR, 0, 0 );
                }  /* end we don't have write access */
              }  /* turn off sect 4 avail flag */
            if( (gpsr_rc == FSCK_OK) &&
                (this_inorec->inlineea_on) )  {  /* turn on sect 4 avail flag */
              if( agg_recptr->processing_readwrite ) {  /* we can fix this */
                agg_recptr->corrections_approved = 1;
                fsck_send_msg( fsck_WILLFIXINOMINOR, 0, 0 );
                }  /* end we can fix this */
              else {       /* we don't have write access */
                this_inorec->inlineea_on = 0;
                fsck_send_msg( fsck_INOMINOR, 0, 0 );
                }  /* end we don't have write access */
              }  /* turn on sect 4 avail flag */
            if( (gpsr_rc == FSCK_OK) &&
                (this_inorec->adj_entries) )  {  /* adjust dir entries */
              other_adjustments = 0;
              ext_list = this_inorec->ext_rec;
              this_inorec->ext_rec = NULL;
              while (ext_list != NULL ) {
                this_ext = ext_list;
                ext_list = ext_list->next;
                if( (this_ext->ext_type == add_direntry_extension) ||
                    (this_ext->ext_type == rmv_direntry_extension)    ) {
                  other_adjustments = -1;
                  agg_recptr->corrections_approved = 1;
                  }
                if( this_ext->ext_type != rmv_badentry_extension ) {
                  this_ext->next = this_inorec->ext_rec;
                  this_inorec->ext_rec = this_ext;
                  }
                else {  /* it represents a bad entry */
                  msgprms[0] = message_parm_0;
                  msgprmidx[0] = msg_info_ptr->msg_inopfx;
                  sprintf( message_parm_1, "%ld", this_ext->inonum );
                  msgprms[1] = message_parm_1;
                  msgprmidx[1] = 0;
                  if( agg_recptr->processing_readwrite ) {  /* we can fix this */
                    agg_recptr->corrections_approved = 1;
                    this_ext->next = this_inorec->ext_rec;
                    this_inorec->ext_rec = this_ext;
                    other_adjustments = -1;
                    fsck_send_msg( fsck_WILLRMVBADENTRY, 0, 2 );
                    }  /* end we can fix this */
                  else {       /* we don't have write access */
                    release_inode_extension( this_ext );
                    agg_recptr->ag_dirty = 1;
                    fsck_send_msg( fsck_BADDIRENTRY, 0, 1 );
                    }  /* end we don't have write access */
                  }  /* end else it represents a bad entry */
                }  /* end while */
              if( !other_adjustments ) {
                this_inorec->adj_entries = 0;
                }
              else {
                this_inorec->adj_entries = 1;
                agg_recptr->corrections_needed = 1;
                }
              }  /* end adjust dir entries */
            if( (gpsr_rc == FSCK_OK) &&
                (this_inorec->cant_chkea) )  {  /* 
                           * wasn't able to check the EA format
                           */
              fsck_send_msg( fsck_CANTCHKEA, 0, 0 );
              }  /* end wasn't able to check the EA format */
            }  /* end not corrupt and either never selected to release... */
          }  /* end else inode is in use */
        }  /* end nothing fatal */
      }  /* end a record is allocated */
    if( gpsr_rc == FSCK_OK ) {
      gpsr_rc = get_inorecptr_next( aggregate_inode, &ino_idx, &this_inorec );
      }
    }  /* end while */
  return( gpsr_rc );
}                          /* end of report_problems_setup_repairs ()  */
	
	
/*****************************************************************************
 * NAME: initial_processing
 *
 * FUNCTION: Parse and verify invocation parameters.  Open the device and
 *           verify that it contains a JFS file system.  Check and repair 
 *           the superblock.  Initialize the fsck aggregate record.  Refresh
 *           the boot sector on the volume.  Issue some opening messages.    
 *
 *
 * PARAMETERS:  as specified to main()     
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t initial_processing ( int32 argc, char **argv )
{
  retcode_t  pi_rc = FSCK_OK;
  retcode_t  iml_rc = FSCK_OK;
  fsblkidx_t  fsblk_offset_bak;
  agg_byteidx_t  byte_offset_bak;
	
  /*
   * Initiate fsck service logging
   */
  iml_rc = fscklog_start();
	
  /*
   * Log the beginning of the chkdsk session
   */
  DosGetDateTime( &(fsck_DateTime) );
  if( fsck_DateTime.year > 2000 ) {
    sprintf( message_parm_0, "%d/%d/%d%d.%d.%d", 
             fsck_DateTime.month, fsck_DateTime.day, (fsck_DateTime.year%2000),
             fsck_DateTime.hours, fsck_DateTime.minutes, fsck_DateTime.seconds );
    }
  else {
    sprintf( message_parm_0, "%d/%d/%d%d.%d.%d", 
             fsck_DateTime.month, fsck_DateTime.day, (fsck_DateTime.year%1900),
             fsck_DateTime.hours, fsck_DateTime.minutes, fsck_DateTime.seconds );
    }
  msgprms[0] = message_parm_0;
  msgprmidx[0] = 0;
  fsck_send_msg( fsck_SESSSTART, 0, 1 ); 
	
  /*
   * Process the parameters given by the user
   */
  pi_rc = parse_parms( argc, argv );  /* parse the parms and record
                                       * them in the aggregate wsp record
                                       */
  if( pi_rc == FSCK_OK ) {  /* the parms are good */
    pi_rc = verify_parms();  /* validate the parms */
    }
	
  /*
   * If we're in autocheck mode, print a blank line to prime for messages
   */
  if( agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK] ) {
    printf("\n");
    }

  /*
   * Open the device and verify that it contains a valid JFS aggregate
   * If it does, check/repair the superblock.
   */
  if( pi_rc == FSCK_OK )  {  /* parms are good */
    msgprms[0] = Vol_Label;
    msgprmidx[0] = 0;
    fsck_send_msg( fsck_DRIVEID, 0, 1 );
    pi_rc = open_volume( Vol_Label );
    if( pi_rc != FSCK_OK ) {  /* device open failed */
      fsck_send_msg( fsck_CNTRESUPB, 0, 0 );
      }  /* end device open failed */
    else {  /* device is open */
      agg_recptr->device_is_open = 1;
      pi_rc = validate_repair_superblock();
      if(pi_rc == FSCK_OK) {    /* a valid superblock */
        fsck_send_msg( fsck_DRIVETYPE, 0, 0 );
             /*
              * add some stuff to the agg_record which is based on
              * superblock fields
              */
        agg_recptr->log2_blksize = log2shift( sb_ptr->s_bsize );
        agg_recptr->blksperpg = BYTESPERPAGE / sb_ptr->s_bsize;
        agg_recptr->log2_blksperpg = log2shift( agg_recptr->blksperpg );
        agg_recptr->log2_blksperag = log2shift( sb_ptr->s_agsize );
        agg_recptr->highest_valid_fset_datablk =
                              addressPXD(&(sb_ptr->s_fsckpxd)) - 1; /*
                                      * highest is the last one before
                                      * the in-aggregate journal log
                                      */
        agg_recptr->lowest_valid_fset_datablk =
                               addressPXD(&(sb_ptr->s_ait2)) +
                               (INODE_EXTENT_SIZE / sb_ptr->s_bsize) + 1; /*
                                      * lowest is the first after the
                                      * secondary aggreg inode table
                                      */
        agg_recptr->num_ag = ((sb_ptr->s_size * sb_ptr->s_pbsize /
                             sb_ptr->s_bsize ) + sb_ptr->s_agsize - 1) /
                             sb_ptr->s_agsize;  /*
                                      *  agg size in logical blks is
                                      *    (size in phys blks times
                                      *     phys blk size divided by
                                      *     log blk size)
                                      *  number of AGs in the aggregate:
                                      *     (agg size in log blks plus
                                      *      AG size in log blks minus 1)
                                      *     divided by (AG size in log
                                      *     blks)
                                      */
         agg_recptr->sb_agg_fsblk_length =
                       sb_ptr->s_size * sb_ptr->s_pbsize / sb_ptr->s_bsize;
         agg_recptr->ondev_jlog_fsblk_length =
                               lengthPXD( &(sb_ptr->s_logpxd) ); /*
                                      * length of the on-device
                                      * journal log
                                      */
         agg_recptr->ondev_jlog_fsblk_offset =
                               addressPXD( &(sb_ptr->s_logpxd) ); /*
                                      * aggregate block offset of
                                      * the on-device journal log
                                      */
ondev_jlog_byte_offset = 
   agg_recptr->ondev_jlog_fsblk_offset * sb_ptr->s_bsize;
         agg_recptr->ondev_fscklog_byte_length = 
                               sb_ptr->s_fsckloglen * sb_ptr->s_bsize;  /*
                                      * length of the on-device fsck
                                      * service log
                                      */
         agg_recptr->ondev_fscklog_fsblk_length = sb_ptr->s_fsckloglen;  /*
                                      * length of the on-device fsck
                                      * service log
                                      */
         agg_recptr->ondev_wsp_fsblk_length = 
                                lengthPXD( &(sb_ptr->s_fsckpxd) ) - 
                                agg_recptr->ondev_fscklog_fsblk_length; /*
                                      * length of the on-device fsck
                                      * workspace
                                      */
         agg_recptr->ondev_wsp_byte_length =
                   agg_recptr->ondev_wsp_fsblk_length * sb_ptr->s_bsize; /*
                                      * length of the on-device fsck
                                      * workspace
                                      */
         agg_recptr->ondev_wsp_fsblk_offset =
                               addressPXD( &(sb_ptr->s_fsckpxd) ); /*
                                      * aggregate block offset of
                                      * the on-device fsck workspace
                                      */
         agg_recptr->ondev_wsp_byte_offset =
                   agg_recptr->ondev_wsp_fsblk_offset * sb_ptr->s_bsize; /*
                                      * byte offset of the on-device
                                      * fsck workspace
                                      */
         agg_recptr->ondev_fscklog_fsblk_offset =
                               agg_recptr->ondev_wsp_fsblk_offset + 
                               agg_recptr->ondev_wsp_fsblk_length; /*
                                      * aggregate block offset of
                                      * the on-device fsck workspace
                                      */
         agg_recptr->ondev_fscklog_byte_offset =
                               agg_recptr->ondev_wsp_byte_offset + 
                               agg_recptr->ondev_wsp_byte_length; /*
                                      * byte offset of the on-device
                                      * fsck workspace
                                      */
              /* 
               * The offsets now assume the prior log (the one to overwrite) is
               * 1st in the aggregate fsck service log space.  Adjust if needed.
               */
         if( sb_ptr->s_fscklog == 0 ) {  /* first time ever for this aggregate */
           fsblk_offset_bak = agg_recptr->ondev_fscklog_fsblk_offset;
           byte_offset_bak = agg_recptr->ondev_fscklog_byte_offset;
		/*
		 * initialize the 2nd service log space
		 *
		 * (we'll actually write the log to the 1st space, so
		 * we'll initialize it below)
		 */
           agg_recptr->ondev_fscklog_fsblk_offset +=
                                    agg_recptr->ondev_fscklog_fsblk_length / 2;
           agg_recptr->ondev_fscklog_byte_offset += 
                                    agg_recptr->ondev_fscklog_byte_length / 2;
           agg_recptr->fscklog_agg_offset = 
                                    agg_recptr->ondev_fscklog_byte_offset;
           fscklog_init();
           sb_ptr->s_fscklog = 1;
           agg_recptr->ondev_fscklog_fsblk_offset = fsblk_offset_bak;
           agg_recptr->ondev_fscklog_byte_offset = byte_offset_bak;
           }  /* end first time ever for this aggregate */
         else if( sb_ptr->s_fscklog == 1 ) {  /* the 1st is most recent */
           sb_ptr->s_fscklog = 2;
           agg_recptr->ondev_fscklog_fsblk_offset +=
                                    agg_recptr->ondev_fscklog_fsblk_length / 2;
           agg_recptr->ondev_fscklog_byte_offset += 
                                    agg_recptr->ondev_fscklog_byte_length / 2;
           }  /* end the 1st is most recent */
         else {  /* the 1st is the one to overwrite */
           sb_ptr->s_fscklog = 1;
           }  /* end the 1st is the one to overwrite */
         agg_recptr->fscklog_agg_offset = agg_recptr->ondev_fscklog_byte_offset;
              /* 
               * Initialize the service log 
               */
         fscklog_init();
         agg_recptr->blocks_used_in_aggregate =
                                  agg_recptr->ondev_wsp_fsblk_length +
                                  agg_recptr->ondev_fscklog_fsblk_length +
                                  agg_recptr->ondev_jlog_fsblk_length; /*
                                      * from the user's perspective, these
                                      * are in use (by jfs)
                                      */
         agg_recptr->superblk_ok = 1;
		/*
		 * note whether dasd limits are enabled for the volume
		 */
       if( agg_recptr->processing_readwrite ) {  /* have write access */        /* @D3 */
         if((sb_ptr->s_flag & JFS_DASD_ENABLED) == JFS_DASD_ENABLED ) {         /* @F1 */
              agg_recptr->active_dasd_limits = 1;                               /* @F1 */
              }                                                                 /* @F1 */
         if((sb_ptr->s_flag & JFS_DASD_PRIME) == JFS_DASD_PRIME ) {             /* @F1 */
              agg_recptr->prime_dasd_limits = 1;                                /* @F1 */
              agg_recptr->corrections_needed = 1;                               /* @F1 */
              agg_recptr->corrections_approved = 1;                             /* @F1 */
              }                                                                 /* @F1 */
         }  /* end have write access */                                         /* @D3 */

         if( agg_recptr->processing_readwrite ) {
           refresh_bootsec();        /* refresh the boot sector and continue, 
                                      * regardless of outcome
                                      */
           }
             /*
              * give the caller some information
              */
         if( ((sb_ptr->s_state & FM_MOUNT) == FM_MOUNT) &&
               (!agg_recptr->parm_options[UFS_CHKDSK_CLRBDBLKLST]) &&
               (agg_recptr->processing_readonly)             ) {  /* agg is mounted */
           fsck_send_msg( fsck_FSMNTD, 0, 0 );
           fsck_send_msg( fsck_MNTFSYS2, 0, 0 );
           }  /* end agg is mounted */
         if( (!agg_recptr->parm_options[UFS_CHKDSK_LEVEL0]) &&
             (agg_recptr->processing_readonly) ) { /* user did not
                                      * specify check only but we can
                                      * only do check because we don't
                                      * have write access
                                      */
           fsck_send_msg( fsck_WRSUP, 0, 0 );
           }
        }  /* end a valid superblock */
      }  /* end device is open */
    }  /* end parms are good */
	
  if( pi_rc != FSCK_OK ) {
    agg_recptr->fsck_is_done = 1;
    }
	
  return( pi_rc );
}                              /* end of initial_processing ()  */
	
	
/*****************************************************************************
 * NAME: parse_parms
 *
 * FUNCTION:  Parse the invocation parameters.  If any unrecognized
 *            parameters are detected, or if any required parameter is
 *            omitted, issue a message and exit.
 *
 * PARAMETERS:  as specified to main()     
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t parse_parms ( int32 argc, char **argv )
{
  retcode_t pp_rc = FSCK_OK;
  int       i = 0;
  int       bad_level_parm = 0;
  char      *lvolume = NULL;
  char      *argp;
  for (i = 1; i < argc; i++) {  /* for all parms on command line */
    argp = argv[i];
    if (*argp == '/' || *argp == '-') {  /* leading / or - */
      argp++;
      if (*argp == 'd' || *argp == 'D') {  /* debug */
              /*
               * Debug option
               */
        agg_recptr->parm_options[UFS_CHKDSK_DEBUG] = -1;
        fsck_send_msg( fsck_PARMDEBUG, 0, 0 ); 
        }  /* end debug */
      else if (*argp == 'a' || *argp == 'A') {  /* autocheck */
              /*
               * Autocheck option
               */
        agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK] = -1;
        fsck_send_msg( fsck_PARMAUTOCHK, 0, 0 ); 
        }  /* end autocheck */
      else if (*argp == 'p' || *argp == 'P') {  /* pmchkdsk */
              /*
               * PMchkdsk option
               */
        pm_front_end = 1;
        fsck_send_msg( fsck_PARMPMCHK, 0, 0 ); 
        }  /* end autocheck */
      else if (*argp == 'f' || *argp == 'F')  {  /* level selector */
        argp++;
        if( *argp == '\0' ) {  /* nothing follows the F|f */
                /*
                 * Level2 (preen) option
                 */
          agg_recptr->parm_options[UFS_CHKDSK_LEVEL2] = -1;
          }  /* end nothing follows the F|f */
        else if( *argp == ':' ) {  /* a level number will be given */
          argp++;
          if( *(argp + 1) == '\0' ) {  /* a single char follows the colon */
            if( *argp == '0' ) {  /* level 0 */
                    /*
                     * Level0 (no write access) option
                     */
              agg_recptr->parm_options[UFS_CHKDSK_LEVEL0] = -1;
              sprintf( message_parm_0, "%d", 0 );
              msgprms[0] = message_parm_0;
              msgprmidx[0] = 0;
              fsck_send_msg( fsck_PARMFIXLVL, 0, 1 ); 
              }  /* end level 0 */
            else if( *argp == '1' ) {  /* level 1 */
                    /*
                     * Level1 (ask about everything) option
                     */
              agg_recptr->parm_options[UFS_CHKDSK_LEVEL1] = -1;
              sprintf( message_parm_0, "%d", 1 );
              msgprms[0] = message_parm_0;
              msgprmidx[0] = 0;
              fsck_send_msg( fsck_PARMFIXLVL, 0, 1 ); 
              }  /* end level 1 */
            else if( *argp == '2' ) {  /* level 2 */
                    /*
                     * Level2 (preen) option
                     */
              agg_recptr->parm_options[UFS_CHKDSK_LEVEL2] = -1;
              sprintf( message_parm_0, "%d", 2 );
              msgprms[0] = message_parm_0;
              msgprmidx[0] = 0;
              fsck_send_msg( fsck_PARMFIXLVL, 0, 1 ); 
              }  /* end level 2 */
            else if( *argp == '3' ) {  /* level 3 */
                    /*
                     * Level3 (yes to everything) option
                     */
              agg_recptr->parm_options[UFS_CHKDSK_LEVEL3] = -1;
              sprintf( message_parm_0, "%d", 3 );
              msgprms[0] = message_parm_0;
              msgprmidx[0] = 0;
              fsck_send_msg( fsck_PARMFIXLVL, 0, 1 ); 
              }  /* end level 3 */
            else {  /* given level number is not valid */
              bad_level_parm = -1;
              }  /* end given level number is not valid */
            }  /* end a single char follows the colon */
          else {  /* given level number is not valid */
            bad_level_parm = -1;
            }  /* end given level number is not valid */
          }  /* end a level number will be given */
        else {  /* something not valid follows the F|f */
          bad_level_parm = -1;
          }  /* end something not valid follows the F|f */
        if( bad_level_parm ) {  /* don't know what they want */
          msgprms[0] = argv[i];
          msgprmidx[0] = 0;
          fsck_send_msg( fsck_PRMUNRECOPTION, 0, 1 );
          return( FSCK_FAILED );
          }  /* end don't know what they want */
        }  /* end level selector */
      else if (*argp == 'c' || *argp == 'C') {  /* ifdirty */
              /*
               * IfDirty option
               */
        agg_recptr->parm_options[UFS_CHKDSK_IFDIRTY] = -1;
        fsck_send_msg( fsck_PARMIFDRTY, 0, 0 ); 
        }  /* end ifdirty */
      else if (*argp == 'v' || *argp == 'V') {  /* verbose */
              /*
               * Verbose option
               */
        agg_recptr->parm_options[UFS_CHKDSK_SHOWFILES] = -1;
        fsck_send_msg( fsck_PARMVERBOSE, 0, 0 ); 
        }  /* end verbose */
      else if (*argp == 'b' || *argp == 'B') {  /* Clear LVM Bad Block List utility */
              /*
               * Clear LVM Bad Block List utility option
               */
        agg_recptr->parm_options[UFS_CHKDSK_CLRBDBLKLST] = -1;
        fsck_send_msg( fsck_PARMCLRBDBLKLST, 0, 0 ); 
        }  /* end  Clear LVM Bad Block List utility */
      else if (*argp == 'o' || *argp == 'O') {  /* Omit logredo() processing */
              /*
               * Omit logredo() processing option
               */
        agg_recptr->parm_options[UFS_CHKDSK_SKIPLOGREDO] = -1;
        agg_recptr->parm_options_nologredo = 1;
        fsck_send_msg( fsck_PARMOMITLOGREDO, 0, 0 ); 
        }  /* end  Omit logredo() processing */
      else {  /* unrecognized parm */
        msgprms[0] = argp;
        msgprmidx[0] = 0;
        fsck_send_msg( fsck_PRMUNSUPPENUM, 0, 1 );
        fsck_send_msg( fsck_PRMUSAGE, 0, 0 );
        return( FSCK_FAILED );
        }  /* end unrecognized parm */
      }  /* end leading / or - */
    else if (argp[1] == ':') {  /* 2nd char is : */
      lvolume = argp;
      }  /* end 2nd char is : */
    else {  /* unrecognized parm */
      msgprms[0] = argp;
      msgprmidx[0] = 0;
      fsck_send_msg( fsck_PRMUNSUPPENUM, 0, 1 );
      fsck_send_msg( fsck_PRMUSAGE, 0, 0 );
      return( FSCK_FAILED );
      }  /* end unrecognized parm */
    }  /* end for all parms on command line */
  if (lvolume == NULL) {  /* no device specified */
    Vol_Label = default_vol;
    pp_rc = default_volume();
    }  /* end no device specified */
  else {  /* got a device */
    Vol_Label = lvolume;
    }  /* end got a device */
  return( pp_rc );
}                            /* end of parse_parms()  */
	
	
/*****************************************************************************
 * NAME: phase0_processing
 *
 * FUNCTION:  Log Redo processing.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t  phase0_processing ( )
{
  retcode_t  p0_rc = FSCK_OK;
  retcode_t logformat_rc = FSCK_OK;
  fsblkidx_t agg_blks;
  int32  use_2ndary_superblock = 0;
	 
  agg_recptr->logredo_rc = FSCK_OK;
	/*
	 * if this flag is set then the primary superblock is 
	 * corrupt.  The secondary superblock is good, but we
	 * weren't able to fix the primary version.  logredo can
	 * run, but must use the secondary version of the 
	 * aggregate superblock
	 */
  if( agg_recptr->cant_write_primary_sb == 1 ) {
      use_2ndary_superblock = -1;
      }

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("JFS chkdsk starting PHASE 0\n\r");
}
#endif

  /*
   * start off phase 0
   */
  msgprms[0] = message_parm_0;
  msgprmidx[0] = 0;
  sprintf( message_parm_0, "%ld", sb_ptr->s_bsize );
  fsck_send_msg( fsck_FSSMMRY1, 0, 1 );
  agg_blks = agg_recptr->sb_agg_fsblk_length +
             agg_recptr->ondev_jlog_fsblk_length +
             agg_recptr->ondev_fscklog_fsblk_length +
             agg_recptr->ondev_wsp_fsblk_length; /*
                                * aggregate size in fs blocks, by
                                * the user's point of view.
                                */
  sprintf( message_parm_0, "%lld", agg_blks );
  msgprms[0] = message_parm_0;
  msgprmidx[0] = 0;
  fsck_send_msg( fsck_FSSMMRY2, 0, 1 );
  /*
   * logredo processing
   */
  if( (agg_recptr->processing_readwrite) &&  /* read/write access AND */
      (!agg_recptr->parm_options_nologredo)  /* user didn't say not to */
     ) {  /* need to invoke logredo */
    fsck_send_msg( fsck_PHASE0, 0, 0 );

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("JFS chkdsk calling logredo \n\r" );
}
#endif
	/*
	 * write the superblock to commit any changes we have made in it
	 */
    if( use_2ndary_superblock ) {
        ujfs_put_superblk(Dev_IOPort, sb_ptr, 0); /* put 2ndary */
        }
    else {
        ujfs_put_superblk(Dev_IOPort, sb_ptr, 1); /* put primary  */
        }

    agg_recptr->logredo_rc =
                     jfs_logredo( Vol_Label, Dev_IOPort, use_2ndary_superblock );
#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("JFS chkdsk back from logredo, rc = %ld \n\r", 
       agg_recptr->logredo_rc );
}
#endif

    sprintf( message_parm_0, "%ld", agg_recptr->logredo_rc );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    if( agg_recptr->logredo_rc != FSCK_OK )  {  /* logredo failed */
      fsck_send_msg( fsck_LOGREDOFAIL, 0, 1 );
      }  /* end logredo failed */
    else {
      fsck_send_msg( fsck_LOGREDORC, 0, 1 );
      }
	/*
	 * logredo may change the superblock, so read it in again
	 */
    if( use_2ndary_superblock ) {
        ujfs_get_superblk(Dev_IOPort, sb_ptr, 0); /* get 2ndary */
        }
    else {
        ujfs_get_superblk(Dev_IOPort, sb_ptr, 1); /* get primary  */
        }
    }  /* end need to invoke logredo */

  if( agg_recptr->parm_options[UFS_CHKDSK_IFDIRTY] &&
      (!agg_recptr->parm_options_nologredo)   && 
      ((sb_ptr->s_state & FM_DIRTY) != FM_DIRTY)   &&
      (agg_recptr->logredo_rc == FSCK_OK)  &&
      ((sb_ptr->s_flag & JFS_BAD_SAIT) != JFS_BAD_SAIT)   ) { /*                   @D1C
                                * user specified 'only if dirty'
			* and didn't specify 'omit logredo()'
			* and logredo was successful 
                                * and the aggregate is clean
                                */
    agg_recptr->fsck_is_done = 1;
    }  /* end user specified 'only if dirty' and the aggregate is clean */
	/*
	 * if things look ok so far, make storage allocated by logredo()
	 * available to chkdsk processing.
	 */
  if( p0_rc == FSCK_OK ) {
    p0_rc = release_logredo_allocs();
    }

  if( p0_rc != FSCK_OK ) {
    agg_recptr->fsck_is_done = 1;
    }
  return( p0_rc );
}                              /* end of phase0_processing ()  */
	
	
/*****************************************************************************
 * NAME: phase1_processing
 *
 * FUNCTION:  Initialize the fsck workspace.  Process the aggregate-owned
 *            inodes.  Process fileset special inodes.  
 *            
 *            If any aggregate block is now multiply-allocated, then it
 *            is allocated to more than 1 special inode.  Exit.
 * 
 *            Process all remaining inodes.  Count links from directories
 *            to their child inodes.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t  phase1_processing ( )
{
  retcode_t  p1_rc = FSCK_OK;
  int  dup_allocs_detected = 0;

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("JFS chkdsk starting PHASE 1\n\r");
}
#endif
  fsck_send_msg( fsck_PHASE1, 0, 0 );
  p1_rc = establish_io_buffers();
  if( p1_rc == FSCK_OK ) {  /* I/O buffers established */
    p1_rc = establish_agg_workspace();  /* establish workspace
                                * related to the aggregate
                                */
    if( p1_rc == FSCK_OK ) {  /* aggregate workspace established */
      p1_rc = record_fixed_metadata();
#ifdef _JFS_DEBUG
 printf("phase1_processing  Record fixed metadata=%u\n\r",p1_rc);
#endif
      if( p1_rc == FSCK_OK ) {  /* fixed metadata recorded */
		/*
		 * note that this processing will use the vlarge
		 * buffer and then return it for reuse
		 */
        p1_rc = validate_select_agg_inode_table();
#ifdef _JFS_DEBUG
 printf("phase1_processing  validate_select_agg_inode_table=%u\n\r",p1_rc);
#endif
        if( p1_rc == FSCK_OK ) {  /* we have an ait to work with */
          p1_rc = establish_fs_workspace();  /* establish workspace
                                * related to the fileset
                                */
#ifdef _JFS_DEBUG
 printf("phase1_processing establish=%u\n\r",p1_rc);
#endif
          if( p1_rc == FSCK_OK )  {  /* no surprises when
                                * revisiting the imap inodes
                                */
            p1_rc = record_dupchk_inode_extents();
#ifdef _JFS_DEBUG
 printf("phase1_processing record dupcheck=%u\n\r",p1_rc);
#endif
            if( p1_rc == FSCK_OK ) {  /* 
			* fs workspace established 
			*/
              establish_ea_iobuf( );  /* claim the vlarge buffer for
			* validating EAs.  We do this now because
			* it IS possible that the root directory (validated
			* in the call that follows) has an EA attached.
			*/
              p1_rc = validate_fs_metadata();  /* verify the metadata
                                * inodes for all filesets in the aggregate
                                */
#ifdef _JFS_DEBUG
 printf("phase1_processing  validate metadata=%u\n\r",p1_rc);
#endif
              if( p1_rc == FSCK_OK ) {  /* no show stoppers in fs
                                * metadata inodes
                                */
                p1_rc = fatal_dup_check();  /* check for blocks allocated
                                * to 2 or more metadata objects
                                */
#ifdef _JFS_DEBUG
 printf("phase1_processing fatal dup check=%u\n\r",p1_rc);
#endif
                if( p1_rc == FSCK_OK ) {  /*
                                * no fatal duplicate allocs
                                */
			/*
			 * validate the fileset inodes
			 */
                  p1_rc = validate_fs_inodes ();
#ifdef _JFS_DEBUG
 printf("phase1_processing validate inode=%u\n\r",p1_rc);
#endif
			/*
			 * return the vlarge buffer for reuse
			 */
                  agg_recptr->ea_buf_ptr = NULL;
                  agg_recptr->ea_buf_length = 0;
                  agg_recptr->vlarge_current_use = NOT_CURRENTLY_USED; 
                  }  /* end no fatal duplicate allocs */
                }  /* end no show stoppers in fs metadata inodes */
              }  /* end fs workspace established */
            }  /* end no surprises when revisiting the imap inodes */
          }  /* end we have an ait to work with */
        }  /* end fixed metadata recorded */
      }  /* end aggregate workspace established */
    }  /* end I/O buffers established */
  if( p1_rc != FSCK_OK ) {
    agg_recptr->fsck_is_done = 1;
    if( p1_rc > 0 ) {	/* this isn't a chkdsk failure */
      p1_rc = 0;
      }
    }
  return( p1_rc );
}                              /* end of phase1_processing ()  */
	
	
/*****************************************************************************
 * NAME: phase2_processing
 *
 * FUNCTION:  Scan the inodes.  If any inode has more than 1 link from
 *	      any single directory then, in Release I of JFS, the 
 *	      directory must be corrupt.
 *
 *	      Count the link from each directory inode to its parent inode.
 *            Verify that the link count stored in each in-use inode matches
 *            the link count observed by fsck.  If not, get (once to cover
 *            all incorrect link counts) for permission to correct them.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t  phase2_processing ( )
{
  retcode_t  p2_rc = FSCK_OK;

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("JFS chkdsk starting PHASE 2\n\r");
}
#endif
  
  fsck_send_msg( fsck_PHASE2, 0, 0 );
 
  p2_rc = check_dir_integrity();
  if( p2_rc == FSCK_OK ) {
    p2_rc = check_link_counts();
    }
  
  if( p2_rc != FSCK_OK ) {
    agg_recptr->fsck_is_done = 1;
    }
  return( p2_rc );
}                              /* end of phase2_processing ()  */
 	
 	
/*****************************************************************************
 * NAME: phase3_processing
 *
 * FUNCTION:  If any mulitply-allocated blocks have been detected, find
 *            the first reference to each.  For each in-use directory inode,
 *            verify that it has exactly 1 parent and that the parent inode
 *            number stored in the inode matches the parent observed by
 *            fsck.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t  phase3_processing ( )
{
  retcode_t  p3_rc = FSCK_OK;
  inoidx_t   inoidx;                     /* loop control */
  fsck_inode_recptr this_inode_record;
  int repair_needed = 0;

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("JFS chkdsk starting PHASE 3\n\r");
}
#endif

  fsck_send_msg( fsck_PHASE3, 0, 0 );
  if( agg_recptr->unresolved_1stref_count > 0 )  {  /*
                        * there are unresolved first references to
                        * multiply-allocated blocks
                        */
    p3_rc = first_ref_check_fixed_metadata( );  /* see if any first
                        * references are by aggregate fixed metadata
                        */
    }  /* end there are unresolved first references to ... */
  if( (p3_rc == FSCK_OK) &&
      (agg_recptr->unresolved_1stref_count > 0) )  {  /*
                        * there are still unresolved first references
                        * to multiply-allocated blocks
                        */
    p3_rc = first_ref_check_agg_metadata( );  /* see if any first
                        * references are by aggregate metadata inodes
                        */
    }  /* end there are unresolved first references to ... */
  if( (p3_rc == FSCK_OK) &&
      (agg_recptr->unresolved_1stref_count > 0) )  {  /*
                        * there are still unresolved first references
                        * to multiply-allocated blocks
                        */
    p3_rc = first_ref_check_fs_metadata( );  /* see if any first
                        * references are by fileset metadata inodes
                        */
    }  /* end there are still unresolved first references ... */
  if( (p3_rc == FSCK_OK) &&
      (agg_recptr->unresolved_1stref_count > 0) )  {  /*
                        * there are still unresolved first references
                        * to multiply-allocated blocks
                        */
    p3_rc = first_ref_check_inode_extents( );  /* see if any first
                        * references are by inode extents (described in
                        * the IAGs)
                        */
    }  /* end there are still unresolved first references ... */
  if( p3_rc == FSCK_OK ) {  /* nothing fatal yet */
    p3_rc = check_parents_and_first_refs( );
    }  /* end nothing fatal yet */
  if( p3_rc != FSCK_OK ) {
    agg_recptr->fsck_is_done = 1;
    }

  return( p3_rc );
}                              /* end of phase3_processing ()  */
	
	
/*****************************************************************************
 * NAME: phase4_processing
 *
 * FUNCTION:  For each inode record, in fsck's workspace inode map, which
 *            has been flagged for some repair, get permission to perform
 *            the repair and adjust the map for repairs implied by approved
 *            repairs.  (E.g., if an inode is approved for release, then
 *            any directory entry for the inode is approved for removal,
 *            by implication.)
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t  phase4_processing ( )
{
  retcode_t  p4_rc = FSCK_OK;
  inoidx_t   inoidx;                     /* loop control */
  fsck_inode_recptr this_inode_record;
  int repair_needed = 0;

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("JFS chkdsk starting PHASE 4\n\r");
}
#endif

     /*
      * issue this message whether or not there are any corrections to ask
      * about so that when the next message comes out the user will know
      * that notifications are completed.
      */
  fsck_send_msg( fsck_PHASE4, 0, 0 );
  if( agg_recptr->corrections_needed ||
      agg_recptr->warning_pending       ) {  /* something was detected */
        /*
         * Get permission to perform indicated repairs.
         */
    p4_rc = report_problems_setup_repairs( );
    }  /* end something was detected */
  if( p4_rc != FSCK_OK ) {
    agg_recptr->fsck_is_done = 1;
    }
  return( p4_rc );
}                              /* end of phase4_processing ()  */
	
	
/*****************************************************************************
 * NAME: phase5_processing
 *
 * FUNCTION: Detect problems related to inode connectedness.  Identify
 *           each unconnected, in-use inode and flag it for reconnection.
 *           Identify each directory inode with multiple parents and get
 *           permission to repair it.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t  phase5_processing ( )
{
  retcode_t  p5_rc = FSCK_OK;
  inoidx_t   inoidx;                     /* loop control */
  fsck_inode_recptr this_inode_record;
  dinode_t    *this_inode;
  struct dtroot_t  *dtroot_ptr;

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("JFS chkdsk starting PHASE 5\n\r");
}
#endif

  if( agg_recptr->processing_readwrite )  {  /* have read/write access */
    fsck_send_msg( fsck_PHASE5, 0, 0 );
    p5_rc = check_connectedness( );

	/*
	 * now that all the parent child relationships have stabilized,
	 * if there are any active dasd limits in the file system, we
	 * can calculate the dasd used and verify the dinode field.
	 *
	 * If we're not going to just reset all dasd used fields, then
	 * check to see which ones are now incorrect.
	 */
    if( (p5_rc == FSCK_OK) && 
        ((agg_recptr->active_dasd_limits) || (agg_recptr->prime_dasd_limits)) 
      ) {                                                                                      /* @F1 */
      p5_rc = calculate_dasd_used();                                               /* @F1 */
      if( (p5_rc == FSCK_OK) && (!agg_recptr->prime_dasd_limits) ) {  /* @F1 */
        p5_rc = validate_dasd_used();                                               /* @F1 */
        }                                                                                       /* @F1 */
      }                                                                                         /* @F1 */

    if( p5_rc != FSCK_OK ) {
      agg_recptr->fsck_is_done = 1;
      }
    }  /* end have read/write access */
  return( p5_rc );
}                              /* end of phase5_processing ()  */
	
	
/*****************************************************************************
 * NAME: phase6_processing
 *
 * FUNCTION: Perform all approved inode corrections.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t  phase6_processing ( )
{
  retcode_t  p6_rc = FSCK_OK;
  retcode_t  intermed_rc;

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("JFS chkdsk starting PHASE 6\n\r");
}
#endif

  if( agg_recptr->processing_readwrite )  {  /* have read/write access */
    if( agg_recptr->corrections_approved ) {  /* we're going to do something */
         /*
          * don't issue this message unless we're actually going to do
          * something to one of the inodes, otherwise a user running
          * in terse/quiet mode might think we've messed around without
          * permission.
          */
      fsck_send_msg( fsck_PHASE6, 0, 0 );
         /*
          * we're about to write repairs.  if something stops
          * after we start and before we finish, we'll leave the
          * filesystem in an inconsistent state.  
          * Mark the superblock (and write it to the disk) to
          * show the filesystem is unmountable.  If we complete
          * successfully and all errors are corrected, we'll
          * be marking it clean later on.
          */
      sb_ptr->s_state |= FM_DIRTY;
      ujfs_put_superblk(Dev_IOPort, sb_ptr, 1); /* write to the
                              * primary superblock on the device.
                              */
      fsck_send_msg( fsck_REPAIRSINPROGRESS, 0, 0 );  /*
                              * log the fact the the superblock
                              * has just been marked dirty
                              */
      if( agg_recptr->inode_reconn_extens != NULL) {  /*
                              * there are inodes to reconnect
                              */
        intermed_rc = resolve_lost_and_found( );
        if( intermed_rc < 0 ) {  /* something fatal */
          p6_rc = intermed_rc;
          }
        }  /* end there are inodes to reconnect */
      if( p6_rc == FSCK_OK )  { /* nothing fatal */
        p6_rc = directory_buffers_alloc();  /* Allocate space for
                              * use during directory entry processing.
                              * (used both to add and to delete entries,
                              * so not restricted to reconnect processing)
                              */
        if( p6_rc == FSCK_OK ) {  /* reconnect buffers initialized */
          p6_rc = repair_fs_inodes( );
  
          if( p6_rc == FSCK_OK ) {  /* repairs successful */
            if( agg_recptr->dup_alloc_lst != NULL )  {  /*
                              * there are unresolved duplicate block
                              * allocations
                              */
              agg_recptr->ag_dirty = 1;
              }  /* end there are unresolved duplicate block allocations */
            if( agg_recptr->inode_reconn_extens != NULL )  {  /*
                              * there's something to reconnect
                              */
                /*
                 * Does /lost+found/ need anything before we start?
                 */
              if( (agg_recptr->lsfn_ok) &&
                  (agg_recptr->avail_inonum == agg_recptr->lsfn_inonum) ) { /*
                              * /lost+found/ needs to be created
                              */
                intermed_rc = create_lost_and_found();
                if( intermed_rc > 0 ) {  /* couldn't create it */
                  agg_recptr->lsfn_ok = 0;    /* can't reconnect anything */
                  }  /* end couldn't create it */
                else if( intermed_rc < 0 )  {  /* something fatal */
                  p6_rc = intermed_rc;         /* pass it back */
                  }  /* end something fatal */
                }  /* end /lost+found/ needs to be created */
              if( (p6_rc == FSCK_OK) && (agg_recptr->lsfn_ok) ) {
                p6_rc = reconnect_fs_inodes( );
                }
              }  /* end there's something to reconnect */
            }  /* end repairs successful */
          intermed_rc = directory_buffers_release();
          }  /* end reconnect buffers initialized */
        }  /* end nothing fatal */
      if( p6_rc != FSCK_OK ) {
        agg_recptr->fsck_is_done = 1;
        }
      }  /* end we're going to do something */
    }  /* end have read/write access */
  return( p6_rc );
}                              /* end of phase6_processing ()  */
	
	
/*****************************************************************************
 * NAME: phase7_processing
 *
 * FUNCTION: Rebuild (or verify, processing read-only) the Aggregate Inode
 *           Allocation Map, the Aggregate Inode Allocation Table, the
 *           Fileset Inode Allocation Map, and the Fileset Inode Allocation
 *           Table.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t  phase7_processing ( )
{
  retcode_t p7_rc = FSCK_OK;

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("JFS chkdsk starting PHASE 7\n\r");
}
#endif

  if( agg_recptr->processing_readwrite )  {  /* have read/write access */
    fsck_send_msg( fsck_PHASE7R, 0, 0 );
    p7_rc = rebuild_agg_iamap();
    if( p7_rc >= FSCK_OK )  { /* nothing fatal happened */
      p7_rc = rebuild_fs_iamaps();
      if( p7_rc >= FSCK_OK ) {  /* still in business */
	/*
	 * note that this processing will use the vlarge
	 * buffer and then return it for reuse
	 */
        p7_rc = AIS_replication();  /* 
                               * refresh the redundancy of the
                               * aggregate inode table and map
                               */
        }  /* end still in business */
      }  /* nothing fatal happened */
    }  /* end have read/write access */
  else {  /* else processing read only */
    fsck_send_msg( fsck_PHASE7V, 0, 0 );
    p7_rc = verify_agg_iamap();
    if( p7_rc >= FSCK_OK )  { /* nothing fatal happened */
      p7_rc = verify_fs_iamaps();
      }  /* nothing fatal happened */
    }  /* end else processing read only */
  if( p7_rc != FSCK_OK ) {
    agg_recptr->fsck_is_done = 1;
    }
  return( p7_rc );
}                              /* end of phase7_processing ()  */
	
	
/*****************************************************************************
 * NAME: phase8_processing
 *
 * FUNCTION:  Rebuild (or verify, processing read-only) the Aggregate Block
 *            Allocation Map.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t  phase8_processing ( )
{
  retcode_t  p8_rc = FSCK_OK;

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("JFS chkdsk starting PHASE 8\n\r");
}
#endif

  if( agg_recptr->processing_readwrite )  {  /* have read/write access */
     fsck_send_msg( fsck_PHASE8R, 0, 0 );
    p8_rc = rebuild_blkall_map( );
    }  /* end have read/write access */
  else {  /* else processing read only */
    fsck_send_msg( fsck_PHASE8V, 0, 0 );
    p8_rc = verify_blkall_map( );
    }  /* end else processing read only */
  if( p8_rc != FSCK_OK ) {
    agg_recptr->fsck_is_done = 1;
    }
  return( p8_rc );
}                              /* end of phase8_processing ()  */
	
	
/*****************************************************************************
 * NAME: phase9_processing
 *
 * FUNCTION:  If logredo failed during its processing, reformat the journal
 *            log.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t  phase9_processing ( )
{
  retcode_t  p9_rc = FSCK_OK;
  int logformat_called = 0;

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("JFS chkdsk starting PHASE 9\n\r");
}
#endif

  agg_recptr->logformat_rc = FSCK_OK;
  if( agg_recptr->processing_readwrite )  {  /* have read/write access */
    if( (agg_recptr->logredo_rc < 0) ||
         (agg_recptr->parm_options_nologredo) ) {  /* log needs reformat */
      if( sb_ptr->s_flag & JFS_INLINELOG ) {  /* log is inline */
        fsck_send_msg( fsck_PHASE9, 0, 0 );
        logformat_called = -1;

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("JFS chkdsk calling logformat \n\r");
}
#endif

	agg_recptr->logformat_rc = jfs_logform( Dev_IOPort, sb_ptr->s_bsize,
                                                sb_ptr->s_l2bsize,sb_ptr->s_flag,
                                                addressPXD( &(sb_ptr->s_logpxd) ),
                                                lengthPXD( &(sb_ptr->s_logpxd) ),
                                                NULL, 0 );

#ifdef _JFS_DEBUG
if(agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK]) {
 printf("JFS chkdsk back from logformat, rc = %ld \n\r", 
       agg_recptr->logformat_rc );
}
#endif

        }  /* end log is inline */
      else if( sb_ptr->s_logdev != 0 ) {  /* log is outline */
        fsck_send_msg( fsck_PHASE9, 0, 0 );
        /*
         * the first release of JFS/OS2 does not support a log
         * outside the aggregate
         */
        fsck_send_msg( fsck_LOGSPECINVALID, 0, 0 );
/*
        logformat_called = -1;
	agg_recptr->logformat_rc = jfs_logform( -1, sb_ptr->s_bsize, 
                                                sb_ptr->s_l2bsize, sb_ptr->s_flag, 
			                        0, 0, sb_ptr->s_logdev, 0 );
*/
        }  /* end log is outline */
      if( logformat_called ) {  /* we did call logformat */
        sprintf( message_parm_0, "%ld", agg_recptr->logformat_rc );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        if( agg_recptr->logformat_rc != FSCK_OK )  {  /* logredo failed */
          fsck_send_msg( fsck_LOGFORMATFAIL, 0, 1 );
          }  /* end logformat failed */
        else {
          fsck_send_msg( fsck_LOGFORMATRC, 0, 1 );
          }
        }  /* end we did call logformat */
      }  /* end and need to reformat the log */
    }  /* end have read/write access */
  if( p9_rc != FSCK_OK ) {
    agg_recptr->fsck_is_done = 1;
    }
  return( p9_rc );
}                              /* end of phase9_processing ()  */
	
	
/*****************************************************************************
 * NAME: repair_fs_inodes
 *
 * FUNCTION:  Perform approved repairs on fileset inodes, including the
 *            implied repairs.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t repair_fs_inodes ( )
{
  retcode_t rfsi_rc = FSCK_OK;
  inoidx_t ino_idx;
  fsck_inode_extptr this_ext;
  fsck_inode_extptr ext_list;
  fsck_inode_recptr this_inorec;
  int aggregate_inode = 0;      /* going for fileset inodes only */
  int alloc_ifnull = 0;
  int write_inode;                                                                     /* @F1 */
  int which_it = 16;             /* in release 1 there is only fileset 0 */
  dinode_t *ino_ptr;
  struct fsck_ino_msg_info  ino_msg_info;
  fsck_msg_info_ptr msg_info_ptr;
  int64 dasd_used, dasd_limit;
                                                   /* @F1 */

  msg_info_ptr = &ino_msg_info;
  msg_info_ptr->msg_inopfx = fsck_fset_inode;  /* all fileset owned */

  rfsi_rc = get_inorecptr_first( aggregate_inode, &ino_idx, &this_inorec );

  while( (rfsi_rc == FSCK_OK) && (this_inorec != NULL) ) {
    if( (this_inorec->in_use)                 &&
        ( (this_inorec->selected_to_rls)   ||
          (this_inorec->crrct_prnt_inonum) ||
          (this_inorec->adj_entries )      	||
          (this_inorec->clr_ea_fld)        	||
          (this_inorec->clr_acl_fld)        	||
          (this_inorec->crrct_cumm_blks) ||
          (this_inorec->inlineea_on)         ||
          (this_inorec->inlineea_off)       ||
          (this_inorec->reconnect)         	||
          (this_inorec->link_count != 0)      )   ) {  /* flagged for repair */
      rfsi_rc = inode_get( aggregate_inode, which_it,
                             ino_idx, &ino_ptr );  /* read it in */
      if( rfsi_rc == FSCK_OK ) {  /* nothing fatal */
        msg_info_ptr->msg_inonum = ino_idx;
        if( this_inorec->inode_type == directory_inode ) {
	  msg_info_ptr->msg_inotyp = fsck_directory;
	  }
        else if( this_inorec->inode_type == symlink_inode ) {
	  msg_info_ptr->msg_inotyp = fsck_symbolic_link;
	  }
        else {  /* a regular file */
	  msg_info_ptr->msg_inotyp = fsck_file;
	  }
        if( this_inorec->selected_to_rls )  {  /* release the inode */
          rfsi_rc = release_inode( ino_idx, this_inorec, ino_ptr );
          if( rfsi_rc == FSCK_OK ) {  /* it worked */
            msgprms[0] = message_parm_0;
            msgprmidx[0] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            fsck_send_msg( fsck_INOCLRD, 0, 2 );
            }  /* end it worked */
          }  /* release the inode */
        else {  /* not releasing the inode */

          if( this_inorec->link_count != 0 ) { /*
                               * correct/adjust link count
                               */
            ino_ptr->di_nlink += this_inorec->link_count;
            msgprms[0] = message_parm_0;
            msgprmidx[0] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            fsck_send_msg( fsck_INOLKCTFXD, 0, 2 );
            }  /* end correct/adjust link count */

          if( this_inorec->crrct_prnt_inonum )  {  /*
                               * correct parent inode number
                               */
            ino_ptr->di_parent = this_inorec->parent_inonum;
            msgprms[0] = message_parm_0;
            msgprmidx[0] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            fsck_send_msg( fsck_INCINOREFCRCT, 0, 2 );
            }  /* end correct parent inode number */

          if( this_inorec->crrct_cumm_blks )  {  /*                                     @F1
                               * correct DASD blocks used
                               */
            setDASDUSED(&(ino_ptr->di_DASD), this_inorec->cumm_blocks); /* @F1 */
            msgprms[0] = message_parm_0;                                         /* @F1 */
            msgprmidx[0] = msg_info_ptr->msg_inopfx;                         /* @F1 */
            sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum ); /* @F1 */
            msgprms[1] = message_parm_1;                                         /* @F1 */
            msgprmidx[1] = 0;                                                            /* @F1 */
            fsck_send_msg( fsck_INCDASDUSEDCRCT, 0, 2 );                   /* @F1 */
            fsck_send_msg( fsck_INOMINORFXD, 0, 2 );                           /* @F1 */
            }  /* end correct DASD blocks used                                        @F1 */                    

          else if( this_inorec->inode_type == directory_inode )  {  /*          @F1 
			* it's a directory not flagged for dasd used 
			* correction.
			*/
		/* 
		 * if we're priming the dasd used fields, do so.
		 * if dasd limits have just now been enabled on the
		 * volume, zero the limit field as well.
		 */
              if( agg_recptr->prime_dasd_limits ) {  /*                                 @F1 
			* we're priming dasd used fields
			*/
                  setDASDUSED(&(ino_ptr->di_DASD), this_inorec->cumm_blocks); /* @F1 */
                  if( ! agg_recptr->active_dasd_limits ) {                         /* @F1 */
                      setDASDLIMIT( &(ino_ptr->di_DASD), 0 );                   /* @F1 */
                      }                                                                          /* @F1 */
                  }  /* end we're priming dasd used fields                             @F1 */ 
              }  /* end else if it's a directory not flagged for dasd used...       @F1 */
	 
          if( this_inorec->reconnect ) { /*
                               * reconnect
                               */
            if( this_inorec->inode_type == directory_inode ) {  /*
                               * only directory inodes point back to their parent 
                               */
              if( agg_recptr->lsfn_ok )  {  /* /lost+found/ resolved */
                ino_ptr->di_parent = agg_recptr->lsfn_inonum;
                }  /* end /lost+found/ resolved */
              else {  /* couldn't resolve /lost+found/ */
                ino_ptr->di_parent = (ino_t) 0;
                }  /* end else couldn't resolve /lost+found/ */
              }  /* end only directory inodes point back to their parent */
                     /*
                      * N.B. No message is issued at this time since
                      *      reconnect processing for the inode is
                      *      not completed.  When all other inode
                      *      corrections have been processed, a directory
                      *      entry will be added to /lost+found/ for
                      *      each inode needing reconnection.  At that
                      *      time a message will be issued (assuming
                      *      the terse/verbose messaging switch is
                      *      set appropriately for that message).
                      */
            }  /* end reconnect */

          if( (rfsi_rc == FSCK_OK ) &&
              (this_inorec->clr_ea_fld)      )  {  /* clear EA */
            rfsi_rc = clear_EA_field( this_inorec, ino_ptr );
            if( rfsi_rc == FSCK_OK ) {
              msgprms[0] = message_parm_0;
              msgprmidx[0] = msg_info_ptr->msg_inopfx;
              sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
              msgprms[1] = message_parm_1;
              msgprmidx[1] = 0;
              fsck_send_msg( fsck_INOEACLRD, 0, 2 );
              }
            }  /* end clear EA */

          if( (rfsi_rc == FSCK_OK ) &&
              (this_inorec->clr_acl_fld)      )  {  /* clear ACL */
            rfsi_rc = clear_ACL_field( this_inorec, ino_ptr );
            if( rfsi_rc == FSCK_OK ) {
              msgprms[0] = message_parm_0;
              msgprmidx[0] = msg_info_ptr->msg_inopfx;
              sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
              msgprms[1] = message_parm_1;
              msgprmidx[1] = 0;
              fsck_send_msg( fsck_INOACLCLRD, 0, 2 );
              }
            }  /* end clear ACL */

          if( (rfsi_rc == FSCK_OK ) &&
              (this_inorec->inlineea_on)      )  {  /* sect 4 available flag on */
            ino_ptr->di_mode |= INLINEEA;
            msgprms[0] = message_parm_0;
            msgprmidx[0] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            fsck_send_msg( fsck_INOMINORFXD, 0, 2 );
            }  /* end sect 4 available flag on */

          if( (rfsi_rc == FSCK_OK ) &&
              (this_inorec->inlineea_off)      )  {  /* sect 4 available flag off */
            ino_ptr->di_mode &= ~INLINEEA;
            msgprms[0] = message_parm_0;
            msgprmidx[0] = msg_info_ptr->msg_inopfx;
            sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            fsck_send_msg( fsck_INOMINORFXD, 0, 2 );
            }  /* end sect 4 available flag off */

          if( (rfsi_rc == FSCK_OK ) && (this_inorec->adj_entries) )  {  /* 
                               * adjust  dir entries */
            ext_list = this_inorec->ext_rec;
            this_inorec->ext_rec = NULL;
            while ( (ext_list != NULL) && (rfsi_rc == FSCK_OK) ) {
              this_ext = ext_list;
              ext_list = ext_list->next;
              if( (this_ext->ext_type == rmv_badentry_extension) ||
                  (this_ext->ext_type == rmv_direntry_extension)    ) {
                rfsi_rc = direntry_remove( ino_ptr, this_ext->inonum ); /*
                                       * n.b. that the link count adjustment
                                       *      has already been handled.
                                       */
                if( rfsi_rc == FSCK_OK ) {
                  msgprms[0] = message_parm_0;
                  msgprmidx[0] = msg_info_ptr->msg_inopfx;
                  sprintf( message_parm_1, "%ld", msg_info_ptr->msg_inonum );
                  msgprms[1] = message_parm_1;
                  msgprmidx[1] = 0;
                  msgprms[2] = msgprms[0];
                  msgprmidx[2] = msgprmidx[0];
                  sprintf( message_parm_3, "%ld", this_ext->inonum );
                  msgprms[3] = message_parm_3;
                  msgprmidx[3] = 0;
                  fsck_send_msg( fsck_INOREFRMV, 0, 4 );
                  release_inode_extension( this_ext );
                  }
                }
              else {
                this_ext->next = this_inorec->ext_rec;
                this_inorec->ext_rec = this_ext;
                }
              }  /* end while */
            }  /* end adjust dir entries */
          }  /* end else not releasing the inode */

        if( rfsi_rc == FSCK_OK ) {
          rfsi_rc = inode_put( ino_ptr );   /* write it back to the device */
          }
        }  /* end nothing fatal */
      }  /* end flagged for repair */
	/*
	 * if the inode isn't flagged for repair, check to see if we
	 * are priming dasd used fields, and if so, whether this is an
	 * in-use directory inode.
	 */
    else if( agg_recptr->prime_dasd_limits && 
               this_inorec->in_use                &&
               (this_inorec->inode_type == directory_inode) )  {            /* @F1 */
 		/*
		 * read in the inode, set dasd used, and write it back out
		 */
        rfsi_rc = inode_get( aggregate_inode, which_it, ino_idx, &ino_ptr ); /* @F1 */

        if( rfsi_rc == FSCK_OK ) {  /* nothing fatal                                  @F1 */
            write_inode = 0;                                                              /* @F1 */
            dasd_used = DASDUSED( &(ino_ptr->di_DASD) );                  /* @F1 */
            if( dasd_used != this_inorec->cumm_blocks ) {                     /* @F1 */
                setDASDUSED(&(ino_ptr->di_DASD), this_inorec->cumm_blocks); /* @F1 */
                write_inode = -1;                                                         /* @F1 */
                }                                                                               /* @F1 */
            if( ! agg_recptr->active_dasd_limits ) {                               /* @F1 */
                  dasd_limit = DASDLIMIT( &(ino_ptr->di_DASD) );             /* @F1 */
                  if( dasd_limit != 0 )  {                                                  /* @F1 */
                      setDASDLIMIT( &(ino_ptr->di_DASD), 0 );                   /* @F1 */
                      write_inode = -1;                                                   /* @F1 */
                      }                                                                         /* @F1 */
                  }                                                                             /* @F1 */
            if( write_inode ) {                                                            /* @F1 */
                rfsi_rc = inode_put( ino_ptr );                                        /* @F1 */
                }                                                                               /* @F1 */
            }  /* end nothing fatal                                                         @F1 */
        }  /* not flagged for repair                                                       @F1 */

    if( rfsi_rc == FSCK_OK ) {
      rfsi_rc = get_inorecptr_next( aggregate_inode, &ino_idx, &this_inorec );
      }
    }  /* end while */

  return( rfsi_rc );
}                              /* end of repair_fs_inodes ()  */
	
	
/*****************************************************************************
 * NAME: report_dynstg_error
 *
 * FUNCTION:  Report a failure to allocate dynamic storage.
 *
 * PARAMETERS:  none  
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 */
void report_dynstg_error ( )
{
  sprintf( message_parm_0, "%d", wsp_dynstg_action );
  msgprms[0] = message_parm_0;
  msgprmidx[0] = 0;
  sprintf( message_parm_1, "%d", wsp_dynstg_object );
  msgprms[1] = message_parm_1;
  msgprmidx[1] = 0;
  fsck_send_msg( fsck_EXHDYNSTG, 0, 2 );
  return;
}                                         /* end report_dynstg_error () */
	
	
/*****************************************************************************
 * NAME: report_readait_error 
 *
 * FUNCTION:  Report failure to read the Aggregate Inode Table
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
void report_readait_error( retcode_t local_rc,
                           retcode_t global_rc,
                           int8      which_it
                          )
{
           /* 
            * message to user
            */
  msgprms[0] = message_parm_0;
  msgprmidx[0] = fsck_metadata;
  msgprms[1] = Vol_Label;
  msgprmidx[1] = 0;
  fsck_send_msg( fsck_URCVREAD, 0, 2 );
           /*
            * message to debugger
            */
  sprintf( message_parm_0, "%d", global_rc );
  msgprms[0] = message_parm_0;
  msgprmidx[0] = 0;
  sprintf( message_parm_1, "%d", local_rc );
  msgprms[1] = message_parm_1;
  msgprmidx[1] = 0;
  sprintf( message_parm_2, "%d", which_it );
  msgprms[2] = message_parm_2;
  msgprmidx[2] = 0;
  fsck_send_msg( fsck_ERRONAITRD, 0, 3 );
  return;
}                                       /* end report_readait_error () */
	
	
/*****************************************************************************
 * NAME: resolve_lost_and_found
 *
 * FUNCTION: Determine whether /lost+found/ already exists and, if so,
 *           whether it is a valid directory inode.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t resolve_lost_and_found ( )
{
  retcode_t rlaf_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  ino_t found_inonum = (ino_t) 0;
  int found_inoidx;
  int lsfn_length_in_UniChars;
  int LSFN_length_in_UniChars;
  int aggregate_inode = 0;            /* going for fileset inodes only */
  int alloc_ifnull = 0;
  int which_fsit = 0;                 /* release 1 has only 1 fileset */
  dinode_t *ino_ptr;
  fsck_inode_recptr inorecptr;
  /*
   * see if one exists now
   */
  lsfn_length_in_UniChars = UniStrlen(agg_recptr->UniChar_lsfn_name);
  LSFN_length_in_UniChars = UniStrlen(agg_recptr->UniChar_LSFN_NAME);
  intermed_rc = direntry_get_inonum( ROOT_I,
                                     lsfn_length_in_UniChars,
                                     agg_recptr->UniChar_lsfn_name,
                                     LSFN_length_in_UniChars,
                                     agg_recptr->UniChar_LSFN_NAME,
                                     &found_inonum );
  if( intermed_rc < 0 )  {  /* something fatal */
    rlaf_rc = intermed_rc;
    }  /* end something fatal */
  else if( found_inonum == (ino_t) 0 )  {  /* there isn't one yet */
    fsck_send_msg( fsck_LSFNNOTFOUND, 0, 0 );
    if( agg_recptr->avail_inonum == (ino_t) 0 ) {  /* and we didn't see any
                               * available inodes laying around
                               */
      
      agg_recptr->lsfn_inonum = (ino_t) 0;  /* don't have one  */
      agg_recptr->lsfn_ok = 0;             /* we won't reconnect anything */
      fsck_send_msg( fsck_LSFNCNTCRE, 0, 0 );
      }  /* end and we didn't see any available inodes laying around */
    else {  /* an available inode was seen */
      agg_recptr->lsfn_inonum = agg_recptr->avail_inonum;
      agg_recptr->lsfn_ok = 1;              /* we'll be able to create l+f */
      }  /* end else an available inode was seen */
    }  /* end there isn't one yet */
  else {  /* we found an entry for an object named lost+found */
    found_inoidx = (inoidx_t) found_inonum;
    agg_recptr->lsfn_inonum = found_inoidx;
    rlaf_rc = get_inorecptr(aggregate_inode, alloc_ifnull,
                            found_inoidx, &inorecptr );
    if( (rlaf_rc == FSCK_OK) && (inorecptr == NULL) )  {  /*
                             * a parent but no record allocated
                             */
      rlaf_rc = FSCK_INTERNAL_ERROR_5;
      }  /* end a parent but no record allocated */
    else  if( rlaf_rc == FSCK_OK ) {  /* a record is allocated */
      if( !inorecptr->in_use ) {  /*
                             * Inode not in use.
                             * This is another presentation of the
                             * no inode allocated yet case
                             */
        agg_recptr->avail_inonum = agg_recptr->lsfn_inonum;
        agg_recptr->lsfn_ok = 1;
        fsck_send_msg( fsck_LSFNNOTFOUND, 0, 0 );
        }  /* end Inode not in use.  This is another... */
      else {  /* inode is in use */
        if( inorecptr->selected_to_rls ) { /*
                             * we've released it.
                             * This is another presentation of the
                             * no inode allocated yet case
                             */
          agg_recptr->avail_inonum = agg_recptr->lsfn_inonum;
          agg_recptr->lsfn_ok = 1;
          fsck_send_msg( fsck_LSFNNOTFOUND, 0, 0 );
          }  /* end we've released it.  This is ... */
        else if( inorecptr->ignore_alloc_blks ) {  /*
                              * but we must have tried to release it and
                              * been turned down because we discovered
                              * that its data is corrupt. 
                              */
          agg_recptr->lsfn_inonum = (ino_t) 0;  /* don't have one  */
          agg_recptr->lsfn_ok = 0;              /* and don't have hope */
              /*
               * no message here because we already told them that
               * /lost+found/ is screwed up and they wouldn't let
               * us fix it.  
               */
          }  /* end but we must have tried to release it and ... */
        else if( inorecptr->inode_type != directory_inode ) {  /*
                              * but it must be user data (or corrupt
                              * in some subtle way) because it's not
                              * a directory
                              */
          agg_recptr->lsfn_inonum = (ino_t) 0;  /* don't have one  */
          agg_recptr->lsfn_ok = 0;              /* and don't have hope */
          fsck_send_msg( fsck_LSFNNOTDIR, 0, 0 );
          }  /* end but it must be user data (or corrupt... */
        else if( (inorecptr->involved_in_dups) &&
                 (agg_recptr->dup_alloc_lst != NULL) ) { /*
                             * This inode lays claim to one or more blocks
                             * found to be multiply-allocated.  There are
                             * now multiply-allocated blocks which we
                             * were not allowed to correct.  It isn't
                             * certain that this inode refers to any
                             * block(s) STILL unresolved, but it might.
                             * Since this filesystem is going to be marked
                             * dirty anyway, don't attempt to reconnect
                             * disconnected inodes.
                             * (N.B. The danger in proceeding is to data
                             * integrity.)
                             */
          agg_recptr->lsfn_inonum = (ino_t) 0;  /* don't have one  */
          agg_recptr->lsfn_ok = 0;              /* and don't have hope */
              /*
               * no message here because the reasoning is too 
               * conservative to explain easily.  They'll be informed
               * about inodes not reconnected (which isn't the end of
               * the world anyway).
               */
          }  /* end this inode lays claim to one or more blocks ... */
        else {  /* looks like we're in business here */
          agg_recptr->avail_inonum = 0;
          agg_recptr->lsfn_ok = 1;
          }  /* end else looks like we're in business here */
        }  /* end else inode is in use */
      }  /* end a record is allocated */
    }  /* end we found an entry for an object named lost+found */
  return( rlaf_rc );
}                              /* end of resolve_lost_and_found ()  */
	
	
/*****************************************************************************
 * NAME: validate_fs_inodes
 *
 * FUNCTION:  Verify the fileset inodes and structures rooted in them.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t validate_fs_inodes ( )
{
  retcode_t vfi_rc = FSCK_OK;
  inoidx_t ino_idx;
  dinode_t *ino_ptr;
  int aggregate_inode = 0;  /* going for fileset inodes only */
  int which_it = FILESYSTEM_I;     /* in release 1 there is only 1 fileset */
  struct fsck_ino_msg_info  ino_msg_info;
  fsck_msg_info_ptr msg_info_ptr;
	
  msg_info_ptr = &ino_msg_info;
  msg_info_ptr->msg_inopfx = fsck_fset_inode;  /* all fileset owned */
	
  vfi_rc = inode_get_first_fs( which_it, &ino_idx, &ino_ptr );  /*
                             * get the first non-metadata inode after
                             * the fileset root directory
                             */
  while( (vfi_rc == FSCK_OK) &&      /* no fatal errors and */
         (ino_ptr != NULL) )  {            /* haven't seen the last inode */
    if( inode_is_in_use(ino_ptr, (ino_t) ino_idx) ) {  /* inode is in use */
      vfi_rc = validate_record_fileset_inode( (ino_t) ino_idx, ino_idx,
                                              ino_ptr, msg_info_ptr  );
      }  /* end inode is in use */
    else {  /* inode is allocated but is not in use */
      if( !agg_recptr->avail_inode_found )  {  /*
                             * this is the first allocated, available
                             * inode we've seen all day
                             */
        agg_recptr->avail_inonum = (ino_t) ino_idx;
        agg_recptr->avail_inode_found = 1;
        }  /* end this is the first allocated, available inode ... */
      }  /* end else inode is allocated but is not in use */
    if( vfi_rc == FSCK_OK ) {
      vfi_rc = inode_get_next( &ino_idx, &ino_ptr );
      }
    }  /* end while no fatal errors and haven't seen the last inode */
  return( vfi_rc );
}                              /* end of validate_fs_inodes ()  */
	
	
/*****************************************************************************
 * NAME: verify_parms
 *
 * FUNCTION:  Verify that mutually exclusive invocation parameters were not 
 *            specified.  Determine the level of messaging to be used for
 *            this fsck session.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t verify_parms ( )
{
  retcode_t   mutexcl_parm_count = 0;
  /*
   * Verify that only one of the mutually exclusive parms was specified
   * Set default value if none was specified.
   */
  mutexcl_parm_count = agg_recptr->parm_options[UFS_CHKDSK_LEVEL0] +
                       agg_recptr->parm_options[UFS_CHKDSK_LEVEL1] +
                       agg_recptr->parm_options[UFS_CHKDSK_LEVEL2] +
                       agg_recptr->parm_options[UFS_CHKDSK_LEVEL3];
  switch( mutexcl_parm_count ) {
    case 0:                        /* set the default */
      agg_recptr->parm_options[UFS_CHKDSK_LEVEL0] = 1;
      fsck_send_msg( fsck_SESSPRMDFLT, 0, 0 );
      break;
    case (-1):                      /* exactly one was specified */
          /*
           * in the conversion of chkdsk from a unix-like utility to
           * an os/2-like utility, all levels which write to the
           * device are the unix level 3 (assume permission to fix
           * everything)
           */
      if( !agg_recptr->parm_options[UFS_CHKDSK_LEVEL0] )  { /* not read only */
        agg_recptr->parm_options[UFS_CHKDSK_LEVEL1] = 0;
        agg_recptr->parm_options[UFS_CHKDSK_LEVEL2] = 0;
        agg_recptr->parm_options[UFS_CHKDSK_LEVEL3] = -1;
		/*
		 * we'll be doing the Bad Block List function as part
		 * of the /f processing.  Turn off the flag that specifies
		 * it.  This flag is only used when we're in read-only mode.
		 */
        agg_recptr->parm_options[UFS_CHKDSK_CLRBDBLKLST] = 0; 
        }  /* end not read only */
      break;
    default:
      fsck_send_msg( fsck_PRMMUTEXCLLEVELS, 0, 0 );
      return( FSCK_FAILED );
    }
    /*
     * the parms are valid.  record the messaging level they imply.
     */
          /*
           * in the conversion of chkdsk from a unix-like utility to
           * an os/2-like utility, there is only 1 documented 
           * message level.  In addition, the undocumented messaging
           * level, debug, is supported.  This displays, to stdout,
           * in English, all messages which are logged.
           */
  if( agg_recptr->parm_options[UFS_CHKDSK_LEVEL0] ) {
    agg_recptr->effective_msg_level = fsck_quiet;
    }
  else if( agg_recptr->parm_options[UFS_CHKDSK_LEVEL1] ) {
    agg_recptr->effective_msg_level = fsck_quiet;
    }
  else if( agg_recptr->parm_options[UFS_CHKDSK_LEVEL2] ) {
    agg_recptr->effective_msg_level = fsck_quiet;
    }
  else {  /* agg_recptr->parm_options[UFS_CHKDSK_LEVEL3]   */
    agg_recptr->effective_msg_level = fsck_quiet;
    }
  if( agg_recptr->parm_options[UFS_CHKDSK_SHOWFILES] ) {
    agg_recptr->effective_msg_level = fsck_quiet;
/*    agg_recptr->messaging_verbose = 1;   */
    }
  if( agg_recptr->parm_options[UFS_CHKDSK_DEBUG] ) {
    agg_recptr->effective_msg_level = fsck_debug;
/*    agg_recptr->messaging_verbose = 1;    */
    }
  return( FSCK_OK );
}                            /* end of verify_parms()  */
