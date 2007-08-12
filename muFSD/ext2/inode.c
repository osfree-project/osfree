//
// $Header: /cur/cvsroot/boot/muFSD/ext2/inode.c,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
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
#include <linux/stat.h>
#include <microfsd.h>
#include <ext2fs.h>
#include <mfs.h>
#include <linux/errno.h>

extern struct super_block superblk;

#define NR_INODES 4

struct inode ilist[NR_INODES] = {
    {
     0, //   dev_t           i_dev;
     0, //   unsigned long   i_ino;
     0, //   umode_t         i_mode;
     0, //   nlink_t         i_nlink;
     0, //   uid_t           i_uid;
     0, //   gid_t           i_gid;
     0, //   dev_t           i_rdev;
     0, //   off_t           i_size;
     0, //   time_t          i_atime;
     0, //   time_t          i_mtime;
     0, //   time_t          i_ctime;
     0, //   unsigned long   i_blksize;
     0, //   unsigned long   i_blocks;
     0, //   unsigned long    i_version;
//        struct semaphore i_sem;
//        unsigned char    i_lock;

//        unsigned long i_wait;
//        struct inode * i_next, * i_prev;
//        struct inode * i_hash_next, * i_hash_prev;

     0, //   unsigned short i_count;
//        unsigned char  i_dirt;
     &superblk, //   struct super_block      * i_sb;     // superblock
//        struct inode_operations * i_op;
     0, //   unsigned short i_flags;

    {
        0    // struct ext2_inode_info ext2_i;
    },
    INODE_MAGIC

},
    {
     0, //   dev_t           i_dev;
     0, //   unsigned long   i_ino;
     0, //   umode_t         i_mode;
     0, //   nlink_t         i_nlink;
     0, //   uid_t           i_uid;
     0, //   gid_t           i_gid;
     0, //   dev_t           i_rdev;
     0, //   off_t           i_size;
     0, //   time_t          i_atime;
     0, //   time_t          i_mtime;
     0, //   time_t          i_ctime;
     0, //   unsigned long   i_blksize;
     0, //   unsigned long   i_blocks;
     0, //   unsigned long    i_version;
//        struct semaphore i_sem;
//        unsigned char    i_lock;

//        unsigned long i_wait;
//        struct inode * i_next, * i_prev;
//        struct inode * i_hash_next, * i_hash_prev;

     0, //   unsigned short i_count;
//        unsigned char  i_dirt;
     &superblk, //   struct super_block      * i_sb;     // superblock
//        struct inode_operations * i_op;
     0, //   unsigned short i_flags;

    {
        0    // struct ext2_inode_info ext2_i;
    },
    INODE_MAGIC

},
    {
     0, //   dev_t           i_dev;
     0, //   unsigned long   i_ino;
     0, //   umode_t         i_mode;
     0, //   nlink_t         i_nlink;
     0, //   uid_t           i_uid;
     0, //   gid_t           i_gid;
     0, //   dev_t           i_rdev;
     0, //   off_t           i_size;
     0, //   time_t          i_atime;
     0, //   time_t          i_mtime;
     0, //   time_t          i_ctime;
     0, //   unsigned long   i_blksize;
     0, //   unsigned long   i_blocks;
     0, //   unsigned long    i_version;
//        struct semaphore i_sem;
//        unsigned char    i_lock;

//        unsigned long i_wait;
//        struct inode * i_next, * i_prev;
//        struct inode * i_hash_next, * i_hash_prev;

     0, //   unsigned short i_count;
//        unsigned char  i_dirt;
     &superblk, //   struct super_block      * i_sb;     // superblock
//        struct inode_operations * i_op;
     0, //   unsigned short i_flags;

    {
        0    // struct ext2_inode_info ext2_i;
    },
    INODE_MAGIC

    },
    {
     0, //   dev_t           i_dev;
     0, //   unsigned long   i_ino;
     0, //   umode_t         i_mode;
     0, //   nlink_t         i_nlink;
     0, //   uid_t           i_uid;
     0, //   gid_t           i_gid;
     0, //   dev_t           i_rdev;
     0, //   off_t           i_size;
     0, //   time_t          i_atime;
     0, //   time_t          i_mtime;
     0, //   time_t          i_ctime;
     0, //   unsigned long   i_blksize;
     0, //   unsigned long   i_blocks;
     0, //   unsigned long    i_version;
//        struct semaphore i_sem;
//        unsigned char    i_lock;

//        unsigned long i_wait;
//        struct inode * i_next, * i_prev;
//        struct inode * i_hash_next, * i_hash_prev;

     0, //   unsigned short i_count;
//        unsigned char  i_dirt;
     &superblk, //   struct super_block      * i_sb;     // superblock
//        struct inode_operations * i_op;
     0, //   unsigned short i_flags;

    {
        0    // struct ext2_inode_info ext2_i;
    },
    INODE_MAGIC

}
};


void ext2_read_inode(struct inode *inode);


struct inode *iget(unsigned long nr_ino) {
    int i;
    struct inode *inode;

