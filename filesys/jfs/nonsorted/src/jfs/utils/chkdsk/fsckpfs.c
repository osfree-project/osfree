/* $Id: fsckpfs.c,v 1.3 2004/03/21 02:43:24 pasha Exp $ */

static char *SCCSID = "@(#)1.21  3/26/99 20:42:45 src/jfs/utils/chkdsk/fsckpfs.c, jfschk, w45.fs32, 990417.1";
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
 *   MODULE_NAME:		fsckpfs.c
 *
 *   COMPONENT_NAME: 	jfschk
 *
 *   FUNCTIONS:
 *              ait_node_get
 *              ait_node_put
 *              ait_special_read_ext1
 *              alloc_high_mem
 *              blkmap_find_bit
 *              blkmap_flush
 *              blkmap_get_ctl_page
 *              blkmap_get_page
 *              blkmap_put_ctl_page
 *              blkmap_put_page
 *              blktbl_ctl_page_put
 *              blktbl_dmap_get
 *              blktbl_dmap_put
 *              blktbl_dmaps_flush
 *              blktbl_Ln_page_get
 *              blktbl_Ln_page_put
 *              blktbl_Ln_pages_flush
 *              close_volume
 *              default_volume
 *              dnode_get
 *              ea_get
 *              fscklog_put_buffer
 *              iag_get
 *              iag_get_first
 *              iag_get_next
 *              iag_put
 *              iags_flush
 *              inodes_flush
 *              inode_get
 *              inode_get_first_fs
 *              inode_get_next
 *              inode_put
 *              inotbl_get_ctl_page
 *              inotbl_put_ctl_page
 *              mapctl_flush
 *              mapctl_get
 *              mapctl_put
 *              node_get
 *              open_volume
 *              readwrite_device
 *              recon_dnode_assign
 *              recon_dnode_get
 *              recon_dnode_put
 *              recon_dnode_release
 *              refresh_bootsec
 *
 *              imapleaf_get
 *              open_device_read
 *              open_device_rw_exclusive
 *              open_device_rw_shared
 *
*/
/* History
 *   PS15032004 - fix for work on flash drive
 */
/*
 * defines and includes common among the xfsck modules
 */
#include "xfsckint.h"

#include "jfs_cntl.h"
#include <direct.h>
#include "fssubs.h"
#include "jfs_chkdsk.h"
#include "debug.h"

 /* + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
  *
  * DosDevIOCtl parameter stuff
  *
  *      defined in devices.c
  */
extern struct DPB Datap;

extern TRACKLAYOUT *ptrklay;

extern agg_byteidx_t ondev_jlog_byte_offset;

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
extern char *msgs_txt_ptr;
extern ULONG msgs_txt_maxlen;

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
extern HFILE Dev_IOPort;
extern uint32 Dev_blksize;
extern int32 Dev_SectorSize;

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

retcode_t imapleaf_get ( agg_byteidx_t, xtpage_t ** );   

retcode_t open_device_read ( char  * );

retcode_t open_device_rw_exclusive ( char  * );

retcode_t open_device_rw_shared ( char  * );

uint32 checksum( uint8 *, uint32 );

/* VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV */
	 
	 
/*****************************************************************************
 * NAME: ait_node_get
 *
 * FUNCTION:  Read the specified AIT xTree node into the specified buffer
 *
 * PARAMETERS:
 *      node_fsblk_offset  - input - offset, in aggregate blocks, into the
 *                                   aggregate, of the xTree node wanted
 *      xtpage_ptr            - input - pointer an fsck buffer into which the
 *                                   xTree node should be read.
 * 
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t ait_node_get ( fsblkidx_t   node_fsblk_offset,
                     xtpage_t  *xtpage_ptr
                   )
{
  retcode_t anodg_rc = FSCK_OK;
  agg_byteidx_t node_start_byte;
  uint32 bytes_read;
		
  node_start_byte = node_fsblk_offset * sb_ptr->s_bsize;
		
  if( (agg_recptr->ondev_wsp_fsblk_offset != 0) &&
      (node_fsblk_offset > agg_recptr->ondev_wsp_fsblk_offset) ) { /*
			      * the offset is beyond the range 
			      * valid for fileset objects
			      */
	/*
	 * This case is not caused by an I/O error, but by
	 * invalid data in an inode.  Let the caller handle
	 * the consequences.
	 */
    anodg_rc = FSCK_BADREADTARGET2;
    }  /* end the offset is beyond the range valid ... */
		
  else {  
    anodg_rc = readwrite_device( node_start_byte,
                                XTPAGE_SIZE,
                                &(bytes_read),
                                (void *) xtpage_ptr,
                                fsck_READ );
    if( anodg_rc == FSCK_OK ) {  /* read appears successful */
      if( bytes_read < XTPAGE_SIZE ) {  /*
                                  * didn't get the minimum number of bytes 
                                  */
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
        sprintf( message_parm_0, "%d", FSCK_FAILED_BADREAD_NODE1 );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", anodg_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_READ );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", node_start_byte );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", XTPAGE_SIZE );
        msgprms[4] = message_parm_4;
        msgprmidx[4] = 0;
        sprintf( message_parm_5, "%ld", bytes_read );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONAGG, 0, 6 );
        anodg_rc = FSCK_FAILED_BADREAD_NODE1;
        }  /* end else didn't get the minimum number of bytes */
      }  /* end read appears successful */
    else {  /* bad return code from read */
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
      sprintf( message_parm_0, "%d", FSCK_FAILED_READ_NODE );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", anodg_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_READ );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", node_start_byte );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", XTPAGE_SIZE );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", bytes_read );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONAGG, 0, 6 );
      anodg_rc = FSCK_FAILED_READ_NODE;
      }  /* end else bad return code from read */
    }  /* end else we'll have to read it from the disk */
	  
  return( anodg_rc );
}                              /* end of ait_node_get ()  */
	 
	 
/*****************************************************************************
 * NAME: ait_node_put
 *
 * FUNCTION:  Write the specified buffer into the specified AIT xTree node
 *
 * PARAMETERS:
 *      node_fsblk_offset  - input - offset, in aggregate blocks, to which 
 *                                             the buffer is to be written
 *      xtpage_ptr            - input - pointer to the buffer to write 
 * 
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t ait_node_put ( fsblkidx_t   node_fsblk_offset,
                                   xtpage_t  *xtpage_ptr
                                 )
{
  retcode_t anodp_rc = FSCK_OK;
  agg_byteidx_t node_start_byte;
  uint32  bytes_written;

    node_start_byte = node_fsblk_offset * sb_ptr->s_bsize;

    anodp_rc = readwrite_device( node_start_byte, PSIZE, 
                                             &bytes_written, (void *) xtpage_ptr,
                                             fsck_WRITE
                                           );

    if( anodp_rc == FSCK_OK ) {

      if( bytes_written != PSIZE )  {  /* didn't write correct number of bytes */
            /*
             * message to user
             */
  DBG_ERROR(("CHK:ait_node_put %d written != PSIZE\n",bytes_written));

        msgprms[0] = message_parm_0;
        msgprmidx[0] = fsck_metadata;
        msgprms[1] = Vol_Label;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", 2 );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        fsck_send_msg( fsck_URCVWRT, 0, 3 );
            /*
             * message to debugger
             */
        sprintf( message_parm_0, "%d", FSCK_BADWRITE_FBLKMP );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", anodp_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_WRITE );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", node_start_byte );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", PSIZE );
        msgprms[4] = message_parm_4;
        msgprmidx[3] = 0;
        sprintf( message_parm_5, "%ld", bytes_written );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONAGG, 0, 6 );

        anodp_rc= FSCK_BADWRITE_FBLKMP;
        }  /* end else didn't write correct number of bytes */
      }
    else {  /* else the write was not successful */
         /*
          * message to user 
          */
  DBG_ERROR(("CHK:ait_node_put %d written not success\n",bytes_written));
      msgprms[0] = message_parm_0;
      msgprmidx[0] = fsck_metadata;
      msgprms[1] = Vol_Label;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", 3 );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_URCVWRT, 0, 3 );
         /*
          * message to debugger
          */
      sprintf( message_parm_0, "%d", FSCK_BADWRITE_FBLKMP );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", anodp_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_WRITE );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", node_start_byte );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", PSIZE );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", bytes_written );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONAGG, 0, 6 );

      anodp_rc= FSCK_BADWRITE_FBLKMP;
      }  /* end else the write was not successful */
	  
  return( anodp_rc );
}                              /* end of ait_node_put ()  */
	 
	 
/*****************************************************************************
 * NAME: ait_special_read_ext1
 *
 * FUNCTION:  Reads the first extent of either the Primary or Secondary 
 *            Aggregate Inode Table into the fsck inode buffer.
 *
 * PARAMETERS:
 *      which_ait  - input - { fsck_primary | fsck_secondary }
 *
 * NOTES:  This routine is used during the early stages of fsck processing
 *         when the normal mechanisms for reading inodes have not yet been
 *         established.
 *
 *         This routine may also be used later in fsck processing as a fast
 *         read routine for the inodes in the first extent of the AIT.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t ait_special_read_ext1( int     which_ait )
{
  retcode_t aree_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  reg_idx_t bytes_read;
  agg_byteidx_t offset_1stext;

  aree_rc = inodes_flush();

  /*
   * calculate the byte offset of the first extent
   */
  if( (which_ait == fsck_primary) ) {
    offset_1stext = AITBL_OFF;
    }
  else {  /* must be secondary */
    offset_1stext = addressPXD(&(sb_ptr->s_ait2)) *
                              sb_ptr->s_bsize;
    }  /* end else must be secondary */

  if( agg_recptr->ino_buf_agg_offset != offset_1stext ) {  /*
                            * we don't already have the one we want
                            */
    intermed_rc = readwrite_device( offset_1stext,
                                    INODE_IO_BUFSIZE,
                                    &(agg_recptr->ino_buf_data_len),
                                    (void *) agg_recptr->ino_buf_ptr,
                                    fsck_READ
                                   );
    if( intermed_rc != FSCK_OK )  {  /* didn't get anything */
      aree_rc = FSCK_CANTREADAITEXT1;
      msgprms[0] = message_parm_0;
      msgprmidx[0] = which_ait;
      fsck_send_msg( fsck_CANTREADAITEXT1, 0, 1 );
     }  /* end didn't get anything */
    else {  /* got something */
      agg_recptr->ino_for_aggregate = -1;
      agg_recptr->ino_which_it = which_ait;
      agg_recptr->ino_buf_1st_ino = 0;
      agg_recptr->ino_fsnum = 0;
      agg_recptr->ino_buf_agg_offset = offset_1stext;
      PXDaddress(&(agg_recptr->ino_ixpxd), offset_1stext / sb_ptr->s_bsize );
      PXDlength(&(agg_recptr->ino_ixpxd), 
                INODE_EXTENT_SIZE / sb_ptr->s_bsize );

      if( agg_recptr->ino_buf_data_len < INODE_EXTENT_SIZE )  {  /*
                                * didn't get enough
                                */
        aree_rc = FSCK_CANTREADAITEXT1;
        msgprms[0] = message_parm_0;
        msgprmidx[0] = which_ait;
        fsck_send_msg( fsck_CANTREADEAITEXT1, 0, 1 );
        }  /* end didn't get enough */
      }  /* end else got something */
    }  /* we don't already have the one we want */

  return( aree_rc );
}                    /* end of ait_special_read_ext1 ()       */
	 
	 
/*****************************************************************************
 * NAME: alloc_high_mem
 *
 * FUNCTION:  Allocate the high memory on the system for use in the
 *            dynamic storage pool.
 *
 * PARAMETERS:
 *      addr_ext_ptr    - input - pointer to a variable in which to return
 *                                the starting address of the storage 
 *                                obtained or, if this routine is not
 *                                successful, NULL
 *      addr_ext_length - input - pointer to a variable in which to return
 *                                the length of the storage obtained
 *
 * NOTES:  o The high memory can only be allocated ONCE in a chkdsk session
 *
 *         o This routine is only used in the autocheck environment
 *
 *         o This routine is not called unless/until dynamic storage
 *           available via malloc() is exhausted since this call causes
 *           the system to reboot.
 * 
 *         o For this routine to work, the CHKDSK.SYS device driver must
 *           be installed in the system:
 *
 *               In config.sys,  BASEDEV=CHKDSK.SYS
 *
 *               CHKDSK.SYS must reside in \OS2\BOOT\
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
void alloc_high_mem ( char   **addr_ext_ptr,
                      int32   *addr_ext_length
                     )
{
  retcode_t ahm_rc = FSCK_OK;
  HFILE  DriverHandle;
  ULONG  Action;
  struct parm_s parm;             /* required by CHKDSK.SYS   */
  struct data_s data;             /* required by CHKDSK.SYS   */
  ULONG  parmLen;                 /* length of parm structure */
  ULONG  dataLen;                 /* length of data structure */

  *addr_ext_ptr = NULL;           /* assume it won't work     */

   /*
    * open the device driver 
    */
  ahm_rc = DosOpen( "CHKDSK$", &DriverHandle, &Action, 0, 0,
                    OPEN_ACTION_OPEN_IF_EXISTS,
                    OPEN_FLAGS_WRITE_THROUGH|OPEN_FLAGS_FAIL_ON_ERROR|
                    OPEN_SHARE_DENYNONE|OPEN_ACCESS_READWRITE,
                    0
                   );
  sprintf( message_parm_0, "%ld", ahm_rc );
  msgprms[0] = message_parm_0;
  msgprmidx[0] = 0;
  fsck_send_msg( fsck_CHKDSKSYSOPENRC, 0, 1 );  

  if( ahm_rc == NO_ERROR ) {  /* successful open */
    data.rc = -1;               /* initialize */
    parm.size = 4096;           /* initialize */

       /* 
        * get the high memory
        */
    ahm_rc = DosDevIOCtl( DriverHandle, 128, 40, 
                          &parm, sizeof(parm), &parmLen, 
                          &data, sizeof(data), &dataLen
                         );
    sprintf( message_parm_0, "%ld", ahm_rc );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    sprintf( message_parm_1, "%ld", data.rc );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    fsck_send_msg( fsck_CHKDSKSYSCALLRC, 0, 2 );  

    if( (ahm_rc == NO_ERROR) && (data.rc == 0) ) {  /*
                          * it worked!!
                          */
      *addr_ext_ptr = (unsigned char *) data.addr;
      *addr_ext_length = data.cb;
      agg_recptr->high_mem_allocated = 1;  /* we can only do this once */
      sprintf( message_parm_0, "%ld", data.cb );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%lx", data.addr );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      fsck_send_msg( fsck_CHKDSKSYSALLOC, 0, 2 );  
      }  /* end it worked!! */
    }  /* end successful open */

  return;
}                    /* end of alloc_high_mem ()       */
	 
	 
/*****************************************************************************
 * NAME: blkmap_find_bit
 *
 * FUNCTION:  Calculate the position, in the fsck workspace block map,
 *            of the bit representing the given aggregate block.
 *
 * PARAMETERS:
 *      blk_number   - input - ordinal number of the aggregate block whose
 *                             bit is to be located
 *      page_number  - input - pointer to a variable in which to return
 *                             the ordinal number of the page, in the fsck
 *                             workspace block map, containing the bit 
 *                             for the given block
 *      byte_offset  - input - pointer to a variable in which to return
 *                             the ordinal number of the byte, in page_number
 *                             page, containing the bit for the given block
 *      mask_ptr     - input - pointer to a variable in which to return
 *                             a mask to apply to the byte at byte_offset
 *                             in order to reference the bit for the given 
 *                             block
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t blkmap_find_bit( fsblkidx_t       blk_number,
                           mappgidx_t      *page_number,
                           reg_idx_t       *byte_offset,
                           fsck_bitmask_ptr mask_ptr
                          )
{
  retcode_t bfb_rc = FSCK_OK;
  uint64 remainder;
  uint32 bit_position;

  *page_number = blk_number >> log2BITSPERPAGE;
  remainder = blk_number - ((*page_number) << log2BITSPERPAGE);
  *byte_offset = (remainder >> log2BITSPERDWORD) * BYTESPERDWORD;
  bit_position = remainder - ((*byte_offset) << log2BITSPERBYTE);
  *mask_ptr = 0x80000000u >> bit_position;

  return( bfb_rc );
}                    /* end of blkmap_find_bit ()       */
	 
	 
/*****************************************************************************
 * NAME: blkmap_flush
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate
 *            and the current block map buffer has been updated since
 *            the most recent read operation, write the buffer contents to
 *            the device.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t blkmap_flush ( )
{
  retcode_t bmpf_rc = FSCK_OK;
  uint32  bytes_written;

  if( agg_recptr->blkmp_buf_write ) {  /* buffer has been updated since
                               * most recent write
                               */
    bmpf_rc = readwrite_device( agg_recptr->blkmp_agg_offset,
                                agg_recptr->blkmp_buf_data_len,
                                &bytes_written,
                                (void *) agg_recptr->blkmp_buf_ptr,
                                fsck_WRITE
                              );

    if( bmpf_rc == FSCK_OK ) {

      if( bytes_written == agg_recptr->blkmp_buf_data_len )  {

        agg_recptr->blkmp_buf_write = 0; /* buffer has been written
                               * to the device and won't need to be
                               * written again unless/until the
                               * buffer contents have been altered.
                               */
        }
      else {  /* didn't write correct number of bytes */
            /*
             * message to user
             */
  DBG_ERROR(("CHK:blkmap_flush %d written \n",bytes_written));
        msgprms[0] = message_parm_0;
        msgprmidx[0] = fsck_metadata;
        msgprms[1] = Vol_Label;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", 2 );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        fsck_send_msg( fsck_URCVWRT, 0, 3 );
            /*
             * message to debugger
             */
        sprintf( message_parm_0, "%d", FSCK_FAILED_FBMAP_BADFLUSH );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", bmpf_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_WRITE );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", agg_recptr->blkmp_agg_offset );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", agg_recptr->blkmp_buf_data_len );
        msgprms[4] = message_parm_4;
        msgprmidx[3] = 0;
        sprintf( message_parm_5, "%ld", bytes_written );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONWSP, 0, 6 );

        bmpf_rc = FSCK_FAILED_FBMAP_BADFLUSH;
        }  /* end else didn't write correct number of bytes */
      }
    else {  /* else the write was not successful */
         /*
          * message to user 
          */
  DBG_ERROR(("CHK:blkmap_flush %d written not success\n",bytes_written));
      msgprms[0] = message_parm_0;
      msgprmidx[0] = fsck_metadata;
      msgprms[1] = Vol_Label;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", 3 );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_URCVWRT, 0, 3 );
         /*
          * message to debugger
          */
      sprintf( message_parm_0, "%d", FSCK_FAILED_FBMAP_FLUSH );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", bmpf_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_WRITE );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", agg_recptr->blkmp_agg_offset );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", agg_recptr->blkmp_buf_data_len );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", bytes_written );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONWSP, 0, 6 );

      bmpf_rc = FSCK_FAILED_FBMAP_FLUSH;
      }  /* end else the write was not successful */
    }  /* end buffer has been updated since most recent write */

  return( bmpf_rc );
}                              /* end of blkmap_flush ()  */


