/* $Id: super.c,v 1.2 2004/03/21 02:43:18 pasha Exp $ */

static char *SCCSID = "@(#)1.13  5/14/99 10:58:14 src/jfs/utils/libfs/super.c, jfslib, w45.fs32, currbld";
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
 *   COMPONENT_NAME: jfslib
 *
 *   FUNCTIONS: ujfs_get_superblk
 *              ujfs_init_superblock
 *              ujfs_put_superblk
 *              validate_sizes
 *              ujfs_validate_super
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define INCL_DOSPROFILE
#define INCL_DOSERRORS
#include <os2.h>
#include "jfs_types.h"
#include <jfs_aixisms.h>
#include "jfs_filsys.h"
#include "jfs_superblock.h"
#include "libjufs.h"
#include "jfs_dinode.h"
#include "devices.h"
#include "jfs_imap.h"
#include "jfs_dmap.h"
#include "super.h"


/*
 * NAME: inrange
 *
 * FUNCTION: Checks to see that <num> is a power-of-2 multiple of <low> that is
 *      less than or equal to <high>.
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *
 * NOTES:
 *
 * DATA STRUCTURES:
 *
 * RETURNS: If it is, it returns 1, else 0
 */
int32 inrange(  uint32 num,
                uint32 low,
                uint32 high )
{
    if( low ) {
        for( ; low <= high; low <<= 1 ) {
            if( low == num ) {
                return 1;
            }
        }
    }
    return 0;
}


/*
 * NAME: validate_sizes
 *
 * FUNCTION: Ensure that all configurable sizes fall within their respective
 *      version specific limits.
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *      sb      - superblock to check sizes
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      success: 0
 *      failure:  any other value
 */
static int32 validate_sizes( struct superblock *sb )
{
    /*
     * Make sure AG size is at least 32M
     */
    if( sb->s_agsize >= (1 << L2BPERDMAP) ) {
        return 0;
    }

    return EINVAL;
}


/*
 * NAME: ujfs_validate_super
 *
 * FUNCTION: Check if superblock is valid
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *      sb      - superblock to validate
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      success: 0
 *      failure: LIBFS_CORRUPTSUPER, LIBFS_BADVERSION, LIBFS_BADMAGIC
 */
int32 ujfs_validate_super( struct superblock *sb )
{
    if( memcmp(sb->s_magic, JFS_MAGIC, sizeof(sb->s_magic)) == 0 ) {
        if( sb->s_version != JFS_VERSION )
            return LIBFS_BADVERSION;
        if( validate_sizes(sb) == EINVAL )
            return LIBFS_CORRUPTSUPER;
    } else {
        return LIBFS_BADMAGIC;
    }
    return 0;
}


/*
 * NAME: ujfs_put_superblk
 *
 * FUNCTION: Write primary or secondary aggregate superblock
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *      fd              - open port for device to write superblock to
 *      sb              - pointer to struct superblock to be written
 *      is_primary      - 0 value means we are putting the secondary superblock;
 *                        non-zero value means we are putting the primary
 *                        superblock.
 *
 * NOTES: The sizeof(struct superblock) is less than the amount of disk space
 *      being allowed for the superblock (SIZE_OF_SUPER).  This function will
 *      write 0's to the space following the actual superblock structure to fill
 *      the entire allocated disk space.
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      success: 0
 *      failure: any other value
 */
int32 ujfs_put_superblk( HFILE                  fd,
                         struct superblock      *sb,
                         int16                  is_primary )
{
    char        buf[SIZE_OF_SUPER];
    int32       rc;

    memset( buf, 0, SIZE_OF_SUPER );
    memcpy( buf, sb, sizeof(*sb) );
    rc = ujfs_rw_diskblocks( fd, (is_primary ? SUPER1_OFF : SUPER2_OFF),
                             SIZE_OF_SUPER, buf, PUT );

    return rc;
}


/*
 * NAME: ujfs_init_superblock
 *
 * FUNCTION: Initialize primary and secondary aggregate superblock and write to
 *      disk
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *      aggr_superblock         - structure to be filled in with superblock
 *                                information.
 *      dev_ptr                 - open port for device to write superblock to
 *      volume_label            - Volume label for superblock
 *      number_of_blocks        - Number of blocks for aggregate
 *      compress                - Whether this is a compressed aggregate
 *      fs_block_size           - block size for aggregate
 *      phys_block_size         - physical block size for device
 *      type_jfs                - JFS type to create; s_flag field of superblock
 *      secondary_ait_address   - block offset of first extent of secondary
 *                                aggregate inode table
 *      secondary_ait_length    - number of blocks of first extent of secondary
 *                                aggregate inode table
 *      fsck_svclog_length      - number of blocks of fsck_wspace_length is
 *                                intended for the fsck service log.
 *      ag_size                 - AG size
 *      fsck_wspace_address     - block offset of start of fsck working space
 *      fsck_wspace_length      - number of blocks of fsck working space
 *      logloc                  - block offset of start of log in aggr.
 *      logsize                 - number of blocks of log in aggr.
 *      secondary_aim_address   - block offset of first extent of secondary
 *                                aggregate inode map
 *      secondary_aim_length    - number of blocks of first extent of secondary
 *                                aggregate inode map
 *
 * RETURNS:
 *      0: success
 *      Any other return value indicates failure
 */