    for (i = 0 ; i < NR_INODES ; i++) {
        if (ilist[i].i_magic != INODE_MAGIC) {
            microfsd_panic("iget - invalid magic !");
        }
        if (ilist[i].i_ino == nr_ino) {
            ilist[i].i_count ++;
            return ilist + i;
        }
    }
    for (i = 0 ; i < NR_INODES ; i++) {
        if (ilist[i].i_magic != INODE_MAGIC) {
            microfsd_panic("iget - invalid magic !");
        }
        if (ilist[i].i_count == 0) {
            ilist[i].i_count ++;
            ilist[i].i_ino  = nr_ino;
            ext2_read_inode(ilist + i);
            return ilist + i;
        }
    }
    microfsd_panic("iget - no more inodes");

}

void iput(struct inode *inode) {
    if (inode) {
        if (inode->i_count) {
            inode->i_count --;
        } else {
            microfsd_panic("iput: Trying to free free inode");
        }
    }
}


/*********************
 *********************
 *********************/

void ext2_read_inode (struct inode * inode)
{
        struct buffer_head * bh;
        struct ext2_inode * raw_inode;
        unsigned long block_group;
        unsigned long group_desc;
        unsigned long desc;
        unsigned long block;
        struct ext2_group_desc * gdp;

#ifdef MICROFSD_TRACE
        printk("ext2_read_inode(%lu)", inode->i_ino);
#endif
        if ((inode->i_ino != EXT2_ROOT_INO && inode->i_ino != EXT2_ACL_IDX_INO &&
             inode->i_ino != EXT2_ACL_DATA_INO && inode->i_ino < EXT2_FIRST_INO) ||
            inode->i_ino > inode->i_sb->u.ext2_sb.s_es->s_inodes_count) {
                /*
                 * panic
                 */
                microfsd_panic("ext2_read_inode : bad inode number %lu", inode->i_ino);

        }
        block_group = (inode->i_ino - 1) / EXT2_INODES_PER_GROUP(inode->i_sb);
        if (block_group >= inode->i_sb->u.ext2_sb.s_groups_count)
                /*
                 * panic
                 */
                microfsd_panic("ext2_read_inode : group >= groups count");

        group_desc = block_group / EXT2_DESC_PER_BLOCK(inode->i_sb);
        desc = block_group % EXT2_DESC_PER_BLOCK(inode->i_sb);
#if 0
        bh = inode->i_sb->u.ext2_sb.s_group_desc[group_desc];
#else
        bh = bread(group_desc + 2);
#endif
        if (!bh)
                /*
                 * panic
                 */
                microfsd_panic("ext2_read_inode : Descriptor not loaded");

        gdp = (struct ext2_group_desc *) bh->b_data;
        block = gdp[desc].bg_inode_table +
                (((inode->i_ino - 1) % EXT2_INODES_PER_GROUP(inode->i_sb))
                 / EXT2_INODES_PER_BLOCK(inode->i_sb));
#if 1
        brelse(bh);
#endif
        bh = bread(block);
        /*
         * failure in bread = panic
         */

        raw_inode = ((struct ext2_inode *) bh->b_data) +
                (inode->i_ino - 1) % EXT2_INODES_PER_BLOCK(inode->i_sb);
        inode->i_mode = raw_inode->i_mode;
        inode->i_uid = raw_inode->i_uid;
        inode->i_gid = raw_inode->i_gid;
        inode->i_nlink = raw_inode->i_links_count;
        inode->i_size = raw_inode->i_size;
        inode->i_atime = raw_inode->i_atime;
        inode->i_ctime = raw_inode->i_ctime;
        inode->i_mtime = raw_inode->i_mtime;
        inode->u.ext2_i.i_dtime = raw_inode->i_dtime;
        inode->i_blksize = inode->i_sb->s_blocksize;
        inode->i_blocks = raw_inode->i_blocks;
//        inode->i_version = ++event;
        inode->u.ext2_i.i_flags = raw_inode->i_flags;
        inode->u.ext2_i.i_faddr = raw_inode->i_faddr;
        inode->u.ext2_i.i_frag_no = raw_inode->i_frag;
        inode->u.ext2_i.i_frag_size = raw_inode->i_fsize;
        inode->u.ext2_i.i_osync = 0;
        inode->u.ext2_i.i_file_acl = raw_inode->i_file_acl;
        inode->u.ext2_i.i_dir_acl = raw_inode->i_dir_acl;
        inode->u.ext2_i.i_version = raw_inode->i_version;
        inode->u.ext2_i.i_block_group = block_group;
        inode->u.ext2_i.i_next_alloc_block = 0;
        inode->u.ext2_i.i_next_alloc_goal = 0;
//        if (inode->u.ext2_i.i_prealloc_count)
//                ext2_error (inode->i_sb, "ext2_read_inode",
//                            "New inode has non-zero prealloc count!");
        if (S_ISCHR(inode->i_mode) || S_ISBLK(inode->i_mode))
                inode->i_rdev = raw_inode->i_block[0];
        else for (block = 0; block < EXT2_N_BLOCKS; block++)
                inode->u.ext2_i.i_data[block] = raw_inode->i_block[block];
        brelse (bh);
//        inode->i_op = NULL;
#if 0
        if (inode->i_ino == EXT2_ACL_IDX_INO ||
            inode->i_ino == EXT2_ACL_DATA_INO)
                /* Nothing to do */ ;
        else if (S_ISREG(inode->i_mode))
                inode->i_op = &ext2_file_inode_operations;
        else if (S_ISDIR(inode->i_mode))
                inode->i_op = &ext2_dir_inode_operations;
#ifndef OS2
        else if (S_ISLNK(inode->i_mode))
                inode->i_op = &ext2_symlink_inode_operations;
        else if (S_ISCHR(inode->i_mode))
                inode->i_op = &chrdev_inode_operations;
        else if (S_ISBLK(inode->i_mode))
                inode->i_op = &blkdev_inode_operations;
        else if (S_ISFIFO(inode->i_mode))
                init_fifo(inode);
#endif
        if (inode->u.ext2_i.i_flags & EXT2_SYNC_FL)
                inode->i_flags |= MS_SYNCHRONOUS;
        if (inode->u.ext2_i.i_flags & EXT2_APPEND_FL)
                inode->i_flags |= S_APPEND;
        if (inode->u.ext2_i.i_flags & EXT2_IMMUTABLE_FL)
                inode->i_flags |= S_IMMUTABLE;
#endif
}