/*****************************************************************************
 * NAME: blkmap_get_ctl_page
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate,
 *            write the contents of the given buffer over the current fsck
 *            fsck workspace block map control page on the device.
 *
 * PARAMETERS:
 *      blk_ctlptr  - input -  pointer to the buffer into the current fsck 
 *                              workspace block map control page should be read.
 *
 * NOTES:  
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t  blkmap_get_ctl_page ( blk_cntrlptr blk_ctlptr )
{
  retcode_t bmgcp_rc = FSCK_OK;
  uint32 bytes_read;

  if( agg_recptr->processing_readwrite ) {  /* have write access */

    bmgcp_rc = readwrite_device( agg_recptr->ondev_wsp_byte_offset,
                                 BYTESPERPAGE,
                                 &bytes_read,
                                 (void *) agg_recptr->blkmp_ctlptr,
                                 fsck_READ
                                );

    if( bmgcp_rc == FSCK_OK ) {

      if( bytes_read != (uint32) BYTESPERPAGE )  {  /*
                               * didn't read correct number of bytes
                               */
            /*
             * message to debugger
             */
        sprintf( message_parm_0, "%d", FSCK_BADREAD_FBLKMP );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", bmgcp_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_READ );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", agg_recptr->ondev_wsp_byte_offset );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", BYTESPERPAGE );
        msgprms[4] = message_parm_4;
        msgprmidx[4] = 0;
        sprintf( message_parm_5, "%ld", bytes_read );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONWSP, 0, 6 );

        bmgcp_rc = FSCK_BADREAD_FBLKMP;
        }  /* end else didn't write correct number of bytes */
      }
    else {  /* else the write was not successful */
          /*
           * message to debugger 
           */
      sprintf( message_parm_0, "%d", FSCK_BADREAD_FBLKMP );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", bmgcp_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_READ );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", agg_recptr->ondev_wsp_byte_offset );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", BYTESPERPAGE );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", bytes_read );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONWSP, 0, 6 );

      bmgcp_rc = FSCK_BADREAD_FBLKMP;
      }  /* end else the write was not successful */
    }  /* end have write access */

  return( bmgcp_rc );
}                              /* end of blkmap_get_ctl_page ()  */
	 
	 
/*****************************************************************************
 * NAME: blkmap_get_page
 *
 * FUNCTION:  Read the requested fsck workspace block map page into and/or 
 *            locate the requested fsck workspace block map page in the
 *            fsck block map buffer.
 *            
 * PARAMETERS:
 *      page_num       - input - ordinal number of the fsck workspace
 *                               block map page which is needed
 *      addr_page_ptr  - input - pointer to a variable in which to return
 *                               the address of the page in an fsck buffer
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t blkmap_get_page( mappgidx_t   page_num,
                           blk_pageptr *addr_page_ptr
                          )
{
  retcode_t bgp_rc = FSCK_OK;
  agg_byteidx_t page_start_byte, page_end_byte;

  page_start_byte = page_num * BYTESPERPAGE;
  page_end_byte = page_start_byte + BYTESPERPAGE -1;
  if( (page_start_byte >= agg_recptr->blkmp_blkmp_offset) &&
      (page_end_byte <= (agg_recptr->blkmp_blkmp_offset +
                         agg_recptr->blkmp_buf_data_len   ))  ) {  /*
                          * the desired page is already in the buffer
                          */
    *addr_page_ptr = (blk_pageptr)
                     ((int32) agg_recptr->blkmp_buf_ptr +
                      page_start_byte - agg_recptr->blkmp_blkmp_offset);
    }  /* end the desired page is already in the buffer */

  else {  /* else need to read it in from dasd */

    if( !agg_recptr->processing_readwrite )  {  /* this isn't supposed
                          * to happen.  If we don't have write access
                          * to the aggregate then we're always supposed
                          * to get a hit in the buffer!
                          */
      bgp_rc = FSCK_INTERNAL_ERROR_6;
      }  /* end this isn't supposed to happen.  If we don't ... */

    else {  /* we have read/write access */
      bgp_rc = blkmap_flush();   /* if the buffer has been modified,
                          * write it to dasd
                          */
      if( bgp_rc == FSCK_OK )  {  /* successful write */

        agg_recptr->blkmp_blkmp_offset = page_start_byte;
        agg_recptr->blkmp_agg_offset =
                               page_start_byte + (BYTESPERPAGE * 1) +
                               agg_recptr->ondev_wsp_byte_offset; /*
                          * The byte offset in the fsck block map plus
                          * one page of control information plus the
                          * aggregate bytes which precede the on-dasd
                          * fsck workspace
                          */
        bgp_rc = readwrite_device( agg_recptr->blkmp_agg_offset,
                                   agg_recptr->blkmp_buf_length,
                                   &(agg_recptr->blkmp_buf_data_len),
                                   (void *) agg_recptr->blkmp_buf_ptr,
                                   fsck_READ
                                 );

        if( bgp_rc == FSCK_OK )  {  /* successful read */

          if( agg_recptr->blkmp_buf_data_len >= BYTESPERPAGE )  {
            *addr_page_ptr = agg_recptr->blkmp_buf_ptr;
            }
          else {  /* but didn't get enough to continue */
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
            sprintf( message_parm_0, "%d", FSCK_FAILED_BADREAD_FBLKMP );
            msgprms[0] = message_parm_0;
            msgprmidx[0] = 0;
            sprintf( message_parm_1, "%d", bgp_rc );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            sprintf( message_parm_2, "%d", fsck_READ );
            msgprms[2] = message_parm_2;
            msgprmidx[2] = 0;
            sprintf( message_parm_3, "%lld", agg_recptr->blkmp_agg_offset );
            msgprms[3] = message_parm_3;
            msgprmidx[3] = 0;
            sprintf( message_parm_4, "%ld", agg_recptr->blkmp_buf_length );
            msgprms[4] = message_parm_4;
            msgprmidx[4] = 0;
            sprintf( message_parm_5, "%ld", agg_recptr->blkmp_buf_data_len );
            msgprms[5] = message_parm_5;
            msgprmidx[5] = 0;
            fsck_send_msg( fsck_ERRONWSP, 0, 6 );

            bgp_rc = FSCK_FAILED_BADREAD_FBLKMP;
            }  /* end else but didn't get enough to continue */
          }  /* end successful read */

        else {  /* read failed */
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
          sprintf( message_parm_0, "%d", FSCK_FAILED_READ_FBLKMP );
          msgprms[0] = message_parm_0;
          msgprmidx[0] = 0;
          sprintf( message_parm_1, "%d", bgp_rc );
          msgprms[1] = message_parm_1;
          msgprmidx[1] = 0;
          sprintf( message_parm_2, "%d", fsck_READ );
          msgprms[2] = message_parm_2;
          msgprmidx[2] = 0;
          sprintf( message_parm_3, "%lld", agg_recptr->blkmp_agg_offset );
          msgprms[3] = message_parm_3;
          msgprmidx[3] = 0;
          sprintf( message_parm_4, "%ld", agg_recptr->blkmp_buf_length );
          msgprms[4] = message_parm_4;
          msgprmidx[4] = 0;
          sprintf( message_parm_5, "%ld", agg_recptr->blkmp_buf_data_len );
          msgprms[5] = message_parm_5;
          msgprmidx[5] = 0;
          fsck_send_msg( fsck_ERRONWSP, 0, 6 );

          bgp_rc = FSCK_FAILED_READ_FBLKMP;
          }  /* end else read failed */
        }  /* end successful flush */
      }  /* end else we have read/write access */
    }  /* end else need to read it in from dasd */

  return( bgp_rc );
}                    /* end of blkmap_get_page ()       */
	 
	 
/*****************************************************************************
 * NAME: blkmap_put_ctl_page
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate,
 *            write the contents of the given buffer over the current fsck
 *            fsck workspace block map control page on the device.
 *
 * PARAMETERS:
 *      blk_ctlptr  - input -  pointer to the buffer which should be written
 *                             over the current fsck workspace block map
 *                             control page.
 *
 * NOTES:  Unlike most _put_ routines in this module, blkmap_put_ctl_page
 *         actually writes to the device.  This is done because the block
 *         map control page contains state and footprint information which
 *         provide crucial serviceability should the fsck session be
 *         interrupted.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t  blkmap_put_ctl_page ( blk_cntrlptr blk_ctlptr )
{
  retcode_t bmpcp_rc = FSCK_OK;
  uint32 bytes_written;

  if( agg_recptr->processing_readwrite ) {  /* have write access */

    bmpcp_rc = readwrite_device( agg_recptr->ondev_wsp_byte_offset,
                                 BYTESPERPAGE,
                                 &bytes_written,
                                 (void *) agg_recptr->blkmp_ctlptr,
                                 fsck_WRITE
                                );

    if( bmpcp_rc == FSCK_OK ) {

      if( bytes_written != (uint32) BYTESPERPAGE )  {  /*
                               * didn't write correct number of bytes
                               */
            /*
             * message to user
             */
  DBG_ERROR(("CHK:blkmap_put_ctl_page %d written != BYTESPERPAGE \n",bytes_written));
        msgprms[0] = message_parm_0;
        msgprmidx[0] = fsck_metadata;
        msgprms[1] = Vol_Label;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", 4 );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        fsck_send_msg( fsck_URCVWRT, 0, 3 );
            /*
             * message to debugger
             */
        sprintf( message_parm_0, "%d", FSCK_FAILED_BADWRITE_FBLKMP );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", bmpcp_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_WRITE );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", agg_recptr->ondev_wsp_byte_offset );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", BYTESPERPAGE );
        msgprms[4] = message_parm_4;
        msgprmidx[4] = 0;
        sprintf( message_parm_5, "%ld", bytes_written );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONWSP, 0, 6 );

        bmpcp_rc = FSCK_FAILED_BADWRITE_FBLKMP;
        }  /* end else didn't write correct number of bytes */
      }
    else {  /* else the write was not successful */
         /*
          * message to user 
          */
  DBG_ERROR(("CHK:blkmap_put_ctl_page %d written not success \n",bytes_written));
      msgprms[0] = message_parm_0;
      msgprmidx[0] = fsck_metadata;
      msgprms[1] = Vol_Label;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", 5 );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_URCVWRT, 0, 3 );
          /*
           * message to debugger 
           */
      sprintf( message_parm_0, "%d", FSCK_FAILED_WRITE_FBLKMP );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", bmpcp_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_WRITE );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", agg_recptr->ondev_wsp_byte_offset );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", BYTESPERPAGE );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", bytes_written );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONWSP, 0, 6 );

      bmpcp_rc = FSCK_FAILED_WRITE_FBLKMP;
      }  /* end else the write was not successful */
    }  /* end have write access */

  return( bmpcp_rc );
}                              /* end of blkmap_put_ctl_page ()  */
	 
	 
/*****************************************************************************
 * NAME: blkmap_put_page
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate,
 *            note, in the fsck workspace, that the current fsck workspace
 *            block map buffer has been modified and should be written to 
 *            the device in the next flush operation on this buffer.
 *
 * PARAMETERS:
 *      page_num  - input - ordinal number of the page in the fsck workspace
 *                          block map to write from the buffer to the device
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t blkmap_put_page( mappgidx_t  page_num )
{
  retcode_t bpp_rc = FSCK_OK;

  if( agg_recptr->processing_readwrite )  {
    agg_recptr->blkmp_buf_write = 1;
    }

  return( bpp_rc );
}                    /* end of blkmap_put_page ()       */
	 
	 
/*****************************************************************************
 * NAME: blktbl_ctl_page_put
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate,
 *            note, in the fsck workspace, that the current JFS Block Map
 *            control page buffer has been modified and should be written 
 *            to the device in the next flush operation on this buffer.
 *
 * PARAMETERS:
 *      ctlpage_ptr  - input - the address, in an fsck buffer, of the page
 *                             which has been modified.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t blktbl_ctl_page_put ( dbmap_t  *ctlpage_ptr )
{
  retcode_t bcpp_rc = FSCK_OK;

  if( agg_recptr->processing_readwrite )  {
    bcpp_rc = mapctl_put( (void *) ctlpage_ptr );
    }

  return( bcpp_rc );
}                    /* end of blktbl_ctl_page_put ()       */
	 
	 
/*****************************************************************************
 * NAME: blktbl_dmap_get
 *
 * FUNCTION: Read the JFS Block Table dmap page describing the specified 
 *           aggregate block into and/or locate the JFS Block Table dmap
 *           locate the requested page describing the specified 
 *           aggregate block in the fsck dmap buffer.
 *
 * PARAMETERS:
 *      for_block           - input - ordinal number of the aggregate block
 *                                    whose dmap page is needed 
 *      addr_dmap_page_ptr  - input - pointer to a variable in which to return
 *                                    the address of the found dmap page in
 *                                    an fsck buffer
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t blktbl_dmap_get ( fsblkidx_t   for_block,
                            dmap_t     **addr_dmap_page_ptr
                           )
{
  retcode_t bdg_rc = FSCK_OK;
  fsblkidx_t dmp_logical_fsblk_offset;
  agg_byteidx_t dmp_logical_byte_offset;
  agg_byteidx_t ext_logical_byte_offset;
  agg_byteidx_t ext_byte_offset;
  agg_byteidx_t last_in_buffer;
  dinode_t *bmap_inoptr;
  xad_t    *xad_ptr;
  int8     offset_found;
  int      is_aggregate = -1;            /* the bmap inode is in the ait */
  int      which_it;
  reg_idx_t bytes_read, ext_bytes_left;
  agg_byteidx_t  ext_bytes, offset_into_extent;

  dmp_logical_fsblk_offset = BLKTODMAP( for_block, agg_recptr->log2_blksperpg );
  dmp_logical_byte_offset = dmp_logical_fsblk_offset * sb_ptr->s_bsize;
  *addr_dmap_page_ptr = NULL;

  if( dmp_logical_byte_offset >= agg_recptr->bmapdm_logical_offset ) {
    last_in_buffer = agg_recptr->bmapdm_logical_offset +
                     agg_recptr->bmapdm_buf_data_len - 1;
    if( (dmp_logical_byte_offset+sizeof(dmap_t)-1) <= last_in_buffer ) {  /*
                           * the one we want is already in the buffer
                           */
      *addr_dmap_page_ptr = (dmap_t *) (agg_recptr->bmapdm_buf_ptr +
                                        dmp_logical_byte_offset -
                                        agg_recptr->bmapdm_logical_offset);
      }  /* end the one we want is already in the buffer */
    }

  if( *addr_dmap_page_ptr == NULL ) {  /* we have to read it in */
    bdg_rc = blktbl_dmaps_flush();       /* perform any pending writes */

    if( bdg_rc == FSCK_OK ) {  /* flush worked ok */

      if( agg_recptr->primary_ait_4part1 ) {
        which_it = fsck_primary;
        }
      else {
        which_it = fsck_secondary;
        }
      bdg_rc = ait_special_read_ext1( which_it );
      if( bdg_rc != FSCK_OK ) {  
        report_readait_error( bdg_rc, 
                              FSCK_FAILED_CANTREADAITEXTF, 
                              which_it );
        bdg_rc = FSCK_FAILED_CANTREADAITEXTF;
        }

      else {  /* got the first agg extent */
        bmap_inoptr = (dinode_t *)
                      (agg_recptr->ino_buf_ptr + BMAP_I * sizeof(dinode_t));

        bdg_rc = xTree_search( bmap_inoptr, dmp_logical_fsblk_offset,
                               &xad_ptr, &offset_found );
        if( bdg_rc == FSCK_OK ) {  /* nothing extraordinary happened */
          if( !offset_found ) {
            bdg_rc = FSCK_INTERNAL_ERROR_51;
            }
          else {  /* we have the xad which describes the dmap */
            ext_logical_byte_offset = offsetXAD(xad_ptr) * sb_ptr->s_bsize;
            ext_byte_offset = addressXAD(xad_ptr) * sb_ptr->s_bsize;
            agg_recptr->bmapdm_agg_offset = ext_byte_offset +
                                            dmp_logical_byte_offset -
                                            ext_logical_byte_offset;

            bdg_rc = readwrite_device( agg_recptr->bmapdm_agg_offset,
                                       agg_recptr->bmapdm_buf_length, &bytes_read,
                                       (void *) agg_recptr->bmapdm_buf_ptr,
                                       fsck_READ );
            if( bdg_rc == FSCK_OK ) {  /* got the extent containing the dmap */
              agg_recptr->bmapdm_logical_offset = dmp_logical_byte_offset;
              *addr_dmap_page_ptr = (dmap_t *) agg_recptr->bmapdm_buf_ptr;
                     /*
                      * we need to set the buffer data length to the number of
                      * bytes with actual bmap data.  That is, we may have read
                      * beyond the end of the extent, and if so, we need to ignore
                      * the tag-along data.
                      */
              ext_bytes = lengthXAD(xad_ptr) * sb_ptr->s_bsize; 
              offset_into_extent = dmp_logical_byte_offset - ext_logical_byte_offset;
              ext_bytes_left = ext_bytes - offset_into_extent;
              agg_recptr->bmapdm_buf_data_len = MIN( bytes_read, ext_bytes_left );
              }  /* end got the extent containing the dmap */
            else {
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
              sprintf( message_parm_0, "%d", FSCK_FAILED_READ_BMPDM );
              msgprms[0] = message_parm_0;
              msgprmidx[0] = 0;
              sprintf( message_parm_1, "%d", bdg_rc );
              msgprms[1] = message_parm_1;
              msgprmidx[1] = 0;
              sprintf( message_parm_2, "%d", fsck_READ );
              msgprms[2] = message_parm_2;
              msgprmidx[2] = 0;
              sprintf( message_parm_3, "%lld", agg_recptr->bmapdm_agg_offset );
              msgprms[3] = message_parm_3;
              msgprmidx[3] = 0;
              sprintf( message_parm_4, "%ld", agg_recptr->bmapdm_buf_length );
              msgprms[4] = message_parm_4;
              msgprmidx[4] = 0;
              sprintf( message_parm_5, "%ld", agg_recptr->bmapdm_buf_data_len );
              msgprms[5] = message_parm_5;
              msgprmidx[5] = 0;
              fsck_send_msg( fsck_ERRONAGG, 0, 6 );

              bdg_rc = FSCK_FAILED_READ_BMPDM;
              }
            }  /* end else we have the xad which describes the dmap */
          }  /* end nothing extraordinary happened */
        }  /*  end else got the first agg extent */
      }  /* end flush worked ok */
    }  /* end we have to read it in */

  return( bdg_rc );
}                    /* end of blktbl_dmap_get ()       */
	 
	 
/*****************************************************************************
 * NAME: blktbl_dmap_put
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate,
 *            note, in the fsck workspace, that the current fsck dmap 
 *            buffer has been modified and should be written to the device
 *            in the next flush operation on this buffer.
 *
 * PARAMETERS:
 *      dmap_page_ptr  - input - address of the dmap page, in the fsck buffer,
 *                               which has been modified.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t blktbl_dmap_put( dmap_t *dmap_page_ptr )
{
  retcode_t bdp_rc = FSCK_OK;

  if( agg_recptr->processing_readwrite )  {
    agg_recptr->bmapdm_buf_write = 1;
    }

  return( bdp_rc );
}                    /* end of blktbl_dmap_put ()       */
	 
	 
/*****************************************************************************
 * NAME: blktbl_dmaps_flush
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate
 *            and the current dmap buffer has been updated since the most
 *            recent read operation, write the buffer contents to the device.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t blktbl_dmaps_flush ( )
{
  retcode_t bdf_rc = FSCK_OK;
  uint32 bytes_written;

  if( agg_recptr->bmapdm_buf_write )  {  /* buffer has been updated since
                               * most recent write
                               */
    bdf_rc = readwrite_device ( agg_recptr->bmapdm_agg_offset,
                                agg_recptr->bmapdm_buf_data_len,
                                &bytes_written,
                                (void *) agg_recptr->bmapdm_buf_ptr,
                                fsck_WRITE
                              );
    if( bdf_rc == FSCK_OK ) {

      if( bytes_written == agg_recptr->bmapdm_buf_data_len )  {

        agg_recptr->bmapdm_buf_write = 0; /* buffer has been written to
                               * the device and won't need to be
                               * written again unless/until the
                               * buffer contents have been altered again.
                               */
        }
      else {  /* didn't write the correct number of bytes */
           /* 
            * message to user
            */
  DBG_ERROR(("CHK:blktbl_dmaps_flush %d written != agg_recptr->bmapdm_buf_data_len \n",bytes_written));
        msgprms[0] = message_parm_0;
        msgprmidx[0] = fsck_metadata;
        msgprms[1] = Vol_Label;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", 6 );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        fsck_send_msg( fsck_URCVWRT, 0, 3 );
             /*
              * message to debugger
              */
        sprintf( message_parm_0, "%d", FSCK_FAILED_BMPDM_BADFLUSH );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", bdf_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_WRITE );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", agg_recptr->bmapdm_agg_offset );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", agg_recptr->bmapdm_buf_data_len );
        msgprms[4] = message_parm_4;
        msgprmidx[4] = 0;
        sprintf( message_parm_5, "%ld", bytes_written );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONAGG, 0, 6 );

        bdf_rc = FSCK_FAILED_BMPDM_BADFLUSH;
        }  /* end else didn't write the correct number of bytes */
      }

    else {  /* else the write was not successful */
           /* 
            * message to user
            */
  DBG_ERROR(("CHK:blktbl_dmaps_flush %d written not success \n",bytes_written));
      msgprms[0] = message_parm_0;
      msgprmidx[0] = fsck_metadata;
      msgprms[1] = Vol_Label;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", 7 );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_URCVWRT, 0, 3 );
             /*
              * message to debugger
              */
      sprintf( message_parm_0, "%d", FSCK_FAILED_BMPDM_FLUSH );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", bdf_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_WRITE );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", agg_recptr->bmapdm_agg_offset );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", agg_recptr->bmapdm_buf_data_len );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", bytes_written );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONAGG, 0, 6 );

      bdf_rc = FSCK_FAILED_BMPDM_FLUSH;
      }  /* end else the write was not successful */
    }  /* end buffer has been updated since most recent write */

  return( bdf_rc );
}                              /* end of blktbl_dmaps_flush ()  */
	 
	 
/*****************************************************************************
 * NAME: blktbl_Ln_page_get
 *
 * FUNCTION: Read the JFS Block Map page describing the specified aggregate
 *           block at the specified summary level into and/or locate the
 *           locate the requested JFS Block Map page describing the specified
 *           aggregate block at the specified summary level in the
 *           fsck Level n page buffer.
 *
 * PARAMETERS:
 *      level             - input - Summary level of the page to get
 *      for_block         - input - ordinal number of the aggregate block
 *                                  whose summary page is needed
 *      addr_Ln_page_ptr  - input - pointer to a variable in which to return
 *                                  the address of the requested page in an
 *                                  fsck buffer
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t blktbl_Ln_page_get ( int8         level,
                               fsblkidx_t   for_block,
                               dmapctl_t  **addr_Ln_page_ptr )
{
  retcode_t blpg_rc = FSCK_OK;
  fsblkidx_t Lnpg_logical_fsblk_offset;
  agg_byteidx_t Lnpg_logical_byte_offset;
  agg_byteidx_t ext_logical_byte_offset;
  agg_byteidx_t ext_byte_offset;
  agg_byteidx_t last_in_buffer;
  dinode_t *bmap_inoptr;
  xad_t    *xad_ptr;
  int8     offset_found;
  int      is_aggregate = -1;            /* the bmap inode is in the ait */
  int      which_it;

  Lnpg_logical_fsblk_offset =
                    BLKTOCTL( for_block, agg_recptr->log2_blksperpg, level );
  Lnpg_logical_byte_offset = Lnpg_logical_fsblk_offset * sb_ptr->s_bsize;

  *addr_Ln_page_ptr = NULL;

  if( Lnpg_logical_byte_offset >= agg_recptr->bmaplv_logical_offset ) {
    last_in_buffer = agg_recptr->bmaplv_logical_offset +
                     agg_recptr->bmaplv_buf_data_len;
    if( (Lnpg_logical_byte_offset+sizeof(dmapctl_t)) <= last_in_buffer ) {  /*
                           * the one we want is already in the buffer
                           */
      *addr_Ln_page_ptr = (dmapctl_t *) ((uint32) agg_recptr->bmaplv_buf_ptr +
                                        Lnpg_logical_byte_offset -
                                        agg_recptr->bmaplv_logical_offset);
      }  /* end the one we want is already in the buffer */
    }

  if( *addr_Ln_page_ptr == NULL ) {  /* we have to read it in */
    blpg_rc = blktbl_Ln_pages_flush();     /* perform any pending writes */

    if( blpg_rc == FSCK_OK ) {  /* flush worked ok */

      if( agg_recptr->primary_ait_4part1 ) {
        which_it = fsck_primary;
        }
      else {
        which_it = fsck_secondary;
        }
      blpg_rc = ait_special_read_ext1( which_it );
      if( blpg_rc != FSCK_OK ) {
        report_readait_error( blpg_rc, 
                              FSCK_FAILED_CANTREADAITEXTG, 
                              which_it );
        blpg_rc = FSCK_FAILED_CANTREADAITEXTG;
        } 

      else {  /* got the first agg extent */
        bmap_inoptr = (dinode_t *)
                      (agg_recptr->ino_buf_ptr + BMAP_I * sizeof(dinode_t));

        blpg_rc = xTree_search( bmap_inoptr, Lnpg_logical_fsblk_offset,
                                &xad_ptr, &offset_found );
        if( blpg_rc == FSCK_OK ) {  /* nothing extraordinary happened */
          if( !offset_found ) {  /* didn't find it! */
            blpg_rc = FSCK_INTERNAL_ERROR_52;
            }  /* end didn't find it! */
          else {  /* we have the xad which describes the page */
            ext_logical_byte_offset = offsetXAD(xad_ptr) * sb_ptr->s_bsize;
            ext_byte_offset = addressXAD(xad_ptr) * sb_ptr->s_bsize;
            agg_recptr->bmaplv_agg_offset = ext_byte_offset +
                                            Lnpg_logical_byte_offset -
                                            ext_logical_byte_offset;

            blpg_rc = readwrite_device( agg_recptr->bmaplv_agg_offset,
                                        agg_recptr->bmaplv_buf_length,
                                        &(agg_recptr->bmaplv_buf_data_len),
                                        (void *) agg_recptr->bmaplv_buf_ptr,
                                        fsck_READ );
            if( blpg_rc == FSCK_OK ) {  /* got the page */
              agg_recptr->bmaplv_current_level = level;
              agg_recptr->bmaplv_logical_offset = Lnpg_logical_byte_offset;
              *addr_Ln_page_ptr = (dmapctl_t *) agg_recptr->bmaplv_buf_ptr;
              }  /* end got the dmap */
            else {
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
              sprintf( message_parm_0, "%d", FSCK_FAILED_READ_BMPLV );
              msgprms[0] = message_parm_0;
              msgprmidx[0] = 0;
              sprintf( message_parm_1, "%d", blpg_rc );
              msgprms[1] = message_parm_1;
              msgprmidx[1] = 0;
              sprintf( message_parm_2, "%d", fsck_READ );
              msgprms[2] = message_parm_2;
              msgprmidx[2] = 0;
              sprintf( message_parm_3, "%lld", agg_recptr->bmaplv_agg_offset );
              msgprms[3] = message_parm_3;
              msgprmidx[3] = 0;
              sprintf( message_parm_4, "%ld", agg_recptr->bmaplv_buf_length );
              msgprms[4] = message_parm_4;
              msgprmidx[4] = 0;
              sprintf( message_parm_5, "%ld", agg_recptr->bmaplv_buf_data_len );
              msgprms[5] = message_parm_5;
              msgprmidx[5] = 0;
              fsck_send_msg( fsck_ERRONAGG, 0, 6 );

              blpg_rc = FSCK_FAILED_READ_BMPLV;
              }
            }  /* end else we have the xad which describes the page */
          }  /* end nothing extraordinary happened */
        }  /*  end else got the first agg extent */
      }  /* end flush worked ok */
    }  /* end we have to read it in */

  return( blpg_rc );
}                    /* end of blktbl_Ln_page_get ()       */
	 
	 
/*****************************************************************************
 * NAME: blktbl_Ln_page_put
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate,
 *            note, in the fsck workspace, that the current fsck Level n
 *            buffer has been modified and should be written to the device
 *            in the next flush operation on this buffer.
 *
 * PARAMETERS:
 *      Ln_page_ptr  - input - Address, in an fsck buffer, of the block map
 *                             summary page which has been modified.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t blktbl_Ln_page_put ( dmapctl_t *Ln_page_ptr )
{
  retcode_t blpp_rc = FSCK_OK;

  if( agg_recptr->processing_readwrite )  {
    agg_recptr->bmaplv_buf_write = 1;
    }

  return( blpp_rc );
}                    /* end of blktbl_Ln_page_put ()       */
	 
	 
/*****************************************************************************
 * NAME: blktbl_Ln_pages_flush
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate
 *            and the current Level n Page buffer has been updated since
 *            the most recent read operation, write the buffer contents to
 *            the device.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t blktbl_Ln_pages_flush ( )
{
  retcode_t blpf_rc = FSCK_OK;
  uint32 bytes_written;

  if( agg_recptr->bmaplv_buf_write )  {  /* buffer has been updated since
                               * most recent write
                               */
    blpf_rc = readwrite_device ( agg_recptr->bmaplv_agg_offset,
                                 agg_recptr->bmaplv_buf_data_len,
                                 &bytes_written,
                                 (void *) agg_recptr->bmaplv_buf_ptr,
                                 fsck_WRITE
                               );
    if( blpf_rc == FSCK_OK ) {

      if( bytes_written == agg_recptr->bmaplv_buf_data_len )  {

        agg_recptr->bmaplv_buf_write = 0; /* buffer has been written to
                               * the device and won't need to be
                               * written again unless/until the
                               * buffer contents have been altered again.
                               */
        }
      else {  /* didn't write the correct number of bytes */
           /* 
            * message to user
            */
  DBG_ERROR(("CHK:blktbl_Ln_pages_flush %d written != agg_recptr->bmaplv_buf_data_len\n",bytes_written));
        msgprms[0] = message_parm_0;
        msgprmidx[0] = fsck_metadata;
        msgprms[1] = Vol_Label;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", 8 );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        fsck_send_msg( fsck_URCVWRT, 0, 3 );
             /*
              * message to debugger
              */
        sprintf( message_parm_0, "%d", FSCK_FAILED_BMPLV_BADFLUSH );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", blpf_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_WRITE );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", agg_recptr->bmaplv_agg_offset );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", agg_recptr->bmaplv_buf_data_len );
        msgprms[4] = message_parm_4;
        msgprmidx[4] = 0;
        sprintf( message_parm_5, "%ld", bytes_written );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONAGG, 0, 6 );

        blpf_rc = FSCK_FAILED_BMPLV_BADFLUSH;
        }  /* end else didn't write the correct number of bytes */
      }

    else {  /* else the write was not successful */
           /* 
            * message to user
            */
  DBG_ERROR(("CHK:blktbl_Ln_pages_flush %d written not success \n",bytes_written));
      msgprms[0] = message_parm_0;
      msgprmidx[0] = fsck_metadata;
      msgprms[1] = Vol_Label;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", 9 );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_URCVWRT, 0, 3 );
             /*
              * message to debugger
              */
      sprintf( message_parm_0, "%d", FSCK_FAILED_BMPLV_FLUSH );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", blpf_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_WRITE );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", agg_recptr->bmaplv_agg_offset );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", agg_recptr->bmaplv_buf_data_len );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", bytes_written );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONAGG, 0, 6 );

      blpf_rc = FSCK_FAILED_BMPLV_FLUSH;
      }  /* end else the write was not successful */
    }  /* end buffer has been updated since most recent write */

  return( blpf_rc );
}                              /* end of blktbl_Ln_pages_flush ()  */
	 
	 
/**************************************************************************
 * NAME: close_volume
 *
 * FUNCTION:  If fsck holds a lock on the device containing the aggregate,
 *            release that lock.  
 *
 *            Close the device containing the aggregate.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t close_volume( )
{
  retcode_t cv_rc = FSCK_OK;
  retcode_t ioctl_rc = FSCK_OK;
  retcode_t redeterm_rc = FSCK_OK;

  char CommandInfo = 0;
  unsigned long ParmLen = sizeof( CommandInfo );
  unsigned long DataLen = sizeof( CommandInfo );

  if( agg_recptr->parm_options[UFS_CHKDSK_CLRBDBLKLST] ) {  /*
				* bad block list processing only
				*/
    redeterm_rc = ujfs_redeterminemedia( Dev_IOPort );

    sprintf( message_parm_0, "%ld", redeterm_rc );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    fsck_send_msg( fsck_DEVREDETERMRC, 0, 1 );
    }
  else if( !agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK] ) {  /*
                           * not in autocheck mode
                           */
    if( agg_recptr->device_is_locked ) 
      {  /* device now locked */

      redeterm_rc = ujfs_redeterminemedia( Dev_IOPort );

      sprintf( message_parm_0, "%ld", redeterm_rc );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      fsck_send_msg( fsck_DEVREDETERMRC, 0, 1 );

      ioctl_rc = DosDevIOCtl( Dev_IOPort, IOCTL_DISK, DSK_UNLOCKDRIVE,
                              &CommandInfo, sizeof(CommandInfo), &ParmLen,
                              &CommandInfo, sizeof(CommandInfo), &DataLen );
      agg_recptr->device_is_locked = 0;

      sprintf( message_parm_0, "%ld", ioctl_rc );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      fsck_send_msg( fsck_DEVUNLOCKRC, 0, 1 );

      if( ioctl_rc != FSCK_OK ) { /* error during unlock */
        fsck_send_msg( fsck_URCVUNLCK, 0, 0 );
        }  /* end error during unlock */
      }  /* end device now locked */
    }  /* end not in autocheck mode */

  cv_rc = DosClose( Dev_IOPort );

  sprintf( message_parm_0, "%ld", cv_rc );
  msgprms[0] = message_parm_0;
  msgprmidx[0] = 0;
  fsck_send_msg( fsck_DEVCLOSERC, 0, 1 );

  if( cv_rc != FSCK_OK ) { /* error during close */
    fsck_send_msg( fsck_URCVCLOSE, 0, 0 );
    }  /* error during close */
  else {  /* no error during close */
    cv_rc = ioctl_rc;  /* report the one during unlock (if any) */
    }  /* end no error during close */

  free(ptrklay);

  return( cv_rc );
}                           /* end close_volume () */
	 
	 
/**************************************************************************
 * NAME: default_volume
 *
 * FUNCTION:  If fsck holds a lock on the device containing the aggregate,
 *            release that lock.  
 *
 *            Close the device containing the aggregate.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t default_volume( )
{
  retcode_t dv_rc = FSCK_OK;
  int   buf_len;
  char *buf_ptr;

  buf_len = msgs_txt_maxlen; 
  buf_ptr = msgs_txt_ptr;

  getcwd( buf_ptr, buf_len );

  Vol_Label[0] = msgs_txt_ptr[0];
  Vol_Label[1] = msgs_txt_ptr[1];

  msgprms[0] = Vol_Label;
  msgprmidx[0] = 0;
  fsck_send_msg( fsck_DEFAULTVOL, 0, 1 );

  return( dv_rc );
}                           /* end default_volume () */
	 
	 
/*****************************************************************************
 * NAME: dnode_get
 *
 * FUNCTION: Read the requested dnode page into and/or locate the requested
 *           dnode page in the fsck dnode buffer.
 *
 * PARAMETERS:
 *      dnode_fsblk_offset  - input - offset, in aggregate blocks, into the
 *                                    aggregate of the dnode to read
 *      dnode_length        - input - number of bytes in the dnode
 *      addr_dtpage_ptr     - input - pointer to a variable in which to return
 *                                    the address of the requested dnode in an
 *                                    fsck buffer
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t dnode_get ( fsblkidx_t  dnode_fsblk_offset,
                      reg_idx_t   dnode_length,
                      dtpage_t  **addr_dtpage_ptr
                    )
{
  retcode_t dnodg_rc = FSCK_OK;
  agg_byteidx_t dnode_start_byte, dnode_end_byte;

  dnode_start_byte = dnode_fsblk_offset * sb_ptr->s_bsize;
  dnode_end_byte = dnode_start_byte + dnode_length - 1;

  if( (agg_recptr->ondev_wsp_fsblk_offset != 0) &&
      (dnode_fsblk_offset > agg_recptr->ondev_wsp_fsblk_offset) ) { /*
			      * the offset is beyond the range 
			      * valid for fileset objects
			      */
	/*
	 * This case is not caused by an I/O error, but by
	 * invalid data in an inode.  Let the caller handle
	 * the consequences.
	 */
    dnodg_rc = FSCK_BADREADTARGET;
    }  /* end the offset is beyond the range valid ... */
		
  else if( (dnode_start_byte >= agg_recptr->node_agg_offset) &&
      (dnode_end_byte <= (agg_recptr->node_agg_offset +
                          agg_recptr->node_buf_data_len))  )  {  /*
                              * the target dir node is already in
                              * the buffer
                              */
    *addr_dtpage_ptr = (dtpage_t *)
                          (agg_recptr->node_buf_ptr +
                           dnode_start_byte - agg_recptr->node_agg_offset);
    }  /* end the target node is already in the buffer */
		
  else {  /* else we'll have to read it from the disk */
    agg_recptr->node_agg_offset = dnode_start_byte;

    dnodg_rc = readwrite_device( agg_recptr->node_agg_offset,
                                 agg_recptr->node_buf_length,
                                 &(agg_recptr->node_buf_data_len),
                                 (void *) agg_recptr->node_buf_ptr,
                                 fsck_READ
                                );
    if( dnodg_rc == FSCK_OK ) {  /* read appears successful */
      if( agg_recptr->node_buf_data_len >= dnode_length ) {  /*
                              * we may not have gotten all we asked for,
                              * but we got enough to cover the dir node
                              * we were after
                              */
        *addr_dtpage_ptr = (dtpage_t *) agg_recptr->node_buf_ptr;
        }  /* end we may not have gotten all we asked for, ... */
      else {  /* didn't get the minimum number of bytes */
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
        sprintf( message_parm_0, "%d", FSCK_FAILED_BADREAD_DNODE );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", dnodg_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_READ );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", agg_recptr->node_agg_offset );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", agg_recptr->node_buf_length );
        msgprms[4] = message_parm_4;
        msgprmidx[4] = 0;
        sprintf( message_parm_5, "%ld", agg_recptr->node_buf_data_len );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONAGG, 0, 6 );

        dnodg_rc = FSCK_FAILED_BADREAD_DNODE;
        }  /* end else didn't get the minimum number of bytes */
      }  /* end read appears successful */
    else {  /* bad return code from read */
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
      sprintf( message_parm_0, "%d", FSCK_FAILED_READ_DNODE );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", dnodg_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_READ );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", agg_recptr->node_agg_offset );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", agg_recptr->node_buf_length );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", agg_recptr->node_buf_data_len );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONAGG, 0, 6 );

      dnodg_rc = FSCK_FAILED_READ_DNODE;
      }  /* end else bad return code from read */
    }  /* end else we'll have to read it from the disk */

  return( dnodg_rc );
}                              /* end of dnode_get ()  */
	 
	 
/*****************************************************************************
 * NAME: ea_get
 *
 * FUNCTION: Read the specified Extended Attributes data (ea) into 
 *           the specified buffer.
 *
 * PARAMETERS:
 *      ea_fsblk_offset  - input - offset, in aggregate blocks, into the
 *                                    aggregate of the ea to read
 *      ea_byte_length   - input - length, in bytes, of the ea to read
 *      eabuf_ptr        - input - the address (in dynamic storage) of the 
 *                                 buffer into which to read the ea
 *      eabuf_length     - input - pointer to a variable in which contains
 *                                 the length of the buffer at eabuf_ptr
 *      ea_data_length   - input - pointer to a variable in which to return
 *                                 the number of bytes actually read from the
 *                                 device
 *      ea_agg_offset    - input - pointer to a variable in which to return 
 *                                 the offset, in bytes, into the aggregate
 *                                 of the ea to read
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t ea_get ( fsblkidx_t     ea_fsblk_offset,
                   reg_idx_t      ea_byte_length,
                   char          *eabuf_ptr,
                   reg_idx_t     *eabuf_length,
                   reg_idx_t     *ea_data_length,
                   agg_byteidx_t *ea_agg_offset
                 )
{
  retcode_t ea_rc = FSCK_OK;
  agg_byteidx_t start_byte, end_byte;
  start_byte = ea_fsblk_offset * sb_ptr->s_bsize;
  end_byte = start_byte + ea_byte_length - 1;
  ea_rc = readwrite_device( start_byte, ea_byte_length, ea_data_length,
                            (void *) eabuf_ptr, fsck_READ );
  if( ea_rc == FSCK_OK ) {  /* read appears successful */
    *ea_agg_offset = start_byte;
    if( (*ea_data_length) < ea_byte_length ) {  /* we didn't get enough */
      *ea_agg_offset = 0;
      *ea_data_length = 0;
      ea_rc = FSCK_BADEADESCRIPTOR;
      }  /* end we didn't get enough */
    }  /* end read appears successful */
  else {  /* bad return code from read */
    ea_rc = FSCK_CANTREADEA;
    }  /* end else bad return code from read */
  return( ea_rc );
}                              /* end of ea_get ()  */
	 
	 
/*****************************************************************************
 * NAME: fscklog_put_buffer
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate,
 *            and if the in-aggregate fsck log is not full, write the
 *            contents of the current fscklog buffer into the in-aggregate
 *            fsck log.
 *
 * PARAMETERS:  none
 *
 * NOTES:  o Unlike most _put_ routines in this module, 
_buffer
 *           actually writes to the device.  This is done because the fsck
 *           log contains information which provides crucial serviceability 
 *           should the fsck session be interrupted.
 *
 *         o Errors here are recorded in the control page of the fsck 
 *           in-aggregate workspace but never affect other fsck processing.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t  fscklog_put_buffer ( )
{
  retcode_t flpb_rc = FSCK_OK;
  retcode_t io_rc = FSCK_OK;
  uint32 bytes_written = 0;
  uint32 log_bytes_left;
  int32 num_log_errors;
  fscklog_error_t *log_error_recptr;
  if( (!agg_recptr->fscklog_full) && 
      (agg_recptr->processing_readwrite) ) {  /* have write access */
    io_rc = readwrite_device( agg_recptr->fscklog_agg_offset,
                              agg_recptr->fscklog_buf_length,
                              &bytes_written,
                              (void *) agg_recptr->fscklog_buf_ptr,
                              fsck_WRITE
                             );
    if( (io_rc != FSCK_OK) ||
        (bytes_written != (uint32) agg_recptr->fscklog_buf_length) )  {  /*
                               * write failed or didn't write correct 
                               * number of bytes 
                               */
      agg_recptr->fscklog_full = 1;	/* This prevents infinite recursion */
      if (agg_recptr->blkmp_ctlptr)
      {
        num_log_errors = agg_recptr->blkmp_ctlptr->hdr.num_logwrite_errors;
        if( num_log_errors < 120 ) {
          log_error_recptr = 
                    &(agg_recptr->blkmp_ctlptr->hdr.logerr[num_log_errors]);
          log_error_recptr->err_offset = agg_recptr->fscklog_agg_offset;     
          log_error_recptr->bytes_written = bytes_written;     
          log_error_recptr->io_retcode = io_rc;     
          }
        agg_recptr->blkmp_ctlptr->hdr.num_logwrite_errors += 1;
      }
             /*
              * message to debugger
              *
              * N.B. This is NOT a fatal condition!
              */
      sprintf( message_parm_0, "%d", FSCK_BADWRITE_FSCKLOG );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", io_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_WRITE );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", agg_recptr->fscklog_agg_offset );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", agg_recptr->fscklog_buf_length );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", bytes_written );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONLOG, 0, 6 );
      }  /* end write failed or didn't write correct number of bytes */
    }  /* end have write access */
      /*
       * We want to reset the buffer no matter what.
       * It is useful to refill the buffer even if logging is not
       * active because it may provide diagnostic information in
       * a dump.
       */
  agg_recptr->fscklog_agg_offset += agg_recptr->fscklog_buf_length;
  agg_recptr->fscklog_log_offset += agg_recptr->fscklog_buf_length;
  agg_recptr->fscklog_buf_data_len = 0;
  log_bytes_left = (agg_recptr->ondev_fscklog_byte_length / 2) -
                   agg_recptr->fscklog_log_offset;
  if( log_bytes_left < agg_recptr->fscklog_buf_length ) {  /* 
                                * can't fit another buffer full
                                * into the log 
                                */
    if( !agg_recptr->initializing_fscklog ) {  /* this is a false
                                * condition if doing log initialization
                                */
      agg_recptr->fscklog_full = -1;
      agg_recptr->blkmp_ctlptr->hdr.fscklog_full = -1;
      }
    }  /* end can't fit another buffer full into the log */
  return( flpb_rc );
}                              /* end of fscklog_put_buffer ()  */
	 
	 
/*****************************************************************************
 * NAME: iag_get
 *
 * FUNCTION: Read the requested iag into and/or locate the requested iag
 *           in the fsck iag buffer.
 *
 * PARAMETERS:
 *      is_aggregate  - input -  0 => the iag is owned by the fileset
 *                              !0 => the iag is owned by the aggregate
 *      which_it      - input - ordinal number of the aggregate inode
 *                              representing the inode table to which the
 *                              iag belongs.
 *      which_ait     - input - the aggregate inode table { fsck_primary |
 *                              fsck_secondary } containing the version of
 *                              which_it to use for this operation
 *      iag_num       - input - ordinal number of the iag needed
 *      addr_iag_ptr  - input - pointer to a variable in which to return 
 *                              the address, in an fsck buffer, of the
 *                              requested iag.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iag_get ( int       is_aggregate,
                    int       which_it,
                    int       which_ait,
                    iagno_t   iag_num,
                    iag_t   **addr_iag_ptr
                  )
{
  retcode_t     iagg_rc = FSCK_OK;
  fsblkidx_t    imap_logical_block, extent_offset;
  fsblkidx_t    offset_in_extent = 0;
  dinode_t     *imap_inoptr;
  xad_t        *xad_ptr;
  int8          offset_found;
  int           it_tbl_is_agg_owned = -1;
  int           which_agg_inode;
  *addr_iag_ptr = NULL;
  if( (agg_recptr->iag_buf_1st_inode == (iag_num << L2INOSPERIAG)) &&
      (agg_recptr->iag_for_aggregate == is_aggregate)              &&
      (agg_recptr->iag_which_it == which_it)                         ) { /*
                              * the target iag is already in the buffer
                              */
    *addr_iag_ptr = (iag_t *) agg_recptr->iag_buf_ptr;
    }  /* end the target iag is already in the buffer */
  else {  /* need to get the iag */
    iagg_rc = iags_flush();         /* perform any pending writes */
    if( iagg_rc == FSCK_OK ) {  /* flush worked ok */
      if( is_aggregate ) {  /* this is an IAG describing aggregate inodes */
        if( iag_num < agg_recptr->agg_imap.num_iags ) {  /* in bounds */
          which_agg_inode = AGGREGATE_I;
          iagg_rc = ait_special_read_ext1( which_ait );
          if( iagg_rc != FSCK_OK ) { /* read ait failed */
            report_readait_error( iagg_rc, 
                                  FSCK_FAILED_CANTREADAITEXTH, 
                                  which_ait );
            iagg_rc = FSCK_FAILED_CANTREADAITEXTH;
            }  /* end read ait failed */
          }  /* end in bounds */
        else {  /* invalid request */
          iagg_rc = FSCK_IAGNOOOAGGBOUNDS;
          }  /* end invalid request */
        }  /* end this is an IAG describing aggregate inodes */
      else {  /* an IAG describing fileset inodes */
        if( iag_num < agg_recptr->fset_imap.num_iags ) {  /* in bounds */
          which_agg_inode = FILESYSTEM_I;
          iagg_rc = ait_special_read_ext1( which_ait );
          if( iagg_rc != FSCK_OK ) { /* read ait failed */
            report_readait_error( iagg_rc, 
                                  FSCK_FAILED_CANTREADAITEXTK, 
                                  which_ait );
            iagg_rc = FSCK_FAILED_CANTREADAITEXTK;
            }  /* end read ait failed */
          }  /* end in bounds */
        else {  /* invalid request */
          iagg_rc = FSCK_IAGNOOOFSETBOUNDS;
          }  /* end invalid request */
        }  /* end else an IAG describing fileset inodes */
      if( iagg_rc == FSCK_OK ) {  /* got the extent */
        imap_inoptr = (dinode_t *)
                      (agg_recptr->ino_buf_ptr +
                      (which_agg_inode * sizeof(dinode_t)) );
        imap_logical_block = IAGTOLBLK( iag_num, agg_recptr->log2_blksperpg );
        iagg_rc = xTree_search( imap_inoptr, imap_logical_block,
                                &xad_ptr, &offset_found );
        if( iagg_rc == FSCK_OK ) {  /* nothing extraordinary happened */
          if( !offset_found ) {
            iagg_rc = FSCK_INTERNAL_ERROR_50;
            }
          else {  /* we have the xad which describes the iag */
            extent_offset = offsetXAD(xad_ptr);
            if( extent_offset != imap_logical_block ) {
              offset_in_extent = imap_logical_block - extent_offset;
              }
            agg_recptr->iag_agg_offset = sb_ptr->s_bsize *
                                    (addressXAD(xad_ptr) + offset_in_extent);
            iagg_rc = readwrite_device( agg_recptr->iag_agg_offset,
                                        agg_recptr->iag_buf_length,
                                        &(agg_recptr->iag_buf_data_len),
                                        (void *) agg_recptr->iag_buf_ptr,
                                        fsck_READ );
            if( iagg_rc == FSCK_OK ) {  /* got the iag */
              agg_recptr->iag_buf_1st_inode = iag_num << L2INOSPERIAG;
              agg_recptr->iag_fsnum = imap_inoptr->di_fileset;
              agg_recptr->iag_for_aggregate = is_aggregate;
              agg_recptr->iag_which_it = which_it;
              *addr_iag_ptr = (iag_t *) agg_recptr->iag_buf_ptr;
              }  /* end got the iag */
            else {
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
              sprintf( message_parm_0, "%d", FSCK_FAILED_READ_IAG );
              msgprms[0] = message_parm_0;
              msgprmidx[0] = 0;
              sprintf( message_parm_1, "%d", iagg_rc );
              msgprms[1] = message_parm_1;
              msgprmidx[1] = 0;
              sprintf( message_parm_2, "%d", fsck_READ );
              msgprms[2] = message_parm_2;
              msgprmidx[2] = 0;
              sprintf( message_parm_3, "%lld", agg_recptr->iag_agg_offset );
              msgprms[3] = message_parm_3;
              msgprmidx[3] = 0;
              sprintf( message_parm_4, "%ld", agg_recptr->iag_buf_length );
              msgprms[4] = message_parm_4;
              msgprmidx[4] = 0;
              sprintf( message_parm_5, "%ld", agg_recptr->iag_buf_data_len );
              msgprms[5] = message_parm_5;
              msgprmidx[5] = 0;
              fsck_send_msg( fsck_ERRONAGG, 0, 6 );
              iagg_rc = FSCK_FAILED_READ_IAG; 
              }
            }  /* end else we have the xad which describes the iag */
          }  /* end nothing extraordinary happened */
        }  /*  end got the inode */
      }  /* end flush worked ok */
    }  /* end else need to get the iag */
  return( iagg_rc );
}                              /* end of iag_get ()  */
	 
	 
/*****************************************************************************
 * NAME: iag_get_first
 *
 * FUNCTION: Read the first iag in the specified inode table into and/or 
 *           locate the first iag in the specified inode table in the
 *           fsck iag buffer.  Set up for sequential access on the iag's
 *           in this table.
 *
 * PARAMETERS:
 *      is_aggregate  - input -  0 => the iag is owned by the fileset
 *                              !0 => the iag is owned by the aggregate
 *      which_it      - input - ordinal number of the aggregate inode
 *                              representing the inode table to which the
 *                              iag belongs.
 *      which_ait     - input - the aggregate inode table { fsck_primary |
 *                              fsck_secondary } containing the version of
 *                              which_it to use for this operation
 *      addr_iag_ptr  - input - pointer to a variable in which to return
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iag_get_first ( int       is_aggregate,
                          int       it_number,
                          int       which_ait,
                          iag_t   **addr_iag_ptr
                        )
{
  retcode_t iaggf_rc = FSCK_OK;
  dinode_t *imap_inoptr;
  xad_t    *xadptr;
  int      it_tbl_is_agg_owned = -1;  /* the table may not describe the
                                       * aggregate inodes, but the inode
                                       * describing the table is ALWAYS
                                       * an aggregate inode
                                       */
  agg_recptr->fais.this_iagnum = 0;
  agg_recptr->fais.this_inoidx = 0;
  agg_recptr->fais.rootleaf_imap = 0;  /* assume it's not a rootleaf */
      /*
       * get the first leaf (xtpage) of the imap into the mapleaf buffer
       *
       * Note: if the imap B+ Tree has a root-leaf then the
       *       first (and only) leaf is located in the dinode.
       */
  iaggf_rc = ait_special_read_ext1( which_ait );  /*
                             * save inode_get from having to read an
                             * iag as an intermediate step.
                             */
  if( iaggf_rc != FSCK_OK ) { /* read ait failed */
    report_readait_error( iaggf_rc, 
                          FSCK_FAILED_CANTREADAITEXTJ, 
                          which_ait );
    iaggf_rc = FSCK_FAILED_CANTREADAITEXTJ;
    }  /* end read ait failed */
  else {  /* got the inode extent */
    if( !is_aggregate ) {  /* after the fileset inode table */
      if( agg_recptr->fset_imap.imap_is_rootleaf )  { /* it's a root-leaf */
        agg_recptr->fais.rootleaf_imap = -1;
        iaggf_rc = inode_get( it_tbl_is_agg_owned, which_ait,
                              it_number, &imap_inoptr ); /*
                             * read the imap inode into the inode buffer
                             */
        if( iaggf_rc != FSCK_OK ) { /* something went wrong */
          iaggf_rc = FSCK_FAILED_AGFS_READ5;
          }  /* end something went wrong */
        }  /* it's a root-leaf */
      }  /* end after the fileset inode table */
    else {  /* want the aggregate inode table */
      if( agg_recptr->agg_imap.imap_is_rootleaf ) {  /* it's a rootleaf */
        agg_recptr->fais.rootleaf_imap = -1;
        iaggf_rc = inode_get( it_tbl_is_agg_owned, it_number,
                              AGGREGATE_I, &imap_inoptr ); /*
                             * read the imap inode into the inode buffer
                             */
        if( iaggf_rc != FSCK_OK ) { /* something went wrong */
          iaggf_rc = FSCK_FAILED_AGFS_READ5;
          }  /* end something went wrong */
        }  /* it's a root-leaf */
      }  /* end else want the aggregate inode table */
    }  /* end else got the inode extent */
  if( (iaggf_rc == FSCK_OK) && (agg_recptr->fais.rootleaf_imap) ) {  /*
                             * root-leaf imap and we have the inode
                             */
    memcpy( (void *) (agg_recptr->mapleaf_buf_ptr),
            (void *) imap_inoptr,
            sizeof(dinode_t) );  /* copy the inode into the imap leaf buf */
    agg_recptr->mapleaf_buf_data_len = 0;
    agg_recptr->mapleaf_agg_offset = 0;
    agg_recptr->mapleaf_for_aggregate = is_aggregate;
    agg_recptr->mapleaf_which_it = it_number;
    imap_inoptr = (dinode_t *) (agg_recptr->mapleaf_buf_ptr);
    agg_recptr->fais.this_mapleaf = (xtpage_t *) &(imap_inoptr->di_btroot);
    }  /* end root-leaf imap and we have the inode */
  if( (iaggf_rc == FSCK_OK) && (!agg_recptr->fais.rootleaf_imap) ) {  /* 
                             * something below the root
                             */
    if( !is_aggregate ) {  /* after the fileset inode table */
      iaggf_rc = imapleaf_get( agg_recptr->fset_imap.first_leaf_offset,
                              &(agg_recptr->fais.this_mapleaf) ); /* get
                             * the first leaf into the mapleaf buffer
                             */
      }  /* end after the fileset inode table */
    else {  /* must want the aggregate inode table */
      iaggf_rc = imapleaf_get( agg_recptr->agg_imap.first_leaf_offset,
                              &(agg_recptr->fais.this_mapleaf) ); /* get
                             * the first leaf into the mapleaf buffer
                             */
      }  /* end else must want the aggregate inode table */
    }  /* end else something below the root */
  if( iaggf_rc == FSCK_OK )  {  /* the first imap leaf is in the buf */
    agg_recptr->fais.iagidx_now = XTENTRYSTART;   /* first in the leaf */
    agg_recptr->fais.iagidx_max =
                        agg_recptr->fais.this_mapleaf->header.nextindex - 1;
        /*
         * get the first iag of the imap into the iag buffer
         */
    iaggf_rc = iags_flush();
    if( iaggf_rc == FSCK_OK ) {  /* flushed ok */
      xadptr =
         &(agg_recptr->fais.this_mapleaf->xad[agg_recptr->fais.iagidx_now]);
      agg_recptr->iag_agg_offset = (sb_ptr->s_bsize * addressXAD(xadptr)) +
                                   sizeof(dinomap_t);  /*
                              * the first iag is preceded by the IT control
                              * page.
                              */
      agg_recptr->iag_buf_1st_inode = agg_recptr->fais.this_inoidx;
      agg_recptr->iag_fsnum = it_number;
      agg_recptr->iag_for_aggregate = is_aggregate;
      agg_recptr->iag_fsnum = it_number;
      iaggf_rc = readwrite_device( agg_recptr->iag_agg_offset,
                                  agg_recptr->iag_buf_length,
                                  &(agg_recptr->iag_buf_data_len),
                                  (void *) (agg_recptr->iag_buf_ptr),
                                  fsck_READ );
      }  /* end flushed ok */
    }  /* end the first imap leaf is in the buf */
  if( iaggf_rc == FSCK_OK )  {  /* first iag is in iag buffer */
    agg_recptr->fais.iagptr = (iag_t *) agg_recptr->iag_buf_ptr;
    agg_recptr->fais.extidx_now = 0;
    agg_recptr->fais.extidx_max = EXTSPERIAG - 1;
    *addr_iag_ptr = agg_recptr->fais.iagptr;
    }  /* end first iag is in iag buffer */
  else {
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
    sprintf( message_parm_0, "%d", FSCK_FAILED_BADREAD_IAG );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    sprintf( message_parm_1, "%d", iaggf_rc );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    sprintf( message_parm_2, "%d", fsck_READ );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    sprintf( message_parm_3, "%lld", agg_recptr->iag_agg_offset );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    sprintf( message_parm_4, "%ld", agg_recptr->iag_buf_length );
    msgprms[4] = message_parm_4;
    msgprmidx[4] = 0;
    sprintf( message_parm_5, "%ld", agg_recptr->iag_buf_data_len );
    msgprms[5] = message_parm_5;
    msgprmidx[5] = 0;
    fsck_send_msg( fsck_ERRONAGG, 0, 6 );
    iaggf_rc = FSCK_FAILED_BADREAD_IAG; 
    }
  return( iaggf_rc );
}                              /* end of iag_get_first ()  */
	 
	 
/*****************************************************************************
 * NAME: iag_get_next
 *
 * FUNCTION: Read the next iag in the specified inode table into and/or 
 *           locate the next iag in the specified inode table in the
 *           fsck iag buffer.
 *
 * PARAMETERS:
 *      addr_iag_ptr  - input - pointer to a variable in which to return
 *                              the address, in an fsck buffer, of the next
 *                              iag in the aggregate inode table currently
 *                              being traversed.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iag_get_next ( iag_t **addr_iag_ptr )
{
  retcode_t iaggn_rc = FSCK_OK;
  int8 iag_found = 0;
  int8 end_of_imap =0;
  fsblkidx_t ext_blkaddr;
  agg_byteidx_t leaf_byteaddr;
  xad_t    *xadptr;
  agg_recptr->fais.this_iagnum++;
  iaggn_rc = iags_flush();
  if( iaggn_rc == FSCK_OK ) {  /* flushed the iags ok */
    while( (!iag_found) && (!end_of_imap) && (iaggn_rc == FSCK_OK) ) {
      agg_recptr->fais.iagidx_now++;
      if( agg_recptr->fais.iagidx_now <= agg_recptr->fais.iagidx_max ) {  /*
                               * the imap leaf is in the buffer already
                               */
        xadptr = &(
             agg_recptr->fais.this_mapleaf->xad[agg_recptr->fais.iagidx_now]
                   );
        agg_recptr->iag_agg_offset = sb_ptr->s_bsize * addressXAD(xadptr);
        agg_recptr->iag_buf_1st_inode = agg_recptr->fais.this_inoidx;
        iaggn_rc = readwrite_device( agg_recptr->iag_agg_offset,
                                     agg_recptr->iag_buf_length,
                                     &(agg_recptr->iag_buf_data_len),
                                     (void *) (agg_recptr->iag_buf_ptr),
                                     fsck_READ );
        if( iaggn_rc == FSCK_OK ) {  /* got the iag */
          agg_recptr->fais.iagptr = (iag_t *) (agg_recptr->iag_buf_ptr);
          agg_recptr->fais.extidx_now = 0;
          agg_recptr->fais.this_inoidx = agg_recptr->fais.this_iagnum *
                                         NUM_INODE_PER_IAG;
          agg_recptr->iag_buf_1st_inode = agg_recptr->fais.this_inoidx;
          agg_recptr->fais.extidx_max = EXTSPERIAG - 1;
          iag_found = -1;
          }  /* end got the iag */
        else {
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
          sprintf( message_parm_0, "%d", FSCK_FAILED_BADREAD1_IAG );
          msgprms[0] = message_parm_0;
          msgprmidx[0] = 0;
          sprintf( message_parm_1, "%d", iaggn_rc );
          msgprms[1] = message_parm_1;
          msgprmidx[1] = 0;
          sprintf( message_parm_2, "%d", fsck_READ );
          msgprms[2] = message_parm_2;
          msgprmidx[2] = 0;
          sprintf( message_parm_3, "%lld", agg_recptr->iag_agg_offset );
          msgprms[3] = message_parm_3;
          msgprmidx[3] = 0;
          sprintf( message_parm_4, "%ld", agg_recptr->iag_buf_length );
          msgprms[4] = message_parm_4;
          msgprmidx[4] = 0;
          sprintf( message_parm_5, "%ld", agg_recptr->iag_buf_data_len );
          msgprms[5] = message_parm_5;
          msgprmidx[5] = 0;
          fsck_send_msg( fsck_ERRONAGG, 0, 6 );
          iaggn_rc = FSCK_FAILED_BADREAD1_IAG; 
          }
        }  /* end the imap leaf is in the buffer already */
      else {  /* we need to get the next imap leaf (if any) */
        if( agg_recptr->fais.rootleaf_imap ) {  /*
                               * there aren't any more imap leafs
                               */
          end_of_imap = -1;
          }  /* end there aren't any more imap leafs */
        else if( agg_recptr->fais.this_mapleaf->header.next == ((int64)0)) { /*
                               * there aren't any more imap leafs
                               */
          end_of_imap = -1;
          }  /* end there aren't any more imap leafs */
        else {  /* there is another leaf */
          leaf_byteaddr = sb_ptr->s_bsize *
                          agg_recptr->fais.this_mapleaf->header.next;
          iaggn_rc = imapleaf_get( leaf_byteaddr,
                                   &(agg_recptr->fais.this_mapleaf) );
          if( iaggn_rc == FSCK_OK ) {  /* got the imap leaf */
            agg_recptr->fais.iagidx_now = XTENTRYSTART - 1;
            agg_recptr->fais.iagidx_max =
                     agg_recptr->fais.this_mapleaf->header.nextindex - 1;
            }  /* end got the imap leaf */
          }  /* end else there is another leaf */
        }  /* end else we need to get the next imap leaf (if any) */
      }  /* end while !iag_found and !end_of_imap */
    }  /* end flushed the iags ok */
  if( end_of_imap )  {  /* there aren't any more iags */
    agg_recptr->fais.iagptr = NULL;
    agg_recptr->fais.iagidx_now = -1;
    }  /* end there aren't any more iags */
  if( iaggn_rc == FSCK_OK ) {  /* everything worked! */
    *addr_iag_ptr = agg_recptr->fais.iagptr;
    }  /* end everything worked! */
  return( iaggn_rc );
}                              /* end of iag_get_next ()  */
	 
	 
/*****************************************************************************
 * NAME: iag_put
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate,
 *            note, in the fsck workspace, that the current iag buffer
 *            has been modified and should be written to the device in the
 *            next flush operation on this buffer.
 *
 * PARAMETERS:
 *      iagptr  - input -  pointer to the iag, in an fsck buffer, which has 
 *                         been modified.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iag_put ( iag_t  *iagptr )
{
  retcode_t iagp_rc = FSCK_OK;
  if( agg_recptr->processing_readwrite )  {  /* we have write access */
    agg_recptr->iag_buf_write = 1;
    } /* end we have write access */
  return( iagp_rc );
}                              /* end of iag_put ()  */
	 
	 
/*****************************************************************************
 * NAME: iags_flush
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate
 *            and the current iag buffer has been updated since
 *            the most recent read operation, write the buffer contents to
 *            the device.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t iags_flush ( )
{
  retcode_t iagf_rc = FSCK_OK;
  uint32 bytes_written;
  if( agg_recptr->iag_buf_write )  {  /* buffer has been updated since
                               * most recent write
                               */
    iagf_rc = readwrite_device ( agg_recptr->iag_agg_offset,
                                 agg_recptr->iag_buf_data_len,
                                 &bytes_written,
                                 (void *) agg_recptr->iag_buf_ptr,
                                 fsck_WRITE
                               );
    if( iagf_rc == FSCK_OK ) {
      if( bytes_written == agg_recptr->iag_buf_data_len )  {
        agg_recptr->iag_buf_write = 0; /* buffer has been written to
                               * the device and won't need to be
                               * written again unless/until the
                               * buffer contents have been altered again.
                               */
        }
      else {  /* didn't write the correct number of bytes */
           /* 
            * message to user
            */
  DBG_ERROR(("CHK:iags_flush %d written != agg_recptr->iag_buf_data_len\n",bytes_written));
        msgprms[0] = message_parm_0;
        msgprmidx[0] = fsck_metadata;
        msgprms[1] = Vol_Label;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", 10 );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        fsck_send_msg( fsck_URCVWRT, 0, 3 );
             /*
              * message to debugger
              */
        sprintf( message_parm_0, "%d", FSCK_FAILED_IAG_BADFLUSH );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", iagf_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_WRITE );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", agg_recptr->iag_agg_offset );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", agg_recptr->iag_buf_data_len );
        msgprms[4] = message_parm_4;
        msgprmidx[4] = 0;
        sprintf( message_parm_5, "%ld", bytes_written );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONAGG, 0, 6 );
        iagf_rc = FSCK_FAILED_IAG_BADFLUSH;
        }  /* end else didn't write the correct number of bytes */
      }
    else {  /* else the write was not successful */
           /* 
            * message to user
            */
  DBG_ERROR(("CHK:iags_flush %d written not success \n",bytes_written));
      msgprms[0] = message_parm_0;
      msgprmidx[0] = fsck_metadata;
      msgprms[1] = Vol_Label;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", 11 );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_URCVWRT, 0, 3 );
             /*
              * message to debugger
              */
      sprintf( message_parm_0, "%d", FSCK_FAILED_IAG_FLUSH );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", iagf_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_WRITE );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", agg_recptr->iag_agg_offset );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", agg_recptr->iag_buf_data_len );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", bytes_written );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONAGG, 0, 6 );
      iagf_rc = FSCK_FAILED_IAG_FLUSH;
      }  /* end else the write was not successful */
    }  /* end buffer has been updated since most recent write */
  return( iagf_rc );
}                              /* end of iags_flush ()  */
	 
	 
/*****************************************************************************
 * NAME: imapleaf_get
 *
 * FUNCTION: Read the specified inode map leaf node into and/or locate the 
 *           locate the specified inode map leaf node in the
 *           fsck inode map leaf buffer.
 *
 * PARAMETERS:
 *      leaf_start_byte  - input - offset, in bytes, in the aggregate, of 
 *                                 the needed map leaf node.
 *      addr_leaf_ptr    - input - pointer to a variable in which to return
 *                                 the address of the map leaf in an fsck
 *                                 buffer.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t imapleaf_get ( agg_byteidx_t   leaf_start_byte,
                         xtpage_t      **addr_leaf_ptr
                        )
{
  retcode_t imlfg_rc = FSCK_OK;
  agg_byteidx_t leaf_end_byte;
  leaf_end_byte = leaf_start_byte + sizeof(xtpage_t) - 1;
  if( (leaf_start_byte >= agg_recptr->mapleaf_agg_offset) &&
      (leaf_end_byte <= (agg_recptr->mapleaf_agg_offset +
                         agg_recptr->mapleaf_buf_data_len))  )  {  /*
                              * the target leaf is already in
                              * the buffer
                              */
    *addr_leaf_ptr = (xtpage_t *) (agg_recptr->mapleaf_buf_ptr +
                                   leaf_start_byte -
                                   agg_recptr->mapleaf_agg_offset);
    }  /* end the target node is already in the buffer */
  else {  /* else we'll have to read it from the disk */
    agg_recptr->mapleaf_agg_offset = leaf_start_byte;
    imlfg_rc = readwrite_device( agg_recptr->mapleaf_agg_offset,
                                 agg_recptr->mapleaf_buf_length,
                                 &(agg_recptr->mapleaf_buf_data_len),
                                 (void *) agg_recptr->mapleaf_buf_ptr,
                                 fsck_READ
                                );
    if( imlfg_rc == FSCK_OK ) {  /* read appears successful */
      if( agg_recptr->mapleaf_buf_data_len >= (sizeof(xtpage_t)) ) {  /*
                              * we may not have gotten all we asked for,
                              * but we got enough to cover the node we
                              * were after
                              */
        *addr_leaf_ptr = (xtpage_t *) agg_recptr->mapleaf_buf_ptr;
        }  /* end we may not have gotten all we asked for, ... */
      else {  /* didn't get the minimum number of bytes */
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
        sprintf( message_parm_0, "%d", FSCK_FAILED_BADREAD_IMPLF );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", imlfg_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_READ );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", agg_recptr->node_agg_offset );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", agg_recptr->node_buf_length );
        msgprms[4] = message_parm_4;
        msgprmidx[4] = 0;
        sprintf( message_parm_5, "%ld", agg_recptr->node_buf_data_len );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONAGG, 0, 6 );
        imlfg_rc = FSCK_FAILED_BADREAD_IMPLF;
        }  /* end else didn't get the minimum number of bytes */
      }  /* end read appears successful */
    else {  /* bad return code from read */
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
      sprintf( message_parm_0, "%d", FSCK_FAILED_READ_IMPLF );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", imlfg_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_READ );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", agg_recptr->node_agg_offset );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", agg_recptr->node_buf_length );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", agg_recptr->node_buf_data_len );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONAGG, 0, 6 );
      imlfg_rc = FSCK_FAILED_READ_IMPLF;
      }  /* end else bad return code from read */
    }  /* end else we'll have to read it from the disk */
  return( imlfg_rc );
}                              /* end of imapleaf_get ()  */
		
		
/*****************************************************************************
 * NAME: inode_get
 *
 * FUNCTION: Read the specified inode into and/or located the specified inode
 *           in the fsck inode buffer.
 *
 * PARAMETERS:
 *      is_aggregate  - input -  0 => the inode is owned by the fileset
 *                              !0 => the inode is owned by the aggregate
 *      which_it      - input - ordinal number of the aggregate inode
 *                              representing the inode table to which the
 *                              inode belongs.
 *      ino_idx       - input - ordinal number of the inode to read
 *      addr_ino_ptr  - input - pointer to a variable in which to return
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t inode_get ( int         is_aggregate,
                      int         it_number,
                      inoidx_t    ino_idx,
                      dinode_t  **addr_ino_ptr
                     )
{
  retcode_t inog_rc = FSCK_OK;
  int       which_ait;
  iagno_t   iag_num;
  iag_t    *iag_ptr;
  int32     iag_extidx;
  pxd_t    *iag_pxdptr;
  fsblkidx_t    inoext_fsblk_addr;
  agg_byteidx_t inoext_byte_addr;
  *addr_ino_ptr = NULL;
    /*
     * see if the requested inode is already in the buffer
     */
  if( (is_aggregate && agg_recptr->ino_for_aggregate)             ||
      ( ((!is_aggregate) && (!agg_recptr->ino_for_aggregate)) &&
        (it_number == agg_recptr->ino_fsnum)                    )   )  { /*
                           * buffer contains inodes from the proper table
                           */
    if( (ino_idx >= agg_recptr->ino_buf_1st_ino) &&
        (ino_idx <= (agg_recptr->ino_buf_1st_ino + INOSPEREXT - 1)) ) { /*
                           * the requested inode number is in the
                           * range for the extent in the buffer now
                           */
      *addr_ino_ptr = (dinode_t *)
                        ( (ino_idx - agg_recptr->ino_buf_1st_ino) * DISIZE +
                          (reg_idx_t) agg_recptr->ino_buf_ptr
                        );
      }  /* end the requested inode number is in the range for the ... */
    }  /* end buffer contains inodes from the proper table */
  if( *addr_ino_ptr == NULL )  {  /* it isn't in the buffer */
    inog_rc = inodes_flush();  /* handle any pending deferred writes */
         /*
          * get the iag describing its extent
          */
    if( inog_rc == FSCK_OK ) {  /* flushed cleanly */
      if( is_aggregate ) {
        if( ino_idx < FIRST_FILESET_INO ) {  /* in part 1 of AIT */
          if( agg_recptr->primary_ait_4part1 ) {
            which_ait = fsck_primary;
            }
          else {
            which_ait = fsck_secondary;
            }
          }  /* end in part 1 of AIT */
        else {  /* in part 2 of AIT */
          if( agg_recptr->primary_ait_4part2 ) {
            which_ait = fsck_primary;
            }
          else {
            which_ait = fsck_secondary;
            }
          }
        }  /* end else in part 2 of AIT */
      else { /* fileset inode */
        if( agg_recptr->primary_ait_4part2 ) {
          which_ait = fsck_primary;
          }
        else {
          which_ait = fsck_secondary;
          }
        }  /* end else fileset inode */
      iag_num = INOTOIAG(ino_idx);  /* figure out which IAG describes the
                          * extent containing the requested inode
                          */
      inog_rc = iag_get(is_aggregate, it_number, which_ait, iag_num, &iag_ptr);
      if( inog_rc == FSCK_OK ) {  /* got the iag */
        iag_extidx = (ino_idx-agg_recptr->iag_buf_1st_inode) >> L2INOSPEREXT;/*
                          * this is the index into the ixpxd array (in the
                          * iag) for the entry describing the extent with
                          * the desired inode.
                          */
        inoext_fsblk_addr = addressPXD(&(iag_ptr->inoext[iag_extidx]));
        if( inoext_fsblk_addr == 0 )  {  /* the extent isn't allocated */
          inog_rc = FSCK_INOEXTNOTALLOC;
          }  /* end the extent isn't allocated */
        else {  /* the inode extent is allocated */
          inoext_byte_addr = inoext_fsblk_addr * sb_ptr->s_bsize;
          inog_rc = readwrite_device( inoext_byte_addr,
                                      agg_recptr->ino_buf_length,
                                      &(agg_recptr->ino_buf_data_len),
                                      (void *) agg_recptr->ino_buf_ptr,
                                      fsck_READ );
          if( inog_rc == FSCK_OK ) {  /* looks like it worked */
            agg_recptr->ino_buf_agg_offset = inoext_byte_addr;
            agg_recptr->ino_buf_1st_ino = (iag_num << L2INOSPERIAG) +
                                          (iag_extidx << L2INOSPEREXT);
            agg_recptr->ino_fsnum = it_number;
            memcpy( (void *) &(agg_recptr->ino_ixpxd),
                    (void *) &(iag_ptr->inoext[iag_extidx]),
                    sizeof(pxd_t) );
            is_aggregate ? (agg_recptr->ino_for_aggregate = -1)
                         : (agg_recptr->ino_for_aggregate = 0);
            agg_recptr->ino_which_it = it_number;
  
            *addr_ino_ptr = (dinode_t *)
                          ( (ino_idx - agg_recptr->ino_buf_1st_ino) * DISIZE +
                            (reg_idx_t) agg_recptr->ino_buf_ptr
                          );
            }  /* end looks like it worked */
          else {  /* bad return code from read */
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
            sprintf( message_parm_0, "%d", FSCK_FAILED_BADREAD_INODE1 );
            msgprms[0] = message_parm_0;
            msgprmidx[0] = 0;
            sprintf( message_parm_1, "%d", inog_rc );
            msgprms[1] = message_parm_1;
            msgprmidx[1] = 0;
            sprintf( message_parm_2, "%d", fsck_READ );
            msgprms[2] = message_parm_2;
            msgprmidx[2] = 0;
            sprintf( message_parm_3, "%lld", inoext_byte_addr );
            msgprms[3] = message_parm_3;
            msgprmidx[3] = 0;
            sprintf( message_parm_4, "%ld", agg_recptr->ino_buf_length );
            msgprms[4] = message_parm_4;
            msgprmidx[4] = 0;
            sprintf( message_parm_5, "%ld", agg_recptr->ino_buf_data_len );
            msgprms[5] = message_parm_5;
            msgprmidx[5] = 0;
            fsck_send_msg( fsck_ERRONAGG, 0, 6 );
            inog_rc = FSCK_FAILED_BADREAD_INODE1;
            }  /* end else bad return code from read */
          }  /* end else the inode extent is allocated */
        }  /* end got the iag */
      }  /* end flushed cleanly */
    }  /* end it isn't in the buffer */
  return( inog_rc );
}                              /* end of inode_get ()  */
		
		
/*****************************************************************************
 * NAME: inode_get_first_fs
 *
 * FUNCTION: Read the first inode in the specified fileset into and/or 
 *           locate the first inode in the specified fileset in the
 *           fsck inode buffer.
 *
 * PARAMETERS:
 *      it_number     - input - ordinal number of the aggregate inode
 *                              representing the inode table to which the
 *                              inode belongs.
 *      ino_idx       - input - pointer to a variable in which to return
 *                              the ordinal number of the first inode in
 *                              use in the aggregate.
 *      addr_ino_ptr  - input - pointer to a variable in which to return
 *                              the address, in an fsck buffer, of the first
 *                              inode now being used in the aggregate.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t inode_get_first_fs ( int          it_number,
                               inoidx_t    *ino_idx,
                               dinode_t   **addr_ino_ptr
                             )
{
  retcode_t igff_rc = FSCK_OK;
  int which_ait;
  int is_aggregate = 0;
  pxd_t    *pxdptr;
  iag_t   *iagptr;
  if( agg_recptr->primary_ait_4part2 ) {
    which_ait = fsck_primary;
    }
  else {
    which_ait = fsck_secondary;
    }
        /*
         * get the first iag of the imap into the iag buffer
         */
  igff_rc = iag_get_first( is_aggregate, it_number, which_ait, &iagptr);
  if( igff_rc == FSCK_OK ) {  /* first iag is in iag buffer */
       /*
        * get the first inode extent described by the iag into the inode buf
        *
        * note: the very first one must be allocated since it contains
        *       the root directory inode (which has been verified correct)
        */
    igff_rc = inodes_flush();
    if( igff_rc == FSCK_OK ) {  /* flushed ok */
      pxdptr = &(agg_recptr->fais.iagptr->inoext[agg_recptr->fais.extidx_now]);
      agg_recptr->ino_buf_agg_offset = sb_ptr->s_bsize * addressPXD(pxdptr);
      agg_recptr->ino_buf_1st_ino = agg_recptr->fais.this_inoidx;
      agg_recptr->ino_fsnum = it_number;
      agg_recptr->ino_for_aggregate = is_aggregate;
      agg_recptr->ino_which_it = it_number;
      memcpy( (void *) &(agg_recptr->ino_ixpxd),
              (void *) pxdptr,
              sizeof(pxd_t) );
      igff_rc = readwrite_device( agg_recptr->ino_buf_agg_offset,
                                  agg_recptr->ino_buf_length,
                                  &(agg_recptr->ino_buf_data_len),
                                  (void *) (agg_recptr->ino_buf_ptr),
                                  fsck_READ );
      }  /* end flushed ok */
    }  /* end first iag is in iag buffer */
  if( igff_rc == FSCK_OK ) {  /* the first inode extent is in buf */
    agg_recptr->fais.extptr = (dinode_t *) (agg_recptr->ino_buf_ptr);
        /*
         * FILESET_OBJECT_I is the inode number for the first
         * fileset inode not reserved for metadata (excepting the
         * root directory which is handled as a special case)
         */
    agg_recptr->fais.this_inoidx = FILESET_OBJECT_I;
    agg_recptr->fais.inoidx_now = FILESET_OBJECT_I;
    agg_recptr->fais.inoidx_max = INOSPEREXT - 1;
       /*
        * locate the first (regular fileset object) inode in the inode buf
        */
    agg_recptr->fais.inoptr = (dinode_t *)
                                 ( (int32) agg_recptr->fais.extptr +
                                   (agg_recptr->fais.inoidx_now * DISIZE) );
    }  /* end the first inode extent is in buf */
  else {  /* bad return code from read */
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
    sprintf( message_parm_0, "%d", FSCK_FAILED_READ_INODE );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    sprintf( message_parm_1, "%d", igff_rc );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    sprintf( message_parm_2, "%d", fsck_READ );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    sprintf( message_parm_3, "%lld", agg_recptr->ino_buf_agg_offset );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    sprintf( message_parm_4, "%ld", agg_recptr->ino_buf_length );
    msgprms[4] = message_parm_4;
    msgprmidx[4] = 0;
    sprintf( message_parm_5, "%ld", agg_recptr->ino_buf_data_len );
    msgprms[5] = message_parm_5;
    msgprmidx[5] = 0;
    fsck_send_msg( fsck_ERRONAGG, 0, 6 );
    igff_rc = FSCK_FAILED_READ_INODE;
    }  /* end else bad return code from read */
  if( igff_rc == FSCK_OK )  { /* everything worked! */
    *ino_idx = agg_recptr->fais.this_inoidx;
    *addr_ino_ptr = agg_recptr->fais.inoptr;
    }  /* end everything worked! */
  return( igff_rc );
}                              /* end of inode_get_first_fs ()  */
		
		
/*****************************************************************************
 * NAME: inode_get_next
 *
 * FUNCTION: Read the next inode in the specified fileset into and/or 
 *           locate the next inode in the specified fileset in the
 *           fsck inode buffer.
 *
 * PARAMETERS:
 *      ino_idx       - input - pointer to a variable in which to return
 *                              the ordinal number of the next inode in the
 *                              current inode table traversal.
 *      addr_ino_ptr  - input - pointer to a variable in which to return
 *                              the address, in an fsck buffer, of the next
 *                              inode in the current inode table traversal.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t inode_get_next ( inoidx_t   *ino_idx,
                           dinode_t  **addr_ino_ptr
                          )
{
  retcode_t ign_rc = FSCK_OK;
  int8 ext_found, end_of_fileset;
  fsblkidx_t ext_blkaddr;
  agg_byteidx_t leaf_byteaddr;
  xad_t    *xadptr;
  pxd_t    *pxdptr;
  iag_t    *iagptr;
  agg_recptr->fais.this_inoidx++;   /* increment inode ordinal number. If
                               * this one is allocated then it's the one
                               * we want
                               */
  agg_recptr->fais.inoidx_now++;    /* increment index into current extent */
  if( agg_recptr->fais.inoidx_now <= agg_recptr->fais.inoidx_max ) {  /*
                               * the inode is in the buffer already
                               */
    agg_recptr->fais.inoptr = (dinode_t *)
                             ( (int32) agg_recptr->fais.inoptr + DISIZE );
    }  /* end it's in the buffer already */
  else {  /* we need the next allocated inode extent */
    ign_rc = inodes_flush();
    if( ign_rc == FSCK_OK )  {  /* flushed inodes ok */
      ext_found = 0;
      end_of_fileset = 0;
      agg_recptr->fais.extidx_now++;  /* increment index into current iag */
      while( (!ext_found) &&
             (!end_of_fileset) &&
             (ign_rc == FSCK_OK) ) {
        if( agg_recptr->fais.extidx_now <= agg_recptr->fais.extidx_max ) { /*
                               * the iag is in the buffer already
                               */
          pxdptr =
              &(agg_recptr->fais.iagptr->inoext[agg_recptr->fais.extidx_now]);
          ext_blkaddr = addressPXD(pxdptr);
          if( ext_blkaddr == 0 )  {  /* this extent isn't allocated */
            agg_recptr->fais.extidx_now++;
            agg_recptr->fais.this_inoidx += INOSPEREXT;
            }  /* end this extent isn't allocated */
          else {  /* this extent is allocated */
            agg_recptr->ino_buf_agg_offset = sb_ptr->s_bsize * ext_blkaddr;
            agg_recptr->ino_buf_1st_ino = agg_recptr->fais.this_inoidx;
            memcpy( (void *) &(agg_recptr->ino_ixpxd),
                    (void *) pxdptr,
                    sizeof(pxd_t) );
            ign_rc = readwrite_device( agg_recptr->ino_buf_agg_offset,
                                       agg_recptr->ino_buf_length,
                                       &(agg_recptr->ino_buf_data_len),
                                       (void *) (agg_recptr->ino_buf_ptr),
                                       fsck_READ );
            if( ign_rc == FSCK_OK )  {  /* got the extent */
              ext_found = -1;
              agg_recptr->fais.extptr = (dinode_t *) (agg_recptr->ino_buf_ptr);
              agg_recptr->fais.inoidx_now = 0;
              agg_recptr->fais.inoidx_max = INOSPEREXT - 1;
              ext_found = -1;
              agg_recptr->fais.inoptr = (dinode_t *) (agg_recptr->ino_buf_ptr);
              }  /* end got the extent */
            else {  /* bad return code from read */
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
              sprintf( message_parm_0, "%d", FSCK_FAILED_BADREAD_INODE );
              msgprms[0] = message_parm_0;
              msgprmidx[0] = 0;
              sprintf( message_parm_1, "%d", ign_rc );
              msgprms[1] = message_parm_1;
              msgprmidx[1] = 0;
              sprintf( message_parm_2, "%d", fsck_READ );
              msgprms[2] = message_parm_2;
              msgprmidx[2] = 0;
              sprintf( message_parm_3, "%lld", agg_recptr->ino_buf_agg_offset );
              msgprms[3] = message_parm_3;
              msgprmidx[3] = 0;
              sprintf( message_parm_4, "%ld", agg_recptr->ino_buf_length );
              msgprms[4] = message_parm_4;
              msgprmidx[4] = 0;
              sprintf( message_parm_5, "%ld", agg_recptr->ino_buf_data_len );
              msgprms[5] = message_parm_5;
              msgprmidx[5] = 0;
              fsck_send_msg( fsck_ERRONAGG, 0, 6 );
              ign_rc = FSCK_FAILED_BADREAD_INODE;
              }  /* end else bad return code from read */
            }  /* end else this extent is allocated */
          }  /* end the iag is in the buffer already */
        else { /* we need to get the next iag (if any) */
          ign_rc = iag_get_next( &iagptr );
          if( (ign_rc == FSCK_OK) && (iagptr == NULL) ) {
            end_of_fileset = -1;
            }
          }  /* end else we need to get the next iag (if any) */
        }  /* end while !ext_found and !end_of_fileset */
      if( end_of_fileset )  {  /* there aren't any more extents */
        agg_recptr->fais.inoptr = NULL;
        agg_recptr->fais.this_inoidx = -1;
        }  /* end there aren't any more extents */
      }  /* end flushed inodes ok */
    }  /* end we need the next allocated inode extent */
  if( ign_rc == FSCK_OK ) {  /* everything worked! */
    *ino_idx = agg_recptr->fais.this_inoidx;
    *addr_ino_ptr = agg_recptr->fais.inoptr;
    }  /* end everything worked! */
  return( ign_rc );
}                              /* end of inode_get_next ()  */
		
		
/*****************************************************************************
 * NAME: inode_put
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate,
 *            note, in the fsck workspace, that the current inode buffer
 *            has been modified and should be written to the device in the
 *            next flush operation on this buffer.
 *
 * PARAMETERS:
 *      ino_ptr  - input - address, in an fsck buffer, of the inode which has
 *                         been updated.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t inode_put ( dinode_t   *ino_ptr )
{
  retcode_t ip_rc = FSCK_OK;
  if( agg_recptr->processing_readwrite )  {
    agg_recptr->ino_buf_write = -1;  /* buffer has been updated and needs
                                    * to be written to the device
                                    */
    }
  return( ip_rc );
}                              /* end of inode_put ()  */
	 
	 
/*****************************************************************************
 * NAME: inodes_flush
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate
 *            and the current inode buffer has been updated since the most
 *            recent read operation, write the buffer contents to the device.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t inodes_flush ( )
{
  retcode_t inof_rc = FSCK_OK;
  uint32 bytes_written;
  if( agg_recptr->ino_buf_write )  {  /* buffer has been updated since
                               * most recent write
                               */
    inof_rc = readwrite_device( agg_recptr->ino_buf_agg_offset,
                                agg_recptr->ino_buf_data_len,
                                &bytes_written,
                                (void *) agg_recptr->ino_buf_ptr,
                                fsck_WRITE );
    if( inof_rc == FSCK_OK ) {
      if( bytes_written == agg_recptr->ino_buf_data_len ) {
        agg_recptr->ino_buf_write = 0; /* buffer has been written
                               * to the device and won't need to be
                               * written again unless/until the
                               * buffer contents have been altered.
                               */
        }
      else {  /* didn't write the correct number of bytes */
           /* 
            * message to user
            */
  DBG_ERROR(("CHK:inodes_flush %d written != agg_recptr->ino_buf_data_len\n",bytes_written));
        msgprms[0] = message_parm_0;
        msgprmidx[0] = fsck_metadata;
        msgprms[1] = Vol_Label;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", 12 );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        fsck_send_msg( fsck_URCVWRT, 0, 3 );
             /*
              * message to debugger
              */
        sprintf( message_parm_0, "%d", FSCK_FAILED_INODE_BADFLUSH );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", inof_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_WRITE );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", agg_recptr->ino_buf_agg_offset );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", agg_recptr->ino_buf_data_len );
        msgprms[4] = message_parm_4;
        msgprmidx[4] = 0;
        sprintf( message_parm_5, "%ld", bytes_written );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONAGG, 0, 6 );
        inof_rc = FSCK_FAILED_INODE_BADFLUSH;
        }  /* end else didn't write the correct number of bytes */
      }
    else {  /* else the write was not successful */
           /* 
            * message to user
            */
  DBG_ERROR(("CHK:inodes_flush %d written not success \n",bytes_written));
      msgprms[0] = message_parm_0;
      msgprmidx[0] = fsck_metadata;
      msgprms[1] = Vol_Label;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", 14 );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_URCVWRT, 0, 3 );
             /*
              * message to debugger
              */
      sprintf( message_parm_0, "%d", FSCK_FAILED_INODE_FLUSH );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", inof_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_WRITE );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", agg_recptr->ino_buf_agg_offset );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", agg_recptr->ino_buf_data_len );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", bytes_written );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONAGG, 0, 6 );
      inof_rc = FSCK_FAILED_INODE_FLUSH;
      }  /* end else the write was not successful */
    }  /* end buffer has been updated since most recent write */
  return( inof_rc );
}                              /* end of inodes_flush ()  */
	 
	 
/*****************************************************************************
 * NAME: inotbl_get_ctl_page
 *
 * FUNCTION: Read the control page for the specified inode table into and/or 
 *           locate the control page for the specified inode table in the
 *           fsck inode table control page buffer.
 *
 * PARAMETERS:
 *      is_aggregate      - input -  0 => the inode table is fileset owned
 *                                  !0 => the inode table is aggregate owned
 *      addr_ctlpage_ptr  - input - pointer to a variable in which to return
 *                                  the address of the control page in an
 *                                  fsck buffer
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t inotbl_get_ctl_page ( int         is_aggregate,
                                dinomap_t **addr_ctlpage_ptr
                               )
{
  retcode_t igcp_rc = FSCK_OK;
  retcode_t intermed_rc = FSCK_OK;
  reg_idx_t bytes_read;
  fsblkidx_t offset_1stext;
  fsblkidx_t offset_ctlpage;
  int        which_it;
  int        which_ait;
  dinode_t  *inoptr;
  xtpage_t  *xtpg_ptr;
  fsblkidx_t leaf_offset;
    /*
     * get the byte offset of the control page
     */
  if( is_aggregate )  {  /* aggregate inode table wanted */
    if( agg_recptr->primary_ait_4part1 ) {
      offset_ctlpage = AIMAP_OFF / sb_ptr->s_bsize;
      }
    else {
      offset_ctlpage = addressPXD(&(sb_ptr->s_aim2));
      }
    }  /* end aggregate inode table wanted */
  else {  /* fileset inode table wanted */
    which_it = FILESYSTEM_I;
    if( agg_recptr->primary_ait_4part2 ) {
      which_ait = fsck_primary;
      }
    else {
      which_ait = fsck_secondary;
      }
    if( agg_recptr->fset_imap.imap_is_rootleaf ) { /* root leaf inode */
      intermed_rc = ait_special_read_ext1( which_ait );
      if( intermed_rc != FSCK_OK ) {
        report_readait_error( intermed_rc, 
                              FSCK_FAILED_AGFS_READ4, 
                              which_ait );
        igcp_rc = FSCK_FAILED_AGFS_READ4;
        }
      else {  /* got the agg inode extent */
        inoptr = (dinode_t *)
           ((uint32) (agg_recptr->ino_buf_ptr) + which_it * sizeof(dinode_t));
        xtpg_ptr = (xtpage_t *) &(inoptr->di_btroot);
        offset_ctlpage = addressXAD(&(xtpg_ptr->xad[XTENTRYSTART]));
        }  /* end else got the agg inode extent */
      }  /* end root leaf inode */
    else {  /* tree doesn't have a root-leaf */
      intermed_rc =
           imapleaf_get( agg_recptr->fset_imap.first_leaf_offset, &xtpg_ptr );
      if( intermed_rc != FSCK_OK ) {
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
        sprintf( message_parm_0, "%d", FSCK_FAILED_READ_NODE3 );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", igcp_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_READ );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", agg_recptr->ino_buf_agg_offset );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", agg_recptr->ino_buf_length );
        msgprms[4] = message_parm_4;
        msgprmidx[4] = 0;
        sprintf( message_parm_5, "%ld", agg_recptr->ino_buf_data_len );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONAGG, 0, 6 );
        igcp_rc = FSCK_FAILED_READ_NODE3;
        }
      else {  /* got the first leaf node */
        offset_ctlpage = addressXAD(&(xtpg_ptr->xad[XTENTRYSTART]));
        }  /* end else got the first leaf node */
      }  /* end else tree doesn't have a root-leaf */
    }  /* end else fileset inode table wanted */
    /*
     * read the control page into the buffer
     */
  if( igcp_rc == FSCK_OK ) {  /* we have an offset */
    igcp_rc = mapctl_get( offset_ctlpage, (void **) &xtpg_ptr );
    if( igcp_rc != FSCK_OK )  {  /* this is fatal */
      igcp_rc = FSCK_FAILED_CANTREADAITCTL;
      }  /* end this is fatal */
    else {  /* the control page is in the buffer */
      *addr_ctlpage_ptr = (dinomap_t *) xtpg_ptr;
      }  /* end else the control page is in the buffer */
    }  /* end we have an offset */
  return( igcp_rc );
}                    /* end of inotbl_get_ctl_page ()       */
	 
	 
/*****************************************************************************
 * NAME: inotbl_put_ctl_page
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate,
 *            note, in the fsck workspace, that the current fsck inode table
 *            control page buffer has been modified and should be written 
 *            to the device in the next flush operation on this buffer.
 *
 * PARAMETERS:
 *      is_aggregate  - input -  0 => the inode table is fileset owned
 *                              !0 => the inode table is aggregate owned
 *      ctlpage_ptr   - input - the address, in an fsck buffer, of the
 *                               control page which has been updated
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t inotbl_put_ctl_page ( int         is_aggregate,
                                dinomap_t  *ctlpage_ptr
                               )
{
  retcode_t ipcp_rc = FSCK_OK;
  if( agg_recptr->processing_readwrite )  {
    ipcp_rc = mapctl_put( (void *) ctlpage_ptr );
    }
  return( ipcp_rc );
}                    /* end of inotbl_put_ctl_page ()       */
	 
	 
/*****************************************************************************
 * NAME: mapctl_get
 *
 * FUNCTION: Read the specified map control page into and/or locate the 
 *           requested specified map control page in the fsck map control
 *           page buffer.
 *
 * PARAMETERS:
 *      mapctl_fsblk_offset  - input - offset, in aggregate blocks, into the
 *                                     aggregate, of the needed map control
 *                                     page.
 *      addr_mapctl_ptr      - input - pointer to a variable in which to return
 *                                     the address, in an fsck buffer, of the
 *                                     map control page which has been read.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t mapctl_get ( fsblkidx_t   mapctl_fsblk_offset,
                       void       **addr_mapctl_ptr
                     )
{
  retcode_t mg_rc = FSCK_OK;
  agg_byteidx_t start_byte, end_byte;
  start_byte = mapctl_fsblk_offset * sb_ptr->s_bsize;
  end_byte = start_byte + BYTESPERPAGE - 1;
  if( start_byte == agg_recptr->mapctl_agg_offset )  {  /*
                              * the target control page is already in
                              * the buffer
                              */
    *addr_mapctl_ptr = (void *) (agg_recptr->mapctl_buf_ptr);
    }  /* end the target page is already in the buffer */
  else {  /* else we'll have to read it from the disk */
    mg_rc = mapctl_flush();  /* handle any pending deferred writes */
    if( mg_rc == FSCK_OK )  { /* flushed ok */
      agg_recptr->mapctl_agg_offset = start_byte;
      mg_rc = readwrite_device( agg_recptr->mapctl_agg_offset,
                                agg_recptr->mapctl_buf_length,
                                &(agg_recptr->mapctl_buf_data_len),
                                (void *) agg_recptr->mapctl_buf_ptr,
                                fsck_READ );
      if( mg_rc == FSCK_OK ) {  /* read appears successful */
        if( agg_recptr->mapctl_buf_data_len >= BYTESPERPAGE ) {  /*
                              * we may not have gotten all we asked for,
                              * but we got enough to cover the page we
                              * were after
                              */
          *addr_mapctl_ptr = (void *) agg_recptr->mapctl_buf_ptr;
          }  /* end we may not have gotten all we asked for, ... */
        else {  /* didn't get the minimum number of bytes */
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
          sprintf( message_parm_0, "%d", FSCK_FAILED_BADREAD_MAPCTL );
          msgprms[0] = message_parm_0;
          msgprmidx[0] = 0;
          sprintf( message_parm_1, "%d", mg_rc );
          msgprms[1] = message_parm_1;
          msgprmidx[1] = 0;
          sprintf( message_parm_2, "%d", fsck_READ );
          msgprms[2] = message_parm_2;
          msgprmidx[2] = 0;
          sprintf( message_parm_3, "%lld", agg_recptr->mapctl_agg_offset );
          msgprms[3] = message_parm_3;
          msgprmidx[3] = 0;
          sprintf( message_parm_4, "%ld", agg_recptr->mapctl_buf_length );
          msgprms[4] = message_parm_4;
          msgprmidx[4] = 0;
          sprintf( message_parm_5, "%ld", agg_recptr->mapctl_buf_data_len );
          msgprms[5] = message_parm_5;
          msgprmidx[5] = 0;
          fsck_send_msg( fsck_ERRONAGG, 0, 6 );
          mg_rc = FSCK_FAILED_BADREAD_MAPCTL;
          }  /* end else didn't get the minimum number of bytes */
        }  /* end read appears successful */
      else {  /* bad return code from read */
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
        sprintf( message_parm_0, "%d", FSCK_FAILED_READ_MAPCTL );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", mg_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_READ );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", agg_recptr->mapctl_agg_offset );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", agg_recptr->mapctl_buf_length );
        msgprms[4] = message_parm_4;
        msgprmidx[4] = 0;
        sprintf( message_parm_5, "%ld", agg_recptr->mapctl_buf_data_len );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONAGG, 0, 6 );
        mg_rc = FSCK_FAILED_READ_MAPCTL;
        }  /* end else bad return code from read */
      }  /* end flushed ok */
    }  /* end else we'll have to read it from the disk */
  return( mg_rc );
}                              /* end of mapctl_get ()  */
	 
	 
/*****************************************************************************
 * NAME: mapctl_put
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate,
 *            note, in the fsck workspace, that the current fsck map control
 *            page buffer has been modified and should be written to the 
 *            device in the next flush operation on this buffer.
 *
 * PARAMETERS:
 *      mapctl_ptr  - input - address, in an fsck buffer, of the map control
 *                            page which has been updated.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t mapctl_put ( void  *mapctl_ptr )
{
  retcode_t mp_rc = FSCK_OK;
  if( agg_recptr->processing_readwrite )  {
    agg_recptr->mapctl_buf_write = -1; /* buffer has been updated and needs
                                      * to be written to the device
                                      */
    }
  return( mp_rc );
}                              /* end of mapctl_put ()  */
	 
	 
/*****************************************************************************
 * NAME: mapctl_flush
 *
 * FUNCTION:  If the current fsck session has write access to the aggregate
 *            and the current map control buffer has been updated since
 *            the most recent read operation, write the buffer contents to
 *            the device.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t mapctl_flush ( )
{
  retcode_t mf_rc = FSCK_OK;
  uint32 bytes_written;
  if( agg_recptr->mapctl_buf_write )  {  /* buffer has been updated since
                               * most recent write
                               */
    mf_rc = readwrite_device( agg_recptr->mapctl_agg_offset,
                              agg_recptr->mapctl_buf_data_len,
                              &bytes_written,
                              (void *) agg_recptr->mapctl_buf_ptr,
                              fsck_WRITE );
    if( mf_rc == FSCK_OK ) {
      if( bytes_written == agg_recptr->mapctl_buf_length )  {
        agg_recptr->mapctl_buf_write = 0; /* buffer has been written to the
                               * device and won't need to be written again
                               * unless/until it the buffer contents have
                               * been altered.
                               */
        }
      else {  /* didn't write the correct number of bytes */
           /* 
            * message to user
            */
  DBG_ERROR(("CHK:mapctl_flush %d written agg_recptr->mapctl_buf_length\n",bytes_written));
        msgprms[0] = message_parm_0;
        msgprmidx[0] = fsck_metadata;
        msgprms[1] = Vol_Label;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", 15 );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        fsck_send_msg( fsck_URCVWRT, 0, 3 );
             /*
              * message to debugger
              */
        sprintf( message_parm_0, "%d", FSCK_FAILED_MAPCTL_BADFLUSH );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", mf_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_WRITE );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", agg_recptr->mapctl_agg_offset );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", agg_recptr->mapctl_buf_data_len );
        msgprms[4] = message_parm_4;
        msgprmidx[4] = 0;
        sprintf( message_parm_5, "%ld", bytes_written );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONAGG, 0, 6 );
        mf_rc = FSCK_FAILED_MAPCTL_BADFLUSH;
        }  /* end else didn't write the correct number of bytes */
      }
    else {  /* else the write was not successful */
      agg_recptr->ag_dirty;   /* the only thing we know for sure at this
                               * point is that fsck wanted to write
                               * (correct) some storage and was unable
                               * to do so.
                               */
           /* 
            * message to user
            */
  DBG_ERROR(("CHK:mapctl_flush %d written not success \n",bytes_written));
      msgprms[0] = message_parm_0;
      msgprmidx[0] = fsck_metadata;
      msgprms[1] = Vol_Label;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", 16 );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      fsck_send_msg( fsck_URCVWRT, 0, 3 );
             /*
              * message to debugger
              */
      sprintf( message_parm_0, "%d", FSCK_FAILED_MAPCTL_FLUSH );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", mf_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_WRITE );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", agg_recptr->mapctl_agg_offset );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", agg_recptr->mapctl_buf_data_len );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", bytes_written );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONAGG, 0, 6 );
      mf_rc = FSCK_FAILED_MAPCTL_FLUSH ;
      }  /* end else the write was not successful */
    }  /* end buffer has been updated since most recent write */
  return( mf_rc );
}                              /* end of mapctl_flush ()  */
	
	
/*****************************************************************************
 * NAME: node_get
 *
 * FUNCTION: Read the specified xTree node into and/or locate the specified
 *           xTree node in the fsck xTree node buffer.
 *
 * PARAMETERS:
 *      node_fsblk_offset  - input - offset, in aggregate blocks, into the
 *                                   aggregate, of the xTree node wanted
 *      addr_xtpage_ptr    - input - pointer to a variable in which to return
 *                                   the address, in an fsck buffer, of the
 *                                   of the xTree node which has been read.
 * 
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t node_get ( fsblkidx_t   node_fsblk_offset,
                     xtpage_t  **addr_xtpage_ptr
                   )
{
  retcode_t nodg_rc = FSCK_OK;
  agg_byteidx_t node_start_byte, node_end_byte;
		
  node_start_byte = node_fsblk_offset * sb_ptr->s_bsize;
  node_end_byte = node_start_byte + sizeof(xtpage_t) - 1;
		
  if( (agg_recptr->ondev_wsp_fsblk_offset != 0) &&
      (node_fsblk_offset > agg_recptr->ondev_wsp_fsblk_offset) ) { /*
			      * the offset is beyond the range 
			      * valid for fileset objects
			      */
	/*
	 * This case is not caused by an I/O error, but by
	 * invalid data in an inode.  Let the caller handle
	 * the consequences.
	 */
    nodg_rc = FSCK_BADREADTARGET1;
    }  /* end the offset is beyond the range valid ... */
		
  else if( (node_start_byte >= agg_recptr->node_agg_offset) &&
           (node_end_byte <= (agg_recptr->node_agg_offset +
                         agg_recptr->node_buf_data_len))  )  {  /*
                              * the target node is already in
                              * the buffer
                              */
    *addr_xtpage_ptr = (xtpage_t *)
                          (agg_recptr->node_buf_ptr +
                           node_start_byte - agg_recptr->node_agg_offset);
    }  /* end the target node is already in the buffer */
  else {  /* else we'll have to read it from the disk */
    agg_recptr->node_agg_offset = node_start_byte;
    nodg_rc = readwrite_device( agg_recptr->node_agg_offset,
                                agg_recptr->node_buf_length,
                                &(agg_recptr->node_buf_data_len),
                                (void *) agg_recptr->node_buf_ptr,
                                fsck_READ );
    if( nodg_rc == FSCK_OK ) {  /* read appears successful */
      if( agg_recptr->node_buf_data_len >= (sizeof(xtpage_t)) ) {  /*
                              * we may not have gotten all we asked for,
                              * but we got enough to cover the node we
                              * were after
                              */
        *addr_xtpage_ptr = (xtpage_t *) agg_recptr->node_buf_ptr;
        }  /* end we may not have gotten all we asked for, ... */
      else {  /* didn't get the minimum number of bytes */
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
        sprintf( message_parm_0, "%d", FSCK_FAILED_BADREAD_NODE );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        sprintf( message_parm_1, "%d", nodg_rc );
        msgprms[1] = message_parm_1;
        msgprmidx[1] = 0;
        sprintf( message_parm_2, "%d", fsck_READ );
        msgprms[2] = message_parm_2;
        msgprmidx[2] = 0;
        sprintf( message_parm_3, "%lld", agg_recptr->node_agg_offset );
        msgprms[3] = message_parm_3;
        msgprmidx[3] = 0;
        sprintf( message_parm_4, "%ld", agg_recptr->node_buf_length );
        msgprms[4] = message_parm_4;
        msgprmidx[4] = 0;
        sprintf( message_parm_5, "%ld", agg_recptr->node_buf_data_len );
        msgprms[5] = message_parm_5;
        msgprmidx[5] = 0;
        fsck_send_msg( fsck_ERRONAGG, 0, 6 );
        nodg_rc = FSCK_FAILED_BADREAD_NODE;
        }  /* end else didn't get the minimum number of bytes */
      }  /* end read appears successful */
    else {  /* bad return code from read */
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
      sprintf( message_parm_0, "%d", FSCK_FAILED_READ_NODE );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", nodg_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_READ );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", agg_recptr->node_agg_offset );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", agg_recptr->node_buf_length );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", agg_recptr->node_buf_data_len );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONAGG, 0, 6 );
      nodg_rc = FSCK_FAILED_READ_NODE;
      }  /* end else bad return code from read */
    }  /* end else we'll have to read it from the disk */
  return( nodg_rc );
}                              /* end of node_get ()  */
	
	
/*****************************************************************************
 * NAME: open_device_read
 *
 * FUNCTION:  Open the specified device for read access.
 *
 * PARAMETERS:
 *      Device  - input - the device specification
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t open_device_read ( char    *Device )
{
  int32    synch_rc = 0, openrd_rc = 0;
  int32    close_rc = 0;
  ULONG    Action;
  int      i;
  unsigned char CommandInfo = 0;
  unsigned long ParmLen = sizeof(CommandInfo);
  unsigned long DataLen = sizeof(struct DPB);
  USHORT ulSecsPerTrk;
  struct DPB *tmp_dpb = &Datap;
  TRACKLAYOUT *tmplayout;
  int32		pList;
  ULONG 	pListLen = 0;
  chkdsk_t	pData;
  ULONG	pDataLen = 0;
	
  openrd_rc = DosOpen(Device, &Dev_IOPort, &Action, 0, 0,
                      OPEN_ACTION_OPEN_IF_EXISTS,
                      OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR |
                      OPEN_SHARE_DENYNONE | OPEN_ACCESS_READONLY, 0);
  sprintf( message_parm_0, "%ld", openrd_rc );
  msgprms[0] = message_parm_0;
  msgprmidx[0] = 0;
  fsck_send_msg( fsck_DEVOPENRDRC, 0, 1 );  
  if( openrd_rc == NO_ERROR ) {  /* now open for read, allow read/write */
        /*
         * Get the device parameters
         */
    openrd_rc = DosDevIOCtl(Dev_IOPort, IOCTL_DISK,
                            DSK_GETDEVICEPARAMS, &CommandInfo,
                            sizeof(CommandInfo), &ParmLen, &Datap,
                            sizeof(struct DPB), &DataLen);
    if( openrd_rc == NO_ERROR ) {
      Dev_SectorSize = Datap.dev_bpb.bytes_per_sector;
      Dev_blksize = Dev_SectorSize;
      ulSecsPerTrk = Datap.dev_bpb.sectors_per_track;
      ptrklay = (TRACKLAYOUT *)malloc(sizeof(TRACKLAYOUT) + 4 * ulSecsPerTrk );
      if( ptrklay == NULL ) {
        openrd_rc = FSCK_FAILED_DYNSTG_EXHAUST9;
        }
      else {  /* got the dev parms */
        for( i = 0; i < ulSecsPerTrk; i++ ) {
          ptrklay->TrackTable[i].usSectorSize = Dev_SectorSize;
          ptrklay->TrackTable[i].usSectorNumber = i + 1;
          }
	/*
	 * sync fs meta-data 
	 * 
	 * N.B. This is a best-attempt effort.  It doesn't really
	 *	matter much whether it fails.  We log the results for
	 *	for posterity and then forget them.
	 */
        pDataLen = sizeof(chkdsk_t);
        pData.flag = CHKDSK_SYNC;
        if( Device[0] >= 'a' && Device[0] <= 'z' )
          pData.dev = Device[0] - 'a';
        else if( Device[0] >= 'A' && Device[0] <= 'Z' )
          pData.dev = Device[0] - 'A';
        else 
          synch_rc = -1;

        if( synch_rc == 0 ) {
          synch_rc = fscntl(JFSCTL_CHKDSK, 
  			(void *)&pList, &pListLen,
			(void *)&pData, &pDataLen
			);
          sprintf( message_parm_0, "%ld", synch_rc );
          msgprms[0] = message_parm_0;
          msgprmidx[0] = 0;
          fsck_send_msg( fsck_CHKDSKFSSYNCHRC, 0, 1 );  
          }
        }  /* end else got the dev parms */
      }
    }   /* end now open for read, allow read/write */
	
  return( openrd_rc );
}                             /* end of open_device_read ( ) */
	
	
/*************************************************************************** 
 * NAME: open_device_rw_exclusive
 *
 * FUNCTION:  Open the device for read/write access.  Lock the device
 *            to prevent others from reading from or writing to the
 *            device.
 *
 * PARAMETERS:
 *      Device  - input -  The device specification
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t open_device_rw_exclusive ( char    *Device )
{
  retcode_t  openrw_rc = 0;
  retcode_t  close_rc = 0;
  retcode_t redeterm_rc = FSCK_OK;
  ULONG      Action;
  int      i, idx, done;
  unsigned char CommandInfo = 0;
  unsigned long ParmLen = sizeof(CommandInfo);
  unsigned long DataLen = sizeof(struct DPB);
  USHORT ulSecsPerTrk;
  struct DPB *tmp_dpb = &Datap;
  	  
idx = 0;
done = 0;
while( (!done) && (idx < 3) ) {
  openrw_rc = DosOpen(Device, &Dev_IOPort, &Action, 0, 0,
                      OPEN_ACTION_OPEN_IF_EXISTS,
                      OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR |
                      OPEN_SHARE_DENYREADWRITE |         
                      OPEN_ACCESS_READWRITE, 0);
  sprintf( message_parm_0, "%ld", openrw_rc );
  msgprms[0] = message_parm_0;
  msgprmidx[0] = 0;
  fsck_send_msg( fsck_DEVOPENRDWRRC, 0, 1 );  
  if( openrw_rc == 0 ) {
    done = -1;
   }
  else {
    DosSleep(1000);	/* sleep for a second */
    idx++;
    }
  }  /* end while */

  if( openrw_rc == NO_ERROR ) {  /* now open for read/write exclusive */
    if( !agg_recptr->parm_options[UFS_CHKDSK_AUTOCHECK] ) {  /*
                         * not in autocheck mode
                         */
      idx = 0;
      done = 0;
      while( (!done) && (idx < 3) ) {
        /*
         * lock the volume
         */
        openrw_rc = DosDevIOCtl( Dev_IOPort, IOCTL_DISK, DSK_LOCKDRIVE,
                               &CommandInfo, sizeof(CommandInfo),
                               &ParmLen, &CommandInfo, sizeof(CommandInfo),
                               &DataLen);
        sprintf( message_parm_0, "%ld", openrw_rc );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( fsck_DEVLOCKRC, 0, 1 );  
        if( openrw_rc == 0 ) {
          done = -1;
         }
        else {
          DosSleep(1000);	/* sleep for a second */
          idx++;
          }
        }  /* end while */

      if( openrw_rc == NO_ERROR )  {  /* successful volume lock */
        agg_recptr->device_is_locked = 1;
        redeterm_rc = ujfs_beginformat( Dev_IOPort );  /*
                                * unmount to flush the JFS cache.
                                * Don't remount, just prepare for block I/O.
                                */
        sprintf( message_parm_0, "%ld", redeterm_rc );
        msgprms[0] = message_parm_0;
        msgprmidx[0] = 0;
        fsck_send_msg( fsck_DEVBEGINFMTRC, 0, 1 );  /*
                                * log the return code from flushing the
                                * cache but go on no matter what it was.
                                * In general, the only reason we run is
                                * because something is wrong, and the
                                * problems might prevent this flushing
                                * successfully.
                                */
        }  /* end successful volume lock */
      else {  /* unable to lock the volume */
        close_rc = close_volume ( );
        }  /* end unable to lock the volume */
      }  /* end not in autocheck mode */
       
    if( openrw_rc == NO_ERROR ) {  /* 
                         * successful lock and redetermine or not needed 
                         */
          /*
           * Get the device parameters
           */
      openrw_rc = DosDevIOCtl(Dev_IOPort, IOCTL_DISK,
                               DSK_GETDEVICEPARAMS, &CommandInfo,
                               sizeof(CommandInfo), &ParmLen, &Datap,
                               sizeof(struct DPB), &DataLen);
      sprintf( message_parm_0, "%ld", openrw_rc );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      fsck_send_msg( fsck_DEVGETCHARRC, 0, 1 );  
      if( openrw_rc == NO_ERROR ) {  /*  got the dev chars */
        Dev_SectorSize = Datap.dev_bpb.bytes_per_sector;
        Dev_blksize = Dev_SectorSize;
        ulSecsPerTrk = Datap.dev_bpb.sectors_per_track;
        ptrklay = (TRACKLAYOUT *)malloc(sizeof(TRACKLAYOUT) + 
                   4 * ulSecsPerTrk );
        if( ptrklay == NULL ) {
          openrw_rc = FSCK_FAILED_DYNSTG_EXHAUSTA;
          }
        else {  /* got the storage */
          for( i = 0; i < ulSecsPerTrk; i++ ) {
            ptrklay->TrackTable[i].usSectorSize = Dev_SectorSize;
            ptrklay->TrackTable[i].usSectorNumber = i + 1;
            }  /* end for */
          }  /* end else got the storage */
        }  /* end got the dev chars */
      else {    /* can't get the device parameters */
        close_rc = close_volume ( );
        close_rc = DosDevIOCtl( Dev_IOPort, IOCTL_DISK, DSK_UNLOCKDRIVE,
                                  &CommandInfo, sizeof(CommandInfo),
                                  &ParmLen, &CommandInfo,
                                  sizeof(CommandInfo), &DataLen);
        }  /* end else can't get the device parameters */
      }  /* end successful lock and redetermine or not needed */
    }  /* end successful open for read/write exclusive */
  return( openrw_rc );
}                                /* end of open_device_rw_exclusive ( ) */
	
	
/*************************************************************************** 
 * NAME: open_device_rw_shared
 *
 * FUNCTION:  Open the device for read/write access.  
 *
 * PARAMETERS:
 *      Device  - input -  The device specification
 *
 * NOTES:
 *		It is ok to do malloc() here since it's the very first
 *		thing that happens.  These are small allocations, and
 *		will always succeed since they come first.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t open_device_rw_shared ( char    *Device )
{
  retcode_t  openrw_rc = 0;
  retcode_t  close_rc = 0;
  retcode_t redeterm_rc = FSCK_OK;
  ULONG      Action;
  int      i;
  unsigned char CommandInfo = 0;
  unsigned long ParmLen = sizeof(CommandInfo);
  unsigned long DataLen = sizeof(struct DPB);
  USHORT ulSecsPerTrk;
  struct DPB *tmp_dpb = &Datap;
  	  
  openrw_rc = DosOpen(Device, &Dev_IOPort, &Action, 0, 0,
                      OPEN_ACTION_OPEN_IF_EXISTS,
                      OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR |
                      OPEN_SHARE_DENYNONE |         
                      OPEN_ACCESS_READWRITE, 0);
  sprintf( message_parm_0, "%ld", openrw_rc );
  msgprms[0] = message_parm_0;
  msgprmidx[0] = 0;
  fsck_send_msg( fsck_DEVOPENRDWRSRC, 0, 1 );  
       
  if( openrw_rc == NO_ERROR ) {  /*  it's open */ 
          /*
           * Get the device parameters
           */
    openrw_rc = DosDevIOCtl(Dev_IOPort, IOCTL_DISK,
                               DSK_GETDEVICEPARAMS, &CommandInfo,
                               sizeof(CommandInfo), &ParmLen, &Datap,
                               sizeof(struct DPB), &DataLen);
    sprintf( message_parm_0, "%ld", openrw_rc );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    fsck_send_msg( fsck_DEVGETCHARRC, 0, 1 );  
    if( openrw_rc == NO_ERROR ) {  /*  got the dev chars */
      Dev_SectorSize = Datap.dev_bpb.bytes_per_sector;
      Dev_blksize = Dev_SectorSize;
      ulSecsPerTrk = Datap.dev_bpb.sectors_per_track;
      ptrklay = (TRACKLAYOUT *)malloc(sizeof(TRACKLAYOUT) + 
                   4 * ulSecsPerTrk );
      if( ptrklay == NULL ) {
        openrw_rc = FSCK_FAILED_DYNSTG_EXHAUSTA;
        }
      else {  /* got the storage */
        for( i = 0; i < ulSecsPerTrk; i++ ) {
          ptrklay->TrackTable[i].usSectorSize = Dev_SectorSize;
          ptrklay->TrackTable[i].usSectorNumber = i + 1;
          }  /* end for */
        }  /* end else got the storage */
      }  /* end got the dev chars */
    else {    /* can't get the device parameters */
      close_rc = close_volume ( );
      }  /* end else can't get the device parameters */
    }  /* end it's open */
  	  
  return( openrw_rc );
}                                /* end of open_device_rw_shared ( ) */
	
	
/*****************************************************************************
 * NAME: open_volume
 *
 * FUNCTION:  Open the device on which the aggregate resides.
 *
 * PARAMETERS:
 *      volname  - input - The device specifier
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t open_volume ( char *volname )
{
  retcode_t opnvol_rc = FSCK_OK;
	
  if( agg_recptr->parm_options[UFS_CHKDSK_CLRBDBLKLST] ) {  /*
				* bad block list processing only
				*/
    opnvol_rc = open_device_rw_shared( volname );
    if( opnvol_rc != FSCK_OK ) {
     fsck_send_msg( fsck_CLRBBOPENFAILED, 0, 0 );
     }
    }  /* end bad block list processing only  */
  else if( agg_recptr->parm_options[UFS_CHKDSK_LEVEL0] ) { /* 
				* read-only request 
				*/
    opnvol_rc = open_device_read ( volname );
    if( opnvol_rc == FSCK_OK ) {  /* successfully opened for Read */
      agg_recptr->processing_readonly = 1;
      }
    }  /* end read-only request */
  else { /* read-write request */
    opnvol_rc = open_device_rw_exclusive ( volname );
    if( opnvol_rc == 0 ) {  /* successfully opened for Read/Write */
      agg_recptr->processing_readwrite = 1;
      }
    else {  /* unable to open for exclusive Read/Write */
      opnvol_rc = open_device_read ( volname );
      if( opnvol_rc == FSCK_OK ) {  /* successfully opened for Read */
        agg_recptr->processing_readonly = 1;
        }
      }  /* end unable to open for exclusive Read/Write */
    }  /* end read-write request */
  	   
  if( opnvol_rc != FSCK_OK ) {
    agg_recptr->fsck_is_done = 1;
    }
  	  
  return( opnvol_rc );
}                    /* end of open_volume ()       */
	 
	 
/*****************************************************************************
 * NAME: readwrite_device
 *
 * FUNCTION:  Read data from or write data to the device on which the 
 *            aggregate resides.
 *
 * PARAMETERS:
 *      dev_offset           - input - the offset, in bytes, into the aggregate
 *                                     of the data to read or to which to write
 *                                     the data.
 *      requested_data_size  - input - the number of bytes requested
 *      actual_data_size     - input - pointer to a variable in which to return
 *                                     the number of bytes actually read or
 *                                     written
 *      data_buffer          - input - the address of the buffer in which to
 *                                     put the data or from which to write
 *                                     the data
 *      mode                 - input - { fsck_READ | fsck_WRITE }
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t readwrite_device( agg_byteidx_t  dev_offset,
                            uint32         requested_data_size,
                            uint32        *actual_data_size,
                            void          *data_buffer,
                            int            mode
                           )
{
  retcode_t rwdb_rc = FSCK_OK;
  uint32 actual_seek_addr;
  uint32 actual_bytes_moved = 0;
  uint32 requested_bytes_moved;
  ULONG ulPhys_sector;
  ULONG ulNumSectors;
  ULONG ulSectorsPerCylinder;
  USHORT ulSecsPerTrk;
  USHORT numSecs, sCylinder, sHead, sFirstSec;
  uint32 ulbytesPsec;
  unsigned long	ParmLen ;
  struct DPB *Pdpb;
  TRACKLAYOUT *tmp_layout;
  char *ptr;
	 
if( (ondev_jlog_byte_offset > 0) &&
    (ondev_jlog_byte_offset <= dev_offset) ) {  /* into jlog! */