int32 ujfs_init_superblock( struct superblock   *aggr_superblock,
                            HFILE               dev_ptr,
                            char                *volume_label,
                            int64               number_of_blocks,
                            uint32              compress,
                            int32               fs_block_size,
                            int32               phys_block_size,
                            uint32              type_jfs,
                            int64               secondary_ait_address,
                            int32               secondary_ait_length,
                            int32               ag_size,
                            int64               fsck_wspace_address,
                            int32               fsck_wspace_length,
                            int32               fsck_svclog_length,
                            int64               logloc,
                            int32               logsize,
                            int64               secondary_aim_address,
                            int32               secondary_aim_length )
{
    int32       rc;

    /*
     * Initialize all of the fields of the superblock
     */
    strncpy(aggr_superblock->s_magic, JFS_MAGIC, strlen(JFS_MAGIC));
    aggr_superblock->s_version = JFS_VERSION;
    aggr_superblock->s_logdev = 0;
    aggr_superblock->s_logserial = 0;
    aggr_superblock->s_size = (number_of_blocks * fs_block_size) /
                              phys_block_size;
    aggr_superblock->s_bsize = fs_block_size;
    aggr_superblock->s_l2bsize = log2shift( aggr_superblock->s_bsize );
    aggr_superblock->s_l2bfactor = log2shift( aggr_superblock->s_bsize /
                                              phys_block_size );
    aggr_superblock->s_pbsize = phys_block_size;
    aggr_superblock->s_l2pbsize = log2shift( aggr_superblock->s_pbsize );
    aggr_superblock->s_agsize = ag_size;
    aggr_superblock->s_flag = type_jfs;
    aggr_superblock->s_compress = compress;
    aggr_superblock->s_state = FM_CLEAN;
    strncpy(aggr_superblock->s_fpack, volume_label, LV_NAME_SIZE );

    PXDaddress( &(aggr_superblock->s_ait2), secondary_ait_address );
    PXDlength( &(aggr_superblock->s_ait2), secondary_ait_length );

    PXDaddress( &(aggr_superblock->s_aim2), secondary_aim_address );
    PXDlength( &(aggr_superblock->s_aim2), secondary_aim_length );

    PXDaddress( &(aggr_superblock->s_fsckpxd), fsck_wspace_address );
    PXDlength( &(aggr_superblock->s_fsckpxd), fsck_wspace_length );
    aggr_superblock->s_fscklog = 0;
    aggr_superblock->s_fsckloglen = fsck_svclog_length;

    PXDaddress( &(aggr_superblock->s_logpxd), logloc );
    PXDlength( &(aggr_superblock->s_logpxd), logsize );

#ifdef	_JFS_DFS_LFS
    aggr_superblock->s_attach = 0;
    aggr_superblock->totalUsable = (aggr_superblock->s_size
                                        << aggr_superblock->s_l2pbsize) >> 10;
    aggr_superblock->minFree = 0;
    aggr_superblock->realFree = 0;
#endif	/* _JFS_DFS_LFS */
    aggr_superblock->s_time.tv_sec = (uint32) time( NULL );

    /*
     * Write both the primary and secondary superblocks to disk if valid
     */
    rc = ujfs_validate_super( aggr_superblock );
    if( rc != 0 ) {
        return rc;
    }
    rc = ujfs_put_superblk( dev_ptr, aggr_superblock, 1 );
    if( rc != 0 ) return rc;

    rc = ujfs_put_superblk( dev_ptr, aggr_superblock, 0 );

    return( rc );
}


/*
 * NAME: ujfs_get_superblk
 *
 * FUNCTION: read either the primary or secondary superblock from the specified
 *      device
 *
 * PRE CONDITIONS:
 *
 * POST CONDITIONS:
 *
 * PARAMETERS:
 *      fd              - open port for device to read superblock from
 *      sb              - pointer to struct superblock to be filled in on return
 *      is_primary      - 0 indicates to retrieve secondary superblock,
 *                        otherwise retrieve primary superblock
 *
 * NOTES:
 *
 * RECOVERY OPERATION:
 *
 * DATA STRUCTURES:
 *
 * RETURNS:
 *      success: 0
 *      failure: any other value
 */
int32 ujfs_get_superblk( HFILE                  fd,
                         struct superblock      *sb,
                         int32                  is_primary)
{
    int32       rc;
    char        buf[SIZE_OF_SUPER];
    struct superblock   *sblk = (struct superblock *)buf;

    rc = ujfs_rw_diskblocks( fd, (is_primary ? SUPER1_OFF : SUPER2_OFF),
                             SIZE_OF_SUPER, sblk, GET );
    if( rc != 0 ) return rc;

    memcpy( sb, sblk, sizeof(*sb) );
    return 0;
}