struct buffer_head * inode_getblk (struct inode * inode, blk_t nr,
                                          blk_t create, blk_t new_block, int *err)
{
        u32 *p;
        blk_t tmp;
        struct buffer_head * result;


        p = inode->u.ext2_i.i_data + nr;

        tmp = *p;
        if (tmp) {
            return getblk (tmp);
        } else {
            *err = -EFBIG;
            return 0;
        }
}

struct buffer_head * block_getblk (struct inode * inode,
                                          struct buffer_head * bh, blk_t nr,
                                          blk_t create, blk_t blocksize,
                                          blk_t new_block, int *err)
{
        blk_t tmp;
        u32 *p;
        struct buffer_head * result;

        if (!bh)
                return 0;


        if (!bh->b_uptodate) {
            ll_rw_block (READ, 1, &bh);
        }


        p = (u32 *) bh->b_data + nr;

        tmp = *p;
        if (tmp) {
            result = getblk (tmp);
            brelse (bh);
            return result;
        } else {
            brelse (bh);
            *err = -EFBIG;
            return 0;
        }
}


struct buffer_head * ext2_getblk (struct inode * inode, blk_t block,
                                  blk_t create, int *err) {
        struct buffer_head * bh;
        unsigned long b;
        unsigned long addr_per_block = EXT2_ADDR_PER_BLOCK(inode->i_sb);

#ifdef MICROFSD_TRACE
        printk("ext2_getblk(%ld)", block);
#endif
        if (block < 0) {
                microfsd_panic("ext2_getblk : block < 0");
        }
        if (block > EXT2_NDIR_BLOCKS + addr_per_block  +
                    addr_per_block * addr_per_block +
                    addr_per_block * addr_per_block * addr_per_block) {
                microfsd_panic("ext2_getblk : block > big");
        }

        b = block;
        if (block < EXT2_NDIR_BLOCKS)
                return inode_getblk (inode, block, create, b, err);
        block -= EXT2_NDIR_BLOCKS;
        if (block < addr_per_block) {
                bh = inode_getblk (inode, EXT2_IND_BLOCK, create, b, err);
                return block_getblk (inode, bh, block, create,
                                     inode->i_sb->s_blocksize, b, err);
        }
        block -= addr_per_block;
        if (block < addr_per_block * addr_per_block) {
                bh = inode_getblk (inode, EXT2_DIND_BLOCK, create, b, err);
                bh = block_getblk (inode, bh, block / addr_per_block, create,
                                   inode->i_sb->s_blocksize, b, err);
                return block_getblk (inode, bh, block & (addr_per_block - 1),
                                     create, inode->i_sb->s_blocksize, b, err);
        }
        block -= addr_per_block * addr_per_block;
        bh = inode_getblk (inode, EXT2_TIND_BLOCK, create, b, err);
        bh = block_getblk (inode, bh, block/(addr_per_block * addr_per_block),
                           create, inode->i_sb->s_blocksize, b, err);
        bh = block_getblk (inode, bh, (block/addr_per_block) & (addr_per_block - 1),
                           create, inode->i_sb->s_blocksize, b, err);
        return block_getblk (inode, bh, block & (addr_per_block - 1), create,
                             inode->i_sb->s_blocksize, b, err);
}

struct buffer_head * ext2_bread (struct inode * inode, blk_t block,
                                 blk_t create, int *err)
{
        struct buffer_head * bh;

        bh = ext2_getblk (inode, block, create, err);
        if (!bh || bh->b_uptodate)
                return bh;
        ll_rw_block (READ, 1, &bh);
        if (bh->b_uptodate)
                return bh;
/*
        brelse (bh);
        *err = -EIO;
        return NULL;
*/
        microfsd_panic("ext2_bread : read error");
}
