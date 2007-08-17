//
// $Header: /cur/cvsroot/boot/muFSD/ext2/open.c,v 1.2 2006/11/24 11:43:13 valerius Exp $
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

#define INCL_DOS
#define INCL_DOSERRORS
#define INCL_NOPMAPI
#include <os21x/os2.h>

#include <os2/types.h>
#include <os2/magic.h>
#include <microfsd.h>
#include <ext2fs.h>
#include <linux/ext2_fs.h>
#include <linux/stat.h>

#include <string.h>
#include <mfs.h>

extern struct file flist[];
extern unsigned short current_seg;

unsigned short __cdecl mu_Open(char far *pName, unsigned long far *pulFileSize) {
    char LocalName[CCHMAXPATH];
    struct file *f;

    strcpy(LocalName, "C:\\");
    _fstrcat(LocalName, pName);
    printk("**** mu_Open(%s)", LocalName);
    f = open_by_name(LocalName);
    if (f) {
        *pulFileSize = f->f_inode->i_size;

        printk("     size of %s is %lu", LocalName, *pulFileSize);
        return 0;
    } else {
        printk("     FILE NOT_FOUND");
        return ERROR_FILE_NOT_FOUND;
    }
}
