//
// $Header: /cur/cvsroot/boot/muFSD/ext2/dir.c,v 1.2 2006/11/24 11:43:13 valerius Exp $
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

/*
 *  linux/fs/ext2/dir.c
 *
 *  Copyright (C) 1992, 1993, 1994  Remy Card (card@masi.ibp.fr)
 *                                  Laboratoire MASI - Institut Blaise Pascal
 *                                  Universite Pierre et Marie Curie (Paris VI)
 *
 *  from
 *
 *  linux/fs/minix/dir.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 *
 *  ext2 directory handling functions
 */

//#define INCL_DOSERRORS
//#define INCL_NOPMAPI
//#include <os21x/os2.h>

#include <add.h>     // valerius (errors)
#include <string.h>  // for memcpy()

#include <os2/types.h>
#include <os2/magic.h>
#include <microfsd.h>
#include <ext2fs.h>
#include <mfs.h>
#include <linux/ext2_fs.h>
#include <linux/stat.h>

#define NAME_OFFSET(de) ((unsigned short)((de)->d_name - (char *) (de)))
#define ROUND_UP(x) (((x)+3U) & ~3U)


int ext2_check_dir_entry (char * function, struct inode * dir,
                          struct ext2_dir_entry * de, struct buffer_head * bh,
                          unsigned long offset)
{
        char * error_msg = NULL;

        if (de->rec_len < EXT2_DIR_REC_LEN(1))
                error_msg = "rec_len is smaller than minimal";
        else if (de->rec_len % 4 != 0)
                error_msg = "rec_len % 4 != 0";
        else if (de->rec_len < EXT2_DIR_REC_LEN(de->name_len))
                error_msg= "rec_len is too small for name_len";
        else if (dir && ((char *) de - bh->b_data) + de->rec_len >
         dir->i_sb->s_blocksize)
                error_msg = "directory entry across blocks";
        else if (dir && de->inode > dir->i_sb->u.ext2_sb.s_es->s_inodes_count)
                error_msg = "inode out of bounds";

        if (error_msg != NULL)
                microfsd_panic("bad directory entry: %s\n"
                            "offset=%lu, inode=%lu, rec_len=%d, name_len=%d",
                            error_msg, offset, (unsigned long) de->inode, de->rec_len,
                            de->name_len);
        return error_msg == NULL ? 1 : 0;
}

int ext2_readdir (struct inode * inode, struct file * filp,
                         struct dirent * dirent, unsigned int count)
{
        unsigned long offset, blk;
        unsigned int i, num, stored, dlen;
        struct buffer_head * bh, * tmp, * bha[16];
        struct ext2_dir_entry * de;
        struct super_block * sb;

        int err;
        unsigned long version;

#ifdef MICROFSD_TRACE
        printk("ext2_readdir(%ld)", inode->i_ino);
#endif
        if (!inode || !S_ISDIR(inode->i_mode))
            microfsd_panic("ext2_readdir : not a directory");

        sb = inode->i_sb;

        stored = 0;
        bh = 0;
        offset = filp->f_pos & (sb->s_blocksize - 1);

        while (count > 0 && !stored && filp->f_pos < inode->i_size) {
                blk = (filp->f_pos) >> EXT2_BLOCK_SIZE_BITS(sb);
                bh = ext2_bread (inode, blk, 0, &err);
                if (!bh) {
                        filp->f_pos += sb->s_blocksize - offset;
                        continue;
                }
#if 0
                /*
                 * Do the readahead
                 */
                if (!offset) {
                        for (i = 16 >> (EXT2_BLOCK_SIZE_BITS(sb) - 9), num = 0;
                             i > 0; i--) {
                                tmp = ext2_getblk (inode, ++blk, 0, &err);
                                if (tmp && !tmp->b_uptodate)
                                        bha[num++] = tmp;
                                else
                                        brelse (tmp);
                        }
                        if (num) {
                                ll_rw_block (READA, num, bha);
                                for (i = 0; i < num; i++)
                                        brelse (bha[i]);
                        }
                }

#endif

                while (count > 0 && filp->f_pos < inode->i_size
                       && offset < sb->s_blocksize) {
                        de = (struct ext2_dir_entry *) (bh->b_data + offset);
                        if (!ext2_check_dir_entry ("ext2_readdir", inode, de,
                                                   bh, offset)) {
                                /* On error, skip the f_pos to the
                                   next block. */
                                filp->f_pos = (filp->f_pos & (sb->s_blocksize - 1))
                                              + sb->s_blocksize;
                                brelse (bh);
                                return stored;
                        }
                        if (de->inode) {
                                dlen = ROUND_UP(NAME_OFFSET(dirent)
                                                + de->name_len + 1);
                                /* Old libc libraries always use a
                                   count of 1. */
                                if (count == 1 && !stored)
                                        count = dlen;
                                if (count < dlen) {
                                        count = 0;
                                        break;
                                }

                                /* We might block in the next section
                                 * if the data destination is
                                 * currently swapped out.  So, use a
                                 * version stamp to detect whether or
                                 * not the directory has been modified
                                 * during the copy operation. */
                                version = inode->i_version;
                                i = de->name_len;
                                memcpy(dirent->d_name, de->name, i);
                                dirent->d_ino      = de->inode;
                                *(dirent->d_name + i) = 0;
                                dirent->d_reclen   = i;
                                dirent->d_off      = dlen;

                                stored += dlen;
                                count -= dlen;
                                dirent = (struct dirent *) (((char *) dirent) + dlen);
                        }
                        offset += de->rec_len;
                        filp->f_pos += de->rec_len;
                }
                offset = 0;
                brelse (bh);
        }
        return stored;
}

int VFS_readdir(struct file *file, struct dirent *dirent) {

    if (!file)
        microfsd_panic("VFS_readdir : file = 0");
    if (file->f_magic != FILE_MAGIC)
        microfsd_panic("VFS_readdir : file with invalid magic number");
    if (!(file->f_inode))
        microfsd_panic("VFS_readdir : file->f_inode = 0");
    if (file->f_inode->i_magic != INODE_MAGIC)
        microfsd_panic("VFS_readdir : file->f_inode with invalid magic number");

    /*
     * Tests if it's really a directory (panic otherwise)
     */
    if (!S_ISDIR(file->f_inode->i_mode)) {
        microfsd_panic("VFS_readdir : not a directory");
    }

    return (ext2_readdir(file->f_inode, file,
                         dirent, 1) > 0 ? NO_ERROR : ERROR_HANDLE_EOF);
}