#ifdef _JFS_DEBUG
    printf( "READ/WRITE target is in JOURNAL!!\n\r");
    printf("       read/write offset = x%llx\r\n", dev_offset );
    printf("       jlog offset       = x%llx\r\n", ondev_jlog_byte_offset );
//+++    _interrupt(3);
#endif 
    rwdb_rc = FSCK_IOTARGETINJRNLLOG;
    }  /* end into jlog! */
	 
else {	 /* not trying to access the journal log */
  *actual_data_size = 0;
  Pdpb = &Datap;
  ulbytesPsec = Pdpb->dev_bpb.bytes_per_sector;
  if ( (dev_offset % ulbytesPsec) || (requested_data_size % ulbytesPsec ) ) 
    {
    rwdb_rc = FSCK_FAILED_SEEK;
    }
  else 
    {  /* offset seems ok */
    tmp_layout = ptrklay;
    ulSecsPerTrk =  Pdpb->dev_bpb.sectors_per_track;
    ParmLen = sizeof(TRACKLAYOUT);
    ulPhys_sector = dev_offset / Pdpb->dev_bpb.bytes_per_sector;
    ulPhys_sector += Pdpb->dev_bpb.hidden_sectors;
    ulSectorsPerCylinder = ulSecsPerTrk * Pdpb->dev_bpb.number_of_heads;  
    sCylinder = (USHORT)((ulPhys_sector) / ulSectorsPerCylinder);
    if ( sCylinder > Pdpb->number_of_tracks )  
       {
       rwdb_rc = FSCK_FAILED_BADSEEK;
       }
    else 
       {  /* cylinder number seems ok */
       ptr = (char *)data_buffer;
       ulNumSectors = requested_data_size / Pdpb->dev_bpb.bytes_per_sector;
       sHead = (USHORT)((ulPhys_sector % ulSectorsPerCylinder) / ulSecsPerTrk);
       sFirstSec = (USHORT)((ulPhys_sector % ulSectorsPerCylinder) % ulSecsPerTrk);
       ptrklay->bCommand = 0;
       ptrklay->usCylinder = sCylinder;
       while ( ptrklay->usCylinder <= Pdpb->number_of_tracks ) // PS15032004
             {
             ptrklay->usHead = sHead;
             while ( ptrklay->usHead  < Pdpb->dev_bpb.number_of_heads )  
                   {
                   ptrklay->usFirstSector =  sFirstSec;
                   numSecs = ulSecsPerTrk - ptrklay->usFirstSector;
                   ptrklay->cSectors = (numSecs > ulNumSectors)  ? ulNumSectors : numSecs;
                   requested_bytes_moved = ptrklay->cSectors * ulbytesPsec;
          switch( mode ) {
            case fsck_READ:
              rwdb_rc = DosDevIOCtl( Dev_IOPort, IOCTL_DISK, DSK_READTRACK, 
                                    (PVOID)ptrklay, ParmLen, &ParmLen, 
                                    (void *)ptr, 
                                    (ULONG )(requested_bytes_moved),   
                                    (PULONG) &actual_bytes_moved );
              break;
            case fsck_WRITE:
              rwdb_rc = DosDevIOCtl( Dev_IOPort, IOCTL_DISK, DSK_WRITETRACK,
                                     (PVOID)ptrklay, ParmLen, &ParmLen, 
                                     (void *)ptr, 
                                     (ULONG)(requested_bytes_moved),
                                     (PULONG) &actual_bytes_moved );
              break;
            default:  /* it's a bad call */
              rwdb_rc = FSCK_INTERNAL_ERROR_3;
	      break;
            }  /* end it's a bad call */
          if( rwdb_rc != FSCK_OK ) {
            return rwdb_rc;
            }
          *actual_data_size += actual_bytes_moved;
          ulNumSectors -= ptrklay->cSectors;
          if ( ulNumSectors == 0 )
            goto outloop;
          else {
            ptr += actual_bytes_moved;
            ptrklay->usHead++;
            sFirstSec = 0;     /* for the next track, starting from sector 0 */
            }  /* end else */
          }  /* end while */
	ptrklay->usCylinder++;
	sHead = 0;  /* for the next cylinder, starting from head 0 */ 
        }  /* end while */
      if ( ulNumSectors > 0 ) {
        rwdb_rc =  FSCK_INTERNAL_ERROR_53;
        }
      }  /* end else cylinder number seems ok */
    }  /* end else offset seems ok */
  }  /* end else not trying to access the journal log */
	 
