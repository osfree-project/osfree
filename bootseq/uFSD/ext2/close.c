//
// $Header: /cur/cvsroot/boot/muFSD/ext2/close.c,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
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
#include <ext2fs.h>
#include <mfs.h>
#include <linux/ext2_fs.h>
#include <linux/stat.h>

extern struct file flist[];

void __cdecl mu_Close(void) {

    printk("**** mu_Close");
    if (!flist)
        microfsd_panic("mu_Close : flist = 0");
    if (flist->f_magic != FILE_MAGIC)
        microfsd_panic("mu_Close : invalid magic number");

    if (flist->f_count)
        vfs_close(flist); /* failure in vfs_close = panic */
    else
        /*
         * We seem to receive mu_Close even if mu_Open has failed !
         */
        printk("     file already closed");

}
