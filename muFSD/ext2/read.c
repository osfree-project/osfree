//
// $Header: /cur/cvsroot/boot/muFSD/ext2/read.c,v 1.2 2006/11/24 11:43:13 valerius Exp $
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
#define INCL_NOPMAPI
#include <os2/os2.h>

#include <add.h>
#include <i86.h>

#include <os2/types.h>
#include <os2/magic.h>
#include <microfsd.h>
#include <mfs.h>
#include <linux/ext2_fs.h>
#include <linux/stat.h>

extern struct file flist[];

unsigned long __cdecl mu_Read(long loffseek, char far *pBuf, unsigned long cbBuf) {
    unsigned long  len;
    unsigned long  tmplen;
    unsigned long  savecbBuf = cbBuf;
    unsigned long  nsegs;
    unsigned long  i;
    unsigned long  ptr;
    unsigned short seg;

    printk("**** mu_Read(off=%ld, pBuf=%04X:%04X, cbBuf=%lu)", loffseek, FP_SEG(pBuf), FP_OFF(pBuf), cbBuf);

    if (!flist)
        microfsd_panic("mu_Read : flist = 0");
    if (flist->f_magic != FILE_MAGIC)
        microfsd_panic("mu_Read : invalid magic number");

    if (flist->f_pos != loffseek) {
        printk("mu_Read : seeking from %ld to %ld", flist->f_pos, loffseek); //
        flist->f_pos = loffseek;
    }
    seg   = FP_SEG(pBuf);
    len   = 0;
    nsegs = (cbBuf + 32767UL) >> 15;
    for (i = 0; i < nsegs; i++) {
        if (!(savecbBuf & 32767UL))
            cbBuf = 32768UL;
        else
            cbBuf = (i == nsegs - 1 ? savecbBuf & 32767UL : 32768UL);
        VFS_read(flist, pBuf, cbBuf, &tmplen); /* failure in VFS_read = panic */
        seg += 0x800;
        //pBuf = MAKEP(seg, 0);
        pBuf = MK_FP(seg, 0);
        len += tmplen;
    }

    printk("     read %lu bytes", len); //
    return len;
}
