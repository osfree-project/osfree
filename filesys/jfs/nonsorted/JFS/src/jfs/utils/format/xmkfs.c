/* $Id: xmkfs.c,v 1.1.1.1 2003/05/21 13:41:38 pasha Exp $ */

static char *SCCSID = "@(#)1.39  6/16/99 08:33:32 src/jfs/utils/format/xmkfs.c, jfsformat, w45.fs32, fixbld";
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
 *   MODULE_NAME:               xmkfs.c
 *
 *   COMPONENT_NAME:    jfsformat
 *
 *   FUNCTIONS:    create_aggregate
 *                      create_fileset
 *                      format
 *
 * 
 */
#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_DOSDEVIOCTL
#include <os2.h>
#include "jfs_types.h"
#include <jfs_aixisms.h>
#include <sysbloks.h>
#include <extboot.h>
#include <basemid.h>
#include "bootsec.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <ctype.h>
#include <sesrqpkt.h>
#include <ioctl.h>
#include <dskioctl.h>

#include "jfs_filsys.h"
#include "jfs_dinode.h"
#include "jfs_superblock.h"
#include "jfs_xtree.h"
#include "initmap.h"
#include "inodes.h"
#include "inode.h"
#include "inodemap.h"
#include "devices.h"
#include "jfs_imap.h"
#include "super.h"
#include "logform.h"
#include "jfs_dmap.h"
#include "message.h"
#include "debug.h"
#include "lvmbdblk.h"

static int32    AGsize;
extern pm_front_end;               //198316 support pmformat
int32    stdout_redirected;

/* Define a parameter array for messages */
#define MAXPARMS        9
#define MAXSTR          128
char    *msg_parms[MAXPARMS];
char    msgstr[MAXSTR];

#define L2MEGABYTE      20
#define MEGABYTE        (1 << L2MEGABYTE)
#define MEGABYTE32     (MEGABYTE << 5)
#define MAX_LOG_PERCENTAGE  10              /* Log can be at most 10% of disk */

/*
 * The following macro defines the initial number of aggregate inodes which are
 * initialized when a new aggregate is created.  This does not include any
 * fileset inodes as those are initialized separately.
 */
#define INIT_NUM_AGGR_INODES    (BADBLOCK_I + 1)

static struct dinode    aggr_inodes[INIT_NUM_AGGR_INODES];

/*
 *
 * for use when dealing with the LVM Bad Block functions
 *
 */
struct lvm_bdblk_record  bdblk_record;
struct lvm_bdblk_record *bdblk_recptr;
boolean_t  lvm_BdBlk_enabled;
boolean_t  do_lvm_BdBlks;


/*--------------------------------------------------------------------
 * NAME: create_fileset
 *
 * FUNCTION: Do all work to create a fileset in an aggregate
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *      dev_ptr         - open port of device to write to
 *      aggr_block_size - block size for aggregate
 *      start_block     - Number of blocks used by aggregate metadata, indicates
 *                        first block available for writing fileset metadata.
 *                        All fileset writes will be offsets from this.
 *      inostamp        - Inode stamp value to be used.
 *
 * RETURNS:
 *      success: 0
 *      failure: any other value
 */
static int32 create_fileset( int32      dev_ptr,
                             int32      aggr_block_size,
                             int64      start_block,
                             int32      inostamp)
{
  int32       rc;
  int64       inode_table_loc, AG_table_loc, inode_map_loc;
  int32       inode_table_size, inode_map_size;

  /*
   * Find space for the inode allocation map page
   *
   * Also find the fileset inode map location on disk (inode_map_loc).  We
   * need to know this in order to initialize the fileset inodes with the
   * proper iag value.
   *
   * Since we only have one fileset per aggregate in the first release, we
   * always know where the inode map will start.  Therefore, currently we use
   * a hard-coded value.  When we add multiple filesets per aggregate this
   * will need to be modified to find the space for the inode map by looking
   * in the block allocation map for available space.
   */
  inode_map_size = SIZE_OF_MAP_PAGE << 1;
#ifdef ONE_FILESET_PER_AGGR
  /*
   * The first extent of the fileset inode allocation map follows the first
   * extent of the first extent of the fileset inodes at the beginning of the
   * fileset
   */
  inode_map_loc = start_block  + INODE_EXTENT_SIZE/aggr_block_size;
#else
  inode_map_loc = get_space( inode_map_size );
#endif

  /*
   * Allocate Aggregate Inodes for Fileset
   */
  rc = init_fileset_inodes( aggr_block_size, dev_ptr, inode_map_loc,
                            inode_map_size, start_block, inostamp );
  if ( rc != 0 ) return(rc);

  /*
   * Create Fileset Inode Table - first extent
   */
  rc = init_fileset_inode_table( aggr_block_size, dev_ptr, &inode_table_loc,
                                 &inode_table_size, start_block,
                                 inode_map_loc, inostamp );
  if ( rc != 0 ) return(rc);

  /*
   * Create Fileset Inode Allocation Map - first extent
   */
  rc = init_inode_map( aggr_block_size, dev_ptr, inode_table_loc,
                       inode_table_size, inode_map_loc, inode_map_size,
                       (ACL_I + 1), AGsize, FILESYSTEM_I );

  return rc;
}