outloop:
  return( rwdb_rc );
}                    /* end of readwrite_device ()       */
	 
	 
/*****************************************************************************
 * NAME: recon_dnode_assign 
 *
 * FUNCTION:  Allocate a buffer for a new dnode at the specified aggregate
 *            offset.
 *
 * PARAMETERS:
 *      fsblk_offset  - input - The offset, in aggregate blocks, into the
 *                              aggregate, of the new dnode.
 *      addr_buf_ptr  - input - pointer to a variable in which to return
 *                              the address of the buffer allocated.
 *
 *
 * NOTES: The offset of the dnode being created is a required input because
 *        these buffers have a trailer record which contains the offset
 *         at which to write the dnode.  If this is not stored when the
 *         buffer is allocated for a new dnode, the other routines would
 *         have to treat a new dnode as a special case.
 * 
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t recon_dnode_assign ( fsblkidx_t   fsblk_offset, 
                               dtpage_t   **addr_buf_ptr 
                              )
{
  retcode_t rda_rc = FSCK_OK;
  recon_bufrec_ptr bufrec_ptr;

  rda_rc = dire_buffer_alloc( addr_buf_ptr );

  if( rda_rc != FSCK_OK ) {
    sprintf( message_parm_0, "%ld", 1 );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    fsck_send_msg( fsck_CANTRECONINSUFSTG, 0, 1 );
    }
  else {
    bufrec_ptr = (recon_bufrec_ptr) *addr_buf_ptr;
    bufrec_ptr->dnode_blkoff = fsblk_offset;
    bufrec_ptr->dnode_byteoff = fsblk_offset * sb_ptr->s_bsize;
    }
  return( rda_rc );
}                    /* end of recon_dnode_assign ()       */
	 
	 
/*****************************************************************************
 * NAME: recon_dnode_get
 *
 * FUNCTION: Allocate an fsck dnode buffer and read the JFS dnode page at 
 *           the specified offset into it.
 *
 * PARAMETERS:
 *      fsblk_offset  - input - the offset, in aggregate blocks, into the 
 *                              aggregate, of the desired dnode.
 *      addr_buf_ptr  - input - pointer to a variable in which to return
 *                              the fsck dnode buffer which has been allocated
 *                              and into which the requested dnode has been
 *                              read.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t recon_dnode_get ( fsblkidx_t   fsblk_offset, 
                            dtpage_t   **addr_buf_ptr 
                           )
{
  retcode_t rdg_rc = FSCK_OK;
  reg_idx_t bytes_read;
  recon_bufrec_ptr bufrec_ptr;

  rdg_rc = dire_buffer_alloc( addr_buf_ptr );

  if( rdg_rc != FSCK_OK ) {  
    sprintf( message_parm_0, "%ld", 2 );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    fsck_send_msg( fsck_CANTRECONINSUFSTG, 0, 1 );
    }
  else {  /* got the buffer */
    bufrec_ptr = (recon_bufrec_ptr) *addr_buf_ptr;
    bufrec_ptr->dnode_blkoff = fsblk_offset;
    bufrec_ptr->dnode_byteoff = fsblk_offset * sb_ptr->s_bsize;
    rdg_rc = readwrite_device( bufrec_ptr->dnode_byteoff,
                               BYTESPERPAGE,
                               &bytes_read,
                               (void *) (*addr_buf_ptr),
                               fsck_READ
                              );
    if( rdg_rc != FSCK_OK ) {
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
      sprintf( message_parm_0, "%d", FSCK_CANTREADRECONDNODE );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", rdg_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_READ );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", bufrec_ptr->dnode_byteoff );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", BYTESPERPAGE );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", bytes_read );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONAGG, 0, 6 );
      rdg_rc = FSCK_CANTREADRECONDNODE;
      dire_buffer_release( *addr_buf_ptr );
      }
    else if( bytes_read != BYTESPERPAGE ) {
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
      sprintf( message_parm_0, "%d", FSCK_CANTREADRECONDNODE1 );
      msgprms[0] = message_parm_0;
      msgprmidx[0] = 0;
      sprintf( message_parm_1, "%d", rdg_rc );
      msgprms[1] = message_parm_1;
      msgprmidx[1] = 0;
      sprintf( message_parm_2, "%d", fsck_READ );
      msgprms[2] = message_parm_2;
      msgprmidx[2] = 0;
      sprintf( message_parm_3, "%lld", bufrec_ptr->dnode_byteoff );
      msgprms[3] = message_parm_3;
      msgprmidx[3] = 0;
      sprintf( message_parm_4, "%ld", BYTESPERPAGE );
      msgprms[4] = message_parm_4;
      msgprmidx[4] = 0;
      sprintf( message_parm_5, "%ld", bytes_read );
      msgprms[5] = message_parm_5;
      msgprmidx[5] = 0;
      fsck_send_msg( fsck_ERRONAGG, 0, 6 );
      rdg_rc = FSCK_CANTREADRECONDNODE1;
      dire_buffer_release( *addr_buf_ptr );
      }
    }  /* end got the buffer */
  return( rdg_rc );
}                    /* end of recon_dnode_get ()       */
	 
	 
/*****************************************************************************
 * NAME: recon_dnode_put 
 *
 * FUNCTION:  Write the dnode in the specified buffer into the aggregate
 *            and then release the buffer.
 *
 * PARAMETERS:
 *      buf_ptr  - input -  the address of the buffer containing the
 *                          dnode to write.
 *
 * NOTES:  Unlike most _put_ routines in this module, blkmap_put_ctl_page
 *         actually writes to the device.  
 *
 *         The buffer has a trailer record which contains the offset
 *         at which to write the dnode.
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t recon_dnode_put ( dtpage_t *buf_ptr )
{
  retcode_t rdp_rc = FSCK_OK;
  reg_idx_t bytes_written;
  recon_bufrec_ptr bufrec_ptr;
  bufrec_ptr = (recon_bufrec_ptr) buf_ptr;
  rdp_rc = readwrite_device( bufrec_ptr->dnode_byteoff,
                             BYTESPERPAGE,
                             &bytes_written,
                             (void *) buf_ptr,
                             fsck_WRITE
                            );
  if( rdp_rc != FSCK_OK ) {
           /* 
            * message to user
            */
  DBG_ERROR(("CHK:recon_dnode_put %d written not success \n",bytes_written));
    msgprms[0] = message_parm_0;
    msgprmidx[0] = fsck_metadata;
    msgprms[1] = Vol_Label;
    msgprmidx[1] = 0;
    sprintf( message_parm_2, "%d", 17 );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    fsck_send_msg( fsck_URCVWRT, 0, 3 );
             /*
              * message to debugger
              */
    sprintf( message_parm_0, "%d", FSCK_CANTWRITRECONDNODE );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    sprintf( message_parm_1, "%d", rdp_rc );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    sprintf( message_parm_2, "%d", fsck_WRITE );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    sprintf( message_parm_3, "%lld", bufrec_ptr->dnode_byteoff );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    sprintf( message_parm_4, "%ld", BYTESPERPAGE );
    msgprms[4] = message_parm_4;
    msgprmidx[4] = 0;
    sprintf( message_parm_5, "%ld", bytes_written );
    msgprms[5] = message_parm_5;
    msgprmidx[5] = 0;
    fsck_send_msg( fsck_ERRONAGG, 0, 6 );
    rdp_rc = FSCK_CANTWRITRECONDNODE;
    }
  else if( bytes_written != BYTESPERPAGE ) {
           /* 
            * message to user
            */
  DBG_ERROR(("CHK:recon_dnode_put %d written != BYTESPERPAGE\n",bytes_written));
    msgprms[0] = message_parm_0;
    msgprmidx[0] = fsck_metadata;
    msgprms[1] = Vol_Label;
    msgprmidx[1] = 0;
    sprintf( message_parm_2, "%d", 18 );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    fsck_send_msg( fsck_URCVWRT, 0, 3 );
             /*
              * message to debugger
              */
    sprintf( message_parm_0, "%d", FSCK_CANTWRITRECONDNODE1 );
    msgprms[0] = message_parm_0;
    msgprmidx[0] = 0;
    sprintf( message_parm_1, "%d", rdp_rc );
    msgprms[1] = message_parm_1;
    msgprmidx[1] = 0;
    sprintf( message_parm_2, "%d", fsck_WRITE );
    msgprms[2] = message_parm_2;
    msgprmidx[2] = 0;
    sprintf( message_parm_3, "%lld", bufrec_ptr->dnode_byteoff );
    msgprms[3] = message_parm_3;
    msgprmidx[3] = 0;
    sprintf( message_parm_4, "%ld", BYTESPERPAGE );
    msgprms[4] = message_parm_4;
    msgprmidx[4] = 0;
    sprintf( message_parm_5, "%ld", bytes_written );
    msgprms[5] = message_parm_5;
    msgprmidx[5] = 0;
    fsck_send_msg( fsck_ERRONAGG, 0, 6 );
    rdp_rc = FSCK_CANTWRITRECONDNODE1;
    }
  bufrec_ptr->dnode_blkoff = 0;
  bufrec_ptr->dnode_byteoff = 0;
  dire_buffer_release( buf_ptr );
  return( rdp_rc );
}                    /* end of recon_dnode_put ()       */
	 
	 
/*****************************************************************************
 * NAME: recon_dnode_release
 *
 * FUNCTION:  Release the specified fsck dnode buffer without writing its
 *            contents to the aggregate.
 *
 * PARAMETERS:
 *      buf_ptr  - input -  Address of the fsck dnode buffer to release.
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t recon_dnode_release ( dtpage_t  *buf_ptr )
{
  retcode_t rdr_rc = FSCK_OK;
  recon_bufrec_ptr bufrec_ptr;
  bufrec_ptr = (recon_bufrec_ptr) buf_ptr;
  bufrec_ptr->dnode_blkoff = 0;
  bufrec_ptr->dnode_byteoff = 0;
  rdr_rc = dire_buffer_release( buf_ptr );
  return( rdr_rc );
}                    /* end of recon_dnode_release ()       */
	 
	 
/*****************************************************************************
 * NAME: refresh_bootsec
 *
 * FUNCTION:  Rewrites the boot sector, including Master Boot Records, on
 *            the volume containing the aggregate.
 *
 * PARAMETERS:  none
 *
 * NOTES:
 *
 * RETURNS:
 *      success: FSCK_OK
 *      failure: something else
 */
