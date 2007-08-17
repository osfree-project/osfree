//
// $Header: /cur/cvsroot/boot/muFSD/ext2/mfs.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
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

#ifndef __mfs_h
#define __mfs_h

#include <microfsd.h>
#include <linux/ext2_fs.h>
#include <linux/ext2_fs_i.h>
#include <linux/ext2_fs_sb.h>

/*
 * Used in place of linux/fs.h
 */

/*
 * Simplified buffer_head structure
 */
struct buffer_head {
    char               *b_data;                  // pointer to data block
    blk_t               b_blocknr;               // block number
    unsigned char       b_uptodate;              // 1 = contents valid
    unsigned char       b_count;                 // use count
    struct buffer_head *b_next;                  // doubly linked list of hash-queue
    struct buffer_head *b_prev;
    struct buffer_head *b_next_free;             // doubly linked list of free/used buffers
    struct buffer_head *b_prev_free;
    magic_t             b_magic;                 //  Magic signature always equal to  BUFFER_HEAD_MAGIC
};

#define READ  0
#define READA 2

#define BLOCK_SIZE 1024


struct buffer_head *bread(blk_t block);
struct buffer_head *getblk(blk_t block);
void brelse(struct buffer_head *buf);
void bforget(struct buffer_head *buf);



//
// From /usr/src/linux-1.2.1/include/linux/fs.h
//
struct inode {
        dev_t           i_dev;
        unsigned long   i_ino;
        umode_t         i_mode;
        nlink_t         i_nlink;
        uid_t           i_uid;
        gid_t           i_gid;
        dev_t           i_rdev;
        off_t           i_size;
        time_t          i_atime;
        time_t          i_mtime;
        time_t          i_ctime;
        unsigned long   i_blksize;
        unsigned long   i_blocks;
        unsigned long    i_version;
//        struct semaphore i_sem;
//        unsigned char    i_lock;

//        unsigned long i_wait;
//        struct inode * i_next, * i_prev;
//        struct inode * i_hash_next, * i_hash_prev;

        unsigned short i_count;
//        unsigned char  i_dirt;
        struct super_block      * i_sb;     // superblock
//        struct inode_operations * i_op;
        unsigned short i_flags;

    union {
        struct ext2_inode_info ext2_i;
    } u;
    magic_t i_magic;  // ALWAYS INODE_MAGIC
};

struct inode *iget(unsigned long nr_ino);
void iput(struct inode *inode);


struct file {
    struct inode      *f_inode;
    mode_t             f_mode;     // open mode
    loff_t             f_pos;      // file pointer
    unsigned short     f_count;    // use count
//    off_t              f_reada;
//    unsigned short     f_flags;
    magic_t            f_magic;        // magic number always equal to FILE_MAGIC
};

struct super_block {
        unsigned long s_blocksize;
        unsigned char s_blocksize_bits;
        unsigned long s_magic;
        union {
                struct ext2_sb_info ext2_sb;
        } u;
};

#ifdef NAME_MAX
#undef NAME_MAX
#endif
#define NAME_MAX 255
struct dirent {
        long                d_ino;
        off_t               d_off;
        unsigned short      d_reclen;
        char                d_name[NAME_MAX + 1];
};

#endif
