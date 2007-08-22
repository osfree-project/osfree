//
// $Header: /cur/cvsroot/boot/include/muFSD/os2/magic.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
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



#ifndef __magic_h
#define __magic_h


typedef unsigned short magic_t;

#define RW_PRIVATE_MAGIC   0x4141       // struct RW_private     in file vfs/ll_rwblk.c
#define REQLIST_MAGIC      0x4142       // struct reqlist        in file vfs/ll_rwblk.c
#define BUFFER_HEAD_MAGIC  0x4143       // struct buffer_head    in file include/linux/fs.h (used only in MICROFSD)
#define FILE_MAGIC         0x4144       // struct file           in file include/linux/fs.h
#define INODE_MAGIC        0x4145       // struct inode          in file include/linux/fs.h (used only in MICROFSD)
#define MINIFSD_DATA_MAGIC 0x4147	// struct minifsd_to_fsd_data in file include/os2/minifsd.h
#define BOOTDATA_MAGIC     0x4148	// struct bootdata       in file microfsd/start.c
#define PG_PRIVATE_MAGIC   0x4149       // struct PG_private     in file vfs/pageio.c
#define SUPER_MAGIC        0x4150       // struct super_block    in file include/linux/fs.h
#define BOOTDATA_MAGIC_LO  0x48	        // struct bootdata       in file microfsd/start.c
#define BOOTDATA_MAGIC_HI  0x41 	// struct bootdata       in file microfsd/start.c


#endif