/*--------------------------------------------------------------------
 * NAME: create_aggregate
 *
 * FUNCTION: Do all work to create an aggregate
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *      dev_ptr                 - open port of device to write to
 *      volume_label            - label for volume
 *      number_of_blocks        - number of blocks for aggregate
 *      compress                - indicates compression for the aggregate
 *      aggr_block_size         - block size for aggregate
 *      phys_block_size         - physical block size of device
 *      type_jfs                - JFS type to create
 *      verify_blocks           - indicates if we should verify every block
 *
 * NOTES: The superblocks are the last things written to disk.  In the event of
 *      a system crash during mkfs this device will not appear to have a real
 *      JFS filesystem.  This should prevent us from attempting to mount a
 *      partially initialized filesystem.
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      success: 0
 *      failure: any other value
 */
#define L2BITSPERBYTE   3
#define BITSPERPAGE     (PSIZE << L2BITSPERBYTE)
static int32 create_aggregate( int32    dev_ptr,
                               char     *volume_label,
                               int64    number_of_blocks,
                               int32    compress,
                               int32    aggr_block_size,
                               int32    phys_block_size,
                               uint32   type_jfs,
                               int64    logloc,
                               int32    logsize,
                               boolean_t        verify_blocks)
{
  struct superblock   aggr_superblock;
  void        *buffer;
  int32       rc, intermed_rc;
  int64       index;
  int64       first_block, last_block;
  int64       reserved_size;
  int64       aggr_inode_map_loc;
  int32       aggr_inode_map_sz;
  xad_t       inode_map_dscr;
  int64       secondary_ait_address, secondary_aimap_address;
  int64       secondary_ait_end;
  int64       fsck_wspace_address, num_bits;
  int32       fsck_wspace_length, fsck_svclog_length, npages;
  uint32      inostamp;
  struct dinode       fileset_inode;
	
  /*
   * Find where fsck working space will live on disk and mark those blocks.
   * The fsck working space is always at the very end of the aggregate so once
   * we know how big it is we can back up from the end to determine where it
   * needs to start.
   *
   * Need enough 4k pages to cover:
   *  - 1 bit per block in aggregate rounded up to BPERDMAP boundary
   *  - 1 extra 4k page to handle control page and intermediate level pages
   *  - 50 extra 4k pages for the chkdsk service log
   */
  num_bits = ((number_of_blocks + BPERDMAP - 1) >> L2BPERDMAP) << L2BPERDMAP;
  npages = ((num_bits + (BITSPERPAGE - 1)) / BITSPERPAGE) + 1 + 50;
  fsck_wspace_length = (npages << L2PSIZE) / aggr_block_size;
  fsck_wspace_address = number_of_blocks - fsck_wspace_length;
  fsck_svclog_length = (50 << L2PSIZE) / aggr_block_size;

  /*
   * Now we want the fsck working space to be ignored as actually being part
   * of the filesystem
   */
  number_of_blocks -= fsck_wspace_length;

  /*
   * Initialize disk block map, so blocks can be marked as they are used
   * Blocks used for fsck working space will be marked here since we don't
   * want those blocks to be accounted for when maxag is set
   */
  inostamp = (uint32) time(NULL);
  rc = calc_map_size( number_of_blocks, aggr_inodes, aggr_block_size,
                      &AGsize, inostamp );
  if ( rc != 0 ) return rc;

  /*
   * Initialize and clear reserved disk blocks
   */
  reserved_size = AGGR_RSVD_BYTES;
  /*
   * Since the LVM on AIX needs the first block for information, don't write
   * over it
   */
  buffer = calloc( reserved_size - PSIZE, sizeof(char));
  if( buffer == NULL ) {
    message_user(MSG_OSO_INSUFF_MEMORY, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
    return( ENOMEM );
    }
  rc = ujfs_rw_diskblocks( dev_ptr, PSIZE, reserved_size - PSIZE, buffer,
                           PUT );
  if ( rc != 0 ) return rc;
  for ( index = 0; ((index < reserved_size/aggr_block_size)&&(rc==0)); index++ )
  {
    rc = markit( index, ALLOC );
  }
  if ( rc != 0 ) return rc;

  /*
   * In case mkfs does not complete, but we have an old superblock already on
   * this device, we will zero the superblock disk blocks at the beginning and
   * then write the real superblock to disk last.  (This keeps the device from
   * appearing to have a complete filesystem when initialization is not
   * complete.)
   */
  rc = ujfs_rw_diskblocks( dev_ptr, SUPER1_OFF, SIZE_OF_SUPER, buffer, PUT );
  if ( rc != 0 ) {
    free( buffer );
    return rc;
    }
  rc = ujfs_rw_diskblocks( dev_ptr, SUPER2_OFF, SIZE_OF_SUPER, buffer, PUT );
  if ( rc != 0 ) {
    free( buffer );
    return rc;
    }
  free( buffer );

  /* Mark blocks allocated for superblocks. */
  first_block = SUPER1_OFF/aggr_block_size;
  last_block = first_block + (SIZE_OF_SUPER/aggr_block_size);
  for ( index = first_block; ((index < last_block)&&(rc==0)); index++ )
  {
    rc = markit( index, ALLOC );
  }
  if ( rc != 0 ) return rc;

  first_block = SUPER2_OFF/aggr_block_size;
  last_block = first_block + (SIZE_OF_SUPER/aggr_block_size);
  for ( index = first_block; ((index < last_block)&&(rc==0)); index++ )
  {
    rc = markit( index, ALLOC );
  }
  if ( rc != 0 ) return rc;

  /*
   * Initialize First Extent of Aggregate Inode Allocation Map
   */
  aggr_inode_map_loc = AIMAP_OFF;
  aggr_inode_map_sz = SIZE_OF_MAP_PAGE << 1;
  rc = init_inode_map( aggr_block_size, dev_ptr, AITBL_OFF, INODE_EXTENT_SIZE,
                       aggr_inode_map_loc/aggr_block_size, aggr_inode_map_sz,
                       INIT_NUM_AGGR_INODES + 1, AGsize, AGGREGATE_I);
  if ( rc != 0 ) return rc;

  /*
   * Initialize first inode extent of Aggregate Inode Table
   */
  rc = init_aggr_inode_table( aggr_block_size, dev_ptr, aggr_inodes,
                              INIT_NUM_AGGR_INODES, AITBL_OFF,
                              aggr_inode_map_loc/aggr_block_size,
                              aggr_inode_map_sz, inostamp );
  if ( rc != 0 ) return rc;

  /*
   * Now initialize the secondary aggregate inode table and map
   *
   * We can use the same aggr_inodes we already initialized except for the
   * aggregate self inode.  This will be updated by the call to
   * init_aggr_inode_table() to point to the secondary table instead of the
   * primary table.
   *
   * First we need to determine the location; it will follow the block map
   * Since the block map might be sparse we need to use the number of blocks
   * instead of the length of the extents.  This works since the extents are
   * in the inode for mkfs
   */
  inode_map_dscr =
  ((xtpage_t *) &(aggr_inodes[BMAP_I].di_DASD))->xad[XTENTRYSTART];     /* @F1 */
  secondary_aimap_address = addressXAD( &inode_map_dscr ) +
                            aggr_inodes[BMAP_I].di_nblocks;
  secondary_ait_address = (secondary_aimap_address * aggr_block_size) +
                          (SIZE_OF_MAP_PAGE << 1);
  secondary_ait_end = (secondary_ait_address + INODE_EXTENT_SIZE) /
                      aggr_block_size;

  rc = init_inode_map( aggr_block_size, dev_ptr, secondary_ait_address,
                       INODE_EXTENT_SIZE, secondary_aimap_address, aggr_inode_map_sz,
                       INIT_NUM_AGGR_INODES + 1, AGsize, AGGREGATE_I);
  if ( rc != 0 ) return rc;

  /*
   * Modify the aggregate inodes ixpxd fields
   */
  PXDaddress(&(aggr_inodes[BMAP_I].di_ixpxd),
             secondary_ait_address/aggr_block_size);
  rc = init_aggr_inode_table( aggr_block_size, dev_ptr, aggr_inodes,
                              INIT_NUM_AGGR_INODES, secondary_ait_address,
                              secondary_aimap_address, aggr_inode_map_sz, inostamp );
  if ( rc != 0 ) return rc;

  /*
   * Mark blocks for the block map
   */
  first_block = BMAP_OFF/aggr_block_size;
  last_block = first_block + aggr_inodes[BMAP_I].di_nblocks;
  for ( index = first_block; ((index < last_block)&&(rc==0)); index++ )
  {
    rc = markit( index, ALLOC );
  }
  if ( rc != 0 ) return rc;

  /*
   * Now we will create a fileset as necessary.
   *
   * Determine the end of the metadata written for the aggregate to tell
   * where to put the fileset to be created.  Since the secondary aggregate
   * inode table is the last thing written to the aggregate, the fileset will
   * start following it.
   */
  rc = create_fileset( dev_ptr, aggr_block_size, secondary_ait_end, inostamp);
  if ( rc != 0 ) return rc;

  /*
   * Copy the fileset inode to the secondary aggregate inode table
   */
  rc = ujfs_rwinode( dev_ptr, &fileset_inode, FILESYSTEM_I, GET,
                     aggr_block_size, AGGREGATE_I );
  if ( rc != 0 ) return rc;

  PXDaddress(&(fileset_inode.di_ixpxd),
             secondary_ait_address/aggr_block_size);
  rc = ujfs_rw_diskblocks( dev_ptr,
                           secondary_ait_address + FILESYSTEM_I * sizeof(struct dinode),
                           sizeof(fileset_inode), &fileset_inode, PUT );
  if ( rc != 0 ) return rc;


  /*
   * If we are supposed to verify all blocks, now is the time to do it
   * 
   * First we tell the LVM to stop doing Bad Block Relocation so we can
   * catch (and record) any bad blocks ourselves.  Next we run through the
   * available file system space looking for bad blocks.  Finally we tell
   * the LVM to go back to doing Bad Block Relocation.
   */
  PXDaddress(&(aggr_inodes[BADBLOCK_I].di_ixpxd),
             AITBL_OFF/aggr_block_size);
  if ( verify_blocks == TRUE )
  {
    if( do_lvm_BdBlks == TRUE ) {
      intermed_rc = disable_LVM_BdBlkReloc( dev_ptr );
      if ( intermed_rc != 0 ) {
        do_lvm_BdBlks = FALSE;
        }
      }
	
    rc = verify_last_blocks( dev_ptr, aggr_block_size,
                             &(aggr_inodes[BADBLOCK_I]));
    if (rc != 0) return rc;

    if( do_lvm_BdBlks == TRUE ) {
      intermed_rc = enable_LVM_BdBlkReloc( dev_ptr );
      if ( intermed_rc != 0 ) {
        do_lvm_BdBlks = FALSE;
        }
      }
  }
  /*
   * If we aren't supposed to verify all blocks, now is the time to record
   * the bad blocks (in the available file system space) which were on the
   * LVM's bad block list.
   */
  else {
    if( do_lvm_BdBlks == TRUE ) {
      intermed_rc = record_LVM_BadBlks( dev_ptr, aggr_block_size, 
					phys_block_size, 
					&(aggr_inodes[BADBLOCK_I]), 
					fsck_wspace_address-1 );
      if( intermed_rc < 0 ) {
        return intermed_rc;
        }
      else if( intermed_rc > 0 ) {
        do_lvm_BdBlks = FALSE;
        }
      }
    }
	
  /*
   * Copy the bad block inode to the secondary aggregate inode table
   */
  PXDaddress(&(aggr_inodes[BADBLOCK_I].di_ixpxd),
             secondary_ait_address/aggr_block_size);
  rc = ujfs_rw_diskblocks( dev_ptr,
                           secondary_ait_address + BADBLOCK_I * sizeof(struct dinode),
                           sizeof(struct dinode), &(aggr_inodes[BADBLOCK_I]), PUT );
  if ( rc != 0 ) return rc;

  /*
   * Now our block allocation map should be complete, write to disk
   */
  rc = write_block_map( dev_ptr, number_of_blocks, aggr_block_size );
  if ( rc != 0 ) return rc;

  /*
   * Initialize Aggregate Superblock - Both primary and secondary
   */
  rc = ujfs_init_superblock( &aggr_superblock, dev_ptr, volume_label,
                             number_of_blocks, compress, aggr_block_size,
                             phys_block_size, type_jfs,
                             secondary_ait_address/aggr_block_size,
                             INODE_EXTENT_SIZE/aggr_block_size, AGsize,
                             fsck_wspace_address, fsck_wspace_length,
                             fsck_svclog_length, logloc,
                             logsize, secondary_aimap_address,
                             aggr_inode_map_sz/aggr_block_size );

  return rc;
}


/*--------------------------------------------------------------------
 * NAME: format
 *
 * FUNCTION:            format the specified partition as a JFS file system.
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *
 * RETURNS:
 *      success: 0
 *      failure: any other value
 */
USHORT _Far16 _Pascal _loadds format(USHORT argc,
                                     UCHAR * _Seg16 * _Seg16 argv,
                                     UCHAR * _Seg16 * _Seg16 envp)
{
  int32       i, j, l2absize;
  char        *lvolume = NULL;
  UCHAR *     argp;
  int32       rc = 0, intermed_rc;
  char        volume_label[12] = {"\0\0\0\0\0\0\0\0\0\0\0\0"};
  int64       number_of_bytes = 0, bytes_on_device, hidden_bytes;
  int64       number_of_blocks, logloc, logsize_in_bytes;
  char        logdev[255] = { '\0'}; /* Need to use a macro for this size */
  int32       compress = 0;
  int32       aggr_block_size = 0;
  int32       phys_block_size, logsize = 0, dev_logsize;
  HFILE       dev_handle, log_handle;
  uint32      type_jfs = JFS_OS2;
  uint32      type_commit = JFS_GROUPCOMMIT;
  boolean_t   verify_blocks = FALSE;
  char            parms = 0;
  unsigned long   parmlen = sizeof(parms);
  struct DPB      dpb;
  unsigned long   dpblen = sizeof(struct DPB);
  char            bootsec_buf[1024];
  struct Extended_Boot *  ext_boot;
  UCHAR *         user_input;
  FSALLOCATE      fsalloc_buf;
  FSINFO          fsinfo_buf;
  int32           free_space;

  /*
   * Parse command line arguments
   *
   * The following parses both the AIX and OS/2 style arguments.  In OS/2,
   * the flags can be preceded by either a dash - or a slash /.  An optional
   * colon or space may separate the flag from its arguments.
   *
   * The following are equivalent:
   *
   * mkfs -v myfs -bs:1024 e:         # AIX style
   * mkfs -vmyfs -BS:1024 e:          # AIX style
   * mkfs /v:myfs /bs:1024 e:         # OS/2 style
   */

  argp = argv[1];     /* set argp to first real arg  remember to skip function name */

  lvolume = argp;     /* first parm must be drive letter         */
                      /* this is validated by format.com so no need to check it */

  for (i = 2; i < argc; i++)
  {

    argp = argv[i];

    if (*argp == '/')
    {
      argp++;
      if (*argp == 'v' || *argp == 'V')
      {
        if (argp[1] == ':')
        {
          strncpy (volume_label, argp+2, 11);
        }
        else
        {
          message_user(MSG_OSO_VALUE_REQUIRED, &argp, 1, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
          return( EINVAL );
        }
      }
      else if (strncmp(argp, "fs", 2) == 0 ||
               strncmp(argp, "FS", 2) == 0)
      {
        if (strcmp(argp+2, ":jfs") != 0 && strcmp(argp+2, ":JFS") != 0)
        {
          DBG_ERROR(("Internal error: Argument of /fs: flag must be jfs\n"))
          return (EINVAL);
        }
      }

      else if (*argp == 's' || *argp == 'S')
      {
        type_jfs |= JFS_SPARSE;
      }
      else if (*argp == 'p' || *argp == 'P')        //198316
      {                                             //198316
        pm_front_end = 1;                           //198316
      }                                             //198316
      else if (strncmp(argp, "bs", 2) == 0 ||
               strncmp(argp, "BS", 2) == 0)
      {
        if (argp[2] == ':')
        {
          aggr_block_size = strtol(argp + 3, NULL, 0);
        }
        else
        {
          message_user(MSG_OSO_VALUE_REQUIRED, &argp, 1, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
          return( EINVAL );
        }
      }
      else if (strncmp(argp, "ls", 2) == 0 ||
               strncmp(argp, "LS", 2) == 0)
      {
        if (argp[2] == ':')
        {
          logsize = strtol(argp + 3, NULL, 0);
          logsize_in_bytes = ((int64) logsize) * MEGABYTE;     /* convert to byte count */
        }
        else
        {
          message_user(MSG_OSO_VALUE_REQUIRED, &argp, 1, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
          return( EINVAL );
        }
      }

#ifdef LOG_DEVICE
      else if (strncmp(argp, "ld", 2) == 0 ||
               strncmp(argp, "LD", 2) == 0)
      {
        if (argp[2] == ':')
        {
          strcpy( logdev, argp + 3 );
        }
        else
        {
          DBG_ERROR(("Missing value for ld option\n"))
          return( EINVAL );
        }
      }
#endif  /* LOG_DEVICE */
      else if (*argp == 'l' || *argp == 'L')
      {
        verify_blocks = TRUE;
      }
      else   /* bad paramanter */
      {
            /* These options are supported by other OS/2 filesystems;
             * they are errors for jfs included here are 4, t, n, f, q, once */

        argp--;     /* back up to include the '/' in the error message */
        message_user(MSG_OSO_INVALID_PARAMETER, &argp, 1, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
        return EINVAL;
      }
    }
    else
    {
      message_user(MSG_OSO_INVALID_PARAMETER, &argp, 1, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
      return EINVAL;
    }
  }

   /*
    * check to see whether standard out has been redirected, and
    * set the flag accordingly.
    */
  if( (ujfs_stdout_redirected()) != 0 ) {
    stdout_redirected = 1;
    }
  else {
    stdout_redirected = 0;
    }


  /*
   * Initialize for the LVM Bad Block Facility functions
   *
   */
    bdblk_recptr = &bdblk_record;
    memset ( bdblk_recptr, 0, sizeof(bdblk_record) );
    memcpy( (void *) &(bdblk_recptr->eyecatcher), (void *) "bdblkrec", 8 );
    do_lvm_BdBlks = TRUE;


  /*
   * Open the device and lock it from all others
   * Get the physical block size of the device.
   */
  rc = ujfs_open_device( lvolume, &dev_handle, &phys_block_size, RDWR_EXCL );
  if ( rc != 0 )
  {
    message_user(MSG_OSO_DISK_LOCKED, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
    return( rc );
  }

    /*
     * Verify that this is an LVM volume 
     */
  rc = ujfs_verify_device_type( dev_handle );
  if ( rc != 0 ) 
  {
    message_user(MSG_JFS_VOL_NOT_LVM, msg_parms, 0, STDOUT_CODE, NO_RESPONSE, JFS_MSG);
    message_user(MSG_OSO_FORMAT_FAILED, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
    ujfs_close( dev_handle );
    return (EINVAL);
  }

  /*
   * Check to see if adapter supports physical memory above 64 MB.
   */
   rc = ujfs_check_adapter();
   if (rc != NO_ERROR)
   {
    message_user(MSG_JFS_LT16MEG_ADAPTER, 0, 0, STDOUT_CODE, NO_RESPONSE, JFS_MSG);
    message_user(MSG_OSO_FORMAT_FAILED, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
    ujfs_close( dev_handle );
    return ERROR_INVALID_FUNCTION;
   }

  /*
   * Check if aggr_block_size was specified.  If not, we need to 4K
   */
  if ( aggr_block_size == 0 )
  {
    aggr_block_size = PSIZE;   /* 4096 */
  }
  else if ( aggr_block_size < phys_block_size )
  {
    /*
     * Make sure the aggr_block_size is not smaller than the logical volume
     * block size
     */
    sprintf(msgstr, "%d", aggr_block_size);
    msg_parms[0] = msgstr;
    msg_parms[1] = "BS";
    message_user(MSG_OSO_VALUE_NOT_ALLOWED, msg_parms, 2, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
    rc = EINVAL;
    ujfs_close( dev_handle );
    return (rc);
  }
  else
  {
    /*
     * Validate user specified aggregate block size
     */
    if ( ! inrange( aggr_block_size, MINBLOCKSIZE, MAXBLOCKSIZE ) )
    {
      sprintf(msgstr, "%d", aggr_block_size);
      msg_parms[0] = msgstr;
      msg_parms[1] = "BS";
      message_user(MSG_OSO_VALUE_NOT_ALLOWED, msg_parms, 2, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
      rc = EINVAL;
      ujfs_close( dev_handle );
      return (rc);
    }
  }

  /*
   * get size of the logical volume
   */
  rc = ujfs_get_dev_size( dev_handle, &bytes_on_device, &hidden_bytes );
  if ( rc != 0 )
  {
    DBG_ERROR(("ujfs_get_dev_size: FAILED rc = %lld\n", rc ))
    message_user(MSG_OSO_FORMAT_FAILED, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
    ujfs_close( dev_handle );
    return (rc);
  }

  number_of_bytes = bytes_on_device;
  DBG_TRACE(("ujfs_get_dev_size: size = %lld\n", number_of_bytes ))

  /*
   * Make sure we have at least MINJFS for our file system
   * Notes: The operating system takes some of the bytes from the partition to
   * use for its own information.  The end user is not aware of this space, so
   * we want to compare a posted minimum size to the actual size of the
   * partition, not just the space available for our use.
   */
  if (number_of_bytes + hidden_bytes < MINJFS)
  {
    /* If MINJFS changes, this hardcoded string should change also! */
    _itoa(MINJFS / MEGABYTE, msgstr, 10);    /* convert MINJFS to string */
    msg_parms[0] = msgstr;
    message_user(MSG_JFS_PART_SMALL, msg_parms, 1, STDOUT_CODE, NO_RESPONSE, JFS_MSG);
    rc = EINVAL;
    ujfs_close( dev_handle );
    return (rc);
  }

  /*
   * Size of filesystem in terms of block size
   */
  number_of_blocks = number_of_bytes / aggr_block_size;
  DBG_TRACE(("number of blocks = %lld\n", number_of_blocks ))


  /* All parameters look good.  Time to begin the format.             */
  /* First get the current volume label from the file system           */
  /* Then prompt the user for the current volume label if there is one */
  /* if the volume labels don't match, exit with error and             */
  /* do not format the drive                                           */

  rc =  DosQueryFSInfo((ULONG )(toupper(*lvolume) -'A'+1), 2L,
                 (PVOID)&fsinfo_buf, sizeof(FSINFO));
  if (rc)
  {            /* Query FSINFO failed, ignore vol label check */
    rc = 0;
  }
  else         /* check if there is a vol label */
  {
    /* if volume label not null */
    if ((fsinfo_buf.vol.cch != 0) && !pm_front_end)  //198316
    {
      msg_parms[0] = lvolume;
                                 /* prompt for vol label */
      user_input = message_user(MSG_OSO_CURR_LABEL, msg_parms, 1, STDOUT_CODE, STRING_RESPONSE, OSO_MSG);

      for( j=fsinfo_buf.vol.cch; j < sizeof(fsinfo_buf.vol.szVolLabel); j++) {
            fsinfo_buf.vol.szVolLabel[j] = '\0';
            }
      if (memicmp(fsinfo_buf.vol.szVolLabel, user_input, sizeof(fsinfo_buf.vol.szVolLabel)))
      {
                               /* tell user bad vol label and exit */
        message_user(MSG_OSO_BAD_LABEL, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
        ujfs_close( dev_handle );
        return 0;
      }
    }
  }


  /* now ask the user if he really wants to destoy his data */
  do
  {
    msg_parms[0] = lvolume;
    user_input = message_user(MSG_OSO_DESTROY_DATA, msg_parms, 1, STDOUT_CODE, YES_NO_RESPONSE, OSO_MSG);
    if (*user_input == NO_ANS)
    {
      ujfs_close( dev_handle );
      return 0;
    }
  } while (*user_input != YES_ANS );


  /*
   * Unmount the filesystem and force JFS to mount it
   */
  rc = ujfs_beginformat(dev_handle);
  if (rc != 0)
  {
    DBG_ERROR(("Internal error: %s(%d): rc: %d.\n", __FILE__, __LINE__,rc))
    message_user(MSG_OSO_FORMAT_FAILED, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
    ujfs_close( dev_handle );
    return (rc);
  }

  /*
   * Create journal log for aggregate to use
   *
   * For the prototype we will only create a journal log if one was specified
   * on the command line.  Eventually we will always need one and we will need
   * a default method of finding and creating the log.
   */
  if ( logdev[0] != '\0' )
  {
    /* A log device was specified on the command line.  Call jfs_logform()
     * to initialize the log */
    rc = jfs_logform( -1, aggr_block_size, log2shift(aggr_block_size),
                      type_jfs|type_commit, 0, 0, logdev, 0 );
    if ( rc != 0 );
    {
      ujfs_redeterminemedia(dev_handle);
      message_user(MSG_OSO_FORMAT_FAILED, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
      DBG_ERROR(("Internal error: Format failed rc=%x\n",rc))

      /* Unlock device whether or not creation of aggregate succeeded.*/
      ujfs_close( dev_handle );

      /* Exit with return code */
      return( rc );
    }
  }
  else
  {
    /* INLINE log */
    type_jfs |= JFS_INLINELOG;

    if ( logsize == 0 )
    {
      /* If no size specified, let's default to .4 of aggregate size;
       * Which for simplicity is 4/1024 == 2**2/2**10 == 1/2**8 == >> 8
       *
       * Round logsize up to a megabyte boundary */

      logsize_in_bytes = number_of_bytes >> 8;  /* BYTES / 256 */
      logsize_in_bytes = (logsize_in_bytes + MEGABYTE - 1) & ~(MEGABYTE - 1);  /* round up to meg */
      if( logsize_in_bytes > MEGABYTE32 ) { 
        logsize_in_bytes = MEGABYTE32;
      }
    }

    /* Convert logsize into aggregate blocks */
    logsize = logsize_in_bytes / aggr_block_size;

    if ( logsize >= (number_of_blocks / MAX_LOG_PERCENTAGE) )
    {
      message_user(MSG_JFS_LOG_LARGE, NULL, 0, STDOUT_CODE, NO_RESPONSE, JFS_MSG);
      rc = ENOSPC;
      ujfs_redeterminemedia(dev_handle);
      message_user(MSG_OSO_FORMAT_FAILED, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
      DBG_ERROR(("Internal error: Format failed rc=%x\n",rc))

      /* Unlock device whether or not creation of aggregate succeeded.*/
      ujfs_close( dev_handle );

      /* Exit with return code */
      return( rc );
    }

	
	/*
	 * Query the LVM about its Bad Block List (set of Bad Block Tables) 
	 * for this partition.
	 *
	 * NOTE:
	 *	if this or any other processing to load the LVM's bad block
	 *	list for the aggregate into the JFS Bad Block Inode fails,
	 *	we simply skip over the rest of that part of the processing.
	 *	
	 *	This is intended as a best-try effort, invisible to the caller,
	 *	so we won't issue a message or stop the format.
	 */
  intermed_rc = get_LVM_BdBlkLst_count( dev_handle );
  if ( intermed_rc != 0 ) {
    do_lvm_BdBlks = FALSE;
    }
	/*
	 * if we aren't doing a long format, then we need to 
	 * get the LVM's tables and save them for later 
	 */
  if( (do_lvm_BdBlks == TRUE) && (verify_blocks == FALSE) ) {	
    intermed_rc = get_LVM_BadBlockLists( dev_handle );
    if ( intermed_rc != 0 ) {
      do_lvm_BdBlks = FALSE;
      }
    }
	/*
	 * Tell the LVM to clear all its tables for the partition
	 */
  if( do_lvm_BdBlks == TRUE ) {
    intermed_rc = tell_LVM_ClearLists( dev_handle );
    if ( intermed_rc != 0 ) {
      do_lvm_BdBlks = FALSE;
      }
    }
	/*
	 * Tell the LVM to turn on Bad Block Relocation so that it will
	 * handle any bad blocks in the fixed position meta data.
	 */
  if( do_lvm_BdBlks == TRUE ) {
    intermed_rc = enable_LVM_BdBlkReloc( dev_handle );
    if ( intermed_rc != 0 ) {
      do_lvm_BdBlks = FALSE;
      }
    }

    logloc = number_of_blocks - logsize;
    number_of_blocks -= logsize;
    l2absize = log2shift(aggr_block_size);
    rc = jfs_logform( dev_handle, aggr_block_size,
                      l2absize, type_jfs|type_commit, logloc,
                      logsize, NULL, 0 );
    if ( rc != 0 )
    {
      ujfs_redeterminemedia(dev_handle);
      message_user(MSG_OSO_FORMAT_FAILED, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
      DBG_ERROR(("Internal error: Format failed rc=%x\n",rc))

      /* Unlock device whether or not creation of aggregate succeeded.*/
      ujfs_close( dev_handle );

      /* Exit with return code */
      return( rc );
    }
  }

  /* query the user for new Volume Label if one was not entered on command line */
  if (volume_label[0] == 0)
  {
    user_input = message_user(MSG_OSO_NEW_LABEL, NULL, 0, STDOUT_CODE, STRING_RESPONSE, OSO_MSG);
    strncpy(volume_label, user_input, 11);
  }

  /*
   * Create aggregate, which will also create a fileset as necessary
   */
  rc = create_aggregate( dev_handle, volume_label, number_of_blocks, compress,
                         aggr_block_size, phys_block_size,
                         type_jfs|type_commit, logloc, logsize,
                         verify_blocks);
  /** Create Boot Sector*/
  if( rc == 0 ) {
    rc = DosDevIOCtl(dev_handle, IOCTL_DISK, DSK_GETDEVICEPARAMS, &parms,
                   sizeof(parms), &parmlen, &dpb, dpblen, &dpblen);
    }
  if( rc == 0 ) {
    rc = write_bootsec(dev_handle, &dpb.dev_bpb, volume_label, 0);
    }

  /* Update Master Boot Record */
  if( rc == 0 ) {
    rc = ujfs_update_mbr(dev_handle);
    }

  /* Remount the file system */
  if (ujfs_redeterminemedia(dev_handle))
  {
    message_user(MSG_OSO_FORMAT_FAILED, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
    DBG_ERROR(("Internal error: Format failed rc=%x\n",rc))
  }

  /* Unlock and close the volume */
  ujfs_close( dev_handle );

  /* Format Complete message */
  if( rc == 0 ) {
    message_user(MSG_OSO_FORMAT_COMPLETE, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
    /* stat messages */
    _itoa(number_of_bytes / 1024, msgstr, 10);    /* convert kbytes avail to string */
    msg_parms[0] = msgstr;                        /* total disk space msg           */
  message_user(MSG_OSO_DISK_SPACE2, msg_parms, 1, STDOUT_CODE, NO_RESPONSE, OSO_MSG); //198316 hack for pmformat

    rc = DosQueryFSInfo((ULONG )(toupper(*lvolume) -'A'+1), 1L,
                 (PVOID)&fsalloc_buf, sizeof(FSALLOCATE));
    }
  if (rc)
  {
    message_user(MSG_OSO_FORMAT_FAILED, NULL, 0, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
    DBG_ERROR(("Internal error: Format failed rc=%x\n",rc))
  }
  else
  {
    free_space = ( fsalloc_buf.cSectorUnit * fsalloc_buf.cUnitAvail ) /
                 ( 1024 / fsalloc_buf.cbSector );

    _itoa(free_space, msgstr, 10);    /* convert kbytes avail to string */
    msg_parms[0] = msgstr;            /* total free disk space msg      */
    message_user(MSG_OSO_FREE_SPACE, msg_parms, 1, STDOUT_CODE, NO_RESPONSE, OSO_MSG);
  }

  /* Exit */
  return rc;
}
