//
// $Header: /cur/cvsroot/boot/muFSD/ext2/super.c,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
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

#include <os2/types.h>
#include <os2/magic.h>
#include <microfsd.h>
#include <mfs.h>
#include <linux/ext2_fs.h>

void read_super (void) {
        printk("read_super()");
        ext2_read_super();
}

struct super_block superblk;

void ext2_read_super (void) {
        struct super_block *sb = &superblk;
        struct buffer_head * bh;
        struct ext2_super_block * es;
        unsigned long sb_block = 1;
        unsigned long logic_sb_block = 1;
        int db_count;
        int i, j;

        printk("ext2_read_super()");
        bh = bread (sb_block); /* failure in bread = panic */
        /*
         * Note: s_es must be initialized s_es as soon as possible because
         * some ext2 macro-instructions depend on its value
         */
        es = (struct ext2_super_block *) bh->b_data;
        sb->u.ext2_sb.s_es = es;
        sb->s_magic = es->s_magic;
	
	for (i = 0; i < 60; i += 10)
	    printk("%u,%u,%u,%u,%u,%u,%u,%u,%u,%u", 
	           ((char *)es)[i], ((char *)es)[i + 1],
		   ((char *)es)[i + 2], ((char *)es)[i + 3],
		   ((char *)es)[i + 4], ((char *)es)[i + 5], 
		   ((char *)es)[i + 6], ((char *)es)[i + 7],
		   ((char *)es)[i + 8], ((char *)es)[i + 9] \
		   );
	
        if (sb->s_magic != EXT2_SUPER_MAGIC) {
            microfsd_panic("Invalid ext2 signature in superblock : 0x%04X", sb->s_magic);
        }
        sb->s_blocksize = EXT2_MIN_BLOCK_SIZE << es->s_log_block_size;
        sb->s_blocksize_bits = EXT2_BLOCK_SIZE_BITS(sb);
        if (sb->s_blocksize != BLOCK_SIZE &&
            (sb->s_blocksize == 1024 || sb->s_blocksize == 2048 ||
             sb->s_blocksize == 4096)) {
#if 1
            microfsd_panic("Invalid block size : %d", sb->s_blocksize);
#else
                unsigned long offset;

                brelse (bh);
                set_blocksize (dev, sb->s_blocksize);
                logic_sb_block = (sb_block*BLOCK_SIZE) / sb->s_blocksize;
                offset = (sb_block*BLOCK_SIZE) % sb->s_blocksize;
                bh = bread (dev, logic_sb_block, sb->s_blocksize);
                if(!bh)
                        return NULL;
                es = (struct ext2_super_block *) (((char *)bh->b_data) + offset);
                sb->u.ext2_sb.s_es = es;
                if (es->s_magic != EXT2_SUPER_MAGIC) {
                        sb->s_dev = 0;
                        unlock_super (sb);
                        brelse (bh);
                        printk ("EXT2-fs: Magic mismatch, very weird !\n");
                        return NULL;
                }
#endif
        }
        sb->u.ext2_sb.s_frag_size = EXT2_MIN_FRAG_SIZE <<
                                   es->s_log_frag_size;
        if (sb->u.ext2_sb.s_frag_size)
                sb->u.ext2_sb.s_frags_per_block = sb->s_blocksize /
                                                  sb->u.ext2_sb.s_frag_size;
        else
                sb->s_magic = 0;
        sb->u.ext2_sb.s_blocks_per_group = es->s_blocks_per_group;
        sb->u.ext2_sb.s_frags_per_group = es->s_frags_per_group;
        sb->u.ext2_sb.s_inodes_per_group = es->s_inodes_per_group;
        sb->u.ext2_sb.s_inodes_per_block = sb->s_blocksize /
                                           sizeof (struct ext2_inode);
        sb->u.ext2_sb.s_itb_per_group = sb->u.ext2_sb.s_inodes_per_group /
                                        sb->u.ext2_sb.s_inodes_per_block;
        sb->u.ext2_sb.s_desc_per_block = sb->s_blocksize /
                                         sizeof (struct ext2_group_desc);
        sb->u.ext2_sb.s_sbh = bh;
        sb->u.ext2_sb.s_es = es;

        if (sb->s_magic != EXT2_SUPER_MAGIC) {
            microfsd_panic("Invalid ext2 signature in superblock : 0x%04X", sb->s_magic);
        }

        if (sb->s_blocksize != sb->u.ext2_sb.s_frag_size) {
            microfsd_panic("fragments not yet supported");
        }

        sb->u.ext2_sb.s_groups_count = (es->s_blocks_count -
                                        es->s_first_data_block +
                                       EXT2_BLOCKS_PER_GROUP(sb) - 1) /
                                       EXT2_BLOCKS_PER_GROUP(sb);
#if 0
        db_count = (sb->u.ext2_sb.s_groups_count + EXT2_DESC_PER_BLOCK(sb) - 1) /
                   EXT2_DESC_PER_BLOCK(sb);
        for (i = 0; i < db_count; i++) {
                sb->u.ext2_sb.s_group_desc[i] = bread (logic_sb_block + i + 1);
                /*
                 * failure in bread = panic
                 */
        }
#endif
        for (i = 0; i < EXT2_MAX_GROUP_LOADED; i++) {
                sb->u.ext2_sb.s_inode_bitmap_number[i] = 0;
                sb->u.ext2_sb.s_inode_bitmap[i] = 0;
                sb->u.ext2_sb.s_block_bitmap_number[i] = 0;
                sb->u.ext2_sb.s_block_bitmap[i] = 0;
        }
        sb->u.ext2_sb.s_loaded_inode_bitmaps = 0;
        sb->u.ext2_sb.s_loaded_block_bitmaps = 0;
        sb->u.ext2_sb.s_db_per_group = db_count;
}