retcode_t refresh_bootsec ( )
{
  retcode_t  rbs_rc = FSCK_OK;
  char parms = 0;
  unsigned long parmlen = sizeof(parms);
  struct DPB dpb;
  unsigned long dpblen = sizeof(struct DPB);
  rbs_rc = DosDevIOCtl(Dev_IOPort, IOCTL_DISK, DSK_GETDEVICEPARAMS, &parms,
                        sizeof(parms), &parmlen, &dpb, dpblen, &dpblen);
  if( rbs_rc != FSCK_OK ) {  /* read dev characteristics failed */
    fsck_send_msg( fsck_CNTRDDEVCHAR, 0, 0 );
    }  /* end read dev characteristics failed */
  else {  /* got the device characteristics */
    rbs_rc = write_bootsec(Dev_IOPort, &dpb.dev_bpb, sb_ptr->s_fpack, 0);
    if( rbs_rc != FSCK_OK )  { /* write to bootsec failed */
      fsck_send_msg( fsck_CNTWRTBS, 0, 0 );
      if (rbs_rc == ERROR_WRITE_PROTECT)
        agg_recptr->fsck_is_done = 1;
      }  /* end write to bootsec failed */
    else {  /* write to bootsec worked */
	/* 
         * Update Master Boot Record 
         */
      rbs_rc = ujfs_update_mbr( Dev_IOPort );
      if( rbs_rc != FSCK_OK ) {  /* failed */
        fsck_send_msg( fsck_CNTWRTBSMBR, 0, 0 );
        }
      else {  /* bootsec has been repaired */
        fsck_send_msg( fsck_BOOTSECFXD, 0, 0 );
        }  /* end else bootsec has been repaired */
      }  /* end else write to bootsec worked */
    }  /* end else got the device characteristics */
  return( rbs_rc );
}
