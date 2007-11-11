//
// $Header: /cur/cvsroot/boot/include/muFSD/linux/ext2_fs_sb.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
//

// 32 bits Linux ext2 file system driver for OS/2 WARP - Allows OS/2 to
// access your Linux ext2fs partitions as normal drive letters.
// Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.



//
// This file is a OS/2 "port" of /usr/src/linux-1.2.1/include/linux/ext2_fs_sb.h
// It contains the ext2 specific superblock structure definition
//

#ifndef __EXT2_FS_SB_H
#define __EXT2_FS_SB_H

#include <linux/ext2_fs.h>

/*
 * The following is not needed anymore since the descriptors buffer
 * heads are now dynamically allocated
 */
#define EXT2_MAX_GROUP_DESC	8

#define EXT2_MAX_GROUP_LOADED	8

/*
 * second extended-fs super-block data in memory
 */
#pragma pack(1)
struct ext2_sb_info {
	unsigned long s_frag_size;	/* Size of a fragment in bytes */
	unsigned long s_frags_per_block;/* Number of fragments per block */
	unsigned long s_inodes_per_block;/* Number of inodes per block */
	unsigned long s_frags_per_group;/* Number of fragments in a group */
	unsigned long s_blocks_per_group;/* Number of blocks in a group */
	unsigned long s_inodes_per_group;/* Number of inodes in a group */
	unsigned long s_itb_per_group;	/* Number of inode table blocks per group */
	unsigned long s_db_per_group;	/* Number of descriptor blocks per group */
	unsigned long s_desc_per_block;	/* Number of group descriptors per block */
	unsigned long s_groups_count;	/* Number of groups in the fs */
	struct buffer_head *s_sbh;	/* Buffer containing the super block */
	struct ext2_super_block *s_es;	/* Pointer to the super block in the buffer */
	struct buffer_head  **s_group_desc;
	unsigned short s_loaded_inode_bitmaps;
	unsigned short s_loaded_block_bitmaps;
	unsigned long s_inode_bitmap_number[EXT2_MAX_GROUP_LOADED];
	struct buffer_head *s_inode_bitmap[EXT2_MAX_GROUP_LOADED];
	unsigned long s_block_bitmap_number[EXT2_MAX_GROUP_LOADED];
	struct buffer_head *s_block_bitmap[EXT2_MAX_GROUP_LOADED];
#ifndef OS2
	int s_rename_lock;
	struct wait_queue * s_rename_wait;
#else
	long          s_rename_lock;
        unsigned long s_rename_wait;
#endif
	unsigned long  s_mount_opt;
	unsigned short s_resuid;
	unsigned short s_resgid;
	unsigned short s_mount_state;
#ifdef EXT2_DYNAMIC_REV
        unsigned short s_pad;
#ifdef OS2
        long s_addr_per_block_bits;
        long s_desc_per_block_bits;
        long s_inode_size;
        long s_first_ino;
#else
        int s_addr_per_block_bits;
        int s_desc_per_block_bits;
        int s_inode_size;
        int s_first_ino;
#endif
#endif
};
#pragma pack()
#endif /* __EXT2_FS_SB_H */
