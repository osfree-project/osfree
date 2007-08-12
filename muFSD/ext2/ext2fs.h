//
// $Header: /cur/cvsroot/boot/muFSD/ext2/ext2fs.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
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
 * valerius, 2006/11/04
 *
 */

/*
 * inode.c
 */
struct buffer_head *ext2_getblk(struct inode * inode, blk_t block, blk_t create, int *err);
struct buffer_head *ext2_bread(struct inode * inode, blk_t block, blk_t create, int *err);

/*
 * file.c
 */
struct file *open_by_name(char *pName);
void vfs_close(struct file *f);
int VFS_read(struct file *file, char far *buf, loff_t len, unsigned long far*pLen);


/*
 * ll_rwblk.c
 */
void ll_rw_block(int rw, int nr, struct buffer_head **bh);

extern struct buffer_head *free_buffers;
